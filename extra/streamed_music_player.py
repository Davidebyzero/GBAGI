#!/usr/bin/env python3
"""Play or export GBAGI streamed PCM music assets on a desktop machine.

Examples:
  python extra/streamed_music_player.py --game larry1 --sound 21 --play
  python extra/streamed_music_player.py --game larry1 --sound 21 --wav out.wav
  python extra/streamed_music_player.py --pcm audio/larry1_sound21_15360_mono_s8.pcm --play
"""

from __future__ import annotations

import argparse
import re
import sys
import tempfile
import wave
from pathlib import Path


PCM_SAMPLE_RATE = 15_360


def repo_root() -> Path:
    return Path(__file__).resolve().parent.parent


def parse_pcm_track_rows(header_path: Path) -> dict[tuple[str, int], tuple[str, int]]:
    text = header_path.read_text(encoding="utf-8")
    mappings: dict[tuple[str, int], tuple[str, int]] = {}
    pattern = re.compile(
        r'X\("([^"]+)",\s*(\d+),\s*([A-Za-z0-9_]+),\s*(\d+)U,\s*\d+,\s*(?:TRUE|FALSE)\)'
    )
    for match in pattern.finditer(text):
        game_id = match.group(1)
        sound_num = int(match.group(2))
        symbol = match.group(3)
        length = int(match.group(4))
        mappings[(game_id, sound_num)] = (symbol, length)
    return mappings


def list_mapped_tracks() -> list[dict[str, object]]:
    mappings = parse_pcm_track_rows(repo_root() / "pcm_music_assets.h")
    rows: list[dict[str, object]] = []
    for (game_id, sound_num), (symbol, expected_length) in sorted(mappings.items()):
        pcm_path = repo_root() / "audio" / f"{symbol}_15360_mono_s8.pcm"
        rows.append(
            {
                "game_id": game_id,
                "sound_num": sound_num,
                "symbol": symbol,
                "expected_length": expected_length,
                "pcm_path": pcm_path,
            }
        )
    return rows


def canonical_game_ids(game_name: str) -> list[str]:
    normalized = game_name.strip().upper()
    aliases = {
        "LARRY1": ["LSL1", "LLLLL"],
        "LSL1": ["LSL1", "LLLLL"],
        "LLLLL": ["LLLLL", "LSL1"],
        "KQ1": ["KQ1"],
    }
    return aliases.get(normalized, [normalized])


def find_pcm_from_mapping(game_name: str, sound_num: int) -> Path:
    root = repo_root()
    mappings = parse_pcm_track_rows(root / "pcm_music_assets.h")
    for game_id in canonical_game_ids(game_name):
        mapped = mappings.get((game_id, sound_num))
        if not mapped:
            continue
        symbol, expected_length = mapped
        pcm_path = root / "audio" / f"{symbol}_15360_mono_s8.pcm"
        if not pcm_path.exists():
            raise FileNotFoundError(f"Mapped PCM file not found: {pcm_path}")
        actual_length = pcm_path.stat().st_size
        if actual_length != expected_length:
            raise ValueError(
                f"Length mismatch for {pcm_path.name}: header says {expected_length}, file has {actual_length}"
            )
        return pcm_path
    raise FileNotFoundError(f"No mapped streamed PCM track for game={game_name!r} sound={sound_num}")


def resolve_pcm_path(game_name: str | None = None, sound_num: int | None = None, pcm_path: Path | None = None) -> Path:
    if pcm_path is not None:
        return pcm_path
    if game_name is None or sound_num is None:
        raise ValueError("game_name and sound_num are required when pcm_path is not provided")
    return find_pcm_from_mapping(game_name, sound_num)


def load_pcm_bytes(game_name: str | None = None, sound_num: int | None = None, pcm_path: Path | None = None) -> tuple[Path, bytes]:
    resolved = resolve_pcm_path(game_name=game_name, sound_num=sound_num, pcm_path=pcm_path)
    return resolved, resolved.read_bytes()


def write_wav_from_pcm(pcm_bytes: bytes, wav_path: Path) -> None:
    wav_path.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(wav_path), "wb") as wav_file:
        wav_file.setnchannels(1)
        wav_file.setsampwidth(1)
        wav_file.setframerate(PCM_SAMPLE_RATE)
        wav_file.writeframes(pcm_bytes)


def play_pcm_bytes(pcm_bytes: bytes) -> None:
    if sys.platform != "win32":
        raise RuntimeError("--play currently uses winsound and is only supported on Windows")

    import winsound

    with tempfile.NamedTemporaryFile(suffix=".wav", delete=False) as temp_file:
        temp_wav = Path(temp_file.name)
    try:
        write_wav_from_pcm(pcm_bytes, temp_wav)
        winsound.PlaySound(str(temp_wav), winsound.SND_FILENAME)
    finally:
        try:
            temp_wav.unlink()
        except OSError:
            pass


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    source = parser.add_mutually_exclusive_group(required=True)
    source.add_argument("--pcm", type=Path, help="Path to a raw signed 8-bit mono PCM file")
    source.add_argument("--game", help="Game alias such as larry1, LSL1, LLLLL, or KQ1")
    parser.add_argument("--sound", type=int, help="AGI sound number when using --game")
    parser.add_argument("--play", action="store_true", help="Play the PCM track immediately")
    parser.add_argument("--wav", type=Path, help="Write a WAV copy for listening or inspection")
    args = parser.parse_args()

    if args.game and args.sound is None:
        parser.error("--sound is required when using --game")

    if not args.play and not args.wav:
        parser.error("choose at least one output action: --play or --wav")

    pcm_path, pcm_bytes = load_pcm_bytes(game_name=args.game, sound_num=args.sound, pcm_path=args.pcm)

    if args.wav:
        write_wav_from_pcm(pcm_bytes, args.wav)
        print(f"Wrote WAV: {args.wav}")

    if args.play:
        print(f"Playing: {pcm_path}")
        play_pcm_bytes(pcm_bytes)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
