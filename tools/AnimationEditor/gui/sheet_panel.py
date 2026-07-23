"""
gui/sheet_panel.py
------------------
The sprite sheet canvas widget.

Responsibilities:
- Display the sprite sheet at a configurable zoom level
- Overlay detected grid cells with hover and selection highlighting
- Middle-click pan
- Eyedropper tool: click to sample a pixel colour
- Select tool: double-click a cell to add it to the current animation
- Right-click context menu: override cell size
"""

from __future__ import annotations
import tkinter as tk
from tkinter import ttk
from typing import Callable, Optional

from PIL import Image, ImageTk

from config import Tool, GRID_COLOUR, GRID_COLOUR_HOVER, SELECTED_COLOUR
from detection import CellRect
from gui.dialogs import CellSizeDialog


class SheetPanel(ttk.Frame):
    """
    Scrollable canvas showing the sprite sheet with grid overlay.

    Callbacks
    ---------
    on_cell_double_click(cell: CellRect) -> None
        Called when the user double-clicks a cell in SELECT mode.
    on_colour_picked(rgba_hex: str) -> None
        Called when the user clicks in EYEDROPPER mode.
    on_cell_resized(cell: CellRect, new_w: int, new_h: int) -> None
        Called when the user overrides a cell size via the context menu.
    """

    def __init__(
        self,
        parent: tk.Widget,
        on_cell_double_click: Callable[[CellRect], None],
        on_colour_picked: Callable[[str], None],
        on_cell_resized: Callable[[CellRect, int, int], None],
    ) -> None:
        super().__init__(parent)

        self._on_cell_double_click = on_cell_double_click
        self._on_colour_picked     = on_colour_picked
        self._on_cell_resized      = on_cell_resized

        self._image: Optional[Image.Image]           = None
        self._tk_image: Optional[ImageTk.PhotoImage] = None
        self._cells: list[CellRect]                  = []
        self._zoom: float                            = 2.0
        self._tool: str                              = Tool.SELECT

        # Middle-click pan state
        self._pan_start_x: int = 0
        self._pan_start_y: int = 0

        # Hovered cell index into self._cells
        self._hovered_cell: Optional[int] = None

        # (row, col) pairs that have been added to the current animation
        self._selected_cells: set[tuple[int, int]] = set()

        self._build()

    # ------------------------------------------------------------------
    # Build
    # ------------------------------------------------------------------

    def _build(self) -> None:
        self._canvas = tk.Canvas(self, bg="#2b2b2b", cursor="arrow")
        hbar = ttk.Scrollbar(self, orient="horizontal",
                             command=self._canvas.xview)
        vbar = ttk.Scrollbar(self, orient="vertical",
                             command=self._canvas.yview)
        self._canvas.configure(
            xscrollcommand=hbar.set,
            yscrollcommand=vbar.set,
        )

        self._canvas.grid(row=0, column=0, sticky="nsew")
        hbar.grid(row=1, column=0, sticky="ew")
        vbar.grid(row=0, column=1, sticky="ns")
        self.rowconfigure(0, weight=1)
        self.columnconfigure(0, weight=1)

        # Left mouse
        self._canvas.bind("<Button-1>",        self._on_left_click)
        self._canvas.bind("<Double-Button-1>", self._on_double_click)

        # Middle mouse — pan
        self._canvas.bind("<ButtonPress-2>",   self._on_pan_start)
        self._canvas.bind("<B2-Motion>",       self._on_pan_motion)
        self._canvas.bind("<ButtonRelease-2>", self._on_pan_end)

        # Right mouse — context menu
        self._canvas.bind("<Button-3>",        self._on_right_click)

        # Hover
        self._canvas.bind("<Motion>",          self._on_motion)

    # ------------------------------------------------------------------
    # Public API
    # ------------------------------------------------------------------

    def set_tool(self, tool: str) -> None:
        self._tool = tool
        cursors = {
            Tool.SELECT:     "arrow",
            Tool.EYEDROPPER: "crosshair",
        }
        self._canvas.configure(cursor=cursors.get(tool, "arrow"))

    def load_image(self, image: Image.Image) -> None:
        """Replace the displayed image and clear all state."""
        self._image         = image
        self._cells         = []
        self._selected_cells = set()
        self._hovered_cell  = None
        self._render()

    def set_cells(self, cells: list[CellRect]) -> None:
        self._cells          = cells
        self._selected_cells = set()
        self._hovered_cell   = None
        self._render()

    def set_zoom(self, zoom: float) -> None:
        self._zoom = max(0.25, min(zoom, 8.0))
        self._render()

    def mark_selected(self, row: int, col: int) -> None:
        """Highlight a cell as having been added to the current animation."""
        self._selected_cells.add((row, col))
        self._render()

    def clear_selection(self) -> None:
        self._selected_cells = set()
        self._render()

    def get_pixel_colour(self, img_x: int, img_y: int) -> Optional[str]:
        """
        Return the RGBA hex colour of the pixel at (img_x, img_y).
        Returns None if out of bounds or no image is loaded.
        """
        if self._image is None:
            return None
        img = self._image.convert("RGBA")
        w, h = img.size
        if 0 <= img_x < w and 0 <= img_y < h:
            r, g, b, a = img.getpixel((img_x, img_y))
            return f"{r:02X}{g:02X}{b:02X}{a:02X}"
        return None

    # ------------------------------------------------------------------
    # Rendering
    # ------------------------------------------------------------------

    def _render(self) -> None:
        if self._image is None:
            return

        z      = self._zoom
        disp_w = int(self._image.width  * z)
        disp_h = int(self._image.height * z)

        # NEAREST resize preserves pixel art crispness
        scaled         = self._image.resize((disp_w, disp_h), Image.NEAREST)
        self._tk_image = ImageTk.PhotoImage(scaled)

        self._canvas.delete("all")
        self._canvas.create_image(0, 0, anchor="nw", image=self._tk_image)
        self._canvas.configure(scrollregion=(0, 0, disp_w, disp_h))

        self._draw_grid()

    def _draw_grid(self) -> None:
        z = self._zoom
        for i, cell in enumerate(self._cells):
            key = (cell.row, cell.col)
            if key in self._selected_cells:
                colour = SELECTED_COLOUR
                width  = 2
            elif i == self._hovered_cell:
                colour = GRID_COLOUR_HOVER
                width  = 2
            else:
                colour = GRID_COLOUR
                width  = 1

            x0 = int(cell.x * z)
            y0 = int(cell.y * z)
            x1 = int((cell.x + cell.w) * z)
            y1 = int((cell.y + cell.h) * z)
            self._canvas.create_rectangle(
                x0, y0, x1, y1,
                outline=colour, width=width, fill="",
            )

    # ------------------------------------------------------------------
    # Coordinate helpers
    # ------------------------------------------------------------------

    def _canvas_to_image(self, cx: int, cy: int) -> tuple[int, int]:
        """Convert canvas (viewport) coords to image-space coords."""
        # Account for scroll offset
        scroll_x = self._canvas.canvasx(cx)
        scroll_y = self._canvas.canvasy(cy)
        return int(scroll_x / self._zoom), int(scroll_y / self._zoom)

    def _cell_at(self, img_x: int, img_y: int) -> Optional[int]:
        """Return the index of the cell that contains (img_x, img_y), or None."""
        for i, cell in enumerate(self._cells):
            if (cell.x <= img_x < cell.x + cell.w
                    and cell.y <= img_y < cell.y + cell.h):
                return i
        return None

    # ------------------------------------------------------------------
    # Event handlers
    # ------------------------------------------------------------------

    def _on_motion(self, event: tk.Event) -> None:
        img_x, img_y = self._canvas_to_image(event.x, event.y)
        idx = self._cell_at(img_x, img_y)
        if idx != self._hovered_cell:
            self._hovered_cell = idx
            self._render()

    def _on_left_click(self, event: tk.Event) -> None:
        if self._tool == Tool.EYEDROPPER:
            img_x, img_y = self._canvas_to_image(event.x, event.y)
            colour = self.get_pixel_colour(img_x, img_y)
            if colour:
                self._on_colour_picked(colour)

    def _on_double_click(self, event: tk.Event) -> None:
        if self._tool == Tool.SELECT:
            img_x, img_y = self._canvas_to_image(event.x, event.y)
            idx = self._cell_at(img_x, img_y)
            if idx is not None:
                cell = self._cells[idx]
                self.mark_selected(cell.row, cell.col)
                self._on_cell_double_click(cell)

    # --- Pan (middle mouse) ---

    def _on_pan_start(self, event: tk.Event) -> None:
        self._canvas.configure(cursor="fleur")
        self._pan_start_x = event.x
        self._pan_start_y = event.y

    def _on_pan_motion(self, event: tk.Event) -> None:
        dx = self._pan_start_x - event.x
        dy = self._pan_start_y - event.y
        self._canvas.xview_scroll(int(dx), "units")
        self._canvas.yview_scroll(int(dy), "units")
        self._pan_start_x = event.x
        self._pan_start_y = event.y

    def _on_pan_end(self, _event: tk.Event) -> None:
        cursors = {Tool.SELECT: "arrow", Tool.EYEDROPPER: "crosshair"}
        self._canvas.configure(cursor=cursors.get(self._tool, "arrow"))

    # --- Right-click context menu ---

    def _on_right_click(self, event: tk.Event) -> None:
        if self._tool != Tool.SELECT:
            return
        img_x, img_y = self._canvas_to_image(event.x, event.y)
        idx = self._cell_at(img_x, img_y)
        if idx is None:
            return

        cell = self._cells[idx]
        menu = tk.Menu(self, tearoff=0)
        menu.add_command(
            label="Override cell size…",
            command=lambda: self._override_cell_size(cell),
        )
        menu.tk_popup(event.x_root, event.y_root)

    def _override_cell_size(self, cell: CellRect) -> None:
        dlg = CellSizeDialog(self, cell.canvas_w, cell.canvas_h)
        if dlg.result:
            new_w, new_h = dlg.result
            self._on_cell_resized(cell, new_w, new_h)
