#!/usr/bin/env python3
"""GUI to extract AGI sound cues, convert them to approximate MIDI, render via SoundFont, and export GBAGI PCM."""

from __future__ import annotations

import threading
import tkinter as tk
from dataclasses import dataclass
from pathlib import Path
from tkinter import filedialog, messagebox, ttk

from agi_soundfont_tools import (
    convert_agi_sound_to_midi,
    list_present_sound_nums,
    render_midi_to_wav,
)
from rip_streamed_audio import preview_source_audio, repo_root, rip_audio_file, stop_playback

DEFAULT_FFMPEG = Path(r"H:\CODEX\GBAGI\ffmpeg\bin\ffmpeg.exe")
DEFAULT_FLUIDSYNTH = Path(r"H:\CODEX\GBAGI\fluidsynth\bin\fluidsynth.exe")
DEFAULT_PROJECT_ROOT = repo_root() / "extra" / "agi_soundfont_projects"


@dataclass
class CueRow:
    sound_num: int
    symbol: str
    sf2_override: str = ""
    loop: bool = False


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
    subprocess.run(cmd, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)


class AGISoundfontRipperApp:
    def __init__(self, root: tk.Tk):
        self.root = root
        self.root.title("AGI SoundFont Ripper")
        self.root.geometry("1320x780")

        self.game_dir_var = tk.StringVar()
        self.game_id_var = tk.StringVar(value="LSL1")
        self.default_sf2_var = tk.StringVar(value=str(Path(r"H:\CODEX\GBAGI\audio upgrade\Gravis Ultrasound.sf2")))
        self.ffmpeg_var = tk.StringVar(value=str(DEFAULT_FFMPEG))
        self.fluidsynth_var = tk.StringVar(value=str(DEFAULT_FLUIDSYNTH))
        self.status_var = tk.StringVar(value="Select a game and extract cues.")
        self.register_var = tk.BooleanVar(value=False)

        self.rows: list[CueRow] = []
        self.project_dir: Path | None = None

        self._build_ui()

    def _build_ui(self) -> None:
        outer = ttk.Frame(self.root, padding=10)
        outer.pack(fill=tk.BOTH, expand=True)

        cfg = ttk.LabelFrame(outer, text="Project", padding=10)
        cfg.pack(fill=tk.X)

        ttk.Label(cfg, text="Game folder").grid(row=0, column=0, sticky="w")
        ttk.Entry(cfg, textvariable=self.game_dir_var).grid(row=0, column=1, sticky="ew", padx=(8, 8))
        ttk.Button(cfg, text="Browse...", command=self._browse_game).grid(row=0, column=2)
        ttk.Button(cfg, text="Extract Cues", command=self.extract_cues).grid(row=0, column=3, padx=(12, 0))

        ttk.Label(cfg, text="Game ID").grid(row=1, column=0, sticky="w", pady=(8, 0))
        ttk.Entry(cfg, textvariable=self.game_id_var, width=12).grid(row=1, column=1, sticky="w", padx=(8, 8), pady=(8, 0))
        ttk.Checkbutton(cfg, text="Register PCM header rows", variable=self.register_var).grid(row=1, column=3, sticky="w", pady=(8, 0))

        ttk.Label(cfg, text="Default .sf2").grid(row=2, column=0, sticky="w", pady=(8, 0))
        ttk.Entry(cfg, textvariable=self.default_sf2_var).grid(row=2, column=1, sticky="ew", padx=(8, 8), pady=(8, 0))
        ttk.Button(cfg, text="Browse...", command=self._browse_sf2).grid(row=2, column=2, pady=(8, 0))

        ttk.Label(cfg, text="FluidSynth").grid(row=3, column=0, sticky="w", pady=(8, 0))
        ttk.Entry(cfg, textvariable=self.fluidsynth_var).grid(row=3, column=1, sticky="ew", padx=(8, 8), pady=(8, 0))
        ttk.Button(cfg, text="Browse...", command=self._browse_fluidsynth).grid(row=3, column=2, pady=(8, 0))

        ttk.Label(cfg, text="ffmpeg").grid(row=4, column=0, sticky="w", pady=(8, 0))
        ttk.Entry(cfg, textvariable=self.ffmpeg_var).grid(row=4, column=1, sticky="ew", padx=(8, 8), pady=(8, 0))
        ttk.Button(cfg, text="Browse...", command=self._browse_ffmpeg).grid(row=4, column=2, pady=(8, 0))

        cfg.columnconfigure(1, weight=1)

        actions = ttk.Frame(outer)
        actions.pack(fill=tk.X, pady=(10, 8))
        ttk.Button(actions, text="Set Song SF2", command=self.set_song_sf2).pack(side=tk.LEFT)
        ttk.Button(actions, text="Clear Song SF2", command=self.clear_song_sf2).pack(side=tk.LEFT, padx=(8, 0))
        ttk.Button(actions, text="Generate MIDI", command=self.generate_midi_selected).pack(side=tk.LEFT, padx=(20, 0))
        ttk.Button(actions, text="Render Selected", command=self.render_selected).pack(side=tk.LEFT, padx=(8, 0))
        ttk.Button(actions, text="Render All", command=self.render_all).pack(side=tk.LEFT, padx=(8, 0))
        ttk.Button(actions, text="Play MP3", command=self.play_selected_mp3).pack(side=tk.LEFT, padx=(20, 0))
        ttk.Button(actions, text="Stop Preview", command=lambda: stop_playback()).pack(side=tk.LEFT, padx=(8, 0))

        columns = ("sound", "symbol", "sf2", "midi", "mp3", "pcm")
        self.tree = ttk.Treeview(outer, columns=columns, show="headings", height=24)
        self.tree.heading("sound", text="Sound #")
        self.tree.heading("symbol", text="Symbol")
        self.tree.heading("sf2", text="Song SF2 Override")
        self.tree.heading("midi", text="MIDI")
        self.tree.heading("mp3", text="MP3")
        self.tree.heading("pcm", text="PCM Status")
        self.tree.column("sound", width=80, anchor=tk.CENTER)
        self.tree.column("symbol", width=180, anchor=tk.W)
        self.tree.column("sf2", width=280, anchor=tk.W)
        self.tree.column("midi", width=240, anchor=tk.W)
        self.tree.column("mp3", width=260, anchor=tk.W)
        self.tree.column("pcm", width=220, anchor=tk.W)
        self.tree.pack(fill=tk.BOTH, expand=True)

        status = ttk.Label(outer, textvariable=self.status_var, relief=tk.SUNKEN, anchor=tk.W)
        status.pack(fill=tk.X, pady=(8, 0))

    def _browse_game(self) -> None:
        path = filedialog.askdirectory(title="Select AGI game folder")
        if path:
            self.game_dir_var.set(path)

    def _browse_sf2(self) -> None:
        path = filedialog.askopenfilename(title="Select SoundFont", filetypes=[("SoundFonts", "*.sf2 *.sf3"), ("All files", "*.*")])
        if path:
            self.default_sf2_var.set(path)

    def _browse_fluidsynth(self) -> None:
        path = filedialog.askopenfilename(title="Select fluidsynth.exe", filetypes=[("Executable", "*.exe"), ("All files", "*.*")])
        if path:
            self.fluidsynth_var.set(path)

    def _browse_ffmpeg(self) -> None:
        path = filedialog.askopenfilename(title="Select ffmpeg.exe", filetypes=[("Executable", "*.exe"), ("All files", "*.*")])
        if path:
            self.ffmpeg_var.set(path)

    def _selected_index(self) -> int | None:
        selection = self.tree.selection()
        if not selection:
            return None
        return int(selection[0])

    def _current_project_dir(self) -> Path:
        game_id = self.game_id_var.get().strip().lower() or "agi_game"
        DEFAULT_PROJECT_ROOT.mkdir(parents=True, exist_ok=True)
        return DEFAULT_PROJECT_ROOT / game_id

    def _midi_path(self, row: CueRow) -> Path:
        return self._current_project_dir() / "midi" / f"{row.symbol}.mid"

    def _wav_path(self, row: CueRow) -> Path:
        return self._current_project_dir() / "wav" / f"{row.symbol}.wav"

    def _mp3_path(self, row: CueRow) -> Path:
        return self._current_project_dir() / "mp3" / f"{row.symbol}.mp3"

    def _pcm_path(self, row: CueRow) -> Path:
        return repo_root() / "audio" / f"{row.symbol}_15360_mono_s8.pcm"

    def _sf2_for_row(self, row: CueRow) -> Path:
        return Path(row.sf2_override.strip() or self.default_sf2_var.get().strip())

    def _refresh_tree(self) -> None:
        for item in self.tree.get_children():
            self.tree.delete(item)
        for index, row in enumerate(self.rows):
            midi_exists = self._midi_path(row).name if self._midi_path(row).exists() else ""
            mp3_exists = self._mp3_path(row).name if self._mp3_path(row).exists() else ""
            pcm_status = self._pcm_path(row).name if self._pcm_path(row).exists() else ""
            self.tree.insert(
                "",
                tk.END,
                iid=str(index),
                values=(
                    row.sound_num,
                    row.symbol,
                    Path(row.sf2_override).name if row.sf2_override else "(default)",
                    midi_exists,
                    mp3_exists,
                    pcm_status,
                ),
            )
        self.status_var.set(f"{len(self.rows)} cue(s)")

    def extract_cues(self) -> None:
        game_dir = Path(self.game_dir_var.get().strip())
        if not game_dir.exists():
            messagebox.showerror("Game Folder", "Select a valid AGI game folder first.")
            return
        try:
            sound_nums = list_present_sound_nums(game_dir)
        except Exception as exc:  # noqa: BLE001
            messagebox.showerror("Extract Error", str(exc))
            return
        game_id = self.game_id_var.get().strip().lower() or "agi_game"
        self.rows = [CueRow(sound_num=n, symbol=f"{game_id}_sound{n}_sf2") for n in sound_nums]
        self._refresh_tree()

    def set_song_sf2(self) -> None:
        index = self._selected_index()
        if index is None:
            messagebox.showinfo("No Selection", "Select a cue first.")
            return
        path = filedialog.askopenfilename(title="Select override SoundFont", filetypes=[("SoundFonts", "*.sf2 *.sf3"), ("All files", "*.*")])
        if path:
            self.rows[index].sf2_override = path
            self._refresh_tree()

    def clear_song_sf2(self) -> None:
        index = self._selected_index()
        if index is None:
            messagebox.showinfo("No Selection", "Select a cue first.")
            return
        self.rows[index].sf2_override = ""
        self._refresh_tree()

    def _render_rows(self, selected_only: bool) -> None:
        game_dir = Path(self.game_dir_var.get().strip())
        game_id = self.game_id_var.get().strip().upper()
        ffmpeg_path = Path(self.ffmpeg_var.get().strip())
        fluidsynth_path = Path(self.fluidsynth_var.get().strip())
        if not game_dir.exists():
            raise ValueError("Select a valid game folder.")
        if not fluidsynth_path.exists():
            raise ValueError("FluidSynth executable not found.")
        if not ffmpeg_path.exists():
            raise ValueError("ffmpeg executable not found.")
        if not self.default_sf2_var.get().strip():
            raise ValueError("Select a default SoundFont.")

        indices = [self._selected_index()] if selected_only else list(range(len(self.rows)))
        if indices == [None]:
            raise ValueError("Select a cue first.")

        for index in indices:
            if index is None:
                continue
            row = self.rows[index]
            sf2_path = self._sf2_for_row(row)
            if not sf2_path.exists():
                raise FileNotFoundError(f"SoundFont not found for sound {row.sound_num}: {sf2_path}")

            midi_path = self._midi_path(row)
            wav_path = self._wav_path(row)
            mp3_path = self._mp3_path(row)
            convert_agi_sound_to_midi(game_dir, row.sound_num, midi_path)
            render_midi_to_wav(fluidsynth_path, sf2_path, midi_path, wav_path)
            convert_wav_to_mp3(ffmpeg_path, wav_path, mp3_path)
            rip_audio_file(
                input_path=wav_path,
                symbol=row.symbol,
                output_path=self._pcm_path(row),
                preview_wav_path=None,
                ffmpeg_path=ffmpeg_path,
                game_id=game_id,
                sound_num=row.sound_num,
                register=self.register_var.get(),
                loop=row.loop,
                header_path=repo_root() / "pcm_music_assets.h",
            )
            self.root.after(0, self._refresh_tree)

    def _run_background(self, func) -> None:
        def worker() -> None:
            try:
                func()
                self.root.after(0, lambda: self.status_var.set("Done"))
            except Exception as exc:  # noqa: BLE001
                self.root.after(0, lambda: messagebox.showerror("AGI SoundFont Ripper", str(exc)))
                self.root.after(0, lambda: self.status_var.set("Failed"))
        threading.Thread(target=worker, daemon=True).start()

    def generate_midi_selected(self) -> None:
        index = self._selected_index()
        if index is None:
            messagebox.showinfo("No Selection", "Select a cue first.")
            return
        game_dir = Path(self.game_dir_var.get().strip())
        row = self.rows[index]
        def action() -> None:
            convert_agi_sound_to_midi(game_dir, row.sound_num, self._midi_path(row))
            self.root.after(0, self._refresh_tree)
        self._run_background(action)

    def render_selected(self) -> None:
        self._run_background(lambda: self._render_rows(True))

    def render_all(self) -> None:
        self._run_background(lambda: self._render_rows(False))

    def play_selected_mp3(self) -> None:
        index = self._selected_index()
        if index is None:
            messagebox.showinfo("No Selection", "Select a cue first.")
            return
        row = self.rows[index]
        mp3_path = self._mp3_path(row)
        ffmpeg_path = Path(self.ffmpeg_var.get().strip())
        if not mp3_path.exists():
            messagebox.showinfo("Not Rendered", "Render this cue first.")
            return
        def action() -> None:
            preview_source_audio(mp3_path, ffmpeg_path=ffmpeg_path, async_play=True)
            self.root.after(0, lambda: self.status_var.set(f"Previewing {mp3_path.name}"))
        self._run_background(action)


def main() -> int:
    root = tk.Tk()
    app = AGISoundfontRipperApp(root)
    root.minsize(1040, 680)
    root.mainloop()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
