#!/usr/bin/env python3
"""GUI for building GBAGI streamed music packs with per-game and per-song soundfont choices."""

from __future__ import annotations

import json
import threading
import tkinter as tk
from dataclasses import asdict, dataclass
from pathlib import Path
from tkinter import filedialog, messagebox, ttk

from rip_streamed_audio import RipResult, preview_source_audio, repo_root, rip_audio_file, stop_playback


PROJECTS_DIR = repo_root() / "extra" / "streamed_music_projects"
DEFAULT_AUDIO_DIR = repo_root() / "audio"
DEFAULT_HEADER_PATH = repo_root() / "pcm_music_assets.h"


@dataclass
class TrackConfig:
    sound_num: int
    symbol: str
    source_path: str = ""
    soundfont_override: str = ""
    loop: bool = False
    preview_wav: bool = False


class TrackDialog(tk.Toplevel):
    def __init__(self, parent: tk.Tk, track: TrackConfig | None = None):
        super().__init__(parent)
        self.title("Track")
        self.resizable(False, False)
        self.result: TrackConfig | None = None

        track = track or TrackConfig(sound_num=1, symbol="")
        self.sound_num_var = tk.StringVar(value=str(track.sound_num))
        self.symbol_var = tk.StringVar(value=track.symbol)
        self.source_var = tk.StringVar(value=track.source_path)
        self.soundfont_var = tk.StringVar(value=track.soundfont_override)
        self.loop_var = tk.BooleanVar(value=track.loop)
        self.preview_var = tk.BooleanVar(value=track.preview_wav)

        frame = ttk.Frame(self, padding=12)
        frame.grid(sticky="nsew")

        ttk.Label(frame, text="Sound #").grid(row=0, column=0, sticky="w")
        ttk.Entry(frame, textvariable=self.sound_num_var, width=10).grid(row=0, column=1, sticky="ew", padx=(8, 0))

        ttk.Label(frame, text="Symbol").grid(row=1, column=0, sticky="w", pady=(8, 0))
        ttk.Entry(frame, textvariable=self.symbol_var, width=36).grid(row=1, column=1, sticky="ew", padx=(8, 0), pady=(8, 0))

        ttk.Label(frame, text="Source audio").grid(row=2, column=0, sticky="w", pady=(8, 0))
        ttk.Entry(frame, textvariable=self.source_var, width=48).grid(row=2, column=1, sticky="ew", padx=(8, 0), pady=(8, 0))
        ttk.Button(frame, text="Browse...", command=self._browse_source).grid(row=2, column=2, padx=(8, 0), pady=(8, 0))

        ttk.Label(frame, text="Song soundfont").grid(row=3, column=0, sticky="w", pady=(8, 0))
        ttk.Entry(frame, textvariable=self.soundfont_var, width=48).grid(row=3, column=1, sticky="ew", padx=(8, 0), pady=(8, 0))
        ttk.Button(frame, text="Browse...", command=self._browse_soundfont).grid(row=3, column=2, padx=(8, 0), pady=(8, 0))

        ttk.Checkbutton(frame, text="Loop track", variable=self.loop_var).grid(row=4, column=0, columnspan=2, sticky="w", pady=(10, 0))
        ttk.Checkbutton(frame, text="Also write preview WAV", variable=self.preview_var).grid(row=5, column=0, columnspan=2, sticky="w", pady=(4, 0))

        button_row = ttk.Frame(frame)
        button_row.grid(row=6, column=0, columnspan=3, sticky="e", pady=(14, 0))
        ttk.Button(button_row, text="Cancel", command=self.destroy).pack(side=tk.RIGHT)
        ttk.Button(button_row, text="OK", command=self._confirm).pack(side=tk.RIGHT, padx=(0, 8))

        self.columnconfigure(0, weight=1)
        frame.columnconfigure(1, weight=1)
        self.grab_set()
        self.transient(parent)

    def _browse_source(self) -> None:
        path = filedialog.askopenfilename(
            title="Select source audio",
            filetypes=[
                ("Audio files", "*.wav *.mp3 *.flac *.ogg *.m4a"),
                ("All files", "*.*"),
            ],
        )
        if path:
            self.source_var.set(path)

    def _browse_soundfont(self) -> None:
        path = filedialog.askopenfilename(
            title="Select soundfont",
            filetypes=[("SoundFonts", "*.sf2 *.sf3"), ("All files", "*.*")],
        )
        if path:
            self.soundfont_var.set(path)

    def _confirm(self) -> None:
        try:
            sound_num = int(self.sound_num_var.get().strip())
        except ValueError:
            messagebox.showerror("Invalid Track", "Sound number must be an integer.", parent=self)
            return
        symbol = self.symbol_var.get().strip()
        if not symbol:
            messagebox.showerror("Invalid Track", "Symbol is required.", parent=self)
            return
        self.result = TrackConfig(
            sound_num=sound_num,
            symbol=symbol,
            source_path=self.source_var.get().strip(),
            soundfont_override=self.soundfont_var.get().strip(),
            loop=self.loop_var.get(),
            preview_wav=self.preview_var.get(),
        )
        self.destroy()


