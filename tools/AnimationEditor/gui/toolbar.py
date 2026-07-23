"""
gui/toolbar.py
--------------
Top toolbar: tool selector, zoom, detection settings, mask colour picker,
dict name, and Save button.
"""

from __future__ import annotations
import tkinter as tk
from tkinter import ttk
from typing import Callable

from config import Tool


class Toolbar(ttk.Frame):
    """
    Emits callbacks when the user changes any setting.

    Callbacks
    ---------
    on_tool_change(tool: str)
    on_zoom_change(zoom: float)
    on_fixed_width_change(enabled: bool)
    on_cell_size_change(w: int, h: int)
    on_margin_change(margin: int)
    on_redetect()
    on_mask_colour_change(rgba_hex: str)
    on_dict_name_change(name: str)
    on_save()
    """

    def __init__(
        self,
        parent: tk.Widget,
        on_tool_change: Callable[[str], None],
        on_zoom_change: Callable[[float], None],
        on_fixed_width_change: Callable[[bool], None],
        on_cell_size_change: Callable[[int, int], None],
        on_margin_change: Callable[[int], None],
        on_redetect: Callable[[], None],
        on_mask_colour_change: Callable[[str], None],
        on_dict_name_change: Callable[[str], None],
        on_save: Callable[[], None],
    ) -> None:
        super().__init__(parent, relief="raised", padding=4)

        self._on_tool_change        = on_tool_change
        self._on_zoom_change        = on_zoom_change
        self._on_fixed_width_change = on_fixed_width_change
        self._on_cell_size_change   = on_cell_size_change
        self._on_margin_change      = on_margin_change
        self._on_redetect           = on_redetect
        self._on_mask_colour_change = on_mask_colour_change
        self._on_dict_name_change   = on_dict_name_change
        self._on_save               = on_save

        self._build()

    # ------------------------------------------------------------------
    # Build
    # ------------------------------------------------------------------

    def _build(self) -> None:
        col = 0

        def sep() -> None:
            nonlocal col
            ttk.Separator(self, orient="vertical").grid(
                row=0, column=col, sticky="ns", padx=6)
            col += 1

        # --- Tools ---
        ttk.Label(self, text="Tool:").grid(row=0, column=col, padx=(0, 2))
        col += 1

        self._tool_var = tk.StringVar(value=Tool.SELECT)
        for label, value in [
            ("Select", Tool.SELECT),
            ("Eyedropper", Tool.EYEDROPPER),
        ]:
            rb = ttk.Radiobutton(
                self, text=label, variable=self._tool_var, value=value,
                command=lambda v=value: self._on_tool_change(v),
            )
            rb.grid(row=0, column=col, padx=2)
            col += 1

        sep()

        # --- Zoom ---
        ttk.Label(self, text="Zoom:").grid(row=0, column=col, padx=(0, 2))
        col += 1
        self._zoom_var = tk.DoubleVar(value=2.0)
        zoom_spin = ttk.Spinbox(
            self, from_=0.25, to=8.0, increment=0.25,
            textvariable=self._zoom_var, width=5,
            command=lambda: self._on_zoom_change(self._zoom_var.get()),
        )
        zoom_spin.grid(row=0, column=col, padx=2)
        col += 1

        sep()

        # --- Detection settings ---
        ttk.Label(self, text="Mask colour:").grid(row=0, column=col, padx=(0, 2))
        col += 1
        self._mask_var = tk.StringVar(value="FFFFFFFF")
        mask_entry = ttk.Entry(self, textvariable=self._mask_var, width=10)
        mask_entry.grid(row=0, column=col, padx=2)
        mask_entry.bind("<Return>",
                        lambda _: self._on_mask_colour_change(self._mask_var.get()))
        col += 1

        self._colour_preview = tk.Label(self, width=2, relief="sunken",
                                        background="#FFFFFF")
        self._colour_preview.grid(row=0, column=col, padx=2)
        col += 1

        sep()

        self._fixed_var = tk.BooleanVar(value=False)
        ttk.Checkbutton(
            self, text="Fixed grid", variable=self._fixed_var,
            command=lambda: self._on_fixed_width_change(self._fixed_var.get()),
        ).grid(row=0, column=col, padx=2)
        col += 1

        ttk.Label(self, text="W:").grid(row=0, column=col, padx=(4, 1))
        col += 1
        self._cell_w_var = tk.IntVar(value=32)
        ttk.Spinbox(self, from_=1, to=4096, textvariable=self._cell_w_var,
                    width=5,
                    command=lambda: self._emit_cell_size()).grid(
            row=0, column=col, padx=1)
        col += 1

        ttk.Label(self, text="H:").grid(row=0, column=col, padx=(4, 1))
        col += 1
        self._cell_h_var = tk.IntVar(value=32)
        ttk.Spinbox(self, from_=1, to=4096, textvariable=self._cell_h_var,
                    width=5,
                    command=lambda: self._emit_cell_size()).grid(
            row=0, column=col, padx=1)
        col += 1

        ttk.Label(self, text="Margin:").grid(row=0, column=col, padx=(4, 1))
        col += 1
        self._margin_var = tk.IntVar(value=0)
        ttk.Spinbox(self, from_=0, to=256, textvariable=self._margin_var,
                    width=4,
                    command=lambda: self._on_margin_change(
                        self._margin_var.get())).grid(
            row=0, column=col, padx=1)
        col += 1

        ttk.Button(self, text="Re-detect",
                   command=self._on_redetect).grid(row=0, column=col, padx=4)
        col += 1

        sep()

        # --- Dict name ---
        ttk.Label(self, text="Dict name:").grid(row=0, column=col, padx=(0, 2))
        col += 1
        self._dict_name_var = tk.StringVar(value="untitled")
        dict_name_entry = ttk.Entry(
            self, textvariable=self._dict_name_var, width=16)
        dict_name_entry.grid(row=0, column=col, padx=2)
        dict_name_entry.bind(
            "<Return>",
            lambda _: self._on_dict_name_change(self._dict_name_var.get()))
        col += 1

        sep()

        ttk.Button(self, text="Save XML",
                   command=self._on_save).grid(row=0, column=col, padx=4)
        col += 1

    # ------------------------------------------------------------------
    # Public setters (called by app when state changes externally)
    # ------------------------------------------------------------------

    def set_mask_colour(self, rgba_hex: str) -> None:
        self._mask_var.set(rgba_hex)
        self._update_colour_preview(rgba_hex)

    def set_dict_name(self, name: str) -> None:
        self._dict_name_var.set(name)

    def get_fixed_width(self) -> bool:
        return self._fixed_var.get()

    def get_cell_size(self) -> tuple[int, int]:
        return self._cell_w_var.get(), self._cell_h_var.get()

    def get_margin(self) -> int:
        return self._margin_var.get()

    def get_mask_colour(self) -> str:
        return self._mask_var.get()

    def get_zoom(self) -> float:
        return self._zoom_var.get()

    # ------------------------------------------------------------------
    # Internal helpers
    # ------------------------------------------------------------------

    def _emit_cell_size(self) -> None:
        self._on_cell_size_change(self._cell_w_var.get(), self._cell_h_var.get())

    def _update_colour_preview(self, rgba_hex: str) -> None:
        try:
            r = int(rgba_hex[0:2], 16)
            g = int(rgba_hex[2:4], 16)
            b = int(rgba_hex[4:6], 16)
            self._colour_preview.configure(
                background=f"#{r:02X}{g:02X}{b:02X}")
        except (ValueError, IndexError):
            pass
