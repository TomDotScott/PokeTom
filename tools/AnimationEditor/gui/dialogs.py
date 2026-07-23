"""
gui/dialogs.py
--------------
Modal dialogs for the animation tool.
"""

from __future__ import annotations
import tkinter as tk
from tkinter import ttk
from typing import Optional

from config import ANCHOR_OPTIONS, Anchor, DEFAULT_DURATION_MS
from models import Animation, Frame


class NewAnimationDialog(tk.Toplevel):
    """
    Dialog for creating a new animation.
    Returns None if cancelled, or an Animation with no frames if confirmed.
    """

    def __init__(self, parent: tk.Widget, existing_names: list[str]) -> None:
        super().__init__(parent)
        self.title("New Animation")
        self.resizable(False, False)
        self.grab_set()

        self._existing_names = existing_names
        self.result: Optional[Animation] = None

        self._build()
        self.wait_window()

    def _build(self) -> None:
        pad = {"padx": 8, "pady": 4}

        ttk.Label(self, text="Name:").grid(row=0, column=0, sticky="e", **pad)
        self._name_var = tk.StringVar()
        ttk.Entry(self, textvariable=self._name_var, width=24).grid(
            row=0, column=1, **pad)

        ttk.Label(self, text="Looping:").grid(row=1, column=0, sticky="e", **pad)
        self._looping_var = tk.BooleanVar(value=True)
        ttk.Checkbutton(self, variable=self._looping_var,
                        command=self._on_looping_change).grid(
            row=1, column=1, sticky="w", **pad)

        ttk.Label(self, text="On End:").grid(row=2, column=0, sticky="e", **pad)
        self._on_end_var = tk.StringVar()
        self._on_end_entry = ttk.Entry(
            self, textvariable=self._on_end_var, width=24, state="disabled")
        self._on_end_entry.grid(row=2, column=1, **pad)

        ttk.Label(self, text="Anchor:").grid(row=3, column=0, sticky="e", **pad)
        self._anchor_var = tk.StringVar(value="BM")
        anchor_names = [name for name, _ in ANCHOR_OPTIONS]
        ttk.Combobox(self, textvariable=self._anchor_var,
                     values=anchor_names, width=6,
                     state="readonly").grid(row=3, column=1, sticky="w", **pad)

        btn_frame = ttk.Frame(self)
        btn_frame.grid(row=4, column=0, columnspan=2, pady=8)
        ttk.Button(btn_frame, text="OK", command=self._ok).pack(
            side="left", padx=4)
        ttk.Button(btn_frame, text="Cancel", command=self.destroy).pack(
            side="left", padx=4)

    def _on_looping_change(self) -> None:
        state = "normal" if not self._looping_var.get() else "disabled"
        self._on_end_entry.configure(state=state)

    def _ok(self) -> None:
        name = self._name_var.get().strip()
        if not name:
            tk.messagebox.showerror("Error", "Name cannot be empty.", parent=self)
            return
        if name in self._existing_names:
            tk.messagebox.showerror(
                "Error", f"'{name}' already exists.", parent=self)
            return

        anchor_name = self._anchor_var.get()
        anchor = next(a for n, a in ANCHOR_OPTIONS if n == anchor_name)
        on_end = self._on_end_var.get().strip() or None
        looping = self._looping_var.get()

        self.result = Animation(
            name=name,
            looping=looping,
            anchor=anchor,
            on_end=on_end if not looping else None,
        )
        self.destroy()


