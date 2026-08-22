"""
font_tool/gui/dialogs.py
------------------------
Dialogs specific to the bitmap font editor.
"""

from __future__ import annotations

import tkinter as tk
from tkinter import ttk
from typing import Optional

import sys, os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "..", "shared"))
from dialogs import BaseDialog  # type: ignore[import]

try:
    from PIL import Image, ImageTk  # type: ignore[import]
    _HAS_PIL = True
except ImportError:
    _HAS_PIL = False


# ── Glyph character dialog ────────────────────────────────────────────────────

class GlyphCharDialog(BaseDialog):
    """
    Shown after the user draws a selection rectangle on the sheet.

    Displays a preview of the cropped region and asks:
      • Which character does this glyph represent?
      • What is its advance width? (defaults to region width)

    ``result`` is a dict::

        {"char": str, "advance": int}

    or ``None`` if the user skipped / cancelled.
    """

    def __init__(
        self,
        parent: tk.Misc,
        crop: "Image.Image",
        region_width: int,
        existing_chars: set[str],
    ) -> None:
        self._crop = crop
        self._region_width = region_width
        self._existing_chars = existing_chars
        self._char_var = tk.StringVar()
        self._advance_var = tk.StringVar(value=str(region_width))
        self._error_var = tk.StringVar()
        self._tk_preview: Optional["ImageTk.PhotoImage"] = None
        super().__init__(parent, "Define Glyph")

    def _build_body(self, frame: ttk.Frame) -> None:
        # ── Preview ──
        preview_frame = ttk.LabelFrame(frame, text="Preview", padding=6)
        preview_frame.pack(fill=tk.X, pady=(0, 8))

        if _HAS_PIL and self._crop is not None:
            # Scale up small crops so they're visible (cap at 128 px tall).
            w, h = self._crop.size
            scale = max(1, min(128 // max(h, 1), 8))
            preview_img = self._crop.resize((w * scale, h * scale), Image.NEAREST)
            self._tk_preview = ImageTk.PhotoImage(preview_img)
            lbl = tk.Label(preview_frame, image=self._tk_preview, bg="#2b2b2b")
        else:
            lbl = ttk.Label(preview_frame, text="(PIL not available)")
        lbl.pack()

        size_text = f"{self._crop.width}×{self._crop.height} px" if self._crop else ""
        ttk.Label(preview_frame, text=size_text, foreground="gray").pack()

        # ── Character ──
        char_frame = ttk.LabelFrame(frame, text="Character", padding=6)
        char_frame.pack(fill=tk.X, pady=(0, 8))

        ttk.Label(
            char_frame,
            text="Type the character this glyph represents:",
        ).pack(anchor=tk.W)

        char_entry = ttk.Entry(char_frame, textvariable=self._char_var, width=6,
                               font=("TkDefaultFont", 16))
        char_entry.pack(anchor=tk.W, pady=(4, 0))
        char_entry.focus_set()

        # Limit entry to one character.
        def _on_char_write(*_args: object) -> None:
            v = self._char_var.get()
            if len(v) > 1:
                self._char_var.set(v[-1])

        self._char_var.trace_add("write", _on_char_write)

        # ── Advance ──
        adv_frame = ttk.LabelFrame(frame, text="Advance Width", padding=6)
        adv_frame.pack(fill=tk.X, pady=(0, 8))

        ttk.Label(
            adv_frame,
            text="Pixels to advance the cursor after this glyph\n"
                 "(leave blank to use glyph width):",
        ).pack(anchor=tk.W)
        ttk.Entry(adv_frame, textvariable=self._advance_var, width=8).pack(
            anchor=tk.W, pady=(4, 0)
        )

        # ── Error ──
        ttk.Label(frame, textvariable=self._error_var, foreground="red").pack(anchor=tk.W)

    def _build_buttons(self, frame: ttk.Frame) -> None:
        ttk.Button(frame, text="Save", command=self._ok, width=10).pack(
            side=tk.RIGHT, padx=(4, 0)
        )
        ttk.Button(frame, text="Skip", command=self._cancel, width=10).pack(side=tk.RIGHT)
        self.bind("<Return>", lambda _e: self._ok())

    def _ok(self) -> None:
        char = self._char_var.get()
        if not char:
            self._error_var.set("Please type a character.")
            return

        try:
            advance = int(self._advance_var.get())
        except ValueError:
            advance = self._region_width

        if char in self._existing_chars:
            # Warn but allow overwrite.
            self._error_var.set(
                f"'{char}' already defined — saving will overwrite it.\n"
                "Press Save again to confirm."
            )
            # On second press the error is still showing; we just accept.
            if not hasattr(self, "_warned"):
                self._warned = True
                return

        self.result = {"char": char, "advance": advance}
        self.destroy()


# ── Font metadata dialog ──────────────────────────────────────────────────────

class FontMetaDialog(BaseDialog):
    """
    Edit top-level font metadata: referenceHeight and lineHeight.

    ``result`` is ``{"reference_height": int, "line_height": int}`` or ``None``.
    """

    def __init__(
        self,
        parent: tk.Misc,
        reference_height: int,
        line_height: int,
    ) -> None:
        self._ref_var = tk.StringVar(value=str(reference_height))
        self._line_var = tk.StringVar(value=str(line_height))
        super().__init__(parent, "Font Metadata")

    def _build_body(self, frame: ttk.Frame) -> None:
        fields = [
            ("Reference height (px):", self._ref_var,
             "Height of a capital letter at 1× scale.\n"
             "Used by the engine to compute integer scale factors."),
            ("Line height (px):", self._line_var,
             "Vertical distance between baselines."),
        ]

        for label, var, tip in fields:
            ttk.Label(frame, text=label).pack(anchor=tk.W, pady=(6, 0))
            ttk.Entry(frame, textvariable=var, width=8).pack(anchor=tk.W)
            ttk.Label(frame, text=tip, foreground="gray", wraplength=280).pack(
                anchor=tk.W, pady=(2, 0)
            )

    def _ok(self) -> None:
        try:
            self.result = {
                "reference_height": max(1, int(self._ref_var.get())),
                "line_height": max(1, int(self._line_var.get())),
            }
        except ValueError:
            self.result = None
        self.destroy()


# ── Font area dialog ──────────────────────────────────────────────────────────

class FontAreaDialog(BaseDialog):
    """
    Manually enter or confirm the fontArea bounding box.

    ``result`` is ``{"x": int, "y": int, "width": int, "height": int}`` or ``None``.
    Pre-filled from the last rubber-band selection when provided.
    """

    def __init__(
        self,
        parent: tk.Misc,
        x: int = 0,
        y: int = 0,
        width: int = 0,
        height: int = 0,
    ) -> None:
        self._x_var = tk.StringVar(value=str(x))
        self._y_var = tk.StringVar(value=str(y))
        self._w_var = tk.StringVar(value=str(width))
        self._h_var = tk.StringVar(value=str(height))
        super().__init__(parent, "Set Font Area")

    def _build_body(self, frame: ttk.Frame) -> None:
        ttk.Label(
            frame,
            text="The fontArea is the sub-region of the atlas used by this font.\n"
                 "Drag a rectangle on the sheet first, or enter values manually:",
            wraplength=300,
        ).grid(row=0, column=0, columnspan=2, sticky=tk.W, pady=(0, 8))

        for i, (label, var) in enumerate([
            ("X:", self._x_var),
            ("Y:", self._y_var),
            ("Width:", self._w_var),
            ("Height:", self._h_var),
        ]):
            ttk.Label(frame, text=label).grid(row=i + 1, column=0, sticky=tk.W, pady=2)
            ttk.Entry(frame, textvariable=var, width=8).grid(
                row=i + 1, column=1, sticky=tk.W, padx=(8, 0), pady=2,
            )

    def _ok(self) -> None:
        try:
            self.result = {
                "x": int(self._x_var.get()),
                "y": int(self._y_var.get()),
                "width": max(0, int(self._w_var.get())),
                "height": max(0, int(self._h_var.get())),
            }
        except ValueError:
            self.result = None
        self.destroy()
