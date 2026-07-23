"""
assets.py
---------
Parses assets.yaml to extract texture entries.
"""

from __future__ import annotations
from dataclasses import dataclass
from pathlib import Path
import yaml


@dataclass(frozen=True)
class TextureEntry:
    name: str      # Asset key, e.g. "PLAYER_SPRITESHEET"
    source: str    # Relative path, e.g. "sprites/characters/players.png"


def load_texture_entries(yaml_path: Path) -> list[TextureEntry]:
    """
    Parse an assets.yaml file and return all entries from the `textures` group.
    Returns an empty list if the group is absent or the file is malformed.
    """
    with open(yaml_path, "r", encoding="utf-8") as f:
        data = yaml.safe_load(f)

    textures = data.get("textures", [])
    if not isinstance(textures, list):
        return []

    entries: list[TextureEntry] = []
    for item in textures:
        if isinstance(item, dict) and "name" in item and "source" in item:
            entries.append(TextureEntry(
                name=str(item["name"]),
                source=str(item["source"]),
            ))
    return entries


def resolve_texture_path(yaml_path: Path, source: str) -> Path:
    """
    Resolve a texture source path relative to the assets.yaml location.
    """
    return yaml_path.parent / source
