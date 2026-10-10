#!/usr/bin/env python3
"""
Generate per-state MonsterOverworldAnimations XML files from atlas_index.json (see combine_sprites.py).

One <AnimDict> per National Dex number, in dex order. Formes live in the same AnimDict
with a prefix on their animation names:
  NAME_1, NAME_2, ...   ->  forme1_walk_up, forme2_walk_up, ...
  NAME_female           ->  female_walk_up   (any non-numeric suffix, lower-cased)

One XML file is written per state (MonsterOverworldAnimations_Walking.xml, _Levitating.xml,
_Swimming.xml), each holding only the monsters that have that state. AnimDict names include the
state so they stay unique if all files are loaded into one registry:
  monster_006_overworld_walking_animations, monster_006_overworld_levitating_animations, ...
Animation names keep their state prefix (walk_up, levitate_up, swim_up).

Within an AnimDict, each forme gets its own <Image> element covering its
image on the state's atlas, followed by its four animations. Frame coordinates are
absolute atlas coordinates. Frame size is each image's size / 4, so larger
monsters automatically get larger frames.

Dex order comes from one of:
  --species-json f  a saved PokeAPI pokemon-species list response ({"results":[{"name","url"}]})
  --dex dex.txt     one name per line, National Dex order (line 1 = Bulbasaur)
  --fetch-dex       download the same list from PokeAPI
Sprite file names are matched to dex names ignoring case and punctuation
(MR_MIME == mr-mime). Anything that does not match is listed; fix with --alias.

--alias file lines:  SPRITE_NAME=dex-name     e.g.  NIDORAN_M=nidoran-m

Usage:
  python gen_overworld_xml.py atlases/atlas_index.json --species-json pokemon-species.json -o animations/
"""

import argparse
import json
import re
import sys
import urllib.request
from pathlib import Path

# Row of each direction inside the 4x4 image (rows are DOWN, LEFT, RIGHT, UP).
DIRECTION_ROWS = [("up", 3), ("down", 0), ("left", 1), ("right", 2)]
GRID = 4
STATE_PREFIX = {"walking": "walk", "levitating": "levitate", "swimming": "swim"}
STATE_ORDER = ["walking", "levitating", "swimming"]


def norm(s):
    return re.sub(r"[^a-z0-9]", "", s.lower())


def load_dex_file(path):
    names = []
    for line in path.read_text().splitlines():
        line = line.strip()
        if line and not line.startswith("#"):
            names.append(line)
    return names


def parse_species(data):
    """PokeAPI /pokemon-species list response -> names in National Dex order."""
    by_id = {}
    for item in data["results"]:
        by_id[int(item["url"].rstrip("/").split("/")[-1])] = item["name"]
    ids = sorted(by_id)
    if ids != list(range(1, len(ids) + 1)):
        sys.exit("error: species ids are not contiguous from 1; dex numbers would be wrong")
    return [by_id[i] for i in ids]


def load_species_json(path):
    return parse_species(json.loads(path.read_text(encoding="utf-8")))


def fetch_dex():
    url = "https://pokeapi.co/api/v2/pokemon-species?limit=3000"
    with urllib.request.urlopen(url, timeout=30) as r:
        return parse_species(json.load(r))


def load_aliases(path):
    aliases = {}
    if path:
        for line in path.read_text().splitlines():
            line = line.strip()
            if line and not line.startswith("#") and "=" in line:
                k, v = line.split("=", 1)
                aliases[norm(k)] = norm(v)
    return aliases


def resolve(stem, dex_map, aliases):
    """Return (dex_number, forme_label) or None. Base form has label ''."""
    key = norm(stem)
    key = aliases.get(key, key)
    if key in dex_map:
        return dex_map[key], ""
    m = re.match(r"^(.*)_([A-Za-z0-9]+)$", stem)
    if m:
        base = norm(m.group(1))
        base = aliases.get(base, base)
        if base in dex_map:
            return dex_map[base], m.group(2)
    return None


def forme_sort_key(label):
    if label == "":
        return (0, 0, "")
    if label.isdigit():
        return (1, int(label), "")
    return (2, 0, label.lower())


def forme_tag(label):
    if label == "":
        return ""
    return f"forme{label}_" if label.isdigit() else f"{label.lower()}_"


def build_group(entry, atlas, mask, indent):
    pad = " " * indent
    ox, oy, w, h = entry["x"], entry["y"], entry["width"], entry["height"]
    lines = [
        f"{pad}<Image",
        f'{pad}    source="{atlas["source"]}"',
        f'{pad}    topLeftX="{ox}"',
        f'{pad}    topLeftY="{oy}"',
        f'{pad}    regionWidth="{w}"',
        f'{pad}    regionHeight="{h}"',
        f'{pad}    maskColour="{mask}"',
        f"{pad}/>",
    ]
    return lines, ox, oy, w // GRID, h // GRID


