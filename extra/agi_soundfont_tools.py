#!/usr/bin/env python3
"""Shared helpers for AGI soundfont-based streamed music tooling."""

from __future__ import annotations

import json
import math
import re
import subprocess
from dataclasses import dataclass
from pathlib import Path

import mido


MIDI_TICKS_PER_BEAT = 192
MIDI_TEMPO = 500_000
SCUMMVM_SPEED_FACTOR = 6
SCUMMVM_INSTRUMENTS = (0, 0, 0)
PCM_OUTPUT_MASTER_RATE = 15_360

KNOWN_GAME_CONTEXTS = {
    "larry1": ("larry1", "LSL1", ("LSL1", "LLLLL")),
    "lsl1": ("larry1", "LSL1", ("LSL1", "LLLLL")),
    "lllll": ("larry1", "LSL1", ("LSL1", "LLLLL")),
    "p-q1": ("p-q1", "PQ1", ("PQ1", "PQ")),
    "pq1": ("p-q1", "PQ1", ("PQ1", "PQ")),
    "pq": ("p-q1", "PQ1", ("PQ1", "PQ")),
    "kq1": ("kq1", "KQ1", ("KQ1",)),
    "kq2": ("kq2", "KQ2", ("KQ2",)),
    "kq3": ("kq3", "KQ3", ("KQ3",)),
    "kq4": ("kq4", "KQ4", ("KQ4",)),
    "kq6": ("kq6", "KQ6", ("KQ6",)),
    "sq0": ("sq0", "SQ0", ("SQ0",)),
    "sq1": ("sq1", "SQ1", ("SQ1",)),
    "sq2": ("sq2", "SQ2", ("SQ2",)),
    "goldrush": ("gold-rush", "GOLDRSH", ("GOLDRSH",)),
    "theblackcauldron": ("black-cauldron", "BLKCLDR", ("BLKCLDR",)),
    "mixedupmothergoose": ("mother-goose", "MUMG", ("MUMG",)),
    "manhunter1newyork": ("mh1", "MH1", ("MH1",)),
    "manhunter2sanfrancisco": ("mh2", "MH2", ("MH2",)),
    "enclosure": ("enclosure", "ENCLOSR", ("ENCLOSR",)),
}


@dataclass(frozen=True)
class GameBuildContext:
    game_dir: Path
    folder_name: str
    profile_name: str
    project_slug: str
    game_id: str
    game_id_aliases: tuple[str, ...]


def repo_root() -> Path:
    return Path(__file__).resolve().parent.parent


def normalize_name(value: str) -> str:
    return re.sub(r"[^a-z0-9]+", "", value.strip().lower())


def safe_symbol_name(value: str) -> str:
    return re.sub(r"[^a-z0-9_]+", "_", value.strip().lower()).strip("_")


def _guess_game_id_from_name(name: str) -> str:
    cleaned = re.sub(r"[^A-Za-z0-9]+", "", name).upper()
    if not cleaned:
        return "AGI"
    return cleaned[:7]


def resolve_game_context(game_dir: Path) -> GameBuildContext:
    folder_name = game_dir.name
    key = normalize_name(folder_name)
    profile_name: str
    game_id: str
    aliases: tuple[str, ...]

    if key in KNOWN_GAME_CONTEXTS:
        profile_name, game_id, aliases = KNOWN_GAME_CONTEXTS[key]
    else:
        guessed = _guess_game_id_from_name(folder_name)
        profile_name = safe_symbol_name(folder_name) or "agi-game"
        game_id = guessed
        aliases = (guessed,)

    project_slug = safe_symbol_name(game_id) or safe_symbol_name(profile_name) or "agi_game"
    return GameBuildContext(
        game_dir=game_dir.resolve(),
        folder_name=folder_name,
        profile_name=profile_name,
        project_slug=project_slug,
        game_id=game_id,
        game_id_aliases=aliases,
    )


def generated_projects_root() -> Path:
    return repo_root() / "extra" / "agi_soundfont_projects"


def generated_project_dir(context: GameBuildContext) -> Path:
    return generated_projects_root() / context.project_slug


def generated_manifest_path(context: GameBuildContext) -> Path:
    return generated_project_dir(context) / "manifest.json"


