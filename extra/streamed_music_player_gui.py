#!/usr/bin/env python3
"""Simple GUI for browsing and testing GBAGI streamed PCM music tracks."""

from __future__ import annotations

import threading
import tkinter as tk
from pathlib import Path
from tkinter import filedialog, messagebox, ttk

from streamed_music_player import load_pcm_bytes, list_mapped_tracks, play_pcm_bytes, write_wav_from_pcm


class StreamedMusicPlayerApp:
    def __init__(self, root: tk.Tk):
        self.root = root
        self.root.title("GBAGI Streamed Music Player")
        self.root.geometry("920x560")

        self.status_var = tk.StringVar(value="Ready")
        self.filter_var = tk.StringVar()
        self.current_pcm_path: Path | None = None

        self.rows = list_mapped_tracks()
        self.filtered_rows = list(self.rows)

        self._build_ui()
        self._refresh_tree()

    def _build_ui(self) -> None:
        frame = ttk.Frame(self.root, padding=10)
        frame.pack(fill=tk.BOTH, expand=True)

        controls = ttk.Frame(frame)
        controls.pack(fill=tk.X, pady=(0, 10))

        ttk.Label(controls, text="Filter").pack(side=tk.LEFT)
        filter_entry = ttk.Entry(controls, textvariable=self.filter_var, width=32)
        filter_entry.pack(side=tk.LEFT, padx=(8, 12))
        filter_entry.bind("<KeyRelease>", lambda _event: self._apply_filter())

        ttk.Button(controls, text="Play Selected", command=self.play_selected).pack(side=tk.LEFT)
        ttk.Button(controls, text="Export WAV", command=self.export_selected_wav).pack(side=tk.LEFT, padx=8)
        ttk.Button(controls, text="Open PCM File", command=self.open_pcm_file).pack(side=tk.LEFT)
        ttk.Button(controls, text="Refresh", command=self.refresh_tracks).pack(side=tk.LEFT, padx=(8, 0))

        columns = ("game_id", "sound_num", "symbol", "size_kb", "pcm_path")
        self.tree = ttk.Treeview(frame, columns=columns, show="headings", height=20)
        self.tree.heading("game_id", text="Game")
        self.tree.heading("sound_num", text="Sound")
        self.tree.heading("symbol", text="Symbol")
        self.tree.heading("size_kb", text="Size (KB)")
        self.tree.heading("pcm_path", text="PCM Path")
        self.tree.column("game_id", width=80, anchor=tk.W)
        self.tree.column("sound_num", width=70, anchor=tk.CENTER)
        self.tree.column("symbol", width=180, anchor=tk.W)
        self.tree.column("size_kb", width=90, anchor=tk.E)
        self.tree.column("pcm_path", width=460, anchor=tk.W)
        self.tree.pack(fill=tk.BOTH, expand=True)
        self.tree.bind("<Double-1>", lambda _event: self.play_selected())
        self.tree.bind("<<TreeviewSelect>>", lambda _event: self._update_selection_status())

        scrollbar = ttk.Scrollbar(frame, orient=tk.VERTICAL, command=self.tree.yview)
        self.tree.configure(yscrollcommand=scrollbar.set)
        scrollbar.place(relx=1.0, rely=0.09, relheight=0.84, anchor="ne")

        status = ttk.Label(frame, textvariable=self.status_var, relief=tk.SUNKEN, anchor=tk.W)
        status.pack(fill=tk.X, pady=(10, 0))

    def _apply_filter(self) -> None:
        term = self.filter_var.get().strip().lower()
        if not term:
            self.filtered_rows = list(self.rows)
        else:
            self.filtered_rows = [
                row
                for row in self.rows
                if term in str(row["game_id"]).lower()
                or term in str(row["sound_num"]).lower()
                or term in str(row["symbol"]).lower()
                or term in str(row["pcm_path"]).lower()
            ]
        self._refresh_tree()

    def _refresh_tree(self) -> None:
        for item in self.tree.get_children():
            self.tree.delete(item)
        for row in self.filtered_rows:
            size_kb = row["expected_length"] / 1024.0
            self.tree.insert(
                "",
                tk.END,
                values=(
                    row["game_id"],
                    row["sound_num"],
                    row["symbol"],
                    f"{size_kb:.1f}",
                    str(row["pcm_path"]),
                ),
            )
        self.status_var.set(f"Loaded {len(self.filtered_rows)} mapped tracks")

    def _selected_values(self) -> tuple[str, int, Path] | None:
        selection = self.tree.selection()
        if not selection:
            return None
        values = self.tree.item(selection[0], "values")
        return values[0], int(values[1]), Path(values[4])

    def _update_selection_status(self) -> None:
        selected = self._selected_values()
        if not selected:
            self.status_var.set("Ready")
            return
        game_id, sound_num, pcm_path = selected
        self.status_var.set(f"Selected {game_id} sound {sound_num}: {pcm_path.name}")

    def _play_worker(self, game_id: str | None, sound_num: int | None, pcm_path: Path | None) -> None:
        try:
            resolved_path, pcm_bytes = load_pcm_bytes(game_name=game_id, sound_num=sound_num, pcm_path=pcm_path)
            self.current_pcm_path = resolved_path
            self.root.after(0, lambda: self.status_var.set(f"Playing {resolved_path.name}"))
            play_pcm_bytes(pcm_bytes)
            self.root.after(0, lambda: self.status_var.set(f"Finished {resolved_path.name}"))
        except Exception as exc:  # noqa: BLE001
            self.root.after(0, lambda: messagebox.showerror("Playback Error", str(exc)))
            self.root.after(0, lambda: self.status_var.set("Playback failed"))

    def play_selected(self) -> None:
        selected = self._selected_values()
        if not selected:
            messagebox.showinfo("No Selection", "Select a mapped track first.")
            return
        game_id, sound_num, pcm_path = selected
        threading.Thread(
            target=self._play_worker,
            args=(game_id, sound_num, pcm_path),
            daemon=True,
        ).start()

    def export_selected_wav(self) -> None:
        selected = self._selected_values()
        if not selected:
            messagebox.showinfo("No Selection", "Select a mapped track first.")
            return

        game_id, sound_num, pcm_path = selected
        default_name = f"{game_id.lower()}_sound{sound_num}.wav"
        wav_path = filedialog.asksaveasfilename(
            title="Export WAV",
            defaultextension=".wav",
            initialfile=default_name,
            filetypes=[("WAV Files", "*.wav")],
        )
        if not wav_path:
            return

        try:
            resolved_path, pcm_bytes = load_pcm_bytes(game_name=game_id, sound_num=sound_num, pcm_path=pcm_path)
            write_wav_from_pcm(pcm_bytes, Path(wav_path))
            self.status_var.set(f"Exported WAV from {resolved_path.name}")
        except Exception as exc:  # noqa: BLE001
            messagebox.showerror("Export Error", str(exc))

    def open_pcm_file(self) -> None:
        pcm_path_text = filedialog.askopenfilename(
            title="Open PCM File",
            filetypes=[("PCM Files", "*.pcm"), ("All Files", "*.*")],
        )
        if not pcm_path_text:
            return

        pcm_path = Path(pcm_path_text)
        threading.Thread(
            target=self._play_worker,
            args=(None, None, pcm_path),
            daemon=True,
        ).start()

    def refresh_tracks(self) -> None:
        self.rows = list_mapped_tracks()
        self._apply_filter()


def main() -> int:
    root = tk.Tk()
    app = StreamedMusicPlayerApp(root)
    root.minsize(760, 420)
    root.mainloop()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
