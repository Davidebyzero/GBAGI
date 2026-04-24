#!/usr/bin/env python3
"""Generate streamed PCM assets from a saved streamed_music_projects JSON file."""

from __future__ import annotations

import argparse
import json
import shutil
import tempfile
from pathlib import Path

from agi_soundfont_tools import (
    build_input_fingerprint,
    generated_manifest_path,
    generated_project_dir,
    repo_root,
    resolve_game_context,
    stat_signature,
    validate_pcm_sample_rate,
)
from rip_streamed_audio import rip_audio_file


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--game-dir", required=True, type=Path, help="AGI game directory")
    parser.add_argument("--project", required=True, type=Path, help="Saved streamed_music_projects JSON file")
    parser.add_argument("--sample-rate", type=int, default=5120, help="PCM output sample rate. Default: 5120")
    parser.add_argument("--ffmpeg", type=Path, help="Optional ffmpeg override")
    parser.add_argument("--force", action="store_true", help="Rebuild even if cache matches")
    return parser.parse_args()


def log(message: str) -> None:
    print(message, flush=True)


def load_json(path: Path) -> dict[str, object]:
    return json.loads(path.read_text(encoding="utf-8"))


def manifest_files_exist(manifest: dict[str, object]) -> bool:
    for track in manifest.get("tracks", []):
        for key in ("pcm_path", "repo_pcm_path"):
            value = track.get(key)
            if value and not Path(value).exists():
                return False
    return True


def build_cache_payload(
    *,
    context,
    project_path: Path,
    project_payload: dict[str, object],
    ffmpeg_path: Path | None,
    sample_rate: int,
) -> dict[str, object]:
    track_signatures = []
    for track in project_payload.get("tracks", []):
        source_path = Path(str(track.get("source_path", "")))
        if source_path.exists():
            track_signatures.append(
                {
                    "sound_num": int(track["sound_num"]),
                    "source": stat_signature(source_path),
                }
            )
        else:
            track_signatures.append(
                {
                    "sound_num": int(track["sound_num"]),
                    "source": {"path": str(source_path), "missing": True},
                }
            )

    return {
        "schema_version": 1,
        "game_dir": str(context.game_dir),
        "folder_name": context.folder_name,
        "profile_name": context.profile_name,
        "project_slug": context.project_slug,
        "game_id": context.game_id,
        "project_json": stat_signature(project_path),
        "project_name": str(project_payload.get("project_name", project_path.stem)),
        "sample_rate": sample_rate,
        "ffmpeg": stat_signature(ffmpeg_path) if ffmpeg_path else None,
        "tracks": track_signatures,
    }


def commit_outputs(temp_project_dir: Path, final_project_dir: Path, temp_audio_files: list[tuple[Path, Path]]) -> None:
    if final_project_dir.exists():
        shutil.rmtree(final_project_dir)
    shutil.move(str(temp_project_dir), str(final_project_dir))
    for temp_path, final_path in temp_audio_files:
        final_path.parent.mkdir(parents=True, exist_ok=True)
        temp_path.replace(final_path)