class EditAnimationDialog(tk.Toplevel):
    """
    Dialog for editing an existing animation's properties (not its frames).
    """

    def __init__(
        self,
        parent: tk.Widget,
        animation: Animation,
        existing_names: list[str],
    ) -> None:
        super().__init__(parent)
        self.title("Edit Animation")
        self.resizable(False, False)
        self.grab_set()

        self._anim = animation
        self._existing_names = [n for n in existing_names if n != animation.name]
        self.result: Optional[dict] = None  # dict of updated fields

        self._build()
        self.wait_window()

    def _build(self) -> None:
        pad = {"padx": 8, "pady": 4}

        ttk.Label(self, text="Name:").grid(row=0, column=0, sticky="e", **pad)
        self._name_var = tk.StringVar(value=self._anim.name)
        ttk.Entry(self, textvariable=self._name_var, width=24).grid(
            row=0, column=1, **pad)

        ttk.Label(self, text="Looping:").grid(row=1, column=0, sticky="e", **pad)
        self._looping_var = tk.BooleanVar(value=self._anim.looping)
        ttk.Checkbutton(self, variable=self._looping_var,
                        command=self._on_looping_change).grid(
            row=1, column=1, sticky="w", **pad)

        ttk.Label(self, text="On End:").grid(row=2, column=0, sticky="e", **pad)
        self._on_end_var = tk.StringVar(value=self._anim.on_end or "")
        initial_state = "normal" if not self._anim.looping else "disabled"
        self._on_end_entry = ttk.Entry(
            self, textvariable=self._on_end_var, width=24, state=initial_state)
        self._on_end_entry.grid(row=2, column=1, **pad)

        ttk.Label(self, text="Anchor:").grid(row=3, column=0, sticky="e", **pad)
        current_anchor_name = next(
            (n for n, a in ANCHOR_OPTIONS if a == self._anim.anchor), "BM")
        self._anchor_var = tk.StringVar(value=current_anchor_name)
        anchor_names = [name for name, _ in ANCHOR_OPTIONS]
        ttk.Combobox(self, textvariable=self._anchor_var,
                     values=anchor_names, width=6,
                     state="readonly").grid(row=3, column=1, sticky="w", **pad)

        btn_frame = ttk.Frame(self)
        btn_frame.grid(row=4, column=0, columnspan=2, pady=8)
        ttk.Button(btn_frame, text="OK", command=self._ok).pack(
            side="left", padx=4)
        ttk.Button(btn_frame, text="Cancel", command=self.destroy).pack(
            side="left", padx=4)

    def _on_looping_change(self) -> None:
        state = "normal" if not self._looping_var.get() else "disabled"
        self._on_end_entry.configure(state=state)

    def _ok(self) -> None:
        name = self._name_var.get().strip()
        if not name:
            tk.messagebox.showerror("Error", "Name cannot be empty.", parent=self)
            return
        if name in self._existing_names:
            tk.messagebox.showerror(
                "Error", f"'{name}' already exists.", parent=self)
            return

        anchor_name = self._anchor_var.get()
        anchor = next(a for n, a in ANCHOR_OPTIONS if n == anchor_name)
        looping = self._looping_var.get()
        on_end = self._on_end_var.get().strip() or None

        self.result = {
            "name": name,
            "looping": looping,
            "anchor": anchor,
            "on_end": on_end if not looping else None,
        }
        self.destroy()


