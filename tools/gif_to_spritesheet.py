#!/usr/bin/env python3
"""Build an 8-bit indexed PNG spritesheet grid from one or more animated GIFs.

Every frame is scaled to fit a common cell size.
Frames are taken in input order and laid out left to right, top to bottom.
Palette index 0 is the mask colour and holds every transparent pixel; sprite
pixels are quantized to the remaining 255 palette entries.

Examples:
    gif_to_sprite.py walk.gif run.gif -c 8
    gif_to_sprite.py a.gif b.gif -r 2 -c 10 -m 00FF00 -o sheet.png
"""

import argparse
import math
import sys

from PIL import Image, ImageColor, ImageOps, ImageSequence


def colour(value):
    try:
        return ImageColor.getrgb(value if value.startswith("#") or value.isalpha()
                                 else "#" + value)
    except ValueError:
        raise argparse.ArgumentTypeError(f"bad colour: {value}")


def size_arg(value):
    try:
        w, h = (int(v) for v in value.lower().split("x"))
        if w < 1 or h < 1:
            raise ValueError
        return w, h
    except ValueError:
        raise argparse.ArgumentTypeError(f"bad size (expected WxH): {value}")


def fit(frame, cell):
    """Scale to fit the cell (aspect preserved, nearest), centred on transparent."""
    if frame.size == cell:
        return frame
    fw, fh = frame.size
    scale = min(cell[0] / fw, cell[1] / fh)
    nw, nh = max(1, round(fw * scale)), max(1, round(fh * scale))
    out = Image.new("RGBA", cell, (0, 0, 0, 0))
    out.paste(frame.resize((nw, nh), Image.NEAREST),
              ((cell[0] - nw) // 2, (cell[1] - nh) // 2))
    return out


def load_frames(paths):
    frames = []
    for path in paths:
        with Image.open(path) as im:
            # convert() returns a copy, so frames stay valid after the file closes
            frames.extend(f.convert("RGBA") for f in ImageSequence.Iterator(im))
    return frames


def build_sheet(frames, cols, rows, mask, cell):
    """Return (RGB sheet on mask colour, L sheet that is 255 where opaque)."""
    w, h = cell
    rgb = Image.new("RGB", (w * cols, h * rows), mask)
    opaque = Image.new("L", rgb.size, 0)
    for i, frame in enumerate(frames):
        frame = fit(frame, cell)
        pos = ((i % cols) * w, (i // cols) * h)
        alpha = frame.getchannel("A").point(lambda a: 255 if a >= 128 else 0)
        rgb.paste(frame.convert("RGB"), pos, alpha)
        opaque.paste(alpha, pos)
    return rgb, opaque


def to_indexed(rgb, opaque, mask):
    """Quantize to 255 colours, shift up by one, and put the mask at index 0."""
    q = rgb.quantize(colors=255, method=Image.MEDIANCUT, dither=0)
    idx = q.point(lambda v: v + 1)
    idx.paste(0, mask=ImageOps.invert(opaque))
    pal = list(mask) + (q.getpalette() or [])[:765]
    pal += [0] * (768 - len(pal))
    idx.putpalette(pal)
    return idx


def main():
    ap = argparse.ArgumentParser(description="Make an indexed PNG spritesheet from GIFs.")
    ap.add_argument("gifs", nargs="+", help="input GIF files")
    ap.add_argument("-o", "--output", default="spritesheet.png", help="output file")
    ap.add_argument("-c", "--cols", type=int, help="columns of sprites")
    ap.add_argument("-r", "--rows", type=int, help="rows of sprites")
    ap.add_argument("-s", "--size", type=size_arg, metavar="WxH",
                    help="cell size in px (default: largest frame across all inputs)")
    ap.add_argument("-m", "--mask", type=colour, default=colour("FF00FF"),
                    help="mask colour as hex or name (default FF00FF)")
    args = ap.parse_args()

    frames = load_frames(args.gifs)
    n = len(frames)
    if n == 0:
        sys.exit("no frames found")

    cols, rows = args.cols, args.rows
    if cols and rows:
        if cols * rows < n:
            sys.exit(f"{n} frames do not fit in {cols}x{rows} grid")
    elif cols:
        rows = math.ceil(n / cols)
    elif rows:
        cols = math.ceil(n / rows)
    else:
        cols, rows = n, 1

    w, h = cell = args.size or (max(f.width for f in frames),
                                max(f.height for f in frames))
    print(f"{n} frames of {w}x{h} -> {cols}x{rows} grid "
          f"({w * cols}x{h * rows}px)")
    rgb, opaque = build_sheet(frames, cols, rows, args.mask, cell)
    out = args.output
    if not out.lower().endswith(".png"):
        out = out.rsplit(".", 1)[0] + ".png" if "." in out else out + ".png"
        print(f"output must be PNG, writing to {out} instead")
    to_indexed(rgb, opaque, args.mask).save(out, format="PNG", optimize=True)
    print(f"saved {out}")


if __name__ == "__main__":
    main()