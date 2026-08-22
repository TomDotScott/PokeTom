"""
shared/sheet_canvas.py
----------------------
A reusable Tkinter widget that displays a PIL image and lets the user draw
a rubber-band selection rectangle over it.

Usage::

    canvas = SheetCanvas(parent, on_selection=my_callback)
    canvas.pack(fill=tk.BOTH, expand=True)
    canvas.load_image(pil_image)

    # my_callback receives image-space coordinates:
    def my_callback(x: int, y: int, w: int, h: int) -> None:
        ...

    # Draw coloured overlay rectangles (image-space coords):
    canvas.set_overlays([
        (glyph.x, glyph.y, glyph.width, glyph.height, "#00FF88"),
        ...
    ])

Controls
--------
* **Left-drag**          – draw a rubber-band selection rectangle.
* **Scroll wheel**       – zoom in/out centred on the cursor.
* **Middle-drag / Space+drag** – pan the view.
* **Double-click**       – zoom to fit the full image.
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
    """

    def __init__(
        self,
        parent: tk.Misc,
        on_selection: Optional[Callable[[int, int, int, int], None]] = None,
        **kwargs: object,
    ) -> None:
        super().__init__(parent, **kwargs)

        self._on_selection = on_selection

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

        # Overlays: list of (x, y, w, h, colour) in image-space.
        self._overlays: list[tuple[int, int, int, int, str]] = []
        # IDs of overlay canvas items (rebuilt on every redraw).
        self._overlay_ids: list[int] = []

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
        self._canvas.focus_set()

    # ── Public API ────────────────────────────────────────────────────────────

    def load_image(self, image: "Image.Image") -> None:
        """Display *image* and reset zoom/pan to fit."""
        self._pil_image = image
        self._zoom = 1.0
        self._overlays = []
        self.after_idle(self._fit_to_canvas)

    def clear(self) -> None:
        """Remove the current image and all overlays."""
        self._pil_image = None
        self._tk_image = None
        self._overlays = []
        self._canvas.delete("all")

    def set_overlays(self, rects: list[tuple[int, int, int, int, str]]) -> None:
        """
        Replace overlay rectangles.

        Each entry is ``(x, y, width, height, colour)`` in image-space pixels.
        Call this after loading an image; overlays are redrawn with the image.
        """
        self._overlays = list(rects)
        self._redraw()

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
        """Rebuild the canvas: image then overlays."""
        if self._pil_image is None:
            return

        self._canvas.delete("image")
        self._canvas.delete("overlay")

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

            cx0, cy0 = self._img_to_canvas(x, y)
            cx1, cy1 = self._img_to_canvas(x + w, y + h)
            self._canvas.create_rectangle(
                cx0, cy0, cx1, cy1,
                outline=colour, width=1, fill="", tags="overlay",
            )

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

    # ── Rubber-band selection ─────────────────────────────────────────────────

    def _on_lmb_press(self, event: tk.Event) -> None:
        if self._space_held:
            # Hand off to pan.
            self._on_pan_press(event)
            return
        self._canvas.focus_set()
        self._drag_start_cx = event.x
        self._drag_start_cy = event.y
        self._dragging = True
        if self._rubberband_id is not None:
            self._canvas.delete(self._rubberband_id)
            self._rubberband_id = None

    def _on_lmb_drag(self, event: tk.Event) -> None:
        if self._space_held:
            self._on_pan_drag(event)
            return
        if not self._dragging:
            return
        if self._rubberband_id is not None:
            self._canvas.delete(self._rubberband_id)
        self._rubberband_id = self._canvas.create_rectangle(
            self._drag_start_cx, self._drag_start_cy, event.x, event.y,
            outline=_RUBBERBAND_COLOUR, width=2, dash=(4, 2),
        )

    def _on_lmb_release(self, event: tk.Event) -> None:
        if self._space_held:
            self._on_pan_release(event)
            return
        if not self._dragging:
            return
        self._dragging = False

        if self._rubberband_id is not None:
            self._canvas.delete(self._rubberband_id)
            self._rubberband_id = None

        dx = abs(event.x - self._drag_start_cx)
        dy = abs(event.y - self._drag_start_cy)
        if dx < _MIN_DRAG_PX or dy < _MIN_DRAG_PX:
            return  # Too small — ignore.

        # Convert to image space.
        ix0, iy0 = self._canvas_to_img(
            min(self._drag_start_cx, event.x),
            min(self._drag_start_cy, event.y),
        )
        ix1, iy1 = self._canvas_to_img(
            max(self._drag_start_cx, event.x),
            max(self._drag_start_cy, event.y),
        )
        ix0, iy0 = self._clamp_img(ix0, iy0)
        ix1, iy1 = self._clamp_img(ix1, iy1)

        w = ix1 - ix0
        h = iy1 - iy0
        if w < 1 or h < 1:
            return

        if self._on_selection is not None:
            self._on_selection(ix0, iy0, w, h)

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
