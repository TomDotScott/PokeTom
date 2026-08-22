"""
font_tool/gui/app.py
--------------------
Main application window for the Bitmap Font Editor.

Layout
------

  ┌─────────────────────────────────────────────────────────┐
  │  Menu bar                                               │
  ├─────────────────────────────────────┬───────────────────┤
  │                                     │                   │
  │   SheetCanvas  (left, resizable)    │   GlyphPanel      │
  │                                     │   (right, fixed)  │
  │                                     │                   │
  └─────────────────────────────────────┴───────────────────┘
  │  Status bar                                             │
  └─────────────────────────────────────────────────────────┘

Workflow
--------
1. File → Open from assets.yaml  (or Open Image File…)
2. Drag a rectangle over any glyph on the sheet.
3. GlyphCharDialog pops up: type the character, confirm advance.
4. Glyph appears in the right-hand list.
5. Optionally: drag a rectangle then use Edit → Set as Font Area.
6. File → Export JSON.
"""

from __future__ import annotations

import copy
import sys
import os
from pathlib import Path
from tkinter import filedialog, messagebox
import tkinter as tk
from tkinter import ttk

# ── Path setup so we can import shared/ and font_tool/ siblings ───────────────
_HERE = os.path.dirname(os.path.abspath(__file__))
_FONT_TOOL_ROOT = os.path.join(_HERE, "..")
_TOOLS_ROOT = os.path.join(_FONT_TOOL_ROOT, "..")
_SHARED = os.path.join(_TOOLS_ROOT, "shared")

for _p in (_FONT_TOOL_ROOT, _SHARED):
    if _p not in sys.path:
        sys.path.insert(0, _p)

from models import FontData, GlyphData, FontArea  # type: ignore[import]
from json_io import save_font, load_font           # type: ignore[import]
from assets import (                               # type: ignore[import]
    load_assets_yaml,
    get_texture_list,
    resolve_texture_path,
)
from dialogs import TexturePickerDialog            # type: ignore[import]
from sheet_canvas import SheetCanvas               # type: ignore[import]
from gui.dialogs import GlyphCharDialog, FontMetaDialog, FontAreaDialog  # type: ignore[import]
from gui.glyph_panel import GlyphPanel             # type: ignore[import]

try:
    from PIL import Image  # type: ignore[import]
    _HAS_PIL = True
except ImportError:
    _HAS_PIL = False

_PANEL_WIDTH = 300
_STATUS_TIMEOUT_MS = 5_000


