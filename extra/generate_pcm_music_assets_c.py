#!/usr/bin/env python3
"""Generate split pcm_music_assets_*.c files from mapped PCM files in audio/."""

from __future__ import annotations

import re
from pathlib import Path


def repo_root() -> Path:
    return Path(__file__).resolve().parent.parent


def generated_assets_dir(root: Path) -> Path:
    return root / "_generated" / "pcm_music_assets"


def parse_header_symbols(header_path: Path) -> list[tuple[str, str, int, int, str]]:
    text = header_path.read_text(encoding="utf-8")
    pattern = re.compile(
        r'X\("([^"]+)",\s*(\d+),\s*([A-Za-z0-9_]+),\s*(\d+)U,\s*(\d+)U,\s*(PCM_CODEC_[A-Za-z0-9_]+),\s*\d+,\s*(?:TRUE|FALSE)\)'
    )
    seen: set[str] = set()
    rows: list[tuple[str, str, int, int, str]] = []
    for match in pattern.finditer(text):
        symbol = match.group(3)
        if symbol in seen:
            continue
        seen.add(symbol)
        rows.append((match.group(1), symbol, int(match.group(4)), int(match.group(5)), match.group(6)))
    return rows


def emit_array(symbol: str, pcm_path: Path, comment_game_id: str) -> str:
    data = pcm_path.read_bytes()
    values = [f"(S8)0x{byte:02X}" for byte in data]
    lines = [
        f"/* Generated from {pcm_path.as_posix()} for {comment_game_id} via extra/render_agi_sound_to_pcm.py */",
        f"const S8 {symbol}_pcm_data[] = {{",
    ]
    for i in range(0, len(values), 16):
        lines.append("\t" + ", ".join(values[i:i + 16]) + ",")
    if values:
        lines[-1] = lines[-1].rstrip(",")
    lines.append("};")
    lines.append("")
    return "\n".join(lines)


def safe_unlink(path: Path) -> None:
    try:
        path.unlink()
    except FileNotFoundError:
        pass
    except PermissionError:
        try:
            path.chmod(0o666)
            path.unlink()
        except (FileNotFoundError, PermissionError):
            print(f"WARN: could not remove stale generated file: {path}")


def main() -> int:
    root = repo_root()
    output_root = generated_assets_dir(root)
    header_path = root / "pcm_music_assets.h"
    rows = parse_header_symbols(header_path)
    output_root.mkdir(parents=True, exist_ok=True)
    target_paths = {output_root / f"pcm_music_assets_{symbol}.c" for _, symbol, _, _, _ in rows}

    for stale_path in output_root.glob("pcm_music_assets_*.c"):
        if stale_path in target_paths:
            continue
        safe_unlink(stale_path)
    for stale_path in root.glob("pcm_music_assets_*.c"):
        safe_unlink(stale_path)
    for stale_path in root.glob("pcm_music_assets_*.o"):
        safe_unlink(stale_path)
    safe_unlink(root / "pcm_music_asset_objs.rsp")

    generated_paths: list[Path] = []
    reused_paths: list[Path] = []
    for game_id, symbol, expected_length, sample_rate, codec in rows:
        if codec == "PCM_CODEC_S8":
            pcm_path = root / "audio" / f"{symbol}_{sample_rate}_mono_s8.pcm"
        elif codec == "PCM_CODEC_IMA_ADPCM_WAV":
            pcm_path = root / "audio" / f"{symbol}_{sample_rate}_4bit_ima.wav"
        else:
            raise ValueError(f"Unsupported codec in header: {codec}")
        if not pcm_path.exists():
            raise FileNotFoundError(f"Missing PCM file: {pcm_path}")
        actual_length = pcm_path.stat().st_size
        if actual_length != expected_length:
            raise ValueError(
                f"Length mismatch for {pcm_path.name}: header says {expected_length}, file has {actual_length}"
            )
        output_path = output_root / f"pcm_music_assets_{symbol}.c"
        content = '\n'.join(['#include "types.h"', "", emit_array(symbol, pcm_path, game_id)])
        if output_path.exists():
            existing_content = output_path.read_text(encoding="utf-8")
            if existing_content == content:
                reused_paths.append(output_path)
                continue
        output_path.write_text(
            content,
            encoding="utf-8",
            newline="\n",
        )
        generated_paths.append(output_path)

    print(
        f"Wrote {len(generated_paths)} PCM asset translation units"
        f" ({len(reused_paths)} reused)"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
