#!/usr/bin/env python3
"""Cache compiled GBAGI runtimes for repeated soundtrack builds."""

from __future__ import annotations

import argparse
import hashlib
import json
import shutil
from pathlib import Path

from agi_soundfont_tools import repo_root, resolve_game_context
from generate_pcm_music_assets_c import parse_header_symbols


BUILD_INPUT_PATTERNS = [
    "*.c",
    "*.h",
    "*.s",
]

EXTRA_BUILD_INPUTS = [
    "make.bat",
    "lnkscript",
    "extra/splashdata.c",
    "extra/splashlogodata.c",
]


def iter_build_inputs(gbagi_root: Path) -> list[Path]:
    files: set[Path] = set()
    for pattern in BUILD_INPUT_PATTERNS:
        files.update(
            path
            for path in gbagi_root.glob(pattern)
            if path.is_file() and not path.name.startswith("pcm_music_assets_")
        )
    for relative_path in EXTRA_BUILD_INPUTS:
        path = gbagi_root / relative_path
        if path.is_file():
            files.add(path)
    return sorted(files)


def hash_file(path: Path, relative_path: Path, digest: "hashlib._Hash") -> None:
    digest.update(relative_path.as_posix().encode("utf-8"))
    digest.update(b"\0")
    with path.open("rb") as handle:
        while True:
            chunk = handle.read(1024 * 1024)
            if not chunk:
                break
            digest.update(chunk)
    digest.update(b"\0")


def compute_runtime_fingerprint(
    *,
    gbagi_root: Path,
    context,
    config_mode: str,
    selected_mode: str,
    sample_rate: str,
) -> str:
    digest = hashlib.sha256()
    digest.update(f"game_id={context.game_id}\n".encode("utf-8"))
    digest.update(f"profile_name={context.profile_name}\n".encode("utf-8"))
    digest.update(f"folder_name={context.folder_name}\n".encode("utf-8"))
    digest.update(f"config_mode={config_mode}\n".encode("utf-8"))
    digest.update(f"selected_mode={selected_mode}\n".encode("utf-8"))
    digest.update(f"sample_rate={sample_rate}\n".encode("utf-8"))

    header_path = gbagi_root / "pcm_music_assets.h"
    hash_file(header_path, header_path.relative_to(gbagi_root), digest)
    generator_path = gbagi_root / "extra" / "generate_pcm_music_assets_c.py"
    if generator_path.exists():
        hash_file(generator_path, generator_path.relative_to(gbagi_root), digest)

    for _game_id, symbol, _length, sample_rate_value, codec in parse_header_symbols(header_path):
        if codec == "PCM_CODEC_S8":
            audio_path = gbagi_root / "audio" / f"{symbol}_{sample_rate_value}_mono_s8.pcm"
        elif codec == "PCM_CODEC_IMA_ADPCM_WAV":
            audio_path = gbagi_root / "audio" / f"{symbol}_{sample_rate_value}_4bit_ima.wav"
        else:
            raise ValueError(f"Unsupported codec in header: {codec}")
        hash_file(audio_path, audio_path.relative_to(gbagi_root), digest)

    for path in iter_build_inputs(gbagi_root):
        hash_file(path, path.relative_to(gbagi_root), digest)

    return digest.hexdigest()


def runtime_cache_dir(gbagi_root: Path, context, fingerprint: str) -> Path:
    return gbagi_root / "extra" / "runtime_cache" / context.folder_name / fingerprint


def metadata_payload(
    *,
    context,
    config_mode: str,
    selected_mode: str,
    sample_rate: str,
    fingerprint: str,
    runtime_path: Path,
) -> dict:
    return {
        "fingerprint": fingerprint,
        "game_id": context.game_id,
        "profile_name": context.profile_name,
        "folder_name": context.folder_name,
        "game_dir": str(context.game_dir),
        "config_mode": config_mode,
        "selected_mode": selected_mode,
        "sample_rate": sample_rate,
        "runtime_source": str(runtime_path),
    }


def cache_assets_dir(cache_dir: Path) -> Path:
    return cache_dir / "pcm_assets"


def generated_asset_sources(gbagi_root: Path) -> list[Path]:
    header_path = gbagi_root / "pcm_music_assets.h"
    generated_root = gbagi_root / "_generated" / "pcm_music_assets"
    sources: list[Path] = [header_path]
    for _game_id, symbol, _length, _sample_rate, _codec in parse_header_symbols(header_path):
        sources.append(generated_root / f"pcm_music_assets_{symbol}.c")
    return sources


