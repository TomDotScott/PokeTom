"""
config.py
---------
Global constants and enumerations for the animation tool.
"""

from enum import IntFlag


# ---------------------------------------------------------------------------
# Anchor bit flags
# Each value is a unique bit so they can be combined if needed in future,
# and stored compactly as an integer in the XML.
# ---------------------------------------------------------------------------

class Anchor(IntFlag):
    TL = 1 << 1   # Top-left
    TM = 1 << 2   # Top-middle
    TR = 1 << 3   # Top-right
    ML = 1 << 4   # Middle-left
    MM = 1 << 5   # Middle-middle (centre)
    MR = 1 << 6   # Middle-right
    BL = 1 << 7   # Bottom-left
    BM = 1 << 8   # Bottom-middle
    BR = 1 << 9   # Bottom-right


# Human-readable names for the UI dropdown, in grid order.
ANCHOR_OPTIONS: list[tuple[str, Anchor]] = [
    ("TL", Anchor.TL),
    ("TM", Anchor.TM),
    ("TR", Anchor.TR),
    ("ML", Anchor.ML),
    ("MM", Anchor.MM),
    ("MR", Anchor.MR),
    ("BL", Anchor.BL),
    ("BM", Anchor.BM),
    ("BR", Anchor.BR),
]


# ---------------------------------------------------------------------------
# Tool modes for the sheet panel
# ---------------------------------------------------------------------------

class Tool:
    SELECT     = "select"       # Click cells to add to animation
    EYEDROPPER = "eyedropper"   # Click a pixel to set mask colour
    REGION     = "region"       # Drag to define the image sub-region


# ---------------------------------------------------------------------------
# Misc UI constants
# ---------------------------------------------------------------------------

GRID_COLOUR          = "#FF0000"
GRID_COLOUR_HOVER    = "#FFAA00"
SELECTED_COLOUR      = "#00AAFF"
REGION_COLOUR        = "#FF00FF"
CELL_LABEL_FONT      = ("Courier", 7)

DEFAULT_DURATION_MS  = 100
XML_INDENT           = "  "
