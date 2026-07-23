"""
models.py
---------
Pure-data classes representing the animation dictionary domain model.
No GUI or I/O logic here.
"""

from __future__ import annotations
from dataclasses import dataclass, field
from typing import Optional
from config import Anchor


# ---------------------------------------------------------------------------
# Sprite sheet region
# ---------------------------------------------------------------------------

@dataclass
class SheetRegion:
    """
    Defines the sub-region of a texture atlas that this AnimDict draws from,
    plus the background/mask colour used during cell detection.

    All coordinates are absolute within the full texture.
    """
    source: str          # Asset name key, e.g. "PLAYER_SPRITESHEET"
    top_left_x: int = 0
    top_left_y: int = 0
    region_width: int = 0
    region_height: int = 0
    mask_colour: str = "FFFFFFFF"   # RGBA hex string, e.g. "D3F9BCFF"


# ---------------------------------------------------------------------------
# Cell detection settings
# ---------------------------------------------------------------------------

@dataclass
class DetectionSettings:
    """
    Parameters that control how sprite cells are detected from the sheet.
    Stored so they can be serialised / restored in future if needed.
    """
    fixed_width: bool = False
    cell_width: int = 32
    cell_height: int = 32
    margin: int = 0              # Uniform margin around each cell
    bg_tolerance: int = 30       # Per-channel bg colour tolerance
    col_gap_thresh: float = 0.80
    min_col_gap: int = 10
    row_gap_thresh: float = 0.99
    min_row_gap: int = 3


# ---------------------------------------------------------------------------
# Frame
# ---------------------------------------------------------------------------

@dataclass
class Frame:
    """
    A single frame within an animation.

    top_left_x / top_left_y are absolute coordinates in the full texture.
    sprite_width / sprite_height are the dimensions of the normalised cell
    (i.e. the centred canvas size, not the tight content size).
    extra_attrs stores any additional key/value pairs (e.g. flippedHorizontal).
    """
    top_left_x: int
    top_left_y: int
    duration: int
    sprite_width: int
    sprite_height: int
    extra_attrs: dict[str, str] = field(default_factory=dict)

    def copy(self) -> "Frame":
        return Frame(
            top_left_x=self.top_left_x,
            top_left_y=self.top_left_y,
            duration=self.duration,
            sprite_width=self.sprite_width,
            sprite_height=self.sprite_height,
            extra_attrs=dict(self.extra_attrs),
        )


# ---------------------------------------------------------------------------
# Animation
# ---------------------------------------------------------------------------

@dataclass
class Animation:
    """
    A named, ordered sequence of frames.

    on_end is only written to XML when looping is False.
    anchor is stored as the integer value of the Anchor flag.
    """
    name: str
    looping: bool = True
    anchor: Anchor = Anchor.BM
    frames: list[Frame] = field(default_factory=list)
    on_end: Optional[str] = None    # Target animation name when looping=False

    def copy(self) -> "Animation":
        return Animation(
            name=self.name,
            looping=self.looping,
            anchor=self.anchor,
            frames=[f.copy() for f in self.frames],
            on_end=self.on_end,
        )


# ---------------------------------------------------------------------------
# AnimDict  (top-level document)
# ---------------------------------------------------------------------------

@dataclass
class AnimDict:
    """
    The top-level document: a named collection of animations that all
    reference the same source texture region.
    """
    name: str
    region: Optional[SheetRegion] = None
    animations: list[Animation] = field(default_factory=list)

    def animation_names(self) -> list[str]:
        return [a.name for a in self.animations]

    def get_animation(self, name: str) -> Optional[Animation]:
        for anim in self.animations:
            if anim.name == name:
                return anim
        return None

    def add_animation(self, anim: Animation) -> None:
        if anim.name in self.animation_names():
            raise ValueError(f"Animation name '{anim.name}' already exists.")
        self.animations.append(anim)

    def remove_animation(self, name: str) -> None:
        self.animations = [a for a in self.animations if a.name != name]

    def rename_animation(self, old_name: str, new_name: str) -> None:
        if new_name in self.animation_names():
            raise ValueError(f"Animation name '{new_name}' already exists.")
        anim = self.get_animation(old_name)
        if anim:
            anim.name = new_name
            # Fix any on_end references pointing at the old name
            for a in self.animations:
                if a.on_end == old_name:
                    a.on_end = new_name