def validate_pcm_sample_rate(sample_rate: int) -> int:
    if sample_rate <= 0:
        raise ValueError("PCM sample rate must be positive")
    if (PCM_OUTPUT_MASTER_RATE % sample_rate) != 0:
        raise ValueError(
            f"PCM sample rate {sample_rate} is not supported. Choose a divisor of {PCM_OUTPUT_MASTER_RATE}."
        )
    return sample_rate


def find_game_resource(game_dir: Path, *names: str) -> Path:
    for name in names:
        candidate = game_dir / name
        if candidate.exists():
            return candidate
    raise FileNotFoundError(f"Could not find any of {names} in {game_dir}")


def find_combined_dir_resource(game_dir: Path) -> Path | None:
    for path in sorted(game_dir.iterdir()):
        if not path.is_file():
            continue
        upper_name = path.name.upper()
        if not upper_name.endswith("DIR"):
            continue
        if upper_name in {"LOGDIR", "PICDIR", "VIEWDIR", "SNDDIR"}:
            continue
        return path
    return None


def read_combined_dir_entries(path: Path, section_index: int) -> list[tuple[int, int] | None]:
    data = path.read_bytes()
    if len(data) < 8:
        raise ValueError(f"Combined DIR file is too short: {path}")
    offsets = [data[i] | (data[i + 1] << 8) for i in range(0, 8, 2)]
    start = offsets[section_index]
    end = offsets[section_index + 1] if section_index < 3 else len(data)
    if start > end or end > len(data):
        raise ValueError(f"Invalid combined DIR offsets in {path}")
    entries: list[tuple[int, int] | None] = []
    for i in range(start, end, 3):
        if i + 3 > end:
            break
        raw = (data[i] << 16) | (data[i + 1] << 8) | data[i + 2]
        if raw == 0xFFFFFF:
            entries.append(None)
        else:
            entries.append((((raw >> 20) & 0x0F), raw & 0x0FFFFF))
    return entries


def read_sound_dir_entries(game_dir: Path) -> list[tuple[int, int] | None]:
    try:
        snddir = find_game_resource(game_dir, "snddir", "SNDDIR")
        return read_dir_entries(snddir)
    except FileNotFoundError:
        combined_dir = find_combined_dir_resource(game_dir)
        if combined_dir is None:
            raise
        return read_combined_dir_entries(combined_dir, 3)


def find_volume_resource(game_dir: Path, volume_num: int) -> Path:
    candidates = [
        game_dir / f"VOL.{volume_num}",
        game_dir / f"vol.{volume_num}",
    ]
    for path in sorted(game_dir.iterdir()):
        if not path.is_file():
            continue
        upper_name = path.name.upper()
        if upper_name.endswith(f"VOL.{volume_num}") and upper_name not in {f"VOL.{volume_num}"}:
            candidates.append(path)
    for candidate in candidates:
        if candidate.exists():
            return candidate
    raise FileNotFoundError(f"Could not find a volume file for volume {volume_num} in {game_dir}")


def read_dir_entries(path: Path) -> list[tuple[int, int] | None]:
    data = path.read_bytes()
    entries: list[tuple[int, int] | None] = []
    for i in range(0, len(data), 3):
        if i + 3 > len(data):
            break
        raw = (data[i] << 16) | (data[i + 1] << 8) | data[i + 2]
        if raw == 0xFFFFFF:
            entries.append(None)
        else:
            entries.append((((raw >> 20) & 0x0F), raw & 0x0FFFFF))
    return entries


def list_present_sound_nums(game_dir: Path) -> list[int]:
    entries = read_sound_dir_entries(game_dir)
    return [i for i, entry in enumerate(entries) if entry is not None]


def load_sound_blob(game_dir: Path, sound_num: int) -> bytes:
    entries = read_sound_dir_entries(game_dir)
    if sound_num >= len(entries):
        raise FileNotFoundError(f"Sound {sound_num} is outside the sound directory")
    entry = entries[sound_num]
    if entry is None:
        raise FileNotFoundError(f"Sound {sound_num} not present")
    volume_num, offset = entry
    vol = find_volume_resource(game_dir, volume_num)
    data = vol.read_bytes()
    if data[offset : offset + 2] != b"\x12\x34":
        raise ValueError(f"Invalid AGI header for sound {sound_num}")
    payload_len = data[offset + 3] | (data[offset + 4] << 8)
    return data[offset : offset + 5 + payload_len]


