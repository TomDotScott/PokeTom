"""
gui/app.py
----------
Main application window. Owns the model state and wires all panels together.
"""

from __future__ import annotations
import tkinter as tk
from tkinter import ttk, filedialog, messagebox
from pathlib import Path
from typing import Optional
import numpy as np

from PIL import Image

from assets import load_texture_entries, resolve_texture_path, TextureEntry
from detection import (
    CellRect, detect_cells_auto, detect_cells_fixed, detect_bg_colour,
)
from models import AnimDict, Animation, Frame, SheetRegion, DetectionSettings
from config import Tool, DEFAULT_DURATION_MS
import xml_io

from gui.toolbar import Toolbar
from gui.sheet_panel import SheetPanel
from gui.anim_panel import AnimPanel


def _rgba_hex_to_rgb_array(rgba_hex: str) -> np.ndarray:
    """Convert an 8-char RGBA hex string to a float numpy array [R, G, B]."""
    r = int(rgba_hex[0:2], 16)
    g = int(rgba_hex[2:4], 16)
    b = int(rgba_hex[4:6], 16)
    return np.array([r, g, b], dtype=float)


def _rgb_array_to_rgba_hex(arr: np.ndarray) -> str:
    r, g, b = int(arr[0]), int(arr[1]), int(arr[2])
    return f"{r:02X}{g:02X}{b:02X}FF"


