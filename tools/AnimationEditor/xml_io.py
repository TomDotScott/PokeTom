"""
xml_io.py
---------
Serialises an AnimDict to the AnimDict XML format.
Deserialisation is intentionally not implemented (write-only for now).
"""

from __future__ import annotations
from pathlib import Path
from xml.etree.ElementTree import Element, SubElement, ElementTree, indent
import xml.etree.ElementTree as ET

from models import AnimDict, Animation, Frame, SheetRegion
from config import XML_INDENT


def _frame_to_element(frame: Frame) -> Element:
    attribs: dict[str, str] = {
        "topLeftX":    str(frame.top_left_x),
        "topLeftY":    str(frame.top_left_y),
        "duration":    str(frame.duration),
        "spriteWidth": str(frame.sprite_width),
        "spriteHeight": str(frame.sprite_height),
    }
    # Append any extra attributes (flippedHorizontal, etc.) in insertion order
    attribs.update(frame.extra_attrs)
    return Element("Frame", attribs)


def _animation_to_element(anim: Animation) -> Element:
    attribs: dict[str, str] = {
        "name":    anim.name,
        "looping": str(anim.looping).lower(),
        "anchor":  str(int(anim.anchor)),
    }
    if not anim.looping and anim.on_end:
        attribs["onEnd"] = anim.on_end

    elem = Element("Animation", attribs)
    for frame in anim.frames:
        elem.append(_frame_to_element(frame))
    return elem


def _region_to_element(region: SheetRegion) -> Element:
    return Element("Image", {
        "source":        region.source,
        "topLeftX":      str(region.top_left_x),
        "topLeftY":      str(region.top_left_y),
        "regionWidth":   str(region.region_width),
        "regionHeight":  str(region.region_height),
        "maskColour":    region.mask_colour,
    })


def serialise(anim_dict: AnimDict) -> str:
    """
    Serialise an AnimDict to a formatted XML string.
    """
    root = Element("AnimDict", {"name": anim_dict.name})

    if anim_dict.region:
        root.append(_region_to_element(anim_dict.region))

    for anim in anim_dict.animations:
        root.append(_animation_to_element(anim))

    # Pretty-print with standard-library indent (Python 3.9+)
    indent(root, space=XML_INDENT)

    tree = ElementTree(root)
    ET.indent(tree, space=XML_INDENT)

    import io
    buf = io.BytesIO()
    tree.write(buf, encoding="utf-8", xml_declaration=True)
    return buf.getvalue().decode("utf-8")


def save(anim_dict: AnimDict, path: Path) -> None:
    """Write the AnimDict XML to `path`."""
    xml_str = serialise(anim_dict)
    path.write_text(xml_str, encoding="utf-8")
