#!/usr/bin/env python3
"""Render an AGI sound resource to signed 8-bit mono PCM for hybrid ROM music."""

from __future__ import annotations

import argparse
from pathlib import Path


AGI_HEADER = b"\x12\x34"
PCM_SAMPLE_RATE = 15_360
INTERNAL_OVERSAMPLE = 8
INTERNAL_SAMPLE_RATE = PCM_SAMPLE_RATE * INTERNAL_OVERSAMPLE
FRAME_RATE = 60
PCM_SAMPLES_PER_TICK = PCM_SAMPLE_RATE // FRAME_RATE
INTERNAL_SAMPLES_PER_TICK = INTERNAL_SAMPLE_RATE // FRAME_RATE
SN76496_MASTER_STEP = 3_579_545
PFEED_SN = 0x4000
WFEED_SN = 0x6000
ATTENUATION_TABLE = [
    0xFFFF, 0xCB30, 0xA145, 0x8000,
    0x6598, 0x50A3, 0x4000, 0x32CC,
    0x2851, 0x2000, 0x1966, 0x1428,
    0x1000, 0x0CB3, 0x0A14, 0x0000,
]


def read_dir_entries(path: Path):
    data = path.read_bytes()
    entries = []
    for i in range(0, len(data), 3):
        if i + 3 > len(data):
            break
        raw = (data[i] << 16) | (data[i + 1] << 8) | data[i + 2]
        entries.append(None if raw == 0xFFFFFF else (((raw >> 20) & 0x0F), raw & 0x0FFFFF))
    return entries


def load_sound_blob(game_dir: Path, sound_num: int) -> bytes:
    entries = read_dir_entries(game_dir / "SNDDIR")
    entry = entries[sound_num]
    if entry is None:
        raise FileNotFoundError(f"Sound {sound_num} not present in {game_dir}")
    volume_num, offset = entry
    data = (game_dir / f"VOL.{volume_num}").read_bytes()
    if data[offset:offset + 2] != AGI_HEADER:
        raise ValueError(f"Invalid AGI header for sound {sound_num}")
    payload_len = data[offset + 3] | (data[offset + 4] << 8)
    return data[offset:offset + 5 + payload_len]


def parse_channels(blob: bytes):
    payload = blob[5:]
    offsets = [payload[i] | (payload[i + 1] << 8) for i in range(0, 8, 2)]
    channels = []
    for voice in range(4):
        pos = offsets[voice]
        events = []
        while pos + 5 <= len(payload):
            duration = payload[pos] | (payload[pos + 1] << 8)
            b2, b3, b4 = payload[pos + 2], payload[pos + 3], payload[pos + 4]
            if duration == 0xFFFF:
                break
            attenuation = b4 & 0x0F
            if voice < 3:
                raw_frequency = (((b2 & 0x3F) << 4) | (b3 & 0x0F))
                events.append((duration, attenuation, raw_frequency, 0))
            else:
                events.append((duration, attenuation, 0, b3 & 0x07))
            pos += 5
        channels.append(events)
    return channels


def sn76496_mix_level(attenuation: int) -> int:
    return ATTENUATION_TABLE[attenuation & 0x0F] >> 5


