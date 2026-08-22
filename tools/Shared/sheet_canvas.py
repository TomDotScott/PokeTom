"""
shared/sheet_canvas.py
----------------------
A reusable Tkinter widget that displays a PIL image and lets the user draw
a rubber-band selection rectangle over it, then click existing overlay boxes
to select and reshape them.

Usage::

    canvas = SheetCanvas(
        parent,
        on_selection=my_callback,
        on_overlay_edit=my_edit_callback,
        on_overlay_select=my_select_callback,
    )
    canvas.pack(fill=tk.BOTH, expand=True)
    canvas.load_image(pil_image)

    # my_callback receives image-space coordinates for a *new* selection:
    def my_callback(x: int, y: int, w: int, h: int) -> None:
        ...

    # my_edit_callback fires when an existing overlay's bounds are changed
    # by dragging a handle or the box itself:
    def my_edit_callback(overlay_id, x: int, y: int, w: int, h: int) -> None:
        ...

    # my_select_callback fires when the user clicks an overlay to select it
    # (or None when it's deselected):
    def my_select_callback(overlay_id) -> None:
        ...

    # Draw coloured, identified overlay rectangles (image-space coords):
    canvas.set_overlays([
        (glyph.char, glyph.x, glyph.y, glyph.width, glyph.height, "#00FF88"),
        ...
    ])

Controls
--------
* **Left-drag on empty canvas**   – draw a rubber-band selection rectangle.
* **Left-click an overlay box**   – select it (shows resize handles).
* **Drag a handle**               – resize the selected box.
* **Drag inside a selected box**  – move it.
* **Scroll wheel**                – zoom in/out centred on the cursor.
* **Middle-drag / Space+drag**    – pan the view.
* **Double-click**                – zoom to fit the full image.
"""

from __future__ import annotations

import tkinter as tk
from tkinter import ttk
from typing import Callable, Optional

try:
    from PIL import Image, ImageTk  # type: ignore[import]
    _HAS_PIL = True
except ImportError:
    _HAS_PIL = False

# Colour used for the live rubber-band rectangle.
_RUBBERBAND_COLOUR = "#FF3333"
# Minimum drag size (canvas pixels) before a selection fires.
_MIN_DRAG_PX = 4
# Zoom limits.
_ZOOM_MIN = 0.1
_ZOOM_MAX = 16.0
_ZOOM_STEP = 1.2

# Pixel-grid overlay.
_GRID_COLOUR = "#5a5a5a"
# Only draw the grid once pixels are big enough on screen to be useful;
# below this zoom the lines would just be visual noise.
_GRID_MIN_ZOOM = 4.0

# Resize handles on a selected overlay box.
_HANDLE_SIZE_PX = 6        # square handle side length, in canvas pixels
_HANDLE_HIT_PX = 10        # click tolerance around a handle, in canvas pixels
_HANDLE_FILL = "#FFFFFF"
_HANDLE_OUTLINE = "#000000"
_SELECTED_COLOUR = "#FFCC00"

# Extra tolerance (canvas pixels) so clicking just outside a box's edge still
# selects it — small/thin glyph boxes are fiddly to hit exactly otherwise.
_OVERLAY_HIT_BUFFER_PX = 5

# Cursor shown for each handle while hovering.
_HANDLE_CURSORS = {
    "nw": "top_left_corner", "se": "bottom_right_corner",
    "ne": "top_right_corner", "sw": "bottom_left_corner",
    "n": "sb_v_double_arrow", "s": "sb_v_double_arrow",
    "e": "sb_h_double_arrow", "w": "sb_h_double_arrow",
}