class EditFrameDialog(tk.Toplevel):
    """
    Dialog for editing a single frame's duration and extra attributes.
    Extra attributes are shown as an editable key=value list.
    """

    def __init__(self, parent: tk.Widget, frame: Frame) -> None:
        super().__init__(parent)
        self.title("Edit Frame")
        self.resizable(False, False)
        self.grab_set()

        self._frame = frame
        self.result: Optional[dict] = None

        self._build()
        self.wait_window()

    def _build(self) -> None:
        pad = {"padx": 8, "pady": 4}

        ttk.Label(self, text="Duration (ms):").grid(
            row=0, column=0, sticky="e", **pad)
        self._dur_var = tk.IntVar(value=self._frame.duration)
        ttk.Spinbox(self, from_=1, to=99999, textvariable=self._dur_var,
                    width=8).grid(row=0, column=1, sticky="w", **pad)

        ttk.Label(self, text="flippedHorizontal:").grid(
            row=1, column=0, sticky="e", **pad)
        self._flip_h = tk.BooleanVar(
            value=self._frame.extra_attrs.get("flippedHorizontal", "false") == "true")
        ttk.Checkbutton(self, variable=self._flip_h).grid(
            row=1, column=1, sticky="w", **pad)

        ttk.Label(self, text="flippedVertical:").grid(
            row=2, column=0, sticky="e", **pad)
        self._flip_v = tk.BooleanVar(
            value=self._frame.extra_attrs.get("flippedVertical", "false") == "true")
        ttk.Checkbutton(self, variable=self._flip_v).grid(
            row=2, column=1, sticky="w", **pad)

        # Generic extra attributes editor
        ttk.Separator(self, orient="horizontal").grid(
            row=3, column=0, columnspan=2, sticky="ew", pady=4)
        ttk.Label(self, text="Extra attributes (key=value):").grid(
            row=4, column=0, columnspan=2, sticky="w", padx=8)

        self._extra_text = tk.Text(self, width=30, height=5)
        self._extra_text.grid(row=5, column=0, columnspan=2, padx=8, pady=4)

        # Populate with existing extras (excluding the known ones above)
        known = {"flippedHorizontal", "flippedVertical"}
        for k, v in self._frame.extra_attrs.items():
            if k not in known:
                self._extra_text.insert("end", f"{k}={v}\n")

        btn_frame = ttk.Frame(self)
        btn_frame.grid(row=6, column=0, columnspan=2, pady=8)
        ttk.Button(btn_frame, text="OK", command=self._ok).pack(
            side="left", padx=4)
        ttk.Button(btn_frame, text="Cancel", command=self.destroy).pack(
            side="left", padx=4)

    def _ok(self) -> None:
        extra: dict[str, str] = {}
        if self._flip_h.get():
            extra["flippedHorizontal"] = "true"
        if self._flip_v.get():
            extra["flippedVertical"] = "true"

        raw = self._extra_text.get("1.0", "end").strip()
        if raw:
            for line in raw.splitlines():
                line = line.strip()
                if "=" in line:
                    k, _, v = line.partition("=")
                    extra[k.strip()] = v.strip()

        self.result = {
            "duration": self._dur_var.get(),
            "extra_attrs": extra,
        }
        self.destroy()


class CellSizeDialog(tk.Toplevel):
    """
    Dialog for overriding the width/height of a single detected cell.
    """

    def __init__(
        self,
        parent: tk.Widget,
        current_w: int,
        current_h: int,
    ) -> None:
        super().__init__(parent)
        self.title("Override Cell Size")
        self.resizable(False, False)
        self.grab_set()

        self.result: Optional[tuple[int, int]] = None
        self._build(current_w, current_h)
        self.wait_window()

    def _build(self, cw: int, ch: int) -> None:
        pad = {"padx": 8, "pady": 4}
        ttk.Label(self, text="Width:").grid(row=0, column=0, sticky="e", **pad)
        self._w_var = tk.IntVar(value=cw)
        ttk.Spinbox(self, from_=1, to=4096, textvariable=self._w_var,
                    width=6).grid(row=0, column=1, sticky="w", **pad)

        ttk.Label(self, text="Height:").grid(row=1, column=0, sticky="e", **pad)
        self._h_var = tk.IntVar(value=ch)
        ttk.Spinbox(self, from_=1, to=4096, textvariable=self._h_var,
                    width=6).grid(row=1, column=1, sticky="w", **pad)

        btn_frame = ttk.Frame(self)
        btn_frame.grid(row=2, column=0, columnspan=2, pady=8)
        ttk.Button(btn_frame, text="OK", command=self._ok).pack(
            side="left", padx=4)
        ttk.Button(btn_frame, text="Cancel", command=self.destroy).pack(
            side="left", padx=4)

    def _ok(self) -> None:
        self.result = (self._w_var.get(), self._h_var.get())
        self.destroy()
