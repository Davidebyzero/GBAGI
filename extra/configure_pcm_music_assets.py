#!/usr/bin/env python3
"""Generate pcm_music_assets.h for the current game build context."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from agi_soundfont_tools import generated_manifest_path, repo_root, resolve_game_context


TrackRow = tuple[str, int, str, int, int, str, int, bool]


LARRY_TRACKS: list[TrackRow] = [
    ("LSL1", 1, "larry1_sound1", 34775, 7680, "PCM_CODEC_S8", 0, False),
    ("LSL1", 2, "larry1_sound2", 17120, 7680, "PCM_CODEC_S8", 0, False),
    ("LSL1", 3, "larry1_sound3", 19973, 7680, "PCM_CODEC_S8", 0, False),
    ("LSL1", 4, "larry1_sound4", 21400, 7680, "PCM_CODEC_S8", 0, False),
    ("LSL1", 5, "larry1_sound5", 19617, 7680, "PCM_CODEC_S8", 0, False),
    ("LSL1", 6, "larry1_sound6", 17120, 7680, "PCM_CODEC_S8", 0, False),
    ("LSL1", 7, "larry1_sound7", 21043, 7680, "PCM_CODEC_S8", 0, False),
    ("LSL1", 8, "larry1_sound8", 19617, 7680, "PCM_CODEC_S8", 0, False),
    ("LSL1", 9, "larry1_sound9", 19617, 7680, "PCM_CODEC_S8", 0, False),
    ("LSL1", 10, "larry1_sound10", 19973, 7680, "PCM_CODEC_S8", 0, False),
    ("LSL1", 11, "larry1_sound11", 17833, 7680, "PCM_CODEC_S8", 0, False),
    ("LSL1", 12, "larry1_sound12", 22826, 7680, "PCM_CODEC_S8", 0, False),
    ("LSL1", 13, "larry1_sound13", 21043, 7680, "PCM_CODEC_S8", 0, False),
    ("LSL1", 14, "larry1_sound14", 19617, 7680, "PCM_CODEC_S8", 0, False),
    ("LSL1", 15, "larry1_sound15", 18547, 7680, "PCM_CODEC_S8", 0, False),
    ("LSL1", 21, "larry1_sound21", 436818, 7680, "PCM_CODEC_S8", 0, False),
    ("LSL1", 22, "larry1_sound22", 43602, 7680, "PCM_CODEC_S8", 0, False),
    ("LSL1", 23, "larry1_sound23", 339628, 7680, "PCM_CODEC_S8", 0, False),
    ("LSL1", 24, "larry1_sound24", 263482, 7680, "PCM_CODEC_S8", 0, False),
    ("LSL1", 25, "larry1_sound25", 87560, 7680, "PCM_CODEC_S8", 0, False),
    ("LSL1", 26, "larry1_sound26", 87649, 7680, "PCM_CODEC_S8", 0, False),
    ("LSL1", 27, "larry1_sound27", 98527, 7680, "PCM_CODEC_S8", 0, False),
    ("LSL1", 28, "larry1_sound28", 73204, 7680, "PCM_CODEC_S8", 0, False),
    ("LSL1", 29, "larry1_sound29", 67944, 7680, "PCM_CODEC_S8", 0, False),
    ("LSL1", 30, "larry1_sound30", 390006, 7680, "PCM_CODEC_S8", 0, False),
    ("LSL1", 31, "larry1_sound31", 187335, 7680, "PCM_CODEC_S8", 0, False),
    ("LSL1", 32, "larry1_sound32", 103075, 7680, "PCM_CODEC_S8", 0, False),
    ("LSL1", 33, "larry1_sound33", 67498, 7680, "PCM_CODEC_S8", 0, False),
    ("LSL1", 34, "larry1_sound34", 18432, 7680, "PCM_CODEC_S8", 0, False),
]

PQ1_TRACKS: list[TrackRow] = [
    ("PQ1", 1, "pq1_sound1", 18547, 7680, "PCM_CODEC_S8", 0, False),
    ("PQ1", 2, "pq1_sound2", 18547, 7680, "PCM_CODEC_S8", 0, False),
    ("PQ1", 3, "pq1_sound3", 18547, 7680, "PCM_CODEC_S8", 0, False),
    ("PQ1", 4, "pq1_sound4", 18547, 7680, "PCM_CODEC_S8", 0, False),
    ("PQ1", 5, "pq1_sound5", 18547, 7680, "PCM_CODEC_S8", 0, False),
    ("PQ1", 6, "pq1_sound6", 18547, 7680, "PCM_CODEC_S8", 0, False),
    ("PQ1", 7, "pq1_sound7", 18547, 7680, "PCM_CODEC_S8", 0, False),
    ("PQ1", 8, "pq1_sound8", 18547, 7680, "PCM_CODEC_S8", 0, False),
    ("PQ1", 9, "pq1_sound9", 28979, 7680, "PCM_CODEC_S8", 0, False),
    ("PQ1", 10, "pq1_sound10", 17477, 7680, "PCM_CODEC_S8", 0, False),
    ("PQ1", 11, "pq1_sound11", 18190, 7680, "PCM_CODEC_S8", 0, False),
    ("PQ1", 12, "pq1_sound12", 32545, 7680, "PCM_CODEC_S8", 0, False),
    ("PQ1", 13, "pq1_sound13", 19973, 7680, "PCM_CODEC_S8", 0, False),
    ("PQ1", 14, "pq1_sound14", 17120, 7680, "PCM_CODEC_S8", 0, False),
    ("PQ1", 15, "pq1_sound15", 45207, 7680, "PCM_CODEC_S8", 0, False),
    ("PQ1", 16, "pq1_sound16", 28533, 7680, "PCM_CODEC_S8", 0, False),
    ("PQ1", 17, "pq1_sound17", 30762, 7680, "PCM_CODEC_S8", 0, False),
    ("PQ1", 18, "pq1_sound18", 17477, 7680, "PCM_CODEC_S8", 0, False),
    ("PQ1", 19, "pq1_sound19", 20687, 7680, "PCM_CODEC_S8", 0, False),
    ("PQ1", 20, "pq1_sound20", 18547, 7680, "PCM_CODEC_S8", 0, False),
    ("PQ1", 21, "pq1_sound21", 18547, 7680, "PCM_CODEC_S8", 0, False),
    ("PQ1", 22, "pq1_sound22", 18547, 7680, "PCM_CODEC_S8", 0, False),
    ("PQ1", 23, "pq1_sound23", 17120, 7680, "PCM_CODEC_S8", 0, False),
    ("PQ1", 29, "pq1_sound29", 56352, 7680, "PCM_CODEC_S8", 0, False),
    ("PQ1", 30, "pq1_sound30", 262768, 7680, "PCM_CODEC_S8", 0, False),
    ("PQ1", 31, "pq1_sound31", 62237, 7680, "PCM_CODEC_S8", 0, False),
    ("PQ1", 32, "pq1_sound32", 246541, 7680, "PCM_CODEC_S8", 0, False),
    ("PQ1", 33, "pq1_sound33", 249750, 7680, "PCM_CODEC_S8", 0, False),
    ("PQ1", 34, "pq1_sound34", 65001, 7680, "PCM_CODEC_S8", 0, False),
    ("PQ1", 35, "pq1_sound35", 103877, 7680, "PCM_CODEC_S8", 0, False),
    ("PQ1", 36, "pq1_sound36", 56263, 7680, "PCM_CODEC_S8", 0, False),
    ("PQ1", 37, "pq1_sound37", 56352, 7680, "PCM_CODEC_S8", 0, False),
    ("PQ1", 38, "pq1_sound38", 111991, 7680, "PCM_CODEC_S8", 0, False),
    ("PQ1", 39, "pq1_sound39", 53053, 7680, "PCM_CODEC_S8", 0, False),
    ("PQ1", 40, "pq1_sound40", 59562, 7680, "PCM_CODEC_S8", 0, False),
    ("PQ1", 41, "pq1_sound41", 62059, 7680, "PCM_CODEC_S8", 0, False),
]

CURATED_PROFILES: dict[str, list[TrackRow]] = {
    "larry1": LARRY_TRACKS,
    "p-q1": PQ1_TRACKS,
}


def render_header(rows: list[TrackRow], *, source_description: str) -> str:
    lines = [
        "#ifndef _PCM_MUSIC_ASSETS_H",
        "#define _PCM_MUSIC_ASSETS_H",
        "",
        '#include "types.h"',
        '#include "pcm_music.h"',
        "",
        f"/* Source: {source_description} */",
        f"#define PCM_MUSIC_TRACK_COUNT {len(rows)}",
        "",
    ]

    for _, _, symbol, *_rest in rows:
        lines.append(f"extern const S8 {symbol}_pcm_data[];")

    if rows:
        lines.append("")
        lines.append("#define PCM_MUSIC_TRACKS(X) \\")
        rendered_rows = []
        for game_id, sound_num, symbol, length, sample_rate, codec, loop_offset, loop_enabled in rows:
            rendered_rows.append(
                f'    X("{game_id}", {sound_num}, {symbol}, {length}U, {sample_rate}U, {codec}, {loop_offset}, '
                f'{"TRUE" if loop_enabled else "FALSE"})'
            )
        for index, row in enumerate(rendered_rows):
            suffix = " \\" if index != len(rendered_rows) - 1 else ""
            lines.append(row + suffix)
    else:
        lines.append("#define PCM_MUSIC_TRACKS(X)")

    lines.extend(["", "#endif", ""])
    return "\n".join(lines)


def resolve_context(args: argparse.Namespace):
    if args.game_dir:
        game_dir = args.game_dir.resolve()
    elif args.game:
        game_dir = (repo_root().parent / "games" / args.game).resolve()
    else:
        raise ValueError("Provide --game-dir or --game")

    if not game_dir.exists():
        raise FileNotFoundError(f"Game directory not found: {game_dir}")
    return resolve_game_context(game_dir)


def generated_rows_for_context(context) -> list[TrackRow]:
    manifest_path = generated_manifest_path(context)
    if not manifest_path.exists():
        raise FileNotFoundError(
            f"No generated soundfont manifest found for {context.folder_name}: {manifest_path}"
        )
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    rows: list[TrackRow] = []
    for track in manifest.get("tracks", []):
        rows.append(
            (
                str(manifest.get("game_id", context.game_id)),
                int(track["sound_num"]),
                str(track["symbol"]),
                int(track["data_length"]),
                int(track["sample_rate"]),
                str(track.get("codec", "PCM_CODEC_S8")),
                int(track.get("loop_offset", 0)),
                bool(track.get("loop", False)),
            )
        )
    return rows


def configure_rows(context, mode: str) -> tuple[list[TrackRow], str]:
    curated_rows = CURATED_PROFILES.get(context.profile_name)
    generated_available = generated_manifest_path(context).exists()

    if mode == "curated":
        if curated_rows is None:
            raise FileNotFoundError(f"No curated streamed-music profile exists for {context.folder_name}")
        return curated_rows, f"curated profile {context.profile_name}"

    if mode == "generated":
        return generated_rows_for_context(context), f"generated manifest {generated_manifest_path(context)}"

    if mode == "none":
        return [], "live playback fallback"

    if curated_rows is not None:
        return curated_rows, f"curated profile {context.profile_name}"
    if generated_available:
        return generated_rows_for_context(context), f"generated manifest {generated_manifest_path(context)}"
    return [], "live playback fallback"


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--game", help="Selected game folder name, e.g. larry1 or P-Q1")
    parser.add_argument("--game-dir", type=Path, help="Full game directory path")
    parser.add_argument(
        "--mode",
        choices=["auto", "curated", "generated", "none"],
        default="auto",
        help="Asset selection mode. Default: auto",
    )
    args = parser.parse_args()

    context = resolve_context(args)
    rows, source_description = configure_rows(context, args.mode)

    header_path = repo_root() / "pcm_music_assets.h"
    header_path.write_text(render_header(rows, source_description=source_description), encoding="utf-8", newline="\n")
    print(
        f"Configured pcm_music_assets.h for {context.folder_name}: "
        f"{len(rows)} track(s) from {source_description}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
