"""
detection.py
------------
Sprite cell detection via projection profiling.

All functions are pure (no GUI / side effects) and operate on numpy arrays.
"""

from __future__ import annotations
import numpy as np
from dataclasses import dataclass
from PIL import Image


# ---------------------------------------------------------------------------
# Data types
# ---------------------------------------------------------------------------

@dataclass(frozen=True)
class CellRect:
    """
    A detected sprite cell.  All coordinates are absolute in the full image.

    In auto mode, x/y/w/h are the tight content bounding box of the sprite.
    In fixed mode, x/y/w/h are the uniform grid cell.
    canvas_w / canvas_h are kept for fixed-grid mode; in auto mode they
    equal w/h since there is no padding.
    """
    col: int
    row: int
    x: int          # absolute top-left x in full image
    y: int          # absolute top-left y in full image
    w: int          # cell width
    h: int          # cell height
    canvas_w: int   # sprite width written to XML (== w for auto mode)
    canvas_h: int   # sprite height written to XML (== h for auto mode)


# ---------------------------------------------------------------------------
# Background helpers
# ---------------------------------------------------------------------------

def detect_bg_colour(arr: np.ndarray) -> np.ndarray:
    """
    Estimate the background colour by taking the median of all border pixels.
    Returns a 1-D array of length C (channels).
    """
    border = np.concatenate([
        arr[:5,  :].reshape(-1, arr.shape[2]),
        arr[-5:, :].reshape(-1, arr.shape[2]),
        arr[:,  :5].reshape(-1, arr.shape[2]),
        arr[:, -5:].reshape(-1, arr.shape[2]),
    ])
    return np.median(border, axis=0)


def make_bg_mask(
    arr: np.ndarray,
    bg: np.ndarray,
    tolerance: float,
) -> np.ndarray:
    """
    Return a boolean mask (H, W) that is True where the pixel is within
    `tolerance` (per-channel) of `bg`.
    """
    return (np.abs(arr.astype(float) - bg).max(axis=2) < tolerance)


# ---------------------------------------------------------------------------
# Gap / edge helpers  (used by fixed-mode row detection only)
# ---------------------------------------------------------------------------

def find_gaps(
    fractions: np.ndarray,
    threshold: float,
    min_size: int,
) -> list[tuple[int, int]]:
    """
    Given a 1-D array of background fractions per row (or column), return
    a list of (start, end) inclusive index ranges where the fraction is at or
    above `threshold` for at least `min_size` consecutive positions.
    """
    gaps: list[tuple[int, int]] = []
    in_gap = False
    start = 0
    for i, v in enumerate(fractions):
        if v >= threshold and not in_gap:
            in_gap, start = True, i
        elif v < threshold and in_gap:
            in_gap = False
            if (i - start) >= min_size:
                gaps.append((start, i - 1))
    if in_gap and (len(fractions) - start) >= min_size:
        gaps.append((start, len(fractions) - 1))
    return gaps


