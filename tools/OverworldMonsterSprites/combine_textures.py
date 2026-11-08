# Generates the pokemon overworld textures from the spritesheets in the rar file here: https://eeveeexpo.com/resources/516/

import argparse
import hashlib
import json
import re
import sys
from collections import Counter
from pathlib import Path

from PIL import Image


Image.MAX_IMAGE_PIXELS = None  # these atlases exceed Pillow's default bomb limit
RESAMPLE = {
    "nearest": Image.NEAREST,
    "box": Image.BOX,
    "bilinear": Image.BILINEAR,
    "lanczos": Image.LANCZOS,
}
GPU_TEXTURE_LIMIT = 16384  # common max texture dimension on desktop GPUs
GRID = 4  # frames per row/column inside each image


def natural_key(s):
    return [int(t) if t.isdigit() else t.lower() for t in re.split(r"(\d+)", s)]


def round_up(value, multiple):
    return -(-value // multiple) * multiple


def load_scaled(path, w, h, downscale, resample):
    """Open as RGBA, downscale to w x h, and zero the RGB of fully transparent pixels.

    Zeroing matches GIMP's default ("save color values from transparent pixels" off): invisible
    pixels all become (0,0,0,0), which compresses better and lets identical sprites compare equal.
    """
    with Image.open(path) as im:
        im = im.convert("RGBA")
        if downscale > 1:
            im = im.resize((w, h), resample)
    im.paste((0, 0, 0, 0), mask=im.getchannel("A").point(lambda v: 255 if v == 0 else 0))
    return im


def save_png(atlas, path, max_colors, recompress):
    """Save like GIMP's default export, then optionally recompress. Returns (plain_bytes, final_bytes)."""
    img = atlas
    if max_colors:
        img = atlas.quantize(colors=max_colors, method=Image.FASTOCTREE, dither=Image.Dither.NONE)
    img.save(path, "PNG", compress_level=9, optimize=True)
    plain = path.stat().st_size
    if recompress:
        data = path.read_bytes()
        out = oxipng.optimize_from_memory(data, level=2, strip=oxipng.StripChunks.all())
        if len(out) < len(data):
            path.write_bytes(out)
    return plain, path.stat().st_size


def build_category(name, folder, columns, cell, align, prefix, out_dir, downscale, resample,
                   dedupe, max_colors, recompress):
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
        if w % (GRID * downscale) or h % (GRID * downscale):
            print(f"WARNING: {p.name} is {w}x{h}, not divisible by {GRID * downscale} "
                  f"(frame grid x downscale); skipped", file=sys.stderr)
            continue
        items.append((p.stem, p, w // downscale, h // downscale))

    # Deduplicate pixel-identical images (after scaling and cleaning); first in name order owns the pixels.
    duplicate_of = {}
    to_place = items
    if dedupe:
        seen, to_place = {}, []
        for item in items:
            stem, path, w, h = item
            digest = hashlib.sha1(load_scaled(path, w, h, downscale, resample).tobytes()).digest()
            key = (w, h, digest)
            if key in seen:
                duplicate_of[stem] = seen[key]
            else:
                seen[key] = stem
                to_place.append(item)

    atlas_w = round_up(columns * cell, align)

    groups = {}
    for item in to_place:
        slot = round_up(max(item[2], item[3]), align)
        groups.setdefault(slot, []).append(item)

    placements = []  # (stem, path, x, y, w, h)
    y = 0
    for slot in sorted(groups):
        per_row = atlas_w // slot
        if per_row < 1:
            sys.exit(f"error: {slot}px slot does not fit in an atlas {atlas_w}px wide")
        group = groups[slot]
        for i, (stem, path, w, h) in enumerate(group):
            placements.append((stem, path, (i % per_row) * slot, y + (i // per_row) * slot, w, h))
        y += -(-len(group) // per_row) * slot

    atlas_h = round_up(y, align)
    atlas = Image.new("RGBA", (atlas_w, atlas_h), (0, 0, 0, 0))
    placed = {}
    for stem, path, x, py, w, h in placements:
        atlas.paste(load_scaled(path, w, h, downscale, resample), (x, py))
        placed[stem] = {"x": x, "y": py, "width": w, "height": h}

    entries = {}
    for stem, _, _, _ in items:  # natural name order
        if stem in duplicate_of:
            entries[stem] = {**placed[duplicate_of[stem]], "duplicate_of": duplicate_of[stem]}
        else:
            entries[stem] = placed[stem]

    source = f"{prefix}{name.upper()}"
    filename = f"{source}.png"
    plain, final = save_png(atlas, out_dir / filename, max_colors, recompress)

    sizes = Counter(f"{w}x{h}" for _, _, _, _, w, h in placements)
    breakdown = ", ".join(f"{n} x {s}" for s, n in sorted(sizes.items()))
    dup_note = f", {len(duplicate_of)} duplicates shared" if dedupe else ""
    print(f"{name}: {len(items)} images ({len(placements)} stored{dup_note}; {breakdown}) "
          f"-> {filename} ({atlas_w}x{atlas_h})")
    print(f"  file size: {final / 1e6:.2f} MB"
          + (f" (level-9 save was {plain / 1e6:.2f} MB)" if final != plain else ""))
    if max(atlas_w, atlas_h) > GPU_TEXTURE_LIMIT:
        print(
            f"WARNING: {filename} exceeds {GPU_TEXTURE_LIMIT}px; many GPUs cannot load it as one texture",
            file=sys.stderr,
        )

    return {
        "file": filename,
        "source": source,
        "downscale": downscale,
        "width": atlas_w,
        "height": atlas_h,
        "entries": entries,
    }


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("-c", "--category", action="append", required=True, metavar="NAME=FOLDER",
                    help="category name and its folder of PNGs; repeat per category")
    ap.add_argument("-o", "--out", type=Path, default=Path("atlases"))
    ap.add_argument("--columns", type=int, default=20, help="standard-size slots per row")
    ap.add_argument("--cell", type=int, default=256,
                    help="standard INPUT image size; atlas width = columns * cell / downscale")
    ap.add_argument("--align", type=int, default=64,
                    help="slot/atlas alignment measured in INPUT pixels (divided by downscale)")
    ap.add_argument("--downscale", type=int, default=2,
                    help="integer factor to shrink every image by (default 2; 1 = no scaling)")
    ap.add_argument("--resample", choices=sorted(RESAMPLE), default="nearest",
                    help="resampling filter used when downscaling (default nearest)")
    ap.add_argument("--no-dedupe", action="store_true",
                    help="store every image separately even if pixel-identical")
    ap.add_argument("--no-recompress", action="store_true",
                    help="skip the lossless oxipng pass even if pyoxipng is installed")
    ap.add_argument("--max-colors", type=int, default=0, metavar="N",
                    help="LOSSY: reduce each atlas to an N-colour palette (2-256, default off)")
    ap.add_argument("--source-prefix", default="OVERWORLD_",
                    help="engine texture name prefix; source = PREFIX + CATEGORY upper-cased")
    args = ap.parse_args()

    if args.downscale < 1:
        sys.exit("error: --downscale must be >= 1")
    if args.cell % args.downscale or args.align % args.downscale:
        sys.exit(f"error: --cell ({args.cell}) and --align ({args.align}) must be divisible by --downscale")
    cell, align = args.cell // args.downscale, args.align // args.downscale
    if args.max_colors and not 2 <= args.max_colors <= 256:
        sys.exit("error: --max-colors must be between 2 and 256")
    recompress = oxipng is not None and not args.no_recompress
    if oxipng is None and not args.no_recompress:
        print("note: pyoxipng not installed; skipping lossless recompression "
              "(pip install pyoxipng typically cuts PNG size by 35-55%)", file=sys.stderr)

    args.out.mkdir(parents=True, exist_ok=True)
    index = {}
    for spec in args.category:
        if "=" not in spec:
            sys.exit(f"error: --category expects NAME=FOLDER, got {spec!r}")
        name, folder = spec.split("=", 1)
        name = name.strip().lower()
        index[name] = build_category(
            name, Path(folder), args.columns, cell, align, args.source_prefix, args.out,
            args.downscale, RESAMPLE[args.resample],
            not args.no_dedupe, args.max_colors, recompress,
        )

    (args.out / "atlas_index.json").write_text(json.dumps(index, indent=1))
    print(f"wrote {args.out / 'atlas_index.json'}")


if __name__ == "__main__":
    main()