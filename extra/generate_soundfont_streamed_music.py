#!/usr/bin/env python3
"""Generate per-game streamed GBAGI PCM assets from AGI sounds via SoundFont rendering."""

from __future__ import annotations

import argparse
import json
import shutil
import tempfile
from pathlib import Path

import mido

from agi_soundfont_tools import (
    build_input_fingerprint,
    convert_agi_sound_to_midi,
    convert_wav_to_mp3,
    find_combined_dir_resource,
    generated_manifest_path,
    generated_project_dir,
    list_present_sound_nums,
    render_midi_to_wav,
    repo_root,
    resolve_game_context,
    stat_signature,
    validate_pcm_sample_rate,
)
from rip_streamed_audio import rip_audio_file


DEFAULT_RENDER_RATE = 44_100
MAX_REASONABLE_TRACK_SECONDS = 600.0


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--game-dir", required=True, type=Path, help="AGI game directory")
    parser.add_argument("--soundfont", required=True, type=Path, help="SoundFont file to render with")
    parser.add_argument("--ffmpeg", type=Path, help="Optional ffmpeg executable to write MP3 previews")
    parser.add_argument("--fluidsynth", type=Path, help="FluidSynth executable")
    parser.add_argument("--sample-rate", type=int, default=5120, help="PCM output sample rate. Default: 5120")
    parser.add_argument(
        "--render-sample-rate",
        type=int,
        default=DEFAULT_RENDER_RATE,
        help="Intermediate FluidSynth WAV sample rate. Default: 44100",
    )
    parser.add_argument(
        "--force",
        action="store_true",
        help="Rebuild even if the cached manifest still matches the current inputs",
    )
    return parser.parse_args()


def log(message: str) -> None:
    print(message, flush=True)


def load_manifest(path: Path) -> dict[str, object] | None:
    if not path.exists():
        return None
    return json.loads(path.read_text(encoding="utf-8"))


def manifest_files_exist(manifest: dict[str, object]) -> bool:
    for track in manifest.get("tracks", []):
        for key in ("pcm_path", "repo_pcm_path", "midi_path", "wav_path"):
            value = track.get(key)
            if value and not Path(value).exists():
                return False
        mp3_path = track.get("mp3_path")
        if mp3_path and not Path(mp3_path).exists():
            return False
    return True


def build_cache_payload(
    *,
    context,
    soundfont_path: Path,
    fluidsynth_path: Path,
    ffmpeg_path: Path | None,
    sample_rate: int,
    render_sample_rate: int,
    sound_nums: list[int],
) -> dict[str, object]:
    volume_paths = [
        path
        for path in sorted(context.game_dir.iterdir())
        if path.is_file() and ".VOL." in f".{path.name.upper()}"
    ]
    combined_dir = find_combined_dir_resource(context.game_dir)
    resource_paths = []
    for candidate in [context.game_dir / "snddir", context.game_dir / "SNDDIR", combined_dir, *volume_paths]:
        if candidate and candidate.exists():
            resource_paths.append(candidate)
    return {
        "schema_version": 1,
        "game_dir": str(context.game_dir),
        "folder_name": context.folder_name,
        "profile_name": context.profile_name,
        "project_slug": context.project_slug,
        "game_id": context.game_id,
        "game_id_aliases": list(context.game_id_aliases),
        "soundfont": stat_signature(soundfont_path),
        "fluidsynth": stat_signature(fluidsynth_path),
        "ffmpeg": stat_signature(ffmpeg_path) if ffmpeg_path else None,
        "sample_rate": sample_rate,
        "render_sample_rate": render_sample_rate,
        "sound_nums": sound_nums,
        "resource_signatures": [stat_signature(path) for path in resource_paths],
    }


def commit_project_outputs(
    temp_project_dir: Path,
    final_project_dir: Path,
    temp_audio_files: list[tuple[Path, Path]],
    stale_audio_files: list[Path],
) -> None:
    if final_project_dir.exists():
        shutil.rmtree(final_project_dir)
    shutil.move(str(temp_project_dir), str(final_project_dir))

    for stale_path in stale_audio_files:
        try:
            stale_path.unlink()
        except FileNotFoundError:
            pass

    for temp_path, final_path in temp_audio_files:
        final_path.parent.mkdir(parents=True, exist_ok=True)
        shutil.move(str(temp_path), str(final_path))