def gaps_to_edges(gaps: list[tuple[int, int]], total: int) -> list[int]:
    """
    Convert gap ranges to boundary pixel positions.
    Returns [0, mid_of_gap_1, ..., total-1].
    """
    edges = [0]
    for s, e in gaps:
        edges.append((s + e) // 2)
    edges.append(total - 1)
    return edges


# ---------------------------------------------------------------------------
# Connected-component sprite detection
# ---------------------------------------------------------------------------

def _label_components(content_mask: np.ndarray) -> tuple[np.ndarray, int]:
    """
    Simple flood-fill connected-component labelling on a boolean mask.
    Returns (label_array, num_labels) where label 0 is background.
    Uses a queue-based BFS to avoid Python recursion limits.
    """
    from collections import deque

    h, w = content_mask.shape
    labels = np.zeros((h, w), dtype=np.int32)
    current_label = 0

    for start_y in range(h):
        for start_x in range(w):
            if not content_mask[start_y, start_x] or labels[start_y, start_x]:
                continue
            current_label += 1
            queue: deque[tuple[int, int]] = deque()
            queue.append((start_y, start_x))
            labels[start_y, start_x] = current_label
            while queue:
                cy, cx = queue.popleft()
                for ny, nx in (
                    (cy - 1, cx), (cy + 1, cx),
                    (cy, cx - 1), (cy, cx + 1),
                ):
                    if (0 <= ny < h and 0 <= nx < w
                            and content_mask[ny, nx]
                            and not labels[ny, nx]):
                        labels[ny, nx] = current_label
                        queue.append((ny, nx))

    return labels, current_label


# ---------------------------------------------------------------------------
# Main detection entry points
# ---------------------------------------------------------------------------

def detect_cells_auto(
    image: Image.Image,
    bg_colour: np.ndarray,
    bg_tolerance: float,
    row_gap_thresh: float,
    min_row_gap: int,
    col_gap_thresh: float,
    min_col_gap: int,
    min_cells: int = 4,
    min_cell_h: int = 30,
    min_avg_cell_w: int = 30,
    min_sprite_area: int = 64,
) -> list[CellRect]:
    """
    Auto-detect sprite cells using connected-component analysis on the
    content (non-background) pixels.

    Each connected component becomes one CellRect whose x/y/w/h is the
    tight bounding box of that component.  Small noise components below
    `min_sprite_area` pixels are discarded.

    Returns a flat list of CellRect sorted top-to-bottom, left-to-right,
    with row/col indices assigned by position.
    """
    arr = np.array(image.convert("RGB"))
    mask = make_bg_mask(arr, bg_colour, bg_tolerance)
    content = ~mask   # True where there is a sprite pixel

    labels, num_labels = _label_components(content)

    raw_bboxes: list[tuple[int, int, int, int]] = []  # (x, y, x2, y2)
    for label in range(1, num_labels + 1):
        ys, xs = np.where(labels == label)
        area = len(ys)
        if area < min_sprite_area:
            continue
        x0, y0 = int(xs.min()), int(ys.min())
        x1, y1 = int(xs.max()) + 1, int(ys.max()) + 1
        raw_bboxes.append((x0, y0, x1, y1))

    if not raw_bboxes:
        return []

    # Sort top-to-bottom, left-to-right
    raw_bboxes.sort(key=lambda b: (b[1], b[0]))

    # Assign row/col by clustering Y centres into rows.
    # Two boxes are in the same row if their Y-centre overlap within
    # a tolerance of half the median box height.
    heights = [b[3] - b[1] for b in raw_bboxes]
    median_h = float(np.median(heights)) if heights else 1.0
    row_tol = median_h * 0.6

    cells: list[CellRect] = []
    current_row = 0
    row_y_centre = (raw_bboxes[0][1] + raw_bboxes[0][3]) / 2.0
    col = 0

    for x0, y0, x1, y1 in raw_bboxes:
        yc = (y0 + y1) / 2.0
        if abs(yc - row_y_centre) > row_tol:
            current_row += 1
            row_y_centre = yc
            col = 0

        w, h = x1 - x0, y1 - y0
        cells.append(CellRect(
            col=col, row=current_row,
            x=x0, y=y0,
            w=w, h=h,
            canvas_w=w,
            canvas_h=h,
        ))
        col += 1

    return cells


def detect_cells_fixed(
    image: Image.Image,
    cell_w: int,
    cell_h: int,
    margin: int,
) -> list[CellRect]:
    """
    Divide the image into a uniform grid of cell_w x cell_h cells,
    optionally with a per-edge margin offset.
    Returns a flat list of CellRect in row-major order.
    """
    iw, ih = image.size
    cells: list[CellRect] = []
    row = 0
    y = margin
    while y + cell_h <= ih:
        col = 0
        x = margin
        while x + cell_w <= iw:
            cells.append(CellRect(
                col=col, row=row,
                x=x, y=y,
                w=cell_w, h=cell_h,
                canvas_w=cell_w,
                canvas_h=cell_h,
            ))
            x += cell_w
            col += 1
        y += cell_h
        row += 1
    return cells