class SN76496Chip:
    def __init__(self):
        self.noise_type = (WFEED_SN << 16) | PFEED_SN
        self.reset()

    def reset(self):
        self.rng = self.noise_type & 0xFFFF
        self.noise_fb = self.noise_type >> 16
        self.ch_freq = [1, 1, 1, 0x0010]
        self.ch_att = [0x0F, 0x0F, 0x0F, 0x0F]
        self.ch_volume = [0, 0, 0, 0]
        self.current_bits = 0x1E
        self.ch_phase = [0, 0, 0, 0]
        self.ch3_reg = 0
        self.active_mask = 0
        self.mix_table = [0] * 16
        self.refresh_volumes()

    def refresh_volumes(self):
        self.active_mask = 0
        for voice in range(4):
            self.ch_volume[voice] = sn76496_mix_level(self.ch_att[voice])
            if self.ch_volume[voice] != 0:
                self.active_mask |= 1 << voice

        for i in range(16):
            mix = 0
            mix += self.ch_volume[0] if (i & 0x01) else -self.ch_volume[0]
            mix += self.ch_volume[1] if (i & 0x02) else -self.ch_volume[1]
            mix += self.ch_volume[2] if (i & 0x04) else -self.ch_volume[2]
            mix += self.ch_volume[3] if (i & 0x08) else -self.ch_volume[3]
            mix >>= 8
            if mix < -128:
                mix = -128
            elif mix > 127:
                mix = 127
            self.mix_table[i] = mix & 0xFF

    def set_tone(self, voice: int, raw: int):
        if raw < 6:
            raw = 1
        self.ch_freq[voice] = raw
        if voice == 2 and self.ch3_reg == 3:
            self.ch_freq[3] = self.ch_freq[2]

    def set_attenuation(self, voice: int, attenuation: int):
        self.ch_att[voice] = attenuation & 0x0F
        self.refresh_volumes()

    def update_noise_freq(self, reg: int):
        self.ch3_reg = reg & 0x03
        self.rng = self.noise_type & 0xFFFF
        self.noise_fb = (self.noise_type >> 16) if (reg & 0x04) else (self.noise_type & 0xFFFF)
        if self.ch3_reg == 3:
            self.ch_freq[3] = self.ch_freq[2] if self.ch_freq[2] else 0x0010
        else:
            self.ch_freq[3] = 0x0010 << self.ch3_reg

    def set_noise(self, noise_control: int):
        self.update_noise_freq(noise_control & 0x07)

    def mix_sample_s8(self) -> int:
        value = self.mix_table[(self.current_bits >> 1) & 0x0F]
        return value - 256 if value >= 128 else value

    def step(self):
        for voice, mask in ((0, 0x02), (1, 0x04), (2, 0x08)):
            freq = self.ch_freq[voice]
            if not freq or freq == 0xFFFF or not (self.active_mask & (1 << voice)):
                continue
            threshold = freq * 16 * INTERNAL_SAMPLE_RATE
            self.ch_phase[voice] += SN76496_MASTER_STEP
            while self.ch_phase[voice] >= threshold:
                self.ch_phase[voice] -= threshold
                self.current_bits ^= mask

        freq = self.ch_freq[3]
        if freq and freq != 0xFFFF and (self.active_mask & 0x08):
            threshold = freq * 16 * INTERNAL_SAMPLE_RATE
            self.ch_phase[3] += SN76496_MASTER_STEP
            while self.ch_phase[3] >= threshold:
                self.ch_phase[3] -= threshold
                self.current_bits &= ~0x10
                if self.rng & 1:
                    self.rng >>= 1
                    self.rng ^= self.noise_fb
                    self.current_bits |= 0x10
                else:
                    self.rng >>= 1


def render_tandy_like_pcm(channels):
    chip = SN76496Chip()
    positions = [0, 0, 0, 0]
    remaining = [0, 0, 0, 0]
    active = [True, True, True, True]
    output = bytearray()

    def advance_voice(voice: int):
        if positions[voice] >= len(channels[voice]):
            active[voice] = False
            chip.set_attenuation(voice, 0x0F)
            return

        duration, attenuation, raw_frequency, noise_control = channels[voice][positions[voice]]
        positions[voice] += 1
        remaining[voice] = max(1, duration)
        if voice < 3:
            chip.set_tone(voice, raw_frequency)
        else:
            chip.set_noise(noise_control)
        chip.set_attenuation(voice, attenuation)

    for voice in range(4):
        advance_voice(voice)

    while any(active):
        accumulator = 0
        substep_count = 0
        for _ in range(INTERNAL_SAMPLES_PER_TICK):
            chip.step()
            accumulator += chip.mix_sample_s8()
            substep_count += 1
            if substep_count == INTERNAL_OVERSAMPLE:
                averaged = int(round(accumulator / INTERNAL_OVERSAMPLE))
                if averaged < -128:
                    averaged = -128
                elif averaged > 127:
                    averaged = 127
                output.append(averaged & 0xFF)
                accumulator = 0
                substep_count = 0

        for voice in range(4):
            if not active[voice]:
                continue
            remaining[voice] -= 1
            if remaining[voice] <= 0:
                advance_voice(voice)

    return bytes(output)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--games-dir", required=True)
    parser.add_argument("--game", required=True)
    parser.add_argument("--sound", type=int, required=True)
    parser.add_argument("--output", required=True)
    args = parser.parse_args()

    game_dir = Path(args.games_dir) / args.game
    blob = load_sound_blob(game_dir, args.sound)
    channels = parse_channels(blob)
    pcm = render_tandy_like_pcm(channels)

    output = Path(args.output)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_bytes(pcm)
    print(f"wrote {len(pcm)} bytes to {output}")


if __name__ == "__main__":
    main()