class App(tk.Tk):
    """Top-level window for the Bitmap Font Editor."""

    def __init__(self) -> None:
        super().__init__()
        self.title("Bitmap Font Editor")
        self.geometry("1200x700")
        self.minsize(800, 500)

        # ── State ──
        self._font_data = FontData()
        self._yaml_path: Path | None = None
        self._image_path: Path | None = None
        self._pil_image: Image.Image | None = None
        self._dirty: bool = False           # unsaved changes?
        # Last rubber-band selection (image-space) — used for "Set Font Area".
        self._last_selection: tuple[int, int, int, int] | None = None

        # Undo/redo history: snapshots of _font_data taken before each edit.
        self._undo_stack: list[FontData] = []
        self._redo_stack: list[FontData] = []
        self._undo_limit = 100

        self._build_ui()
        self._bind_shortcuts()
        self._status("Ready.  Use File → Open to load a texture.")

    # ── UI construction ───────────────────────────────────────────────────────

    def _build_ui(self) -> None:
        self._build_menu()

        # ── Main paned layout ──
        pane = ttk.PanedWindow(self, orient=tk.HORIZONTAL)
        pane.pack(fill=tk.BOTH, expand=True)

        # Left: sheet canvas.
        self._sheet = SheetCanvas(
            pane,
            on_selection=self._on_region_selected,
            on_overlay_edit=self._on_overlay_edited,
            on_overlay_select=self._on_overlay_selected,
        )
        pane.add(self._sheet, weight=1)

        # Right: glyph panel (fixed width).
        right_frame = ttk.Frame(pane, width=_PANEL_WIDTH)
        right_frame.pack_propagate(False)
        pane.add(right_frame, weight=0)

        self._panel = GlyphPanel(
            right_frame,
            on_select=self._on_glyph_selected,
            on_delete=self._on_glyph_delete,
            on_edit_meta=self._cmd_edit_meta,
            on_set_font_area=self._cmd_set_font_area,
        )
        self._panel.pack(fill=tk.BOTH, expand=True)

        # ── Status bar ──
        status_frame = ttk.Frame(self, relief=tk.SUNKEN, padding=(4, 2))
        status_frame.pack(side=tk.BOTTOM, fill=tk.X)
        self._status_var = tk.StringVar()
        ttk.Label(status_frame, textvariable=self._status_var, anchor=tk.W).pack(
            fill=tk.X, side=tk.LEFT
        )

        self._refresh_panel()

    def _build_menu(self) -> None:
        menubar = tk.Menu(self)
        self.configure(menu=menubar)

        # File
        file_menu = tk.Menu(menubar, tearoff=False)
        menubar.add_cascade(label="File", menu=file_menu)
        file_menu.add_command(label="Open from assets.yaml…",
                              command=self._cmd_open_yaml, accelerator="Ctrl+O")
        file_menu.add_command(label="Open Image File…",
                              command=self._cmd_open_image)
        file_menu.add_separator()
        file_menu.add_command(label="Load JSON…",
                              command=self._cmd_load_json)
        file_menu.add_separator()
        file_menu.add_command(label="Export JSON…",
                              command=self._cmd_export, accelerator="Ctrl+S")
        file_menu.add_separator()
        file_menu.add_command(label="Quit", command=self._cmd_quit)

        # Edit
        edit_menu = tk.Menu(menubar, tearoff=False)
        menubar.add_cascade(label="Edit", menu=edit_menu)
        edit_menu.add_command(label="Undo", command=self._cmd_undo,
                              accelerator="Ctrl+Z")
        edit_menu.add_command(label="Redo", command=self._cmd_redo,
                              accelerator="Ctrl+Y")
        edit_menu.add_separator()
        edit_menu.add_command(label="Edit Font Metadata…",
                              command=self._cmd_edit_meta, accelerator="Ctrl+M")
        edit_menu.add_command(label="Set Font Area from selection…",
                              command=self._cmd_set_font_area)
        edit_menu.add_separator()
        edit_menu.add_command(label="Clear all glyphs",
                              command=self._cmd_clear_glyphs)

        # View
        view_menu = tk.Menu(menubar, tearoff=False)
        menubar.add_cascade(label="View", menu=view_menu)
        view_menu.add_command(label="Zoom to Fit  (double-click canvas)",
                              state=tk.DISABLED)

    def _bind_shortcuts(self) -> None:
        self.bind("<Control-o>", lambda _e: self._cmd_open_yaml())
        self.bind("<Control-s>", lambda _e: self._cmd_export())
        self.bind("<Control-m>", lambda _e: self._cmd_edit_meta())
        self.bind("<Control-z>", lambda _e: self._cmd_undo())
        self.bind("<Control-y>", lambda _e: self._cmd_redo())
        self.bind("<Control-Shift-Z>", lambda _e: self._cmd_redo())

    # ── File commands ─────────────────────────────────────────────────────────

    def _cmd_open_yaml(self) -> None:
        if not _HAS_PIL:
            messagebox.showerror("Missing dependency", "Pillow (PIL) is required.")
            return
        path = filedialog.askopenfilename(
            title="Open assets.yaml",
            filetypes=[("YAML files", "*.yaml *.yml"), ("All files", "*.*")],
        )
        if not path:
            return
        self._yaml_path = Path(path)
        try:
            assets = load_assets_yaml(self._yaml_path)
            textures = get_texture_list(assets)
        except Exception as exc:
            messagebox.showerror("Error", f"Could not load assets.yaml:\n{exc}")
            return

        if not textures:
            messagebox.showwarning("No textures", "No textures found in assets.yaml.")
            return

        dlg = TexturePickerDialog(self, textures)
        self.wait_window(dlg)
        if dlg.result is None:
            return

        tex = dlg.result
        tex_path = resolve_texture_path(self._yaml_path, tex["source"])
        self._load_texture(tex_path, tex["name"])

    def _cmd_open_image(self) -> None:
        if not _HAS_PIL:
            messagebox.showerror("Missing dependency", "Pillow (PIL) is required.")
            return
        path = filedialog.askopenfilename(
            title="Open Image",
            filetypes=[("Images", "*.png *.bmp *.jpg *.jpeg *.gif"), ("All", "*.*")],
        )
        if not path:
            return
        name = Path(path).stem.upper().replace(" ", "_")
        self._font_data.texture = name
        self._load_texture(Path(path), name)

    def _load_texture(self, path: Path, name: str) -> None:
        try:
            img = Image.open(path).convert("RGBA")
        except Exception as exc:
            messagebox.showerror("Error", f"Could not open image:\n{exc}")
            return
        self._pil_image = img
        self._image_path = path
        self._font_data.texture = name
        self._reset_history()
        self._sheet.load_image(img)
        self._refresh_overlays()
        self._refresh_panel()
        self.title(f"Bitmap Font Editor — {name}")
        self._status(f"Loaded {name}  ({img.width}×{img.height} px).  Drag to define glyphs.")

    def _cmd_export(self) -> None:
        if not self._font_data.texture:
            messagebox.showwarning("Nothing to export", "No texture loaded yet.")
            return
        path = filedialog.asksaveasfilename(
            title="Export JSON",
            defaultextension=".json",
            filetypes=[("JSON files", "*.json"), ("All files", "*.*")],
            initialfile=f"{self._font_data.texture.lower()}_font.json",
        )
        if not path:
            return
        try:
            save_font(self._font_data, Path(path))
        except Exception as exc:
            messagebox.showerror("Error", f"Export failed:\n{exc}")
            return
        self._dirty = False
        self._status(f"Exported → {path}")

    def _cmd_load_json(self) -> None:
        path = filedialog.askopenfilename(
            title="Load Font JSON",
            filetypes=[("JSON files", "*.json"), ("All files", "*.*")],
        )
        if not path:
            return
        try:
            self._font_data = load_font(Path(path))
        except Exception as exc:
            messagebox.showerror("Error", f"Could not load JSON:\n{exc}")
            return
        self._reset_history()
        self._refresh_panel()
        self._refresh_overlays()
        self._status(f"Loaded {Path(path).name}  ({len(self._font_data.glyphs)} glyphs).")

    def _cmd_quit(self) -> None:
        if self._dirty:
            if not messagebox.askyesno("Quit", "You have unsaved changes. Quit anyway?"):
                return
        self.destroy()

    # ── Undo / redo ───────────────────────────────────────────────────────────

    def _snapshot_before_change(self) -> None:
        """Push the current font_data onto the undo stack before mutating it."""
        self._undo_stack.append(copy.deepcopy(self._font_data))
        if len(self._undo_stack) > self._undo_limit:
            self._undo_stack.pop(0)
        self._redo_stack.clear()

    def _reset_history(self) -> None:
        self._undo_stack.clear()
        self._redo_stack.clear()

    def _cmd_undo(self) -> None:
        if not self._undo_stack:
            self._status("Nothing to undo.")
            return
        self._redo_stack.append(copy.deepcopy(self._font_data))
        self._font_data = self._undo_stack.pop()
        self._dirty = True
        self._refresh_panel()
        self._refresh_overlays()
        self._status("Undo.")

    def _cmd_redo(self) -> None:
        if not self._redo_stack:
            self._status("Nothing to redo.")
            return
        self._undo_stack.append(copy.deepcopy(self._font_data))
        self._font_data = self._redo_stack.pop()
        self._dirty = True
        self._refresh_panel()
        self._refresh_overlays()
        self._status("Redo.")

    # ── Edit commands ─────────────────────────────────────────────────────────

    def _cmd_edit_meta(self) -> None:
        dlg = FontMetaDialog(
            self,
            reference_height=self._font_data.reference_height,
            line_height=self._font_data.line_height,
        )
        self.wait_window(dlg)
        if dlg.result is None:
            return
        self._snapshot_before_change()
        self._font_data.reference_height = dlg.result["reference_height"]
        self._font_data.line_height = dlg.result["line_height"]
        self._dirty = True
        self._refresh_panel()
        self._status("Metadata updated.")

    def _cmd_set_font_area(self) -> None:
        sel = self._last_selection
        dlg = FontAreaDialog(
            self,
            x=sel[0] if sel else self._font_data.font_area.x,
            y=sel[1] if sel else self._font_data.font_area.y,
            width=sel[2] if sel else self._font_data.font_area.width,
            height=sel[3] if sel else self._font_data.font_area.height,
        )
        self.wait_window(dlg)
        if dlg.result is None:
            return
        r = dlg.result
        self._snapshot_before_change()
        self._font_data.font_area = FontArea(
            x=r["x"], y=r["y"], width=r["width"], height=r["height"]
        )
        self._dirty = True
        self._refresh_panel()
        self._status(f"Font area set: ({r['x']}, {r['y']}) {r['width']}×{r['height']}")

    def _cmd_clear_glyphs(self) -> None:
        if not self._font_data.glyphs:
            return
        if not messagebox.askyesno("Clear", "Delete all defined glyphs?"):
            return
        self._snapshot_before_change()
        self._font_data.glyphs.clear()
        self._dirty = True
        self._refresh_panel()
        self._refresh_overlays()
        self._status("All glyphs cleared.")

    # ── Selection callback (from SheetCanvas) ─────────────────────────────────

    def _on_region_selected(self, x: int, y: int, w: int, h: int) -> None:
        """Called by SheetCanvas when the user completes a rubber-band drag."""
        self._last_selection = (x, y, w, h)

        if not _HAS_PIL or self._pil_image is None:
            return

        existing_chars = {g.char for g in self._font_data.glyphs}
        crop = self._pil_image.crop((x, y, x + w, y + h))

        dlg = GlyphCharDialog(self, crop=crop, region_width=w,
                              existing_chars=existing_chars)
        self.wait_window(dlg)
        if dlg.result is None:
            return  # User skipped.

        char = dlg.result["char"]
        advance = dlg.result["advance"]

        glyph = GlyphData(char=char, x=x, y=y, width=w, height=h, advance=advance)
        self._snapshot_before_change()
        self._font_data.add_or_replace(glyph)
        self._dirty = True
        self._refresh_panel()
        self._refresh_overlays()
        self._panel.select_char(char)
        self._status(f"Glyph '{char}' saved  ({w}×{h} px, advance={advance}).")

    # ── Glyph panel callbacks ─────────────────────────────────────────────────

    def _on_glyph_selected(self, char: str) -> None:
        self._sheet.select_overlay(char)
        self._status(f"Selected: '{char}'")

    def _on_glyph_delete(self, char: str) -> None:
        if not messagebox.askyesno("Delete", f"Remove glyph '{char}'?"):
            return
        self._snapshot_before_change()
        self._font_data.remove(char)
        self._dirty = True
        self._refresh_panel()
        self._refresh_overlays()
        self._status(f"Deleted glyph '{char}'.")

    # ── Overlay editing callbacks (from SheetCanvas) ──────────────────────────

    def _on_overlay_edited(self, char: str, x: int, y: int, w: int, h: int) -> None:
        """Called when the user resizes/moves an existing glyph box on the canvas."""
        glyph = self._font_data.find_glyph(char)
        if glyph is None:
            return
        self._snapshot_before_change()
        glyph.x, glyph.y, glyph.width, glyph.height = x, y, w, h
        self._dirty = True
        self._refresh_panel()
        self._refresh_overlays()
        self._panel.select_char(char)
        self._status(f"Glyph '{char}' bounds updated  ({w}×{h} px at {x},{y}).")

    def _on_overlay_selected(self, char: object) -> None:
        if char is None:
            return
        self._panel.select_char(str(char))
        self._status(f"Selected: '{char}'")

    # ── Refresh helpers ───────────────────────────────────────────────────────

    def _refresh_panel(self) -> None:
        self._panel.refresh(self._font_data)

    def _refresh_overlays(self) -> None:
        """Push glyph rectangles to the sheet canvas as overlays."""
        overlays = [
            (g.char, g.x, g.y, g.width, g.height, "#00FF88")
            for g in self._font_data.glyphs
        ]
        self._sheet.set_overlays(overlays)

    # ── Status bar ────────────────────────────────────────────────────────────

    def _status(self, msg: str) -> None:
        self._status_var.set(msg)
