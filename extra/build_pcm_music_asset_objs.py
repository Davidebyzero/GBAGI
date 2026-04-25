#!/usr/bin/env python3
"""Compile generated pcm_music_assets_*.c files and emit a linker response file."""

from __future__ import annotations

import os
import shutil
import subprocess
from pathlib import Path

from generate_pcm_music_assets_c import parse_header_symbols


def repo_root() -> Path:
    return Path(__file__).resolve().parent.parent


def generated_assets_dir(root: Path) -> Path:
    return root / "_generated" / "pcm_music_assets"


def resolve_gcc_exe() -> str:
    env_override = os.environ.get("GBAGI_GCC", "").strip()
    if env_override:
        return env_override
    path_hit = shutil.which("gcc")
    if path_hit:
        return path_hit
    default_path = Path(r"H:\CODEX\GBAGI\DevKitAdv\bin\gcc.exe")
    if default_path.exists():
        return str(default_path)
    raise FileNotFoundError("Unable to locate gcc.exe for PCM asset compilation")


def main() -> int:
    root = repo_root()
    generated_root = generated_assets_dir(root)
    header_path = root / "pcm_music_assets.h"
    gcc_exe = resolve_gcc_exe()
    response_path = generated_root / "pcm_music_asset_objs.rsp"
    source_paths = [
        generated_root / f"pcm_music_assets_{symbol}.c"
        for _game_id, symbol, _length, _sample_rate, _codec in parse_header_symbols(header_path)
    ]
    target_objects = {generated_root / f"{source_path.stem}.o" for source_path in source_paths}
    object_names: list[str] = []
    built_count = 0
    reused_count = 0

    generated_root.mkdir(parents=True, exist_ok=True)
    response_path.write_text("", encoding="ascii", newline="\n")

    for stale_object in generated_root.glob("pcm_music_assets_*.o"):
        if stale_object not in target_objects:
            stale_object.unlink(missing_ok=True)

    for source_path in source_paths:
        if not source_path.exists():
            raise FileNotFoundError(f"Missing generated PCM asset source: {source_path}")
        object_name = f"{source_path.stem}.o"
        object_path = generated_root / object_name
        if object_path.exists() and (object_path.stat().st_mtime >= source_path.stat().st_mtime):
            reused_count += 1
        else:
            subprocess.run(
                [
                    gcc_exe,
                    "-c",
                    "-O1",
                    "-mthumb",
                    "-mthumb-interwork",
                    "-I",
                    str(root),
                    "-o",
                    str(object_path),
                    str(source_path),
                ],
                check=True,
                cwd=root,
            )
            built_count += 1
        object_names.append(str(object_path.relative_to(root)))

    if object_names:
        response_path.write_text(
            "\n".join(object_names) + "\n",
            encoding="ascii",
            newline="\n",
        )

    print(
        f"Prepared {len(object_names)} PCM asset object(s)"
        f" ({built_count} built, {reused_count} reused)"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
