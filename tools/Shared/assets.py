"""
shared/assets.py
----------------
Parses an ``assets.yaml`` file to discover texture sources.

Expected YAML structure::

    textures:
      - name: PLAYER_SPRITESHEET
        source: sprites/characters/players.png

PyYAML is used when available; a minimal hand-rolled fallback handles the
specific two-level structure above when PyYAML is not installed.
"""

from __future__ import annotations

from pathlib import Path
from typing import Optional

try:
    import yaml as _yaml  # type: ignore[import]
    _HAS_YAML = True
except ImportError:
    _HAS_YAML = False


# ── YAML loading ──────────────────────────────────────────────────────────────

def _parse_yaml_fallback(path: Path) -> dict:
    result: dict = {}
    current_key: Optional[str] = None
    current_list: list[dict] = []
    current_item: Optional[dict] = None

    with open(path, encoding="utf-8") as fh:
        for raw_line in fh:
            line = raw_line.rstrip()
            if not line or line.lstrip().startswith("#"):
                continue

            indent = len(raw_line) - len(raw_line.lstrip())
            content = line.strip()

            if indent == 0 and content.endswith(":"):
                if current_key is not None:
                    if current_item is not None:
                        current_list.append(current_item)
                        current_item = None
                    result[current_key] = current_list
                current_key = content[:-1]
                current_list = []

            elif indent == 2 and content.startswith("- "):
                if current_item is not None:
                    current_list.append(current_item)
                rest = content[2:]
                current_item = {}
                if ":" in rest:
                    k, _, v = rest.partition(":")
                    current_item[k.strip()] = v.strip()

            elif indent >= 4 and current_item is not None and ":" in content:
                k, _, v = content.partition(":")
                current_item[k.strip()] = v.strip()

    if current_key is not None:
        if current_item is not None:
            current_list.append(current_item)
        result[current_key] = current_list

    return result


def load_assets_yaml(yaml_path: Path) -> dict:
    """Load and parse ``assets.yaml``, returning the full document as a dict."""
    if _HAS_YAML:
        with open(yaml_path, encoding="utf-8") as fh:
            return _yaml.safe_load(fh) or {}
    return _parse_yaml_fallback(yaml_path)


def get_texture_list(assets: dict) -> list[dict]:
    """Return the list of texture entries (each has ``name`` and ``source``)."""
    return assets.get("textures", [])


def resolve_texture_path(yaml_path: Path, source: str) -> Path:
    """Resolve a texture ``source`` string relative to ``assets.yaml``."""
    return (yaml_path.parent / source).resolve()
