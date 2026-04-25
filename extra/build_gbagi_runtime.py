#!/usr/bin/env python3
"""Build gbagi.elf/gbagi.bin deterministically without batch-loop quirks."""

from __future__ import annotations

import os
import subprocess
import sys
from pathlib import Path


def repo_root() -> Path:
    return Path(__file__).resolve().parent.parent


def toolbin_dir() -> Path:
    env_override = os.environ.get("GBAGI_TOOLBIN", "").strip()
    if env_override:
        return Path(env_override)
    return Path(r"H:\CODEX\GBAGI\DevKitAdv\bin")


def generated_assets_dir(root: Path) -> Path:
    return root / "_generated" / "pcm_music_assets"


def keep_generated_assets() -> bool:
    value = os.environ.get("GBAGI_KEEP_GENERATED_PCM_ASSETS", "").strip().lower()
    return value in {"1", "true", "yes", "on"}


def run(cmd: list[str], cwd: Path, env: dict[str, str]) -> None:
    print("")
    print("> " + " ".join(cmd), flush=True)
    subprocess.run(cmd, cwd=cwd, env=env, check=True)


def build_tool_env(toolbin: Path) -> dict[str, str]:
    system_root = os.environ.get("SYSTEMROOT", r"C:\Windows")
    windir = os.environ.get("WINDIR", system_root)
    env = {
        "PATH": f"{toolbin};{system_root}\\System32;{windir}",
        "SYSTEMROOT": system_root,
        "WINDIR": windir,
        "COMSPEC": os.environ.get("COMSPEC", rf"{system_root}\System32\cmd.exe"),
        "TEMP": os.environ.get("TEMP", os.environ.get("TMP", str(Path.cwd()))),
        "TMP": os.environ.get("TMP", os.environ.get("TEMP", str(Path.cwd()))),
        "GBAGI_GCC": str(toolbin / "gcc.exe"),
    }
    return env


