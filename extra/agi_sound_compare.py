#!/usr/bin/env python3
"""Compare AGI sound resources using GBAGI's runtime interpretation.

This script reads raw AGI SNDDIR/VOL.* data, parses sound resources the same
way GBAGI does, and prints summaries plus early-event comparisons for selected
sounds. It is intended as an offline investigation tool for cases where two
tracks are structurally similar but behave differently in playback.
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass
from pathlib import Path
from typing import List, Optional, Sequence, Tuple


AGI_HEADER = b"\x12\x34"


@dataclass
class ToneEvent:
    duration: int
    attenuation: int
    raw_frequency: int


@dataclass
class NoiseEvent:
    duration: int
    attenuation: int
    noise_control: int


@dataclass
class EndEvent:
    pass


Event = ToneEvent | NoiseEvent | EndEvent


@dataclass
class SoundResource:
    game: str
    sound_num: int
    payload_len: int
    offsets: List[int]
    channels: List[List[Event]]


def read_dir_entries(path: Path) -> List[Optional[Tuple[int, int]]]:
    data = path.read_bytes()
    entries: List[Optional[Tuple[int, int]]] = []
    for i in range(0, len(data), 3):
        if i + 3 > len(data):
            break
        raw = (data[i] << 16) | (data[i + 1] << 8) | data[i + 2]
        if raw == 0xFFFFFF:
            entries.append(None)
        else:
            entries.append(((raw >> 20) & 0x0F, raw & 0x0FFFFF))
    return entries


def load_sound_blob(game_dir: Path, sound_num: int) -> Optional[bytes]:
    dir_entries = read_dir_entries(game_dir / "SNDDIR")
    if sound_num >= len(dir_entries):
        return None

    entry = dir_entries[sound_num]
    if entry is None:
        return None

    volume_num, offset = entry
    volume_path = game_dir / f"VOL.{volume_num}"
    if not volume_path.exists():
        return None

    data = volume_path.read_bytes()
    if offset + 5 > len(data):
        return None
    if data[offset : offset + 2] != AGI_HEADER:
        return None

    payload_len = data[offset + 3] | (data[offset + 4] << 8)
    end = offset + 5 + payload_len
    if end > len(data):
        return None
    return data[offset:end]


def parse_sound(game: str, sound_num: int, blob: bytes) -> SoundResource:
    payload = blob[5:]
    offsets = [payload[i] | (payload[i + 1] << 8) for i in range(0, 8, 2)]
    channels: List[List[Event]] = []

    for voice in range(4):
        pos = offsets[voice]
        events: List[Event] = []
        guard = 0
        while pos + 5 <= len(payload) and guard < 100000:
            guard += 1
            duration = payload[pos] | (payload[pos + 1] << 8)
            b2 = payload[pos + 2]
            b3 = payload[pos + 3]
            b4 = payload[pos + 4]
            if duration == 0xFFFF:
                events.append(EndEvent())
                break

            attenuation = b4 & 0x0F
            if voice < 3:
                raw_frequency = (((b2 & 0x3F) << 4) | (b3 & 0x0F))
                events.append(
                    ToneEvent(
                        duration=duration,
                        attenuation=attenuation,
                        raw_frequency=raw_frequency,
                    )
                )
            else:
                events.append(
                    NoiseEvent(
                        duration=duration,
                        attenuation=attenuation,
                        noise_control=b3 & 0x07,
                    )
                )
            pos += 5

        channels.append(events)

    return SoundResource(
        game=game,
        sound_num=sound_num,
        payload_len=len(payload),
        offsets=offsets,
        channels=channels,
    )


def event_is_audible(event: Event) -> bool:
    if isinstance(event, EndEvent):
        return False
    if event.attenuation >= 0x0F:
        return False
    if isinstance(event, ToneEvent):
        return event.raw_frequency != 0
    return True


def format_event(event: Event) -> str:
    if isinstance(event, EndEvent):
        return "END"
    if isinstance(event, ToneEvent):
        return f"d={event.duration} att={event.attenuation} freq={event.raw_frequency}"
    return f"d={event.duration} att={event.attenuation} noise={event.noise_control}"


def summarize(resource: SoundResource) -> List[str]:
    lines = [
        f"{resource.game}:{resource.sound_num}",
        f"payload={resource.payload_len}",
        f"offsets={resource.offsets}",
    ]
    for voice, events in enumerate(resource.channels):
        audible = sum(1 for event in events if event_is_audible(event))
        sustained = sum(
            1
            for event in events
            if event_is_audible(event)
            and not isinstance(event, EndEvent)
            and event.duration >= 8
        )
        lines.append(
            f"voice {voice}: total={len(events)} audible={audible} sustained={sustained}"
        )
    return lines


def compare_channels(
    left: SoundResource, right: SoundResource, voice: int, limit: int
) -> List[str]:
    lines = [f"voice {voice} first {limit} events"]
    left_events = left.channels[voice][:limit]
    right_events = right.channels[voice][:limit]
    max_len = max(len(left_events), len(right_events))
    for index in range(max_len):
        left_text = format_event(left_events[index]) if index < len(left_events) else "-"
        right_text = format_event(right_events[index]) if index < len(right_events) else "-"
        marker = "==" if left_text == right_text else "!="
        lines.append(
            f"  {index:02d} {marker} {left.game}:{left.sound_num} [{left_text}] | "
            f"{right.game}:{right.sound_num} [{right_text}]"
        )
    return lines


def parse_target(target: str) -> Tuple[str, int]:
    game, sep, sound_text = target.partition(":")
    if not sep:
        raise ValueError(f"Target '{target}' must look like GAME:SOUND")
    return game, int(sound_text, 10)


def load_target(base_games_dir: Path, target: str) -> SoundResource:
    game, sound_num = parse_target(target)
    game_dir = base_games_dir / game
    if not game_dir.exists():
        raise FileNotFoundError(f"Game directory not found: {game_dir}")
    blob = load_sound_blob(game_dir, sound_num)
    if blob is None:
        raise FileNotFoundError(f"Sound {sound_num} not found in {game_dir}")
    return parse_sound(game, sound_num, blob)


def main(argv: Optional[Sequence[str]] = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--games-dir",
        default=r"H:\CODEX\GBAGI\GBAGI-fresh-2026-04-16\games",
        help="Path to the AGI games directory",
    )
    parser.add_argument(
        "targets",
        nargs="+",
        help="One or more GAME:SOUND targets, for example sq1:60 SQ2:60 SQ2:61",
    )
    parser.add_argument(
        "--compare-limit",
        type=int,
        default=12,
        help="How many early events per voice to compare",
    )
    args = parser.parse_args(argv)

    base_games_dir = Path(args.games_dir)
    resources: List[SoundResource] = []
    for target in args.targets:
        try:
            resources.append(load_target(base_games_dir, target))
        except FileNotFoundError as exc:
            print(f"missing: {target} ({exc})")

    for resource in resources:
        print("=" * 72)
        for line in summarize(resource):
            print(line)

    if len(resources) >= 2:
        for left, right in zip(resources, resources[1:]):
            print("=" * 72)
            print(f"compare {left.game}:{left.sound_num} -> {right.game}:{right.sound_num}")
            for voice in range(4):
                for line in compare_channels(left, right, voice, args.compare_limit):
                    print(line)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