def main() -> int:
    args = parse_args()
    game_dir = args.game_dir.resolve()
    if not game_dir.exists():
        raise FileNotFoundError(f"Game directory not found: {game_dir}")

    project_path = args.project.resolve()
    if not project_path.exists():
        raise FileNotFoundError(f"Project JSON not found: {project_path}")

    context = resolve_game_context(game_dir)
    project_payload = load_json(project_path)
    tracks = project_payload.get("tracks", [])
    if not tracks:
        raise RuntimeError(f"No tracks found in {project_path}")

    sample_rate = validate_pcm_sample_rate(args.sample_rate)
    ffmpeg_path = args.ffmpeg.resolve() if args.ffmpeg else None
    if ffmpeg_path is None:
        embedded_ffmpeg = str(project_payload.get("ffmpeg", "")).strip()
        if embedded_ffmpeg:
            ffmpeg_path = Path(embedded_ffmpeg).resolve()
    if ffmpeg_path and not ffmpeg_path.exists():
        raise FileNotFoundError(f"ffmpeg executable not found: {ffmpeg_path}")

    project_dir = generated_project_dir(context)
    project_dir.parent.mkdir(parents=True, exist_ok=True)
    manifest_path = generated_manifest_path(context)
    previous_manifest = load_json(manifest_path) if manifest_path.exists() else None
    cache_payload = build_cache_payload(
        context=context,
        project_path=project_path,
        project_payload=project_payload,
        ffmpeg_path=ffmpeg_path,
        sample_rate=sample_rate,
    )
    cache_key = build_input_fingerprint(cache_payload)

    if (
        not args.force
        and previous_manifest
        and previous_manifest.get("cache_key") == cache_key
        and manifest_files_exist(previous_manifest)
    ):
        log(
            f"Manual soundtrack cache is current for {context.folder_name} "
            f"({len(previous_manifest.get('tracks', []))} tracks)."
        )
        return 0

    temp_root = Path(tempfile.mkdtemp(prefix=f"gbagi_mp3_{context.project_slug}_", dir=str(project_dir.parent)))
    temp_project_dir = temp_root / context.project_slug
    temp_project_dir.mkdir(parents=True, exist_ok=True)
    pcm_dir = temp_project_dir / "pcm"
    pcm_dir.mkdir(parents=True, exist_ok=True)
    temp_audio_dir = temp_root / "repo_audio"
    temp_audio_dir.mkdir(parents=True, exist_ok=True)

    repo_audio_dir = repo_root() / "audio"
    repo_audio_dir.mkdir(parents=True, exist_ok=True)

    manifest_tracks: list[dict[str, object]] = []
    temp_audio_files: list[tuple[Path, Path]] = []

    try:
        log(f"Generating manual streamed soundtrack for {context.folder_name} from {project_path.name}")
        for track in tracks:
            sound_num = int(track["sound_num"])
            source_path = Path(str(track.get("source_path", ""))).resolve()
            symbol = str(track.get("symbol", "")).strip()
            if not symbol:
                symbol = f"{context.project_slug}_sound{sound_num}_custom"
            if not source_path.exists():
                raise FileNotFoundError(f"Track source missing for sound {sound_num}: {source_path}")

            loop_enabled = bool(track.get("loop", False))
            project_pcm_path = pcm_dir / f"{symbol}_{sample_rate}_mono_s8.pcm"
            temp_repo_pcm_path = temp_audio_dir / project_pcm_path.name
            final_repo_pcm_path = repo_audio_dir / project_pcm_path.name

            log(f"  Sound {sound_num}: {source_path.name} -> PCM")
            rip_result = rip_audio_file(
                input_path=source_path,
                symbol=symbol,
                output_path=project_pcm_path,
                ffmpeg_path=ffmpeg_path,
                game_id=context.game_id,
                sound_num=sound_num,
                register=False,
                loop=loop_enabled,
                sample_rate=sample_rate,
            )
            shutil.copy2(project_pcm_path, temp_repo_pcm_path)
            temp_audio_files.append((temp_repo_pcm_path, final_repo_pcm_path))
            manifest_tracks.append(
                {
                    "sound_num": sound_num,
                    "symbol": symbol,
                    "loop": loop_enabled,
                    "sample_rate": sample_rate,
                    "codec": "PCM_CODEC_S8",
                    "data_length": rip_result.data_length,
                    "source_path": str(source_path),
                    "pcm_path": str((project_dir / "pcm" / project_pcm_path.name).resolve()),
                    "pcm_filename": project_pcm_path.name,
                    "repo_pcm_path": str(final_repo_pcm_path.resolve()),
                }
            )

        manifest = {
            "schema_version": 1,
            "manifest_type": "manual_streamed_music_project",
            "generator": "generate_streamed_music_project_assets.py",
            "profile_name": context.profile_name,
            "project_slug": context.project_slug,
            "folder_name": context.folder_name,
            "game_id": context.game_id,
            "game_id_aliases": list(context.game_id_aliases),
            "source_game_dir": str(game_dir),
            "source_project_json": str(project_path),
            "project_name": str(project_payload.get("project_name", project_path.stem)),
            "sample_rate": sample_rate,
            "cache_key": cache_key,
            "cache_inputs": cache_payload,
            "tracks": manifest_tracks,
        }
        (temp_project_dir / "manifest.json").write_text(json.dumps(manifest, indent=2), encoding="utf-8", newline="\n")
        commit_outputs(temp_project_dir, project_dir, temp_audio_files)
        log(f"Wrote manifest: {manifest_path}")
        log(f"Generated {len(manifest_tracks)} manual streamed PCM track(s) for {context.folder_name}")
        return 0
    finally:
        shutil.rmtree(temp_root, ignore_errors=True)


if __name__ == "__main__":
    raise SystemExit(main())