def main() -> int:
    root = repo_root()
    generated_root = generated_assets_dir(root)
    toolbin = toolbin_dir()
    gcc = toolbin / "gcc.exe"
    assembler = toolbin / "as.exe"
    objcopy = toolbin / "objcopy.exe"
    cygwin = toolbin / "cygwin1.dll"

    if not gcc.exists():
        raise FileNotFoundError(f"Missing toolchain binary: {gcc}")
    if not cygwin.exists():
        raise FileNotFoundError(f"Missing runtime DLL: {cygwin}")

    env = build_tool_env(toolbin)

    for stale_name in ("gbagi.elf", "gbagi.bin", "pcm_music_asset_objs.rsp"):
        stale_path = root / stale_name
        if stale_path.exists():
            stale_path.unlink()
    for stale_obj in root.glob("pcm_music_assets_*.o"):
        stale_obj.unlink()
    for stale_src in root.glob("pcm_music_assets_*.c"):
        stale_src.unlink()
    if generated_root.exists():
        for stale_path in generated_root.glob("pcm_music_asset_objs.rsp"):
            stale_path.unlink()

    run([str(assembler), "-mthumb-interwork", "-o", "crt0.o", "crt0.s"], root, env)
    run([str(assembler), "-mthumb-interwork", "-o", "sramhook.o", "sramhook.s"], root, env)

    compile_steps = [
        ("extra/splashdata.o", "extra/splashdata.c", "-O3"),
        ("extra/splashlogodata.o", "extra/splashlogodata.c", "-O3"),
        ("main.o", "main.c", "-O3"),
        ("agimain.o", "agimain.c", "-O3"),
        ("gamedata.o", "gamedata.c", "-O3"),
        ("input.o", "input.c", "-O3"),
        ("keyboard.o", "keyboard.c", "-O3"),
        ("invobj.o", "invobj.c", "-O3"),
        ("logic.o", "logic.c", "-O3"),
        ("lsl1hack.o", "lsl1hack.c", "-O3"),
        ("lsl1_phone_easter_egg_clip.o", "lsl1_phone_easter_egg_clip.c", "-O1"),
        ("lsl1_kensentme_clip.o", "lsl1_kensentme_clip.c", "-O1"),
        ("sq2_vohaul_intro_clip.o", "sq2_vohaul_intro_clip.c", "-O1"),
        ("sq2_vohaul_part2_clip.o", "sq2_vohaul_part2_clip.c", "-O1"),
        ("picture.o", "picture.c", "-O3"),
        ("screen.o", "screen.c", "-O3"),
        ("status.o", "status.c", "-O3"),
        ("variables.o", "variables.c", "-O3"),
        ("views.o", "views.c", "-O3"),
        ("system.o", "system.c", "-O3"),
        ("commands.o", "commands.c", "-O3"),
        ("cmdagi.o", "cmdagi.c", "-O3"),
        ("cmdtest.o", "cmdtest.c", "-O3"),
        ("errmsg.o", "errmsg.c", "-O3"),
        ("sn76496.o", "SN76496.c", "-O3"),
        ("enhanced_audio.o", "enhanced_audio.c", "-O3"),
        ("pcm_music.o", "pcm_music.c", "-O3"),
        ("text.o", "text.c", "-O2"),
        ("menu.o", "menu.c", "-O3"),
        ("wingui.o", "wingui.c", "-O3"),
        ("parse.o", "parse.c", "-O3"),
        ("saverestore.o", "saverestore.c", "-O1"),
        ("interrupts.o", "interrupts.c", "-O3"),
    ]
    for obj_name, src_name, opt_flag in compile_steps:
        run(
            [str(gcc), "-c", opt_flag, "-mthumb", "-mthumb-interwork", "-o", obj_name, src_name],
            root,
            env,
        )

    run([sys.executable, "extra/generate_pcm_music_assets_c.py"], root, env)
    run([sys.executable, "extra/build_pcm_music_asset_objs.py"], root, env)

    pcm_asset_objs = sorted(
        str(path.relative_to(root))
        for path in generated_root.glob("pcm_music_assets_*.o")
    )
    link_cmd = [
        str(gcc),
        "-nostartfiles",
        "-Wl,-Tlnkscript",
        "-mthumb",
        "-mthumb-interwork",
        "-o",
        "gbagi.elf",
        "crt0.o",
        "sramhook.o",
        "extra/splashdata.o",
        "extra/splashlogodata.o",
        "main.o",
        "interrupts.o",
        "agimain.o",
        "gamedata.o",
        "input.o",
        "keyboard.o",
        "invobj.o",
        "logic.o",
        "lsl1hack.o",
        "lsl1_phone_easter_egg_clip.o",
        "lsl1_kensentme_clip.o",
        "sq2_vohaul_intro_clip.o",
        "sq2_vohaul_part2_clip.o",
        "picture.o",
        "screen.o",
        "status.o",
        "variables.o",
        "views.o",
        "system.o",
        "commands.o",
        "cmdagi.o",
        "cmdtest.o",
        "errmsg.o",
        "sn76496.o",
        "enhanced_audio.o",
        "pcm_music.o",
        *pcm_asset_objs,
        "text.o",
        "menu.o",
        "parse.o",
        "saverestore.o",
        "wingui.o",
    ]
    run(link_cmd, root, env)
    run([str(objcopy), "-O", "binary", "gbagi.elf", "gbagi.bin"], root, env)

    aginject = root / "gbarom" / "aginject.exe"
    if aginject.exists():
        run([str(aginject)], root, env)

    if generated_root.exists() and (not keep_generated_assets()):
        for temporary_path in generated_root.glob("pcm_music_assets_*.o"):
            temporary_path.unlink()
        for temporary_path in generated_root.glob("pcm_music_asset_objs.rsp"):
            temporary_path.unlink()
    elif generated_root.exists():
        print(
            "Keeping generated PCM asset files because GBAGI_KEEP_GENERATED_PCM_ASSETS is enabled.",
            flush=True,
        )

    print("Build completed successfully.", flush=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