class StreamedMusicRipperApp:
    def __init__(self, root: tk.Tk):
        self.root = root
        self.root.title("GBAGI Streamed Music Ripper")
        self.root.geometry("1280x760")

        self.project_name_var = tk.StringVar(value="larry1_sound_pack")
        self.game_id_var = tk.StringVar(value="LSL1")
        self.default_soundfont_var = tk.StringVar()
        self.ffmpeg_var = tk.StringVar()
        self.status_var = tk.StringVar(value="Ready")
        self.register_var = tk.BooleanVar(value=True)
        self.preview_all_var = tk.BooleanVar(value=False)

        self.tracks: list[TrackConfig] = []

        self._build_ui()
        self._ensure_projects_dir()

    def _ensure_projects_dir(self) -> None:
        PROJECTS_DIR.mkdir(parents=True, exist_ok=True)

    def _build_ui(self) -> None:
        outer = ttk.Frame(self.root, padding=10)
        outer.pack(fill=tk.BOTH, expand=True)

        top = ttk.LabelFrame(outer, text="Project", padding=10)
        top.pack(fill=tk.X)

        ttk.Label(top, text="Project name").grid(row=0, column=0, sticky="w")
        ttk.Entry(top, textvariable=self.project_name_var, width=28).grid(row=0, column=1, sticky="ew", padx=(8, 16))
        ttk.Label(top, text="Game ID").grid(row=0, column=2, sticky="w")
        ttk.Entry(top, textvariable=self.game_id_var, width=12).grid(row=0, column=3, sticky="w", padx=(8, 16))
        ttk.Checkbutton(top, text="Register in pcm_music_assets.h", variable=self.register_var).grid(row=0, column=4, sticky="w")
        ttk.Checkbutton(top, text="Preview WAV for every track", variable=self.preview_all_var).grid(row=0, column=5, sticky="w", padx=(16, 0))

        ttk.Label(top, text="Default soundfont").grid(row=1, column=0, sticky="w", pady=(8, 0))
        ttk.Entry(top, textvariable=self.default_soundfont_var).grid(row=1, column=1, columnspan=4, sticky="ew", padx=(8, 8), pady=(8, 0))
        ttk.Button(top, text="Browse...", command=self._browse_default_soundfont).grid(row=1, column=5, sticky="e", pady=(8, 0))

        ttk.Label(top, text="ffmpeg").grid(row=2, column=0, sticky="w", pady=(8, 0))
        ttk.Entry(top, textvariable=self.ffmpeg_var).grid(row=2, column=1, columnspan=4, sticky="ew", padx=(8, 8), pady=(8, 0))
        ttk.Button(top, text="Browse...", command=self._browse_ffmpeg).grid(row=2, column=5, sticky="e", pady=(8, 0))

        top.columnconfigure(1, weight=1)
        top.columnconfigure(2, weight=0)
        top.columnconfigure(3, weight=0)
        top.columnconfigure(4, weight=1)

        notes = ttk.LabelFrame(outer, text="Notes", padding=10)
        notes.pack(fill=tk.X, pady=(10, 0))
        ttk.Label(
            notes,
            text=(
                "Soundfont choices are stored per game and per song so you can keep orchestral and special-case choices together. "
                "They are metadata for your rip project: use them to track which soundfont you rendered each source file with before conversion."
            ),
            wraplength=1200,
            justify=tk.LEFT,
        ).pack(fill=tk.X)

        buttons = ttk.Frame(outer)
        buttons.pack(fill=tk.X, pady=(10, 8))
        ttk.Button(buttons, text="Add Track", command=self.add_track).pack(side=tk.LEFT)
        ttk.Button(buttons, text="Edit Track", command=self.edit_track).pack(side=tk.LEFT, padx=(8, 0))
        ttk.Button(buttons, text="Remove Track", command=self.remove_track).pack(side=tk.LEFT, padx=(8, 0))
        ttk.Button(buttons, text="Play Source", command=self.play_selected_source).pack(side=tk.LEFT, padx=(24, 0))
        ttk.Button(buttons, text="Stop Preview", command=self.stop_preview).pack(side=tk.LEFT, padx=(8, 0))
        ttk.Button(buttons, text="Save Project", command=self.save_project).pack(side=tk.LEFT, padx=(24, 0))
        ttk.Button(buttons, text="Load Project", command=self.load_project).pack(side=tk.LEFT, padx=(8, 0))
        ttk.Button(buttons, text="Convert Selected", command=self.convert_selected).pack(side=tk.RIGHT)
        ttk.Button(buttons, text="Convert All", command=self.convert_all).pack(side=tk.RIGHT, padx=(0, 8))

        columns = ("sound_num", "symbol", "source", "soundfont", "loop", "status")
        self.tree = ttk.Treeview(outer, columns=columns, show="headings", height=22)
        self.tree.heading("sound_num", text="Sound #")
        self.tree.heading("symbol", text="Symbol")
        self.tree.heading("source", text="Source Audio")
        self.tree.heading("soundfont", text="Soundfont Override")
        self.tree.heading("loop", text="Loop")
        self.tree.heading("status", text="Status")
        self.tree.column("sound_num", width=80, anchor=tk.CENTER)
        self.tree.column("symbol", width=180, anchor=tk.W)
        self.tree.column("source", width=380, anchor=tk.W)
        self.tree.column("soundfont", width=320, anchor=tk.W)
        self.tree.column("loop", width=60, anchor=tk.CENTER)
        self.tree.column("status", width=220, anchor=tk.W)
        self.tree.pack(fill=tk.BOTH, expand=True)

        scrollbar = ttk.Scrollbar(outer, orient=tk.VERTICAL, command=self.tree.yview)
        self.tree.configure(yscrollcommand=scrollbar.set)
        scrollbar.place(relx=1.0, rely=0.32, relheight=0.58, anchor="ne")

        status = ttk.Label(outer, textvariable=self.status_var, relief=tk.SUNKEN, anchor=tk.W)
        status.pack(fill=tk.X, pady=(8, 0))

    def _browse_default_soundfont(self) -> None:
        path = filedialog.askopenfilename(
            title="Select default soundfont",
            filetypes=[("SoundFonts", "*.sf2 *.sf3"), ("All files", "*.*")],
        )
        if path:
            self.default_soundfont_var.set(path)

    def _browse_ffmpeg(self) -> None:
        path = filedialog.askopenfilename(
            title="Select ffmpeg executable",
            filetypes=[("Executable", "*.exe"), ("All files", "*.*")],
        )
        if path:
            self.ffmpeg_var.set(path)

    def _selected_index(self) -> int | None:
        selection = self.tree.selection()
        if not selection:
            return None
        return int(selection[0])

    def _refresh_tree(self, statuses: dict[int, str] | None = None) -> None:
        statuses = statuses or {}
        for item in self.tree.get_children():
            self.tree.delete(item)
        for index, track in enumerate(self.tracks):
            override = track.soundfont_override or "(use default)"
            source = track.source_path or "(missing)"
            status_text = statuses.get(index, "")
            self.tree.insert(
                "",
                tk.END,
                iid=str(index),
                values=(
                    track.sound_num,
                    track.symbol,
                    source,
                    override,
                    "yes" if track.loop else "no",
                    status_text,
                ),
            )
        self.status_var.set(f"{len(self.tracks)} track(s) in project")

    def add_track(self) -> None:
        dialog = TrackDialog(self.root)
        self.root.wait_window(dialog)
        if dialog.result is not None:
            self.tracks.append(dialog.result)
            self._refresh_tree()

    def edit_track(self) -> None:
        index = self._selected_index()
        if index is None:
            messagebox.showinfo("No Selection", "Select a track first.")
            return
        dialog = TrackDialog(self.root, self.tracks[index])
        self.root.wait_window(dialog)
        if dialog.result is not None:
            self.tracks[index] = dialog.result
            self._refresh_tree()

    def remove_track(self) -> None:
        index = self._selected_index()
        if index is None:
            messagebox.showinfo("No Selection", "Select a track first.")
            return
        del self.tracks[index]
        self._refresh_tree()

    def play_selected_source(self) -> None:
        index = self._selected_index()
        if index is None:
            messagebox.showinfo("No Selection", "Select a track first.")
            return
        track = self.tracks[index]
        if not track.source_path:
            messagebox.showinfo("Missing Source", "This track does not have a source audio file yet.")
            return

        ffmpeg_path = Path(self.ffmpeg_var.get().strip()) if self.ffmpeg_var.get().strip() else None

        def worker() -> None:
            try:
                preview_source_audio(Path(track.source_path), ffmpeg_path=ffmpeg_path, async_play=True)
                self.root.after(0, lambda: self.status_var.set(f"Previewing source for sound {track.sound_num}"))
            except Exception as exc:  # noqa: BLE001
                self.root.after(0, lambda: messagebox.showerror("Preview Error", str(exc)))

        threading.Thread(target=worker, daemon=True).start()

    def stop_preview(self) -> None:
        try:
            stop_playback()
            self.status_var.set("Preview stopped")
        except Exception as exc:  # noqa: BLE001
            messagebox.showerror("Preview Error", str(exc))

    def _project_payload(self) -> dict[str, object]:
        return {
            "project_name": self.project_name_var.get().strip(),
            "game_id": self.game_id_var.get().strip(),
            "default_soundfont": self.default_soundfont_var.get().strip(),
            "ffmpeg": self.ffmpeg_var.get().strip(),
            "register": self.register_var.get(),
            "preview_all": self.preview_all_var.get(),
            "tracks": [asdict(track) for track in self.tracks],
        }

    def save_project(self) -> None:
        project_name = self.project_name_var.get().strip() or "streamed_music_project"
        default_path = PROJECTS_DIR / f"{project_name}.json"
        path = filedialog.asksaveasfilename(
            title="Save project",
            initialdir=str(PROJECTS_DIR),
            initialfile=default_path.name,
            defaultextension=".json",
            filetypes=[("JSON", "*.json")],
        )
        if not path:
            return
        Path(path).write_text(json.dumps(self._project_payload(), indent=2), encoding="utf-8")
        self.status_var.set(f"Saved project: {path}")

    def load_project(self) -> None:
        path = filedialog.askopenfilename(
            title="Load project",
            initialdir=str(PROJECTS_DIR),
            filetypes=[("JSON", "*.json")],
        )
        if not path:
            return
        payload = json.loads(Path(path).read_text(encoding="utf-8"))
        self.project_name_var.set(payload.get("project_name", ""))
        self.game_id_var.set(payload.get("game_id", "LSL1"))
        self.default_soundfont_var.set(payload.get("default_soundfont", ""))
        self.ffmpeg_var.set(payload.get("ffmpeg", ""))
        self.register_var.set(bool(payload.get("register", True)))
        self.preview_all_var.set(bool(payload.get("preview_all", False)))
        self.tracks = [TrackConfig(**row) for row in payload.get("tracks", [])]
        self._refresh_tree()

    def _tracks_for_conversion(self, selected_only: bool) -> list[tuple[int, TrackConfig]]:
        if selected_only:
            index = self._selected_index()
            if index is None:
                raise ValueError("Select a track first.")
            return [(index, self.tracks[index])]
        return list(enumerate(self.tracks))

    def _convert_worker(self, selected_only: bool) -> None:
        try:
            todo = self._tracks_for_conversion(selected_only)
        except Exception as exc:  # noqa: BLE001
            self.root.after(0, lambda: messagebox.showerror("Convert Error", str(exc)))
            return

        game_id = self.game_id_var.get().strip().upper()
        default_soundfont = self.default_soundfont_var.get().strip()
        ffmpeg_path = Path(self.ffmpeg_var.get().strip()) if self.ffmpeg_var.get().strip() else None
        statuses: dict[int, str] = {}

        for index, track in todo:
            try:
                if not track.source_path:
                    raise ValueError("No source audio file set")

                output_path = DEFAULT_AUDIO_DIR / f"{track.symbol}_15360_mono_s8.pcm"
                preview_path = None
                if self.preview_all_var.get() or track.preview_wav:
                    preview_path = DEFAULT_AUDIO_DIR / f"{track.symbol}_15360_preview.wav"

                effective_soundfont = track.soundfont_override.strip() or default_soundfont or "(none)"
                result: RipResult = rip_audio_file(
                    input_path=Path(track.source_path),
                    symbol=track.symbol,
                    output_path=output_path,
                    preview_wav_path=preview_path,
                    ffmpeg_path=ffmpeg_path,
                    game_id=game_id,
                    sound_num=track.sound_num,
                    register=self.register_var.get(),
                    loop=track.loop,
                    header_path=DEFAULT_HEADER_PATH,
                )
                statuses[index] = f"OK {result.data_length} bytes | SF2: {Path(effective_soundfont).name if effective_soundfont != '(none)' else '(none)'}"
            except Exception as exc:  # noqa: BLE001
                statuses[index] = f"ERROR: {exc}"

            self.root.after(0, lambda s=statuses.copy(): self._refresh_tree(s))

        self.root.after(0, lambda: self.status_var.set("Conversion finished"))

    def convert_selected(self) -> None:
        threading.Thread(target=self._convert_worker, args=(True,), daemon=True).start()

    def convert_all(self) -> None:
        if not self.tracks:
            messagebox.showinfo("No Tracks", "Add at least one track first.")
            return
        threading.Thread(target=self._convert_worker, args=(False,), daemon=True).start()


def main() -> int:
    root = tk.Tk()
    app = StreamedMusicRipperApp(root)
    root.minsize(980, 640)
    root.mainloop()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
