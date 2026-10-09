# Generates the pokemon overworld textures from the spritesheets in the rar file here: https://eeveeexpo.com/resources/516/

import argparse
import json
import math
import re
import sys
from pathlib import Path

from PIL import Image

# The textures atlases exceed Pillow's default bomb limit
Image.MAX_IMAGE_PIXELS = None  
GPU_TEXTURE_LIMIT = 16384


def natural_key(s):
    return [int(t) if t.isdigit() else t.lower() for t in re.split(r"(\d+)", s)]


def build_category(name, folder, columns, cell, prefix, out_dir):
    files = sorted(
        (p for p in folder.iterdir() if p.is_file() and p.suffix.lower() == ".png"),
        key=lambda p: natural_key(p.stem),
    )
    if not files:
        sys.exit(f"error: no PNG files in {folder}")

    images = []
    for p in files:
        with Image.open(p) as im:
            if im.size != (cell, cell):
                print(
                    f"WARNING: {p.name} is {im.size[0]}x{im.size[1]}, expected {cell}x{cell}; skipped", file=sys.stderr)
                continue
            images.append((p.stem, im.convert("RGBA")))

    rows = math.ceil(len(images) / columns)
    width, height = columns * cell, rows * cell
    atlas = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    entries = {}
    for i, (stem, im) in enumerate(images):
        col, row = i % columns, i // columns
        atlas.paste(im, (col * cell, row * cell))
        entries[stem] = {"col": col, "row": row}

    source = f"{prefix}{name.upper()}"
    filename = f"{source}.png"
    atlas.save(out_dir / filename)

    print(f"{name}: {len(images)} images -> {filename} ({width}x{height})")
    if max(width, height) > GPU_TEXTURE_LIMIT:
        print(
            f"WARNING: {filename} exceeds {GPU_TEXTURE_LIMIT}px; many GPUs cannot load it as one texture "
            f"(try --columns {columns * 2} or split the category)",
            file=sys.stderr,
        )

    return {
        "file": filename,
        "source": source,
        "columns": columns,
        "rows": rows,
        "cell": cell,
        "entries": entries,
    }


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("-c", "--category", action="append", required=True, metavar="NAME=FOLDER",
                    help="category name and its folder of PNGs; repeat per category")
    ap.add_argument("-o", "--out", type=Path, default=Path("atlases"))
    ap.add_argument("--columns", type=int, default=20)
    ap.add_argument("--cell", type=int, default=256,
                    help="size of each input image in pixels")
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
        index[name] = build_category(name, Path(
            folder), args.columns, args.cell, args.source_prefix, args.out)

    (args.out / "atlas_index.json").write_text(json.dumps(index, indent=1))
    print(f"wrote {args.out / 'atlas_index.json'}")


if __name__ == "__main__":
    main()
