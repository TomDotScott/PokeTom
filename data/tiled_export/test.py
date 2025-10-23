import json, base64, zlib, struct

with open("C:\\Users\\tomsc\\Documents\\Github\\SFML-Poketom\\data\\tiled_export\\starter_town.tmj", "r", encoding="utf-8") as f:
    data = json.load(f)

layer = next(l for l in data["layers"] if l["name"] == "GrassAndPaths")

raw = layer["data"]                 # the long Base64 string
decoded = base64.b64decode(raw)     # step 1: Base64 → binary
decompressed = zlib.decompress(decoded)  # step 2: zlib → raw bytes

# step 3: interpret every 4 bytes as an unsigned 32-bit int (little-endian)
tiles = list(struct.unpack("<{}I".format(len(decompressed)//4), decompressed))

# reshape into 2D grid if you want
width, height = layer["width"], layer["height"]
grid = [tiles[i:i+width] for i in range(0, len(tiles), width)]

string = ""
for x in range(len(grid)):
    for y in range(len(grid[x])):
        string += f"{grid[x][y]},"

    string += "\n"

with open("C:\\Users\\tomsc\\Documents\\Github\\SFML-Poketom\\data\\tiled_export\\starter_town.csv", "w", encoding="utf-8") as f:
    f.write(string)