def parse_channels(blob: bytes) -> list[list[tuple[int, int, int, int]]]:
    payload = blob[5:]
    offsets = [payload[i] | (payload[i + 1] << 8) for i in range(0, 8, 2)]
    channels: list[list[tuple[int, int, int, int]]] = []
    for voice in range(4):
        pos = offsets[voice]
        events: list[tuple[int, int, int, int]] = []
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


def scummvm_raw_frequency_to_midi(raw_frequency: int) -> int:
    if raw_frequency <= 0:
        return 0
    ll = math.log10(pow(2.0, 1.0 / 12.0))
    note = int(math.floor((math.log10(111860.0 / float(raw_frequency)) / ll) - 48 + 0.5))
    return max(0, min(127, note))


def append_tempo_track(mid: mido.MidiFile) -> None:
    track = mido.MidiTrack()
    track.append(mido.MetaMessage("set_tempo", tempo=MIDI_TEMPO, time=0))
    track.append(mido.MetaMessage("end_of_track", time=0))
    mid.tracks.append(track)


def append_voice_track(mid: mido.MidiFile, voice_index: int, events: list[tuple[int, int, int, int]]) -> None:
    if not events or voice_index >= 3:
        return
    track = mido.MidiTrack()
    track.append(
        mido.Message("program_change", channel=voice_index, program=SCUMMVM_INSTRUMENTS[voice_index], time=0)
    )
    pending = 0
    for duration, _attenuation, raw_frequency, _noise in events:
        midi_ticks = max(1, duration * SCUMMVM_SPEED_FACTOR)
        if raw_frequency > 0:
            note = scummvm_raw_frequency_to_midi(raw_frequency)
            track.append(mido.Message("note_on", channel=voice_index, note=note, velocity=100, time=pending))
            track.append(mido.Message("note_off", channel=voice_index, note=note, velocity=0, time=midi_ticks))
        else:
            track.append(mido.Message("note_on", channel=voice_index, note=0, velocity=0, time=pending))
            track.append(mido.Message("note_off", channel=voice_index, note=0, velocity=0, time=midi_ticks))
        pending = 0
    track.append(mido.MetaMessage("end_of_track", time=0))
    mid.tracks.append(track)


def convert_agi_sound_to_midi(game_dir: Path, sound_num: int, midi_path: Path) -> None:
    blob = load_sound_blob(game_dir, sound_num)
    channels = parse_channels(blob)
    mid = mido.MidiFile(ticks_per_beat=MIDI_TICKS_PER_BEAT, type=1)
    append_tempo_track(mid)
    for voice_index, events in enumerate(channels[:3]):
        append_voice_track(mid, voice_index, events)
    midi_path.parent.mkdir(parents=True, exist_ok=True)
    mid.save(str(midi_path))


def _run_subprocess(cmd: list[str], cwd: Path | None = None) -> None:
    completed = subprocess.run(
        cmd,
        check=False,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        cwd=str(cwd) if cwd else None,
    )
    if completed.returncode != 0:
        detail = completed.stderr.strip() or completed.stdout.strip() or "unknown tool error"
        raise RuntimeError(f"Command failed ({completed.returncode}): {' '.join(cmd)}\n{detail}")


def render_midi_to_wav(
    fluidsynth_path: Path,
    soundfont_path: Path,
    midi_path: Path,
    wav_path: Path,
    *,
    render_sample_rate: int = 44_100,
) -> None:
    wav_path.parent.mkdir(parents=True, exist_ok=True)
    cmd = [
        str(fluidsynth_path),
        "-ni",
        "-F",
        str(wav_path),
        "-r",
        str(render_sample_rate),
        str(soundfont_path),
        str(midi_path),
    ]
    _run_subprocess(cmd, cwd=fluidsynth_path.parent)


def convert_wav_to_mp3(ffmpeg_path: Path, wav_path: Path, mp3_path: Path) -> None:
    mp3_path.parent.mkdir(parents=True, exist_ok=True)
    cmd = [
        str(ffmpeg_path),
        "-y",
        "-i",
        str(wav_path),
        "-codec:a",
        "libmp3lame",
        "-q:a",
        "2",
        str(mp3_path),
    ]
    _run_subprocess(cmd)


def build_input_fingerprint(payload: dict[str, object]) -> str:
    return json.dumps(payload, sort_keys=True, separators=(",", ":"))


def stat_signature(path: Path) -> dict[str, object]:
    stat = path.stat()
    return {
        "path": str(path.resolve()),
        "size": stat.st_size,
        "mtime_ns": stat.st_mtime_ns,
    }
