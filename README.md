
# PokéTom

An original Pokémon game written from scratch in a custom 2D engine, using SFML 3+ purely for rendering. The engine is developed alongside the game — features get added as the game needs them.

This is intended as a long-term side project to practice modern C++ (C++20) in a game development context.

---

## What's been built

**Engine**
- Entity-Component system with `GridMovementComponent`, `AnimationComponent`, `DialogueComponent`, and `ScriptComponent`
- Sprite animation with per-frame axis flipping and anchor support
- Sprite batching via a `Renderer` class that groups draw calls by texture
- `Event<T>` system for decoupled communication between subsystems
- UI system with nestable `UiPanel` elements and configurable layer ordering
- `TextureManager` for centralised texture loading and reuse
- CRC32 string hashing — all identifiers are `hash_type` (`std::string` in Debug, `uint32_t` in Release/Retail), which reduced runtime memory use by over 90MB
- `Stringtable` and `Language` for localisation support
- Three build configurations: **Debug** (with debug overlays), **Release**, and **Retail** (no console window)

**Game**
- Overworld walking with tile-locked grid movement, walk/sprint speeds, and per-tile collision
- Level loading from Tiled `.tmj`/`.tsx` exports, including zlib-decompressed tile data
- Multi-level world with a `WorldDefinition.xml` manifest defining adjacency and portals
- Screen-fade portal transitions — level swap happens while the screen is black
- Chunk streaming — levels activate/deactivate based on camera visibility, with lazy render data caching to avoid frame spikes at boundaries
- Wild battle encounters trigger in tall grass
- Dialogue system with NPC interaction with localised string lookups
- Lua scripting via sol2 — levels and entities can each have their own sandboxed scripts
- NPCs defined and driven entirely in Lua (e.g. a Cyclist NPC that rides in circles around a pond)

---

## Building

The project uses [Sharpmake](https://github.com/ubisoft/Sharpmake) to generate a Visual Studio 2022 solution.

**Prerequisites**
- Visual Studio 2022
- Sharpmake (path configured in `generate.bat`)

**Steps**

```bat
generate.bat
```

This generates `SFML_Pokemon_Clone.sln` at the repo root. Open it in VS2022 and build. The working directory is pre-configured to `data/` so assets are found automatically.

`setup.bat`, invoked by `generate.bat`, checks out and clones sol2 and SFML locally to specific versions to work with the repository. 

Three runtime configs are available:

| Config | Subsystem | Defines |
|---|---|---|
| Debug | Console | `BUILD_DEBUG` |
| Release | Console | `BUILD_RELEASE` |
| Retail | Windows | `BUILD_MASTER` |

Where `Debug` has the least optimisation and `Retail` has full code optimisation by the compiler. 

---

## Adding assets

A Python script generates a C++ header from the contents of the `data/` folder, so asset paths are referenced as named constants rather than raw strings in the codebase (I hate typos!!!).

The names for the constants are defined in `data/assets.yaml`, and are separated into a few different categories. 

After adding new files to `data/`:

```bat
generate.bat
```

`generate.bat` runs `generate_assets_header.py` as part of its steps. You can also run the script directly if you only need to regenerate the header without touching the solution:

```bat
python -3 generate_assets_header.py
```

The output header is included by the engine wherever asset paths are needed — no raw string literals required.

---

## Level-loading architecture

Levels are created using [Tiled](https://www.mapeditor.org/), exported in JSON `.tmj` files. Levels get registered in `data/WorldDefinition.xml`, defining every level, its adjacency to neighbouring levels and the various portal connections between them. For example...

```xml
<WorldDefinition>
  <Level id="starter_town" tmj="tiled_export\\starter_town.tmj" north="route_1" south="" east="" west="">
    <Portal id="door_player_house" targetLevel="player_house" targetSpawnPoint="front_door_spawn"/>
  </Level>

  <Level id="player_house" tmj="tiled_export\\player_house.tmj" north="" south="" east="" west="">
    <Portal id="door_exit" targetLevel="starter_town" targetSpawnPoint="player_house_exit"/>
  </Level>
</WorldDefinition>
```

At startup, `WorldDefinition` parses the XML, runs `TileParser::ParseTMJ()` on every level, decompresses tile data, builds `TileSheet` objects from `.tsx` tilesheet definitions, and runs a BFS from `starter_town` to assign each level a world-space pixel origin. Interior levels (houses, caves) that have no adjacency entries are stacked off to the side — their exact position doesn't matter as they are never visible at the same time as the overworld.

Each level can optionally have a Lua script, specified as a custom map property in Tiled. Scripts are executed in sandboxed `sol::environment` instances and expose three lifecycle functions:

```lua
function init(self)
    self.onActivate = function(self)
        -- Create any entities here and attach components
    end

    self.onDeactivate = function(self)
       -- Destroy your created entities here
    end

    self.update = function(self, dt)
        -- Per-frame logic for the level
    end
end
```

`onActivate` and `onDeactivate` are called by the chunk streaming system as levels enter and leave the camera's visible rect. This is what gives NPCs the appearance of persistence — they are spawned and destroyed automatically as the player moves around.

---

## Asset credits

- Spritesheet — [scarloxy.itch.io/mpwsp01](https://scarloxy.itch.io/mpwsp01)
- Trainer sprites — [spriters-resource.com](https://www.spriters-resource.com/ds_dsi/pokemonblackwhite/)
- Pokémon sprites — [spriters-resource.com](https://www.spriters-resource.com/ds_dsi/pokemonheartgoldsoulsilver/)