def main() -> int:
    args = parse_args()
    game_dir = args.game_dir.resolve()
    if not game_dir.exists():
        raise FileNotFoundError(f"Game directory not found: {game_dir}")

    sample_rate = validate_pcm_sample_rate(args.sample_rate)
    soundfont_path = args.soundfont.resolve()
    if not soundfont_path.exists():
        raise FileNotFoundError(f"SoundFont not found: {soundfont_path}")

    if args.fluidsynth is None:
        raise ValueError("FluidSynth is required. Pass --fluidsynth <path-to-fluidsynth.exe>.")
    fluidsynth_path = args.fluidsynth.resolve()
    if not fluidsynth_path.exists():
        raise FileNotFoundError(f"FluidSynth executable not found: {fluidsynth_path}")

    ffmpeg_path = args.ffmpeg.resolve() if args.ffmpeg else None
    if ffmpeg_path and not ffmpeg_path.exists():
        raise FileNotFoundError(f"ffmpeg executable not found: {ffmpeg_path}")

    context = resolve_game_context(game_dir)
    sound_nums = list_present_sound_nums(game_dir)
    if not sound_nums:
        raise RuntimeError(f"No AGI sound resources found in {game_dir}")

    project_dir = generated_project_dir(context)
    manifest_path = generated_manifest_path(context)
    project_dir.parent.mkdir(parents=True, exist_ok=True)
    cache_payload = build_cache_payload(
        context=context,
        soundfont_path=soundfont_path,
        fluidsynth_path=fluidsynth_path,
        ffmpeg_path=ffmpeg_path,
        sample_rate=sample_rate,
        render_sample_rate=args.render_sample_rate,
        sound_nums=sound_nums,
    )
    cache_key = build_input_fingerprint(cache_payload)
    previous_manifest = load_manifest(manifest_path)

    if (
        not args.force
        and previous_manifest
        and previous_manifest.get("cache_key") == cache_key
        and manifest_files_exist(previous_manifest)
    ):
        log(
            f"Generated soundtrack cache is current for {context.folder_name} "
            f"({len(previous_manifest.get('tracks', []))} tracks)."
        )
        return 0

    repo_audio_dir = repo_root() / "audio"
    repo_audio_dir.mkdir(parents=True, exist_ok=True)
    temp_root = Path(tempfile.mkdtemp(prefix=f"gbagi_sf2_{context.project_slug}_"))
    temp_project_dir = temp_root / context.project_slug
    temp_project_dir.mkdir(parents=True, exist_ok=True)

    midi_dir = temp_project_dir / "midi"
    wav_dir = temp_project_dir / "wav"
    mp3_dir = temp_project_dir / "mp3"
    pcm_dir = temp_project_dir / "pcm"
    for directory in (midi_dir, wav_dir, mp3_dir, pcm_dir):
        directory.mkdir(parents=True, exist_ok=True)

    temp_audio_dir = temp_root / "repo_audio"
    temp_audio_dir.mkdir(parents=True, exist_ok=True)

    tracks: list[dict[str, object]] = []
    skipped_tracks: list[dict[str, object]] = []
    temp_audio_files: list[tuple[Path, Path]] = []

    try:
        log(f"Generating SoundFont streamed assets for {context.folder_name} -> {context.game_id}")
        log(f"SoundFont: {soundfont_path}")
        log(f"PCM sample rate: {sample_rate}")
        if ffmpeg_path:
            log(f"ffmpeg: {ffmpeg_path}")
        else:
            log("ffmpeg: not provided; MP3 previews will be skipped")

        for sound_num in sound_nums:
            symbol = f"{context.project_slug}_sound{sound_num}_sf2"
            midi_path = midi_dir / f"{symbol}.mid"
            wav_path = wav_dir / f"{symbol}.wav"
            mp3_path = mp3_dir / f"{symbol}.mp3"
            project_pcm_path = pcm_dir / f"{symbol}_{sample_rate}_mono_s8.pcm"
            final_project_midi_path = project_dir / "midi" / midi_path.name
            final_project_wav_path = project_dir / "wav" / wav_path.name
            final_project_mp3_path = project_dir / "mp3" / mp3_path.name
            final_project_pcm_path = project_dir / "pcm" / project_pcm_path.name
            temp_repo_pcm_path = temp_audio_dir / project_pcm_path.name
            final_repo_pcm_path = repo_audio_dir / project_pcm_path.name
            mp3_generated = False

            try:
                log(f"  Sound {sound_num}: MIDI -> WAV -> PCM")
                convert_agi_sound_to_midi(game_dir, sound_num, midi_path)
                midi_length_seconds = mido.MidiFile(str(midi_path)).length
                if midi_length_seconds > MAX_REASONABLE_TRACK_SECONDS:
                    raise RuntimeError(
                        f"converted MIDI is too long ({midi_length_seconds:.2f}s > {MAX_REASONABLE_TRACK_SECONDS:.0f}s)"
                    )
                render_midi_to_wav(
                    fluidsynth_path,
                    soundfont_path,
                    midi_path,
                    wav_path,
                    render_sample_rate=args.render_sample_rate,
                )
                if ffmpeg_path:
                    try:
                        convert_wav_to_mp3(ffmpeg_path, wav_path, mp3_path)
                        mp3_generated = True
                    except Exception as exc:  # noqa: BLE001
                        log(f"  Sound {sound_num}: MP3 preview skipped ({exc})")

                rip_result = rip_audio_file(
                    input_path=wav_path,
                    symbol=symbol,
                    output_path=project_pcm_path,
                    ffmpeg_path=ffmpeg_path,
                    game_id=context.game_id,
                    sound_num=sound_num,
                    register=False,
                    loop=False,
                    sample_rate=sample_rate,
                )
                shutil.copy2(project_pcm_path, temp_repo_pcm_path)
                temp_audio_files.append((temp_repo_pcm_path, final_repo_pcm_path))

                tracks.append(
                    {
                        "sound_num": sound_num,
                        "symbol": symbol,
                        "loop": False,
                        "sample_rate": sample_rate,
                        "codec": "PCM_CODEC_S8",
                        "data_length": rip_result.data_length,
                        "midi_path": str(final_project_midi_path.resolve()),
                        "wav_path": str(final_project_wav_path.resolve()),
                        "mp3_path": str(final_project_mp3_path.resolve()) if mp3_generated else "",
                        "pcm_path": str(final_project_pcm_path.resolve()),
                        "pcm_filename": project_pcm_path.name,
                        "repo_pcm_path": str(final_repo_pcm_path.resolve()),
                    }
                )
            except Exception as exc:  # noqa: BLE001
                log(f"  Sound {sound_num}: skipped ({exc})")
                skipped_tracks.append({"sound_num": sound_num, "reason": str(exc)})

        if not tracks:
            raise RuntimeError(f"No streamed tracks could be generated for {context.folder_name}")

        manifest = {
            "schema_version": 1,
            "manifest_type": "generated_soundfont_streamed_music",
            "generator": "generate_soundfont_streamed_music.py",
            "profile_name": context.profile_name,
            "project_slug": context.project_slug,
            "folder_name": context.folder_name,
            "game_id": context.game_id,
            "game_id_aliases": list(context.game_id_aliases),
            "source_game_dir": str(game_dir),
            "soundfont_path": str(soundfont_path),
            "fluidsynth_path": str(fluidsynth_path),
            "ffmpeg_path": str(ffmpeg_path) if ffmpeg_path else "",
            "sample_rate": sample_rate,
            "render_sample_rate": args.render_sample_rate,
            "sound_nums": sound_nums,
            "generated_sound_nums": [track["sound_num"] for track in tracks],
            "skipped_tracks": skipped_tracks,
            "cache_key": cache_key,
            "cache_inputs": cache_payload,
            "tracks": tracks,
        }
        (temp_project_dir / "manifest.json").write_text(
            json.dumps(manifest, indent=2),
            encoding="utf-8",
            newline="\n",
        )

        stale_audio_files: list[Path] = []
        if previous_manifest:
            previous_paths = {
                Path(track["repo_pcm_path"])
                for track in previous_manifest.get("tracks", [])
                if track.get("repo_pcm_path")
            }
            current_paths = {Path(track["repo_pcm_path"]) for track in tracks}
            stale_audio_files = sorted(previous_paths - current_paths)

        commit_project_outputs(temp_project_dir, project_dir, temp_audio_files, stale_audio_files)
        log(f"Wrote manifest: {manifest_path}")
        log(
            f"Generated {len(tracks)} streamed PCM track(s) for {context.folder_name}; "
            f"skipped {len(skipped_tracks)}"
        )
        return 0
    finally:
        shutil.rmtree(temp_root, ignore_errors=True)


if __name__ == "__main__":
    raise SystemExit(main())
