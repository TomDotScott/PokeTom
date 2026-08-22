"""
font_tool/json_io.py
--------------------
Serialises a :class:`~models.FontData` to the project JSON schema and
deserialises it back (for round-trip editing in the future).

Schema (write-only output)::

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

import json
from pathlib import Path

from models import FontData, GlyphData, FontArea


# ── Write ─────────────────────────────────────────────────────────────────────

def font_to_dict(font: FontData) -> dict:
    """Convert a :class:`FontData` to a plain dict matching the JSON schema."""
    return {
        "texture": font.texture,
        "fontArea": {
            "x":      font.font_area.x,
            "y":      font.font_area.y,
            "width":  font.font_area.width,
            "height": font.font_area.height,
        },
        "referenceHeight": font.reference_height,
        "lineHeight":      font.line_height,
        "glyphs": [
            {
                "char":    g.char,
                "x":       g.x,
                "y":       g.y,
                "width":   g.width,
                "height":  g.height,
                "advance": g.resolved_advance(),
            }
            for g in font.sorted_glyphs()
        ],
    }


def save_font(font: FontData, path: Path) -> None:
    """Serialise *font* and write it to *path* as pretty-printed JSON."""
    data = font_to_dict(font)
    path.write_text(json.dumps(data, indent=2, ensure_ascii=False), encoding="utf-8")


# ── Read ──────────────────────────────────────────────────────────────────────

def load_font(path: Path) -> FontData:
    """
    Load a font JSON file and return a :class:`FontData`.

    Tolerant of missing optional fields.
    """
    raw = json.loads(path.read_text(encoding="utf-8"))

    fa_raw = raw.get("fontArea", {})
    font_area = FontArea(
        x=fa_raw.get("x", 0),
        y=fa_raw.get("y", 0),
        width=fa_raw.get("width", 0),
        height=fa_raw.get("height", 0),
    )

    glyphs: list[GlyphData] = []
    for entry in raw.get("glyphs", []):
        char = entry.get("char", "")
        if not char:
            continue
        glyphs.append(GlyphData(
            char=char[0],
            x=entry.get("x", 0),
            y=entry.get("y", 0),
            width=entry.get("width", 0),
            height=entry.get("height", 0),
            advance=entry.get("advance", -1),
        ))

    return FontData(
        texture=raw.get("texture", ""),
        font_area=font_area,
        reference_height=raw.get("referenceHeight", 8),
        line_height=raw.get("lineHeight", 10),
        glyphs=glyphs,
    )
