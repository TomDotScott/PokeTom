"""
font_tool/models.py
-------------------
Pure-data classes for the Bitmap Font editor.

JSON mapping
------------
FontData  → top-level object
GlyphData → one entry in the "glyphs" array

Schema::

    {
        "texture":         "UPPER_SNAKE_CASE_STRING",
        "fontArea":        {"x": int, "y": int, "width": int, "height": int},
        "referenceHeight": int,
        "lineHeight":      int,
        "glyphs": [
            {"char": "A", "x": int, "y": int, "width": int, "height": int, "advance": int},
            ...
        ]
    }
"""

from __future__ import annotations

import copy
from dataclasses import dataclass, field
from typing import Optional


@dataclass
class GlyphData:
    """A single character's source rectangle and advance width."""

    char: str          # single Unicode character, e.g. "A"
    x: int             # atlas X (absolute)
    y: int             # atlas Y (absolute)
    width: int
    height: int
    advance: int = -1  # -1 means "use width" at export time

    def resolved_advance(self) -> int:
        """Return ``advance``, falling back to ``width`` when -1."""
        return self.advance if self.advance >= 0 else self.width

    def display_label(self) -> str:
        char_repr = repr(self.char) if self.char in (" ",) else self.char
        return (
            f"{char_repr!s:<4}  ({self.x}, {self.y})"
            f"  {self.width}×{self.height}"
            f"  adv={self.resolved_advance()}"
        )

    def clone(self) -> "GlyphData":
        return copy.deepcopy(self)


@dataclass
class FontArea:
    """The sub-region of the atlas that contains this font's glyphs."""

    x: int = 0
    y: int = 0
    width: int = 0
    height: int = 0


@dataclass
class FontData:
    """Top-level container for a bitmap font definition."""

    texture: str = ""                  # e.g. "PLAYER_SPRITESHEET"
    font_area: FontArea = field(default_factory=FontArea)
    reference_height: int = 8         # pixel height at 1× scale
    line_height: int = 10             # vertical advance per line
    glyphs: list[GlyphData] = field(default_factory=list)

    # ── Helpers ───────────────────────────────────────────────────────────────

    def find_glyph(self, char: str) -> Optional[GlyphData]:
        return next((g for g in self.glyphs if g.char == char), None)

    def add_or_replace(self, glyph: GlyphData) -> None:
        """Insert *glyph*, replacing any existing entry for the same char."""
        for i, g in enumerate(self.glyphs):
            if g.char == glyph.char:
                self.glyphs[i] = glyph
                return
        self.glyphs.append(glyph)

    def remove(self, char: str) -> bool:
        """Remove the glyph for *char*.  Returns True if one was found."""
        before = len(self.glyphs)
        self.glyphs = [g for g in self.glyphs if g.char != char]
        return len(self.glyphs) < before

    def sorted_glyphs(self) -> list[GlyphData]:
        """Return glyphs sorted by Unicode code-point."""
        return sorted(self.glyphs, key=lambda g: ord(g.char) if g.char else 0)
