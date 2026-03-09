# Copilot Instructions for defold-astar

## What This Project Is

A Defold native extension that wraps the embedded **MicroPather** library to provide A* pathfinding to Lua. It supports classic rectangular grids and hexagonal grids (ODDR, EVENR, ODDQ, EVENQ), 4- and 8-direction movement, multiple simultaneous maps, dynamic obstacles (entities), and custom tile movement costs.

## Build System

There is no standalone build command. The extension is built by the **Defold editor** or the **Defold build server** as part of the game project. `build_config.json` only contains the local build server port (`50664`). `compile_commands.json` exists for Clang tooling/IDE support.

To test changes, open the project in Defold editor and run it — `main/main.script` exercises the core API (`solve` and `solve_near`) in its `init()`.

## Architecture

```
astar/src/astar.cpp        — Lua bindings, extension lifecycle, hash table management
astar/src/pather.cpp       — Map class: grid logic, adjacency, cost lookup, MicroPather calls
astar/src/micropather/     — Embedded A* solver (third-party, do not refactor)
astar/include/pather.h     — Map class definition
astar/include/micropather/ — MicroPather headers (coordinates, pool, cache, etc.)
astar/annotations.lua      — LuaLS type annotations for the public API
main/main.script           — Working example (reference for expected behavior)
```

**Data flow:** Lua → `astar.cpp` Lua binding functions → `MapData` in the global `dmHashTable<uint16_t, MapData*> maps` → `Map` (pather.cpp) → `MicroPather` solver.

## Key Conventions

### Multi-map storage via dmHashTable
All maps are stored in a single global `dmHashTable<uint16_t, MapData*> maps`. The hash table **must not shrink** — `SetCapacity` only accepts values ≥ current capacity. The correct pattern for initialization:

```cpp
if (maps.Capacity() == 0)
{
    maps.SetCapacity(1, 1);
}
```

When the table is full, it doubles:
```cpp
maps.SetCapacity(maps.Capacity() * 2, maps.Capacity() * 2);
```

### Memory ownership
- `MapData` structs: `new` on creation, `delete` in `clearMaps()` callback
- Tile grid (`world`), cost arrays (`Costs`, `Costs[i].costs`), entity array (`entities`): `malloc`/`free` inside `Map::Clear()`
- `MicroPather` solver: `new`/`delete` inside `Map::Setup()`/`Map::Clear()`
- Never mix — `Map::Clear()` must be called before `delete mapData`

### Lua binding pattern
Every Lua-exposed function in `astar.cpp` follows this structure:
1. Pop an optional `map_id` (last arg) or default to `defaultMapId`
2. Call `get_map(L, nArgs)` which returns `MapData*` or logs an error and returns NULL
3. Return `0` (no values) on NULL — never crash or throw
4. Assert `top == lua_gettop(L)` at the end

### Naming
- Lua binding functions: `astar_<name>` (snake_case)
- `Map` class methods: `PascalCase`
- Constants/enums in `micropather` namespace: `UPPER_CASE`
- All constants registered via the `SETCONSTANT` macro in `LuaInit()`

### Coordinates
- `astar.cpp` works with `int16_t` x/y Lua coordinates
- `pather.cpp` converts x/y ↔ flat array index: `index = y * worldWidth + x`
- Vertical flip (`mapVFlip`) inverts the y-axis at the index conversion layer
- 0-indexed vs 1-indexed mode (`zero` flag) is handled in `astar.cpp` before passing to `Map`

## Extension Lifecycle

```
AppInitializeAstar  — Sets initial hash table capacity (app-global, called once per process)
AppFinalizeAstar    — Iterates & frees all maps, clears hash table
InitializeAstar     — Registers Lua functions (called per collection load)
```

`AppFinalizeAstar` calls `maps.Clear()` but does **not** reset capacity. On re-initialization in the same process (e.g., hot-reload), `AppInitializeAstar` must guard against calling `SetCapacity` with a smaller value.

## Lua API Surface

All functions accept an optional trailing `map_id` (defaults to `0`).

```lua
astar.setup(width, height, direction, allocate, typical_adjacent [, cache, use_zero, map_vflip, map_id])
astar.set_map(tile_table [, map_id])
astar.set_costs(costs_table [, map_id])
astar.set_map_type(map_type [, map_id])   -- only needed for hex grids
astar.set_entities(entities_table [, map_id])
astar.use_entities(bool [, map_id])

local status, size, cost, path = astar.solve(sx, sy, ex, ey [, map_id])
local status, size, nears      = astar.solve_near(x, y, max_cost [, map_id])

-- path entries: {x=, y=, id=}  (id = flat tile index)
-- nears entries: {x=, y=, id=, cost=}
```

**Constants:**
```lua
astar.SOLVED / astar.NO_SOLUTION / astar.START_END_SAME
astar.DIRECTION_FOUR / astar.DIRECTION_EIGHT
astar.GRID_CLASSIC / astar.HEX_ODDR / astar.HEX_EVENR / astar.HEX_ODDQ / astar.HEX_EVENQ
```

## MicroPather (embedded library)

Source lives in `astar/src/micropather/` and `astar/include/micropather/`. Do not refactor or restructure these files — they are a third-party library with a specific attribution requirement. Changes should be confined to the extension layer (`astar.cpp`, `pather.cpp`, `pather.h`).