def restore_generated_assets(gbagi_root: Path, assets_dir: Path) -> None:
    generated_root = gbagi_root / "_generated" / "pcm_music_assets"
    generated_root.mkdir(parents=True, exist_ok=True)
    target_paths = {generated_root / path.name for path in assets_dir.glob("pcm_music_assets_*.c")}
    target_paths.add(gbagi_root / "pcm_music_assets.h")

    for stale_path in generated_root.glob("pcm_music_assets_*.c"):
        if stale_path not in target_paths:
            stale_path.unlink(missing_ok=True)

    for cached_path in assets_dir.glob("pcm_music_assets_*.c"):
        shutil.copy2(cached_path, generated_root / cached_path.name)
    cached_header = assets_dir / "pcm_music_assets.h"
    if cached_header.exists():
        shutil.copy2(cached_header, gbagi_root / "pcm_music_assets.h")


def lookup_cache(args: argparse.Namespace) -> int:
    gbagi_root = repo_root()
    runtime_path = Path(args.runtime).resolve()
    context = resolve_game_context(Path(args.game_dir).resolve())
    fingerprint = compute_runtime_fingerprint(
        gbagi_root=gbagi_root,
        context=context,
        config_mode=args.config_mode,
        selected_mode=args.selected_mode,
        sample_rate=args.sample_rate,
    )
    cache_dir = runtime_cache_dir(gbagi_root, context, fingerprint)
    cache_runtime = cache_dir / "gbagi.bin"
    assets_dir = cache_assets_dir(cache_dir)
    print(f"CACHE_KEY={fingerprint}")
    print(f"CACHE_DIR={cache_dir}")
    if not cache_runtime.exists() or not assets_dir.exists():
        print("CACHE_RESULT=miss")
        return 0

    runtime_path.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(cache_runtime, runtime_path)
    restore_generated_assets(gbagi_root, assets_dir)
    print("CACHE_RESULT=hit")
    print(f"CACHE_RUNTIME={cache_runtime}")
    return 0


def store_cache(args: argparse.Namespace) -> int:
    gbagi_root = repo_root()
    runtime_path = Path(args.runtime).resolve()
    if not runtime_path.exists():
        raise FileNotFoundError(f"Runtime not found: {runtime_path}")

    context = resolve_game_context(Path(args.game_dir).resolve())
    fingerprint = compute_runtime_fingerprint(
        gbagi_root=gbagi_root,
        context=context,
        config_mode=args.config_mode,
        selected_mode=args.selected_mode,
        sample_rate=args.sample_rate,
    )
    cache_dir = runtime_cache_dir(gbagi_root, context, fingerprint)
    cache_dir.mkdir(parents=True, exist_ok=True)
    assets_dir = cache_assets_dir(cache_dir)
    assets_dir.mkdir(parents=True, exist_ok=True)
    cache_runtime = cache_dir / "gbagi.bin"
    shutil.copy2(runtime_path, cache_runtime)
    for source_path in generated_asset_sources(gbagi_root):
        if not source_path.exists():
            raise FileNotFoundError(f"Missing generated asset source: {source_path}")
        shutil.copy2(source_path, assets_dir / source_path.name)
    metadata_path = cache_dir / "metadata.json"
    metadata_path.write_text(
        json.dumps(
            metadata_payload(
                context=context,
                config_mode=args.config_mode,
                selected_mode=args.selected_mode,
                sample_rate=args.sample_rate,
                fingerprint=fingerprint,
                runtime_path=runtime_path,
            ),
            indent=2,
            sort_keys=True,
        )
        + "\n",
        encoding="utf-8",
    )
    print(f"CACHE_KEY={fingerprint}")
    print(f"CACHE_DIR={cache_dir}")
    print(f"CACHE_RUNTIME={cache_runtime}")
    print("CACHE_RESULT=stored")
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(dest="command", required=True)

    for command_name in ("lookup", "store"):
        subparser = subparsers.add_parser(command_name)
        subparser.add_argument("--game-dir", required=True)
        subparser.add_argument("--runtime", required=True)
        subparser.add_argument("--config-mode", required=True)
        subparser.add_argument("--selected-mode", required=True)
        subparser.add_argument("--sample-rate", required=True)

    return parser


def main() -> int:
    parser = build_parser()
    args = parser.parse_args()
    if args.command == "lookup":
        return lookup_cache(args)
    if args.command == "store":
        return store_cache(args)
    raise ValueError(f"Unknown command: {args.command}")


if __name__ == "__main__":
    raise SystemExit(main())
