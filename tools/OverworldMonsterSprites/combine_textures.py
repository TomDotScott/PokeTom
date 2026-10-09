# Generates the pokemon overworld textures from the spritesheets in the rar file here: https://eeveeexpo.com/resources/516/

import argparse
import json
import math
import re
import sys
from collections import Counter
from pathlib import Path


from PIL import Image

# The textures atlases exceed Pillow's default bomb limit
Image.MAX_IMAGE_PIXELS = None
GPU_TEXTURE_LIMIT = 16384


GRID = 4  # frames per row/column inside each image


def natural_key(s):
    return [int(t) if t.isdigit() else t.lower() for t in re.split(r"(\d+)", s)]


def round_up(value, multiple):
    return -(-value // multiple) * multiple


def build_category(name, folder, columns, cell, align, prefix, out_dir):
    paths = sorted(
        (p for p in folder.iterdir() if p.is_file() and p.suffix.lower() == ".png"),
        key=lambda p: natural_key(p.stem),
    )
    if not paths:
        sys.exit(f"error: no PNG files in {folder}")

    # First pass: sizes only (images are opened lazily, so this is cheap).
    items = []  # (stem, path, width, height)
    for p in paths:
        with Image.open(p) as im:
            w, h = im.size
        if w % GRID or h % GRID:
            print(
                f"WARNING: {p.name} is {w}x{h}, not divisible by {GRID}; skipped", file=sys.stderr)
            continue
        items.append((p.stem, p, w, h))

    atlas_w = round_up(columns * cell, align)

    groups = {}
    for item in items:
        slot = round_up(max(item[2], item[3]), align)
        groups.setdefault(slot, []).append(item)

    placements = []  # (stem, path, x, y, w, h)
    y = 0
    for slot in sorted(groups):
        per_row = atlas_w // slot
        if per_row < 1:
            sys.exit(
                f"error: {slot}px slot does not fit in an atlas {atlas_w}px wide")
        group = groups[slot]
        for i, (stem, path, w, h) in enumerate(group):
            placements.append((stem, path, (i % per_row) *
                              slot, y + (i // per_row) * slot, w, h))
        y += -(-len(group) // per_row) * slot

    atlas_h = round_up(y, align)
    atlas = Image.new("RGBA", (atlas_w, atlas_h), (0, 0, 0, 0))
    entries = {}
    for stem, path, x, py, w, h in placements:
        with Image.open(path) as im:
            atlas.paste(im.convert("RGBA"), (x, py))
        entries[stem] = {"x": x, "y": py, "width": w, "height": h}

    source = f"{prefix}{name.upper()}"
    filename = f"{source}.png"
    atlas.save(out_dir / filename)

    sizes = Counter(f"{w}x{h}" for _, _, _, _, w, h in placements)
    breakdown = ", ".join(f"{n} x {s}" for s, n in sorted(sizes.items()))
    print(f"{name}: {len(placements)} images ({breakdown}) -> {filename} ({atlas_w}x{atlas_h})")
    if max(atlas_w, atlas_h) > GPU_TEXTURE_LIMIT:
        print(
            f"WARNING: {filename} exceeds {GPU_TEXTURE_LIMIT}px; many GPUs cannot load it as one texture",
            file=sys.stderr,
        )

    return {
        "file": filename,
        "source": source,
        "width": atlas_w,
        "height": atlas_h,
        "entries": entries,
    }


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("-c", "--category", action="append", required=True, metavar="NAME=FOLDER",
                    help="category name and its folder of PNGs; repeat per category")
    ap.add_argument("-o", "--out", type=Path, default=Path("atlases"))
    ap.add_argument("--columns", type=int, default=20,
                    help="standard-size slots per row")
    ap.add_argument("--cell", type=int, default=256,
                    help="standard image size; atlas width = columns * cell")
    ap.add_argument("--align", type=int, default=64,
                    help="slot and atlas dimensions are multiples of this")
    ap.add_argument("--source-prefix", default="OVERWORLD_",
                    help="engine texture name prefix; source = PREFIX + CATEGORY upper-cased")
    args = ap.parse_args()

    args.out.mkdir(parents=True, exist_ok=True)
    index = {}
    for spec in args.category:
        if "=" not in spec:
            sys.exit(f"error: --category expects NAME=FOLDER, got {spec!r}")
        name, folder = spec.split("=", 1)
        name = name.strip().lower()
        index[name] = build_category(
            name,
            Path(folder),
            args.columns,
            args.cell,
            args.align,
            args.source_prefix,
            args.out
        )

    (args.out / "atlas_index.json").write_text(json.dumps(index, indent=1))
    print(f"wrote {args.out / 'atlas_index.json'}")


if __name__ == "__main__":
    main()