def build_anim(name, ox, oy, row, fw, fh, duration, indent):
    pad = " " * indent
    lines = [f'{pad}<Animation name="{name}" looping="true">']
    for i in range(GRID):
        lines.append(
            f'{pad}    <Frame topLeftX="{ox + i * fw}" topLeftY="{oy + row * fh}" '
            f'duration="{duration}" spriteWidth="{fw}" spriteHeight="{fh}" />'
        )
    lines.append(f"{pad}</Animation>")
    return lines


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("index", type=Path, help="atlas_index.json from combine_sprites.py")
    src = ap.add_mutually_exclusive_group(required=True)
    src.add_argument("--species-json", type=Path, help="saved PokeAPI pokemon-species list response")
    src.add_argument("--dex", type=Path)
    src.add_argument("--fetch-dex", action="store_true")
    ap.add_argument("--alias", type=Path)
    ap.add_argument("-o", "--out-dir", type=Path, default=Path("animations"),
                    help="directory for the per-state XML files (default: animations)")
    ap.add_argument("--duration", type=int, default=200)
    ap.add_argument("--mask", default="00000000", help="maskColour as RRGGBBAA (default: fully transparent)")
    ap.add_argument("--allow-unmatched", action="store_true")
    args = ap.parse_args()

    index = json.loads(args.index.read_text())
    if args.species_json:
        dex_names = load_species_json(args.species_json)
    elif args.dex:
        dex_names = load_dex_file(args.dex)
    else:
        dex_names = fetch_dex()
    dex_map = {norm(n): i + 1 for i, n in enumerate(dex_names)}
    aliases = load_aliases(args.alias)

    # dex number -> forme label -> state -> entry
    data = {}
    unmatched = []
    named_formes = []
    for state, atlas in index.items():
        for stem, entry in atlas["entries"].items():
            res = resolve(stem, dex_map, aliases)
            if res is None:
                unmatched.append(f"{state}/{stem}")
                continue
            number, label = res
            if label and not label.isdigit():
                named_formes.append(f"{stem} -> dex {number} '{label.lower()}'")
            slot = data.setdefault(number, {}).setdefault(label, {})
            if state in slot:
                print(f"WARNING: duplicate {state} sprite for dex {number} forme '{label}': {stem} ignored", file=sys.stderr)
                continue
            slot[state] = entry

    if unmatched and not args.allow_unmatched:
        print("ERROR: sprite names with no dex match (add --alias lines or use --allow-unmatched):", file=sys.stderr)
        for u in sorted(unmatched):
            print(f"  {u}", file=sys.stderr)
        sys.exit(1)

    states = sorted(index, key=lambda st: STATE_ORDER.index(st) if st in STATE_ORDER else 99)
    args.out_dir.mkdir(parents=True, exist_ok=True)

    for state in states:
        atlas = index[state]
        sp = STATE_PREFIX.get(state, state)
        out = ['<?xml version="1.0" encoding="UTF-8"?>', "<MonsterOverworldAnimations>"]
        dicts = formes = 0
        for number in sorted(data):
            labels = [lb for lb in sorted(data[number], key=forme_sort_key) if state in data[number][lb]]
            if not labels:
                continue
            dicts += 1
            dex_name = dex_names[number - 1].replace("-", " ").title()
            out.append(f'    <AnimDict version="0.1" name="monster_{number:03d}_overworld_{state}_animations">')
            out.append(f"        <!-- {dex_name} {state} animations -->")
            for label in labels:
                if label:
                    formes += 1
                tag = forme_tag(label)
                group, ox, oy, fw, fh = build_group(data[number][label][state], atlas, args.mask, 8)
                out.append("")
                out.extend(group)
                for direction, row in DIRECTION_ROWS:
                    out.append("")
                    out.extend(build_anim(f"{tag}{sp}_{direction}", ox, oy, row, fw, fh, args.duration, 8))
            out.append("    </AnimDict>")
            out.append("")
        if out[-1] == "":
            out.pop()
        out.append("</MonsterOverworldAnimations>")

        path = args.out_dir / f"MonsterOverworldAnimations_{state.capitalize()}.xml"
        text = "\n".join(out) + "\n"
        path.write_text(text, encoding="utf-8")
        print(f"{state}: {dicts} dex entries, {formes} extra formes, {len(out)} lines, "
              f"{len(text.encode('utf-8')) / 1e6:.1f} MB -> {path}")

    if named_formes:
        print("note: non-numeric formes (check these are intended):")
        for n in sorted(set(named_formes)):
            print(f"  {n}")
    if unmatched:
        print(f"note: {len(unmatched)} unmatched sprites skipped")


if __name__ == "__main__":
    main()