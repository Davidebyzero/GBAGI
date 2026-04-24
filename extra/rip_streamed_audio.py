#!/usr/bin/env python3
"""Convert source audio into GBAGI streamed PCM assets.

Outputs the exact format used by the hybrid PCM music backend:
  - 8-bit signed PCM
  - mono
  - 15360 Hz

Typical usage:
  py extra/rip_streamed_audio.py --input my_music.wav --symbol larry1_sound40
  py extra/rip_streamed_audio.py --input my_music.mp3 --symbol larry1_sound40 --ffmpeg H:/tools/ffmpeg/bin/ffmpeg.exe
  py extra/rip_streamed_audio.py --input my_music.wav --game LSL1 --sound 40 --register
"""

from __future__ import annotations

import argparse
import math
import shutil
import struct
import subprocess
import sys
import tempfile
import wave
from dataclasses import dataclass
from pathlib import Path


DEFAULT_TARGET_RATE = 15_360


@dataclass
class AudioBuffer:
    samples: list[float]
    sample_rate: int


@dataclass
class RipResult:
    output_path: Path
    data_length: int
    header_line: str | None
    preview_wav_path: Path | None


def repo_root() -> Path:
    return Path(__file__).resolve().parent.parent


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", required=True, type=Path, help="Source audio file")
    parser.add_argument("--symbol", help="Output symbol stem, for example larry1_sound40")
    parser.add_argument("--game", help='Game ID to emit/register, for example LSL1 or LLLLL')
    parser.add_argument("--sound", type=int, help="AGI sound number to emit/register")
    parser.add_argument(
        "--output",
        type=Path,
        help="Output PCM path. Defaults to audio/<symbol>_15360_mono_s8.pcm inside the repo.",
    )
    parser.add_argument(
        "--preview-wav",
        type=Path,
        help="Optional WAV copy for desktop listening.",
    )
    parser.add_argument(
        "--ffmpeg",
        type=Path,
        help="Optional ffmpeg executable for decoding MP3/FLAC/OGG/etc.",
    )
    parser.add_argument(
        "--register",
        action="store_true",
        help="Append/update a PCM_MUSIC_TRACKS row in pcm_music_assets.h",
    )
    parser.add_argument(
        "--loop",
        action="store_true",
        help="Mark the registered track as looping.",
    )
    parser.add_argument(
        "--sample-rate",
        type=int,
        default=DEFAULT_TARGET_RATE,
        help="Target PCM sample rate. Must divide 15360. Defaults to 15360.",
    )
    parser.add_argument(
        "--codec",
        default="PCM_CODEC_S8",
        choices=["PCM_CODEC_S8"],
        help="Codec to emit in header rows. Currently only PCM_CODEC_S8 is generated.",
    )
    return parser.parse_args()


def ensure_symbol(args: argparse.Namespace) -> str:
    if args.symbol:
        return args.symbol
    if args.game and args.sound is not None:
        return f"{args.game.lower()}_sound{args.sound}"
    raise ValueError("Provide --symbol or both --game and --sound")


def find_ffmpeg(explicit: Path | None) -> str | None:
    if explicit:
        return str(explicit)
    return shutil.which("ffmpeg")


def validate_sample_rate(sample_rate: int) -> int:
    if sample_rate <= 0:
        raise ValueError("Sample rate must be positive")
    if (DEFAULT_TARGET_RATE % sample_rate) != 0:
        raise ValueError(f"Sample rate must divide {DEFAULT_TARGET_RATE}")
    return sample_rate


def decode_with_ffmpeg(input_path: Path, ffmpeg_exe: str, target_rate: int) -> Path:
    temp_dir = Path(tempfile.mkdtemp(prefix="gbagi_rip_"))
    temp_wav = temp_dir / "decoded.wav"
    cmd = [
        ffmpeg_exe,
        "-y",
        "-i",
        str(input_path),
        "-vn",
        "-acodec",
        "pcm_s16le",
        "-ac",
        "1",
        "-ar",
        str(target_rate),
        str(temp_wav),
    ]
    subprocess.run(cmd, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    return temp_wav


def bytes_to_float_samples(raw: bytes, sampwidth: int) -> list[float]:
    if sampwidth == 1:
        return [((b - 128) / 128.0) for b in raw]
    if sampwidth == 2:
        count = len(raw) // 2
        ints = struct.unpack("<" + "h" * count, raw)
        return [max(-1.0, min(1.0, sample / 32768.0)) for sample in ints]
    if sampwidth == 3:
        out: list[float] = []
        for i in range(0, len(raw), 3):
            value = raw[i] | (raw[i + 1] << 8) | (raw[i + 2] << 16)
            if value & 0x800000:
                value -= 0x1000000
            out.append(max(-1.0, min(1.0, value / 8388608.0)))
        return out
    if sampwidth == 4:
        count = len(raw) // 4
        ints = struct.unpack("<" + "i" * count, raw)
        return [max(-1.0, min(1.0, sample / 2147483648.0)) for sample in ints]
    raise ValueError(f"Unsupported WAV sample width: {sampwidth}")


def mix_to_mono(samples: list[float], channels: int) -> list[float]:
    if channels == 1:
        return samples
    mono: list[float] = []
    for i in range(0, len(samples), channels):
        frame = samples[i : i + channels]
        mono.append(sum(frame) / len(frame))
    return mono


def load_wav(path: Path) -> AudioBuffer:
    with wave.open(str(path), "rb") as wav_file:
        channels = wav_file.getnchannels()
        sampwidth = wav_file.getsampwidth()
        sample_rate = wav_file.getframerate()
        frame_count = wav_file.getnframes()
        raw = wav_file.readframes(frame_count)

    samples = bytes_to_float_samples(raw, sampwidth)
    samples = mix_to_mono(samples, channels)
    return AudioBuffer(samples=samples, sample_rate=sample_rate)


def resample_linear(buffer: AudioBuffer, target_rate: int) -> AudioBuffer:
    if buffer.sample_rate == target_rate:
        return buffer
    if not buffer.samples:
        return AudioBuffer(samples=[], sample_rate=target_rate)

    ratio = target_rate / buffer.sample_rate
    out_len = max(1, int(round(len(buffer.samples) * ratio)))
    out: list[float] = []
    for i in range(out_len):
        src_pos = i / ratio
        left = int(math.floor(src_pos))
        right = min(left + 1, len(buffer.samples) - 1)
        frac = src_pos - left
        value = buffer.samples[left] * (1.0 - frac) + buffer.samples[right] * frac
        out.append(value)
    return AudioBuffer(samples=out, sample_rate=target_rate)


def float_to_s8_pcm(samples: list[float]) -> bytes:
    out = bytearray()
    for sample in samples:
        clamped = max(-1.0, min(1.0, sample))
        value = int(round(clamped * 127.0))
        out.append(value & 0xFF)
    return bytes(out)


def write_preview_wav(path: Path, pcm_bytes: bytes, sample_rate: int) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(path), "wb") as wav_file:
        wav_file.setnchannels(1)
        wav_file.setsampwidth(1)
        wav_file.setframerate(sample_rate)
        wav_file.writeframes(pcm_bytes)


def play_wav_file(path: Path, async_play: bool = False) -> None:
    if sys.platform != "win32":
        raise RuntimeError("Audio preview currently supports Windows only")

    import winsound

    flags = winsound.SND_FILENAME
    if async_play:
        flags |= winsound.SND_ASYNC
    winsound.PlaySound(str(path), flags)


def stop_playback() -> None:
    if sys.platform != "win32":
        return

    import winsound

    winsound.PlaySound(None, winsound.SND_PURGE)


def preview_source_audio(input_path: Path, ffmpeg_path: Path | None = None, async_play: bool = True) -> None:
    if not input_path.exists():
        raise FileNotFoundError(input_path)

    decode_path = input_path
    cleanup_dir: Path | None = None
    if input_path.suffix.lower() != ".wav":
        ffmpeg_exe = find_ffmpeg(ffmpeg_path)
        if not ffmpeg_exe:
            raise RuntimeError(
                "Previewing non-WAV input requires ffmpeg. Set an ffmpeg path in the GUI first."
            )
        decode_path = decode_with_ffmpeg(input_path, ffmpeg_exe, DEFAULT_TARGET_RATE)
        cleanup_dir = decode_path.parent

    try:
        play_wav_file(decode_path, async_play=async_play)
    finally:
        if cleanup_dir is not None and not async_play:
            shutil.rmtree(cleanup_dir, ignore_errors=True)


def header_row(
    game_id: str,
    sound_num: int,
    symbol: str,
    data_len: int,
    sample_rate: int,
    loop: bool,
    codec: str,
) -> str:
    loop_flag = "TRUE" if loop else "FALSE"
    return f'    X("{game_id}", {sound_num}, {symbol}, {data_len}U, {sample_rate}U, {codec}, 0, {loop_flag})'


def update_header_file(
    header_path: Path,
    game_id: str,
    sound_num: int,
    symbol: str,
    data_len: int,
    sample_rate: int,
    loop: bool,
    codec: str,
) -> None:
    text = header_path.read_text(encoding="utf-8")
    row = header_row(game_id, sound_num, symbol, data_len, sample_rate, loop, codec)
    lines = text.splitlines()

    start_index = None
    end_index = None
    for i, line in enumerate(lines):
        if line.startswith("#define PCM_MUSIC_TRACKS(X)"):
            start_index = i
            continue
        if start_index is not None and line.strip() == "":
            end_index = i
            break
    if start_index is None:
        raise RuntimeError("Could not find PCM_MUSIC_TRACKS(X) block in header")
    if end_index is None:
        end_index = len(lines)

    block = lines[start_index + 1 : end_index]
    target_prefix = f'X("{game_id}", {sound_num},'
    replaced = False
    for i, line in enumerate(block):
        stripped = line.strip().rstrip("\\").rstrip()
        if stripped.startswith(target_prefix):
            suffix = " \\" if line.rstrip().endswith("\\") else ""
            block[i] = row + suffix
            replaced = True
            break

    if not replaced:
        if block:
            block[-1] = block[-1].rstrip()
            if not block[-1].endswith("\\"):
                block[-1] += " \\"
        block.append(row)

    new_lines = lines[: start_index + 1] + block + lines[end_index:]
    header_path.write_text("\n".join(new_lines) + "\n", encoding="utf-8", newline="\n")


def rip_audio_file(
    *,
    input_path: Path,
    symbol: str,
    output_path: Path,
    preview_wav_path: Path | None = None,
    ffmpeg_path: Path | None = None,
    game_id: str | None = None,
    sound_num: int | None = None,
    register: bool = False,
    loop: bool = False,
    codec: str = "PCM_CODEC_S8",
    header_path: Path | None = None,
    sample_rate: int = DEFAULT_TARGET_RATE,
) -> RipResult:
    sample_rate = validate_sample_rate(sample_rate)
    if not input_path.exists():
        raise FileNotFoundError(input_path)

    decode_path = input_path
    cleanup_dir: Path | None = None
    if input_path.suffix.lower() != ".wav":
        ffmpeg_exe = find_ffmpeg(ffmpeg_path)
        if not ffmpeg_exe:
            raise RuntimeError(
                "Non-WAV input requires ffmpeg. Provide an ffmpeg path or convert your source to WAV first."
            )
        decode_path = decode_with_ffmpeg(input_path, ffmpeg_exe, sample_rate)
        cleanup_dir = decode_path.parent

    try:
        buffer = load_wav(decode_path)
        buffer = resample_linear(buffer, sample_rate)
        pcm_bytes = float_to_s8_pcm(buffer.samples)

        output_path.parent.mkdir(parents=True, exist_ok=True)
        output_path.write_bytes(pcm_bytes)

        if preview_wav_path is not None:
            write_preview_wav(preview_wav_path, pcm_bytes, sample_rate)

        emitted_header_line: str | None = None
        if game_id is not None and sound_num is not None:
            emitted_header_line = header_row(game_id, sound_num, symbol, len(pcm_bytes), sample_rate, loop, codec)
            if register:
                resolved_header = header_path or (repo_root() / "pcm_music_assets.h")
                update_header_file(
                    resolved_header,
                    game_id,
                    sound_num,
                    symbol,
                    len(pcm_bytes),
                    sample_rate,
                    loop,
                    codec,
                )
        elif register:
            raise ValueError("register=True requires game_id and sound_num")

        return RipResult(
            output_path=output_path,
            data_length=len(pcm_bytes),
            header_line=emitted_header_line,
            preview_wav_path=preview_wav_path,
        )
    finally:
        if cleanup_dir is not None:
            shutil.rmtree(cleanup_dir, ignore_errors=True)


def main() -> int:
    args = parse_args()
    symbol = ensure_symbol(args)
    root = repo_root()

    if not args.input.exists():
        raise FileNotFoundError(args.input)

    sample_rate = validate_sample_rate(args.sample_rate)
    output_path = args.output or (root / "audio" / f"{symbol}_{sample_rate}_mono_s8.pcm")
    result = rip_audio_file(
        input_path=args.input,
        symbol=symbol,
        output_path=output_path,
        preview_wav_path=args.preview_wav,
        ffmpeg_path=args.ffmpeg,
        game_id=args.game,
        sound_num=args.sound,
        register=args.register,
        loop=args.loop,
        codec=args.codec,
        header_path=root / "pcm_music_assets.h",
        sample_rate=sample_rate,
    )

    print(f"Wrote PCM: {result.output_path} ({result.data_length} bytes)")
    if result.preview_wav_path:
        print(f"Wrote preview WAV: {result.preview_wav_path}")
    if result.header_line:
        print("Header row:")
        print(result.header_line)
        if args.register:
            print(f"Updated header: {root / 'pcm_music_assets.h'}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