class App(tk.Tk):
    """Top-level window for the AnimDict Editor."""

    def __init__(self) -> None:
        super().__init__()
        self.title("AnimDict Editor")
        self.geometry("1280x800")
        self.minsize(900, 600)
        self.protocol("WM_DELETE_WINDOW", self._on_close)

        # --- Model ---
        self._anim_dict: AnimDict                  = AnimDict(name="untitled")
        self._texture_entries: list[TextureEntry]  = []
        self._yaml_path: Optional[Path]            = None
        self._image_path: Optional[Path]           = None
        self._full_image: Optional[Image.Image]    = None
        self._cells: list[CellRect]                = []
        self._detection: DetectionSettings         = DetectionSettings()
        self._dirty: bool                          = False   # unsaved changes flag

        self._build()

    # ------------------------------------------------------------------
    # Build
    # ------------------------------------------------------------------

    def _build(self) -> None:
        menubar   = tk.Menu(self)
        file_menu = tk.Menu(menubar, tearoff=0)
        file_menu.add_command(label="Open assets.yaml…",  command=self._open_yaml)
        file_menu.add_command(label="Open image directly…", command=self._open_image_direct)
        file_menu.add_separator()
        file_menu.add_command(label="Save XML…", command=self._save)
        menubar.add_cascade(label="File", menu=file_menu)
        self.configure(menu=menubar)

        self._toolbar = Toolbar(
            self,
            on_tool_change        = self._on_tool_change,
            on_zoom_change        = self._on_zoom_change,
            on_fixed_width_change = self._on_fixed_width_change,
            on_cell_size_change   = self._on_cell_size_change,
            on_margin_change      = self._on_margin_change,
            on_redetect           = self._run_detection,
            on_mask_colour_change = self._on_mask_colour_change,
            on_dict_name_change   = self._on_dict_name_change,
            on_save               = self._save,
        )
        self._toolbar.pack(side="top", fill="x")

        # Asset selector (hidden until a YAML is loaded)
        self._asset_frame = ttk.Frame(self)
        ttk.Label(self._asset_frame, text="Texture:").pack(side="left")
        self._asset_var   = tk.StringVar()
        self._asset_combo = ttk.Combobox(
            self._asset_frame, textvariable=self._asset_var,
            state="readonly", width=40)
        self._asset_combo.pack(side="left", padx=4)
        ttk.Button(
            self._asset_frame, text="Load",
            command=self._load_selected_texture,
        ).pack(side="left")

        # Main split
        paned = ttk.PanedWindow(self, orient="horizontal")
        paned.pack(fill="both", expand=True, padx=4, pady=4)

        self._sheet_panel = SheetPanel(
            paned,
            on_cell_double_click = self._on_cell_double_click,
            on_colour_picked     = self._on_colour_picked,
            on_cell_resized      = self._on_cell_resized,
        )
        paned.add(self._sheet_panel, weight=3)

        self._anim_panel = AnimPanel(
            paned,
            on_animation_selected = self._on_animation_selected,
            on_dict_changed       = self._on_dict_changed,
            get_image_crop        = self._get_image_crop,
        )
        paned.add(self._anim_panel, weight=1)

        self._anim_panel.set_anim_dict(self._anim_dict)

        self._status_var = tk.StringVar(value="Open an assets.yaml or image to begin.")
        ttk.Label(self, textvariable=self._status_var,
                  relief="sunken", anchor="w").pack(
            side="bottom", fill="x", padx=2, pady=1)

    # ------------------------------------------------------------------
    # Dirty-state helpers
    # ------------------------------------------------------------------

    def _mark_dirty(self) -> None:
        self._dirty = True

    def _prompt_save_if_dirty(self) -> bool:
        """
        If there are unsaved changes, ask the user what to do.
        Returns True if it is safe to proceed (saved or discarded),
        False if the user cancelled.
        """
        if not self._dirty:
            return True
        answer = messagebox.askyesnocancel(
            "Unsaved changes",
            "You have unsaved changes. Save before continuing?",
        )
        if answer is None:       # Cancel
            return False
        if answer:               # Yes — save first
            self._save()
        return True              # No — discard

    def _on_close(self) -> None:
        if self._prompt_save_if_dirty():
            self.destroy()

    # ------------------------------------------------------------------
    # File loading
    # ------------------------------------------------------------------

    def _open_yaml(self) -> None:
        if not self._prompt_save_if_dirty():
            return
        path = filedialog.askopenfilename(
            title="Open assets.yaml",
            filetypes=[("YAML files", "*.yaml *.yml"), ("All files", "*.*")],
        )
        if not path:
            return
        self._open_yaml_path(Path(path))

    def _open_yaml_path(self, path: Path) -> None:
        """Load a YAML file (also used by CLI argument handling)."""
        self._yaml_path = path
        try:
            self._texture_entries = load_texture_entries(path)
        except Exception as exc:
            messagebox.showerror("Error", f"Could not parse YAML:\n{exc}")
            return
        names = [e.name for e in self._texture_entries]
        self._asset_combo["values"] = names
        if names:
            self._asset_combo.current(0)
        self._asset_frame.pack(side="top", fill="x", padx=4, pady=2)
        self._status(f"Loaded {len(names)} texture(s) from {path.name}")

    def _open_image_direct(self) -> None:
        if not self._prompt_save_if_dirty():
            return
        path = filedialog.askopenfilename(
            title="Open sprite sheet",
            filetypes=[("Images", "*.png *.jpg *.jpeg *.bmp *.gif"),
                       ("All files", "*.*")],
        )
        if not path:
            return
        self._load_image(Path(path), source_name=Path(path).stem.upper())

    def _load_selected_texture(self) -> None:
        if not self._prompt_save_if_dirty():
            return
        name  = self._asset_var.get()
        entry = next((e for e in self._texture_entries if e.name == name), None)
        if not entry or not self._yaml_path:
            return
        self._load_image(
            resolve_texture_path(self._yaml_path, entry.source),
            source_name=entry.name,
        )

    def _load_image(self, path: Path, source_name: str) -> None:
        try:
            img = Image.open(path).convert("RGB")
        except Exception as exc:
            messagebox.showerror("Error", f"Could not open image:\n{exc}")
            return

        # Reset all state
        self._image_path = path
        self._full_image = img
        self._cells      = []
        self._anim_dict  = AnimDict(name=path.stem)
        self._dirty      = False

        self._anim_panel.set_anim_dict(self._anim_dict)
        self._toolbar.set_dict_name(self._anim_dict.name)

        # Auto-detect bg colour
        arr      = np.array(img)
        bg       = detect_bg_colour(arr)
        rgba_hex = _rgb_array_to_rgba_hex(bg)
        self._toolbar.set_mask_colour(rgba_hex)
        self._detection.bg_tolerance = 30

        self._anim_dict.region = SheetRegion(
            source        = source_name,
            top_left_x    = 0,
            top_left_y    = 0,
            region_width  = img.width,
            region_height = img.height,
            mask_colour   = rgba_hex,
        )

        self._sheet_panel.load_image(img)
        self._run_detection()
        self._status(f"Loaded {path.name}  ({img.width}x{img.height})")

    # ------------------------------------------------------------------
    # Detection
    # ------------------------------------------------------------------

    def _run_detection(self) -> None:
        if self._full_image is None:
            return

        mask_hex = self._toolbar.get_mask_colour()
        try:
            bg = _rgba_hex_to_rgb_array(mask_hex)
        except (ValueError, IndexError):
            self._status("Invalid mask colour hex.")
            return

        if self._toolbar.get_fixed_width():
            cw, ch  = self._toolbar.get_cell_size()
            margin  = self._toolbar.get_margin()
            cells   = detect_cells_fixed(self._full_image, cw, ch, margin)
        else:
            cells = detect_cells_auto(
                self._full_image,
                bg_colour     = bg,
                bg_tolerance  = self._detection.bg_tolerance,
                row_gap_thresh = self._detection.row_gap_thresh,
                min_row_gap   = self._detection.min_row_gap,
                col_gap_thresh = self._detection.col_gap_thresh,
                min_col_gap   = self._detection.min_col_gap,
            )

        self._cells = cells
        self._sheet_panel.set_cells(cells)
        self._status(f"Detected {len(cells)} cells.")

    # ------------------------------------------------------------------
    # Toolbar callbacks
    # ------------------------------------------------------------------

    def _on_tool_change(self, tool: str) -> None:
        self._sheet_panel.set_tool(tool)

    def _on_zoom_change(self, zoom: float) -> None:
        self._sheet_panel.set_zoom(zoom)

    def _on_fixed_width_change(self, enabled: bool) -> None:
        self._detection.fixed_width = enabled

    def _on_cell_size_change(self, w: int, h: int) -> None:
        self._detection.cell_width  = w
        self._detection.cell_height = h

    def _on_margin_change(self, margin: int) -> None:
        self._detection.margin = margin

    def _on_mask_colour_change(self, rgba_hex: str) -> None:
        if self._anim_dict.region:
            self._anim_dict.region.mask_colour = rgba_hex
        self._toolbar.set_mask_colour(rgba_hex)
        self._run_detection()

    def _on_dict_name_change(self, name: str) -> None:
        self._anim_dict.name = name.strip() or "untitled"

    # ------------------------------------------------------------------
    # Sheet panel callbacks
    # ------------------------------------------------------------------

    def _on_colour_picked(self, rgba_hex: str) -> None:
        """Eyedropper result: set new mask colour and re-detect."""
        self._toolbar.set_mask_colour(rgba_hex)
        if self._anim_dict.region:
            self._anim_dict.region.mask_colour = rgba_hex
        self._run_detection()
        self._status(f"Mask colour set to #{rgba_hex}")

    def _on_cell_double_click(self, cell: CellRect) -> None:
        """Add the clicked cell as a frame to the current animation."""
        # self._cells are already absolute (no region offset anymore)
        abs_cell = next(
            (c for c in self._cells if c.row == cell.row and c.col == cell.col),
            None,
        )
        if abs_cell is None:
            return
        frame = Frame(
            top_left_x   = abs_cell.x,
            top_left_y   = abs_cell.y,
            duration     = DEFAULT_DURATION_MS,
            sprite_width = abs_cell.canvas_w,
            sprite_height = abs_cell.canvas_h,
        )
        self._anim_panel.add_frame_to_current(frame)

    def _on_cell_resized(self, cell: CellRect, new_w: int, new_h: int) -> None:
        """Override the canvas size stored for a cell."""
        for i, c in enumerate(self._cells):
            if c.row == cell.row and c.col == cell.col:
                self._cells[i] = CellRect(
                    col=c.col, row=c.row,
                    x=c.x, y=c.y, w=c.w, h=c.h,
                    canvas_w=new_w, canvas_h=new_h,
                )
                break
        self._status(f"Cell [{cell.row},{cell.col}] overridden to {new_w}x{new_h}")

    # ------------------------------------------------------------------
    # Image crop helper (for the playback preview)
    # ------------------------------------------------------------------

    def _get_image_crop(
        self, x: int, y: int, w: int, h: int
    ) -> Optional[Image.Image]:
        """Return a crop of the full image at the given absolute coords."""
        if self._full_image is None:
            return None
        return self._full_image.crop((x, y, x + w, y + h))

    # ------------------------------------------------------------------
    # Anim panel callbacks
    # ------------------------------------------------------------------

    def _on_animation_selected(self, anim: Optional[Animation]) -> None:
        self._sheet_panel.clear_selection()

    def _on_dict_changed(self) -> None:
        self._mark_dirty()

    # ------------------------------------------------------------------
    # Save
    # ------------------------------------------------------------------

    def _save(self) -> None:
        path = filedialog.asksaveasfilename(
            title="Save AnimDict XML",
            defaultextension=".xml",
            filetypes=[("XML files", "*.xml"), ("All files", "*.*")],
            initialfile=f"{self._anim_dict.name}.xml",
        )
        if not path:
            return
        try:
            xml_io.save(self._anim_dict, Path(path))
            self._dirty = False
            self._status(f"Saved: {path}")
        except Exception as exc:
            messagebox.showerror("Save failed", str(exc))

    # ------------------------------------------------------------------
    # Helpers
    # ------------------------------------------------------------------

    def _status(self, message: str) -> None:
        self._status_var.set(message)
