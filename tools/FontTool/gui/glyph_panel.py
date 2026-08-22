"""
font_tool/gui/glyph_panel.py
-----------------------------
Right-hand side panel: glyph list and font-level metadata display.

The panel exposes a clean interface so ``app.py`` can push state to it
and subscribe to user edits without tight coupling.
"""

from __future__ import annotations

import tkinter as tk
from tkinter import ttk
from typing import Callable, Optional

import sys, os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))
sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "..", "shared"))

from models import FontData, GlyphData  # type: ignore[import]


class GlyphPanel(ttk.Frame):
    """
    Displays font metadata (read-only summary) and the full glyph list.

    Callbacks
    ---------
    on_select(char: str)
        Fired when the user single-clicks a glyph row.  Passes the character.
    on_delete(char: str)
        Fired when the user clicks the Delete button for a row.
    on_edit_meta()
        Fired when the user clicks "Edit Metadata…".
    on_set_font_area()
        Fired when the user clicks "Set Font Area…".
    """

    def __init__(
        self,
        parent: tk.Misc,
        on_select: Optional[Callable[[str], None]] = None,
        on_delete: Optional[Callable[[str], None]] = None,
        on_edit_meta: Optional[Callable[[], None]] = None,
        on_set_font_area: Optional[Callable[[], None]] = None,
        **kwargs: object,
    ) -> None:
        super().__init__(parent, **kwargs)

        self._on_select = on_select
        self._on_delete = on_delete
        self._on_edit_meta = on_edit_meta
        self._on_set_font_area = on_set_font_area

        self._font_data: Optional[FontData] = None

        self._build_ui()

    # ── UI construction ───────────────────────────────────────────────────────

    def _build_ui(self) -> None:
        self.columnconfigure(0, weight=1)

        # ── Metadata summary ──
        meta_frame = ttk.LabelFrame(self, text="Font Metadata", padding=6)
        meta_frame.grid(row=0, column=0, sticky=tk.EW, padx=6, pady=(6, 0))
        meta_frame.columnconfigure(0, weight=1)

        self._meta_texture = ttk.Label(meta_frame, text="Texture: —")
        self._meta_texture.grid(row=0, column=0, sticky=tk.W)

        self._meta_area = ttk.Label(meta_frame, text="Font area: —")
        self._meta_area.grid(row=1, column=0, sticky=tk.W)

        self._meta_heights = ttk.Label(meta_frame, text="Ref / Line height: — / —")
        self._meta_heights.grid(row=2, column=0, sticky=tk.W)

        btn_meta_row = ttk.Frame(meta_frame)
        btn_meta_row.grid(row=3, column=0, sticky=tk.EW, pady=(6, 0))

        ttk.Button(btn_meta_row, text="Edit Metadata…",
                   command=self._fire_edit_meta).pack(side=tk.LEFT)
        ttk.Button(btn_meta_row, text="Set Font Area…",
                   command=self._fire_set_font_area).pack(side=tk.LEFT, padx=(4, 0))

        # ── Glyph count label ──
        self._count_label = ttk.Label(self, text="Glyphs: 0")
        self._count_label.grid(row=1, column=0, sticky=tk.W, padx=6, pady=(8, 2))

        # ── Glyph listbox ──
        list_frame = ttk.Frame(self)
        list_frame.grid(row=2, column=0, sticky=tk.NSEW, padx=6)
        self.rowconfigure(2, weight=1)
        list_frame.columnconfigure(0, weight=1)

        scrollbar = ttk.Scrollbar(list_frame, orient=tk.VERTICAL)
        self._listbox = tk.Listbox(
            list_frame,
            yscrollcommand=scrollbar.set,
            font=("Courier", 10),
            selectmode=tk.SINGLE,
            activestyle="dotbox",
        )
        scrollbar.configure(command=self._listbox.yview)
        self._listbox.grid(row=0, column=0, sticky=tk.NSEW)
        scrollbar.grid(row=0, column=1, sticky=tk.NS)
        list_frame.rowconfigure(0, weight=1)

        self._listbox.bind("<<ListboxSelect>>", self._on_listbox_select)
        self._listbox.bind("<Delete>", self._on_delete_key)
        self._listbox.bind("<BackSpace>", self._on_delete_key)

        # ── Bottom button row ──
        btn_row = ttk.Frame(self)
        btn_row.grid(row=3, column=0, sticky=tk.EW, padx=6, pady=6)

        ttk.Button(btn_row, text="Delete selected",
                   command=self._delete_selected).pack(side=tk.LEFT)

    # ── Public API ────────────────────────────────────────────────────────────

    def refresh(self, font_data: FontData) -> None:
        """Re-populate the panel from *font_data*."""
        self._font_data = font_data
        self._refresh_meta(font_data)
        self._refresh_list(font_data)

    def select_char(self, char: str) -> None:
        """Highlight the row for *char* (if present)."""
        if self._font_data is None:
            return
        glyphs = self._font_data.sorted_glyphs()
        for i, g in enumerate(glyphs):
            if g.char == char:
                self._listbox.selection_clear(0, tk.END)
                self._listbox.selection_set(i)
                self._listbox.see(i)
                return

    # ── Internal refresh helpers ──────────────────────────────────────────────

    def _refresh_meta(self, font_data: FontData) -> None:
        self._meta_texture.configure(
            text=f"Texture: {font_data.texture or '—'}"
        )
        fa = font_data.font_area
        if fa.width > 0 and fa.height > 0:
            area_text = f"Font area: ({fa.x}, {fa.y})  {fa.width}×{fa.height}"
        else:
            area_text = "Font area: not set"
        self._meta_area.configure(text=area_text)
        self._meta_heights.configure(
            text=f"Ref / Line height: {font_data.reference_height} / {font_data.line_height} px"
        )

    def _refresh_list(self, font_data: FontData) -> None:
        glyphs = font_data.sorted_glyphs()
        self._listbox.delete(0, tk.END)
        for g in glyphs:
            self._listbox.insert(tk.END, g.display_label())
        self._count_label.configure(text=f"Glyphs: {len(glyphs)}")

    # ── Event handlers ────────────────────────────────────────────────────────

    def _on_listbox_select(self, _event: tk.Event) -> None:
        char = self._selected_char()
        if char is not None and self._on_select is not None:
            self._on_select(char)

    def _on_delete_key(self, _event: tk.Event) -> None:
        self._delete_selected()

    def _delete_selected(self) -> None:
        char = self._selected_char()
        if char is not None and self._on_delete is not None:
            self._on_delete(char)

    def _selected_char(self) -> Optional[str]:
        if self._font_data is None:
            return None
        sel = self._listbox.curselection()
        if not sel:
            return None
        glyphs = self._font_data.sorted_glyphs()
        idx = sel[0]
        if idx >= len(glyphs):
            return None
        return glyphs[idx].char

    def _fire_edit_meta(self) -> None:
        if self._on_edit_meta is not None:
            self._on_edit_meta()

    def _fire_set_font_area(self) -> None:
        if self._on_set_font_area is not None:
            self._on_set_font_area()