class SheetCanvas(ttk.Frame):
    """
    Zoomable/pannable image canvas with rubber-band region selection.

    Parameters
    ----------
    parent:
        Tkinter parent widget.
    on_selection:
        Callback invoked with ``(x, y, w, h)`` in image-space pixels after the
        user completes a drag selection.  Receives ``None`` if the drag was too
        small to constitute a selection.
    on_overlay_edit:
        Callback invoked with ``(overlay_id, x, y, w, h)`` in image-space
        pixels after the user finishes resizing or moving an existing overlay.
    on_overlay_select:
        Callback invoked with ``overlay_id`` (or ``None``) whenever the
        selected overlay changes.
    """

    def __init__(
        self,
        parent: tk.Misc,
        on_selection: Optional[Callable[[int, int, int, int], None]] = None,
        on_overlay_edit: Optional[Callable[[object, int, int, int, int], None]] = None,
        on_overlay_select: Optional[Callable[[object], None]] = None,
        **kwargs: object,
    ) -> None:
        super().__init__(parent, **kwargs)

        self._on_selection = on_selection
        self._on_overlay_edit = on_overlay_edit
        self._on_overlay_select = on_overlay_select

        # Internal state.
        self._pil_image: Optional[Image.Image] = None
        self._tk_image: Optional[ImageTk.PhotoImage] = None
        self._zoom: float = 1.0
        # Canvas-space offset of the image top-left corner.
        self._offset_x: float = 0.0
        self._offset_y: float = 0.0

        # Pan tracking.
        self._pan_start_x: int = 0
        self._pan_start_y: int = 0
        self._panning: bool = False

        # Rubber-band tracking.
        self._drag_start_cx: int = 0   # canvas coords at drag start
        self._drag_start_cy: int = 0
        self._dragging: bool = False
        self._rubberband_id: Optional[int] = None

        # Overlays: list of (id, x, y, w, h, colour) in image-space.
        self._overlays: list[tuple[object, int, int, int, int, str]] = []

        # Selected overlay (for resize/move editing).
        self._selected_overlay_id: Optional[object] = None
        # Canvas-space positions of the selected overlay's handles, keyed by
        # handle name ('nw', 'n', 'ne', 'e', 'se', 's', 'sw', 'w').
        # Rebuilt on every redraw; used for hit-testing.
        self._handle_positions: dict[str, tuple[float, float]] = {}

        # Active edit drag: "resize" | "move" | None.
        self._edit_mode: Optional[str] = None
        self._active_handle: Optional[str] = None
        # Image-space box of the selected overlay at the start of the drag.
        self._edit_orig_box: Optional[tuple[int, int, int, int]] = None
        # Image-space point under the cursor at the start of a move drag.
        self._edit_move_start_img: Optional[tuple[int, int]] = None
        # Image-space box currently being previewed mid-drag (id -> box).
        self._pending_overlay_box: Optional[tuple[int, int, int, int]] = None

        # Build the canvas + scrollbars.
        self._canvas = tk.Canvas(self, bg="#2b2b2b", cursor="crosshair",
                                 highlightthickness=0)
        hbar = ttk.Scrollbar(self, orient=tk.HORIZONTAL, command=self._canvas.xview)
        vbar = ttk.Scrollbar(self, orient=tk.VERTICAL, command=self._canvas.yview)
        self._canvas.configure(xscrollcommand=hbar.set, yscrollcommand=vbar.set)

        hbar.grid(row=1, column=0, sticky=tk.EW)
        vbar.grid(row=0, column=1, sticky=tk.NS)
        self._canvas.grid(row=0, column=0, sticky=tk.NSEW)
        self.rowconfigure(0, weight=1)
        self.columnconfigure(0, weight=1)

        # Bind events.
        self._canvas.bind("<ButtonPress-1>", self._on_lmb_press)
        self._canvas.bind("<B1-Motion>", self._on_lmb_drag)
        self._canvas.bind("<ButtonRelease-1>", self._on_lmb_release)

        self._canvas.bind("<ButtonPress-2>", self._on_pan_press)
        self._canvas.bind("<B2-Motion>", self._on_pan_drag)
        self._canvas.bind("<ButtonRelease-2>", self._on_pan_release)

        # Space-bar pan (press/release).
        self._space_held = False
        self._canvas.bind("<KeyPress-space>", self._on_space_press)
        self._canvas.bind("<KeyRelease-space>", self._on_space_release)
        self._canvas.bind("<ButtonPress-1>", self._on_lmb_press, add=True)  # re-routed below

        self._canvas.bind("<MouseWheel>", self._on_scroll_win)      # Windows / macOS
        self._canvas.bind("<Button-4>", self._on_scroll_linux_up)   # Linux up
        self._canvas.bind("<Button-5>", self._on_scroll_linux_down) # Linux down

        self._canvas.bind("<Double-Button-1>", self._on_double_click)
        self._canvas.bind("<Configure>", self._on_canvas_resize)
        self._canvas.bind("<Motion>", self._on_hover_motion)
        self._canvas.focus_set()

    # ── Public API ────────────────────────────────────────────────────────────

    def load_image(self, image: "Image.Image") -> None:
        """Display *image* and reset zoom/pan to fit."""
        self._pil_image = image
        self._zoom = 1.0
        self._overlays = []
        self._selected_overlay_id = None
        self.after_idle(self._fit_to_canvas)

    def clear(self) -> None:
        """Remove the current image and all overlays."""
        self._pil_image = None
        self._tk_image = None
        self._overlays = []
        self._selected_overlay_id = None
        self._canvas.delete("all")

    def set_overlays(self, rects: list[tuple[object, int, int, int, int, str]]) -> None:
        """
        Replace overlay rectangles.

        Each entry is ``(id, x, y, width, height, colour)`` in image-space
        pixels. ``id`` identifies the overlay (e.g. the glyph's character) and
        is what gets passed back through ``on_overlay_edit``/``on_overlay_select``.
        Call this after loading an image; overlays are redrawn with the image.
        """
        self._overlays = list(rects)
        valid_ids = {o[0] for o in self._overlays}
        if self._selected_overlay_id not in valid_ids:
            self._selected_overlay_id = None
        self._redraw()

    def select_overlay(self, overlay_id: Optional[object]) -> None:
        """Select (or, with ``None``, deselect) an overlay by id."""
        valid_ids = {o[0] for o in self._overlays}
        self._selected_overlay_id = overlay_id if overlay_id in valid_ids else None
        self._redraw_overlays()

    # ── Drawing ───────────────────────────────────────────────────────────────

    def _fit_to_canvas(self) -> None:
        """Scale the image so it fits the current canvas size, centred."""
        if self._pil_image is None:
            return
        cw = self._canvas.winfo_width() or 1
        ch = self._canvas.winfo_height() or 1
        iw, ih = self._pil_image.size
        scale = min(cw / iw, ch / ih, 1.0)   # never upscale beyond 1:1 on first fit
        self._zoom = scale
        self._offset_x = (cw - iw * scale) / 2
        self._offset_y = (ch - ih * scale) / 2
        self._redraw()

    def _redraw(self) -> None:
        """Rebuild the canvas: image, grid, then overlays."""
        if self._pil_image is None:
            return

        self._canvas.delete("image")

        iw, ih = self._pil_image.size
        nw = max(1, int(iw * self._zoom))
        nh = max(1, int(ih * self._zoom))

        resized = self._pil_image.resize((nw, nh), Image.NEAREST)
        self._tk_image = ImageTk.PhotoImage(resized)

        ox, oy = int(self._offset_x), int(self._offset_y)
        self._canvas.create_image(ox, oy, anchor=tk.NW, image=self._tk_image, tags="image")

        # Scroll region encompasses the image plus some breathing room.
        pad = 40
        self._canvas.configure(
            scrollregion=(
                min(ox, 0) - pad, min(oy, 0) - pad,
                max(ox + nw, self._canvas.winfo_width()) + pad,
                max(oy + nh, self._canvas.winfo_height()) + pad,
            )
        )

        self._draw_grid()
        self._redraw_overlays()

    def _redraw_overlays(self) -> None:
        """
        Redraw only the overlay rectangles + selection handles, leaving the
        (expensive-to-rebuild) base image and grid untouched. This is what
        runs on every mouse-move while resizing/moving a box, so it needs to
        stay cheap.
        """
        self._canvas.delete("overlay")
        self._canvas.delete("handle")

        # The selected overlay is drawn last (on top) and, if a drag is in
        # progress, uses the live pending box instead of its committed bounds.
        others = [o for o in self._overlays if o[0] != self._selected_overlay_id]
        selected = next((o for o in self._overlays if o[0] == self._selected_overlay_id), None)

        for (_id, x, y, w, h, colour) in others:
            cx0, cy0 = self._img_to_canvas(x, y)
            cx1, cy1 = self._img_to_canvas(x + w, y + h)
            self._canvas.create_rectangle(
                cx0, cy0, cx1, cy1,
                outline=colour, width=1, fill="", tags="overlay",
            )

        if selected is not None:
            _id, x, y, w, h, colour = selected
            if self._pending_overlay_box is not None:
                x, y, w, h = self._pending_overlay_box
            cx0, cy0 = self._img_to_canvas(x, y)
            cx1, cy1 = self._img_to_canvas(x + w, y + h)
            self._canvas.create_rectangle(
                cx0, cy0, cx1, cy1,
                outline=_SELECTED_COLOUR, width=2, fill="", tags="overlay",
            )
            self._draw_handles(x, y, w, h)

    def _draw_handles(self, x: int, y: int, w: int, h: int) -> None:
        """Draw resize handles for the box at image-space (x, y, w, h)."""
        cx0, cy0 = self._img_to_canvas(x, y)
        cx1, cy1 = self._img_to_canvas(x + w, y + h)
        mx, my = (cx0 + cx1) / 2, (cy0 + cy1) / 2

        self._handle_positions = {
            "nw": (cx0, cy0), "n": (mx, cy0), "ne": (cx1, cy0),
            "w": (cx0, my), "e": (cx1, my),
            "sw": (cx0, cy1), "s": (mx, cy1), "se": (cx1, cy1),
        }
        r = _HANDLE_SIZE_PX / 2
        for (hx, hy) in self._handle_positions.values():
            self._canvas.create_rectangle(
                hx - r, hy - r, hx + r, hy + r,
                outline=_HANDLE_OUTLINE, fill=_HANDLE_FILL, tags="handle",
            )

    def _draw_grid(self) -> None:
        """Draw a 1px-per-pixel grid over the visible portion of the image."""
        self._canvas.delete("grid")
        if self._pil_image is None or self._zoom < _GRID_MIN_ZOOM:
            return

        iw, ih = self._pil_image.size

        # Only draw within the visible viewport, clamped to the image bounds.
        vx0 = self._canvas.canvasx(0)
        vy0 = self._canvas.canvasy(0)
        vx1 = vx0 + self._canvas.winfo_width()
        vy1 = vy0 + self._canvas.winfo_height()

        ix0, iy0 = self._canvas_to_img(vx0, vy0)
        ix1, iy1 = self._canvas_to_img(vx1, vy1)
        ix0 = max(0, int(ix0))
        iy0 = max(0, int(iy0))
        ix1 = min(iw, int(ix1) + 1)
        iy1 = min(ih, int(iy1) + 1)

        top = self._offset_y + iy0 * self._zoom
        bottom = self._offset_y + iy1 * self._zoom
        for gx in range(ix0, ix1 + 1):
            cx = self._offset_x + gx * self._zoom
            self._canvas.create_line(cx, top, cx, bottom, fill=_GRID_COLOUR, tags="grid")

        left = self._offset_x + ix0 * self._zoom
        right = self._offset_x + ix1 * self._zoom
        for gy in range(iy0, iy1 + 1):
            cy = self._offset_y + gy * self._zoom
            self._canvas.create_line(left, cy, right, cy, fill=_GRID_COLOUR, tags="grid")

    # ── Coordinate helpers ────────────────────────────────────────────────────

    def _img_to_canvas(self, ix: float, iy: float) -> tuple[float, float]:
        """Convert image-space pixel coords to canvas coords."""
        return self._offset_x + ix * self._zoom, self._offset_y + iy * self._zoom

    def _canvas_to_img(self, cx: float, cy: float) -> tuple[float, float]:
        """Convert canvas coords to image-space pixel coords."""
        return (cx - self._offset_x) / self._zoom, (cy - self._offset_y) / self._zoom

    def _clamp_img(self, x: float, y: float) -> tuple[int, int]:
        """Clamp image-space coords to the image bounds and return as ints."""
        if self._pil_image is None:
            return 0, 0
        iw, ih = self._pil_image.size
        return int(max(0, min(x, iw))), int(max(0, min(y, ih)))

    def _snap_to_pixel_img(self, cx: float, cy: float) -> tuple[int, int]:
        """Snap canvas coords to the nearest image-pixel boundary (image space)."""
        ix, iy = self._canvas_to_img(cx, cy)
        ix, iy = round(ix), round(iy)
        if self._pil_image is not None:
            iw, ih = self._pil_image.size
            ix = max(0, min(ix, iw))
            iy = max(0, min(iy, ih))
        return ix, iy

    def _snap_to_pixel(self, cx: float, cy: float) -> tuple[float, float]:
        """
        Snap canvas coords to the nearest image-pixel boundary, returned back
        in canvas coords. Used so drag selections land exactly on pixel edges.
        """
        return self._img_to_canvas(*self._snap_to_pixel_img(cx, cy))

    def _handle_at_canvas_point(self, cx: float, cy: float) -> Optional[str]:
        """Return the handle name under (cx, cy), if any, for the selected overlay."""
        for name, (hx, hy) in self._handle_positions.items():
            if abs(cx - hx) <= _HANDLE_HIT_PX and abs(cy - hy) <= _HANDLE_HIT_PX:
                return name
        return None

    def _point_in_box(
        self, ix: float, iy: float, x: int, y: int, w: int, h: int, buf: float = 0.0
    ) -> bool:
        return (x - buf) <= ix <= (x + w + buf) and (y - buf) <= iy <= (y + h + buf)

    def _overlay_at_canvas_point(
        self, cx: float, cy: float
    ) -> Optional[tuple[object, int, int, int, int, str]]:
        """
        Return the topmost overlay whose box contains (cx, cy), if any.

        Grows each box by ``_OVERLAY_HIT_BUFFER_PX`` canvas pixels on every
        side so small or thin glyph boxes are easier to click on.
        """
        ix, iy = self._canvas_to_img(cx, cy)
        buf = _OVERLAY_HIT_BUFFER_PX / self._zoom if self._zoom else 0
        for overlay in reversed(self._overlays):
            _id, x, y, w, h, _colour = overlay
            if self._point_in_box(ix, iy, x, y, w, h, buf):
                return overlay
        return None

    # ── Rubber-band selection ─────────────────────────────────────────────────

    def _on_lmb_press(self, event: tk.Event) -> None:
        if self._space_held:
            # Hand off to pan.
            self._on_pan_press(event)
            return
        self._canvas.focus_set()

        # 1) Dragging a handle of the currently-selected overlay?
        if self._selected_overlay_id is not None:
            handle = self._handle_at_canvas_point(event.x, event.y)
            if handle is not None:
                self._begin_resize(handle)
                return

            # 2) Dragging inside the currently-selected overlay (move)?
            selected = self._get_overlay(self._selected_overlay_id)
            if selected is not None:
                ix, iy = self._canvas_to_img(event.x, event.y)
                _id, x, y, w, h, _colour = selected
                buf = _OVERLAY_HIT_BUFFER_PX / self._zoom if self._zoom else 0
                if self._point_in_box(ix, iy, x, y, w, h, buf):
                    self._begin_move(event)
                    return

        # 3) Clicking a (different) overlay selects it, without starting a
        #    new rubber-band drag.
        hit = self._overlay_at_canvas_point(event.x, event.y)
        if hit is not None:
            self._select_overlay(hit[0])
            return

        # 4) Empty canvas: deselect, then start a normal rubber-band drag.
        if self._selected_overlay_id is not None:
            self._select_overlay(None)

        cx, cy = self._snap_to_pixel(event.x, event.y)
        self._drag_start_cx = cx
        self._drag_start_cy = cy
        self._dragging = True
        if self._rubberband_id is not None:
            self._canvas.delete(self._rubberband_id)
            self._rubberband_id = None

    def _on_lmb_drag(self, event: tk.Event) -> None:
        if self._space_held:
            self._on_pan_drag(event)
            return
        if self._edit_mode is not None:
            self._update_edit_drag(event)
            return
        if not self._dragging:
            return
        cx, cy = self._snap_to_pixel(event.x, event.y)
        if self._rubberband_id is not None:
            self._canvas.delete(self._rubberband_id)
        self._rubberband_id = self._canvas.create_rectangle(
            self._drag_start_cx, self._drag_start_cy, cx, cy,
            outline=_RUBBERBAND_COLOUR, width=2, dash=(4, 2),
        )

    def _on_lmb_release(self, event: tk.Event) -> None:
        if self._space_held:
            self._on_pan_release(event)
            return
        if self._edit_mode is not None:
            self._finish_edit_drag()
            return
        if not self._dragging:
            return
        self._dragging = False

        if self._rubberband_id is not None:
            self._canvas.delete(self._rubberband_id)
            self._rubberband_id = None

        cx, cy = self._snap_to_pixel(event.x, event.y)

        dx = abs(cx - self._drag_start_cx)
        dy = abs(cy - self._drag_start_cy)
        if dx < _MIN_DRAG_PX or dy < _MIN_DRAG_PX:
            return  # Too small — ignore.

        # Convert to image space.
        ix0, iy0 = self._canvas_to_img(
            min(self._drag_start_cx, cx),
            min(self._drag_start_cy, cy),
        )
        ix1, iy1 = self._canvas_to_img(
            max(self._drag_start_cx, cx),
            max(self._drag_start_cy, cy),
        )
        ix0, iy0 = self._clamp_img(ix0, iy0)
        ix1, iy1 = self._clamp_img(ix1, iy1)

        w = ix1 - ix0
        h = iy1 - iy0
        if w < 1 or h < 1:
            return

        if self._on_selection is not None:
            self._on_selection(ix0, iy0, w, h)

    # ── Overlay selection / editing ───────────────────────────────────────────

    def _get_overlay(self, overlay_id: object) -> Optional[tuple[object, int, int, int, int, str]]:
        return next((o for o in self._overlays if o[0] == overlay_id), None)

    def _select_overlay(self, overlay_id: Optional[object]) -> None:
        self._selected_overlay_id = overlay_id
        self._redraw_overlays()
        if self._on_overlay_select is not None:
            self._on_overlay_select(overlay_id)

    def _begin_resize(self, handle: str) -> None:
        selected = self._get_overlay(self._selected_overlay_id)
        if selected is None:
            return
        _id, x, y, w, h, _colour = selected
        self._edit_mode = "resize"
        self._active_handle = handle
        self._edit_orig_box = (x, y, w, h)
        self._pending_overlay_box = (x, y, w, h)

    def _begin_move(self, event: tk.Event) -> None:
        selected = self._get_overlay(self._selected_overlay_id)
        if selected is None:
            return
        _id, x, y, w, h, _colour = selected
        self._edit_mode = "move"
        self._edit_orig_box = (x, y, w, h)
        self._edit_move_start_img = self._snap_to_pixel_img(event.x, event.y)
        self._pending_overlay_box = (x, y, w, h)

    def _update_edit_drag(self, event: tk.Event) -> None:
        if self._edit_orig_box is None:
            return
        ox, oy, ow, oh = self._edit_orig_box
        iw_img, ih_img = (self._pil_image.size if self._pil_image is not None else (0, 0))
        px, py = self._snap_to_pixel_img(event.x, event.y)

        if self._edit_mode == "resize":
            left, top, right, bottom = ox, oy, ox + ow, oy + oh
            handle = self._active_handle or ""
            if "w" in handle:
                left = px
            if "e" in handle:
                right = px
            if "n" in handle:
                top = py
            if "s" in handle:
                bottom = py

            x0, x1 = sorted((left, right))
            y0, y1 = sorted((top, bottom))
            new_x, new_y = self._clamp_img(x0, y0)
            new_x1, new_y1 = self._clamp_img(x1, y1)
            new_w = max(1, new_x1 - new_x)
            new_h = max(1, new_y1 - new_y)
            self._pending_overlay_box = (new_x, new_y, new_w, new_h)

        elif self._edit_mode == "move":
            start_x, start_y = self._edit_move_start_img
            dx = px - start_x
            dy = py - start_y
            new_x = max(0, min(ox + dx, iw_img - ow))
            new_y = max(0, min(oy + dy, ih_img - oh))
            self._pending_overlay_box = (int(new_x), int(new_y), ow, oh)

        self._redraw_overlays()

    def _finish_edit_drag(self) -> None:
        overlay_id = self._selected_overlay_id
        box = self._pending_overlay_box
        self._edit_mode = None
        self._active_handle = None
        self._edit_orig_box = None
        self._edit_move_start_img = None
        self._pending_overlay_box = None

        if overlay_id is None or box is None:
            return
        x, y, w, h = box

        # Update the local overlay entry immediately so the box doesn't
        # snap back before the caller pushes a fresh set_overlays().
        for i, o in enumerate(self._overlays):
            if o[0] == overlay_id:
                self._overlays[i] = (overlay_id, x, y, w, h, o[5])
                break
        self._redraw_overlays()

        if self._on_overlay_edit is not None:
            self._on_overlay_edit(overlay_id, x, y, w, h)

    def _on_hover_motion(self, event: tk.Event) -> None:
        if self._edit_mode is not None or self._dragging or self._panning or self._space_held:
            return
        if self._selected_overlay_id is not None:
            handle = self._handle_at_canvas_point(event.x, event.y)
            if handle is not None:
                self._canvas.configure(cursor=_HANDLE_CURSORS.get(handle, "crosshair"))
                return
            selected = self._get_overlay(self._selected_overlay_id)
            if selected is not None:
                ix, iy = self._canvas_to_img(event.x, event.y)
                _id, x, y, w, h, _colour = selected
                buf = _OVERLAY_HIT_BUFFER_PX / self._zoom if self._zoom else 0
                if self._point_in_box(ix, iy, x, y, w, h, buf):
                    self._canvas.configure(cursor="fleur")
                    return
        self._canvas.configure(cursor="crosshair")

    # ── Pan ───────────────────────────────────────────────────────────────────

    def _on_pan_press(self, event: tk.Event) -> None:
        self._pan_start_x = event.x
        self._pan_start_y = event.y
        self._panning = True
        self._canvas.configure(cursor="fleur")

    def _on_pan_drag(self, event: tk.Event) -> None:
        if not self._panning:
            return
        dx = event.x - self._pan_start_x
        dy = event.y - self._pan_start_y
        self._pan_start_x = event.x
        self._pan_start_y = event.y
        self._offset_x += dx
        self._offset_y += dy
        self._redraw()

    def _on_pan_release(self, _event: tk.Event) -> None:
        self._panning = False
        self._canvas.configure(cursor="crosshair")

    def _on_space_press(self, _event: tk.Event) -> None:
        if not self._space_held:
            self._space_held = True
            self._canvas.configure(cursor="fleur")

    def _on_space_release(self, _event: tk.Event) -> None:
        self._space_held = False
        self._canvas.configure(cursor="crosshair")

    # ── Zoom ──────────────────────────────────────────────────────────────────

    def _zoom_at(self, cx: float, cy: float, factor: float) -> None:
        """Zoom by *factor* keeping the point at (cx, cy) stationary."""
        new_zoom = max(_ZOOM_MIN, min(_ZOOM_MAX, self._zoom * factor))
        if new_zoom == self._zoom:
            return
        # Adjust offset so the point under the cursor stays fixed.
        self._offset_x = cx - (cx - self._offset_x) * (new_zoom / self._zoom)
        self._offset_y = cy - (cy - self._offset_y) * (new_zoom / self._zoom)
        self._zoom = new_zoom
        self._redraw()

    def _on_scroll_win(self, event: tk.Event) -> None:
        factor = _ZOOM_STEP if event.delta > 0 else 1.0 / _ZOOM_STEP
        self._zoom_at(event.x, event.y, factor)

    def _on_scroll_linux_up(self, event: tk.Event) -> None:
        self._zoom_at(event.x, event.y, _ZOOM_STEP)

    def _on_scroll_linux_down(self, event: tk.Event) -> None:
        self._zoom_at(event.x, event.y, 1.0 / _ZOOM_STEP)

    def _on_double_click(self, _event: tk.Event) -> None:
        """Double-click: zoom to fit."""
        self._fit_to_canvas()

    def _on_canvas_resize(self, _event: tk.Event) -> None:
        self._redraw()
