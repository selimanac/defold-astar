
# Editing Tilemaps

Creates and edits Defold `.tilemap` component files using Protobuf Text Format.

## Overview

A Tilemap is a component that allows you to paint tiles from a Tile Source onto a large grid area. Tilemaps are commonly used to build game level environments. They support multiple layers, collision shapes from the tile source, and runtime manipulation via script.

## File format

Tilemap files (`.tilemap`) use **Protobuf Text Format** based on the `TileGrid` message from `tile_ddf.proto`.

### Canonical example

```protobuf
tile_set: "/assets/tiles/tileset.tilesource"
layers {
  id: "ground"
  z: 0.0
  is_visible: 1
  cell {
    x: 0
    y: 0
    tile: 1
    h_flip: 0
    v_flip: 0
  }
  cell {
    x: 1
    y: 0
    tile: 2
    h_flip: 0
    v_flip: 0
  }
}
material: "/builtins/materials/tile_map.material"
blend_mode: BLEND_MODE_ALPHA
```

## Top-level fields reference

### tile_set (required) — `string`

Absolute resource path to the tile source (`.tilesource`) used by this tilemap.

```protobuf
tile_set: "/assets/tiles/tileset.tilesource"
```

### layers (repeated) — `TileLayer`

Tilemap layers. Each layer is a separate `layers { ... }` block containing cells. Layers are rendered in order, with later layers on top. See **TileLayer fields** below.

```protobuf
layers {
  id: "background"
  z: 0.0
  is_visible: 1
  cell {
    x: 0
    y: 0
    tile: 1
    h_flip: 0
    v_flip: 0
  }
}
```

### material (optional) — `string`

Absolute resource path to the material used for rendering. Default: `"/builtins/materials/tile_map.material"`.

**Omission rule**: Omit if using default material.

```protobuf
material: "/builtins/materials/tile_map.material"
```

### blend_mode (optional) — enum `BlendMode`

How the tilemap graphics blend with background. Default: `BLEND_MODE_ALPHA`.

| Value | Description | Formula |
|-------|-------------|---------|
| `BLEND_MODE_ALPHA` | Normal blending (default) | `src.a * src.rgb + (1 - src.a) * dst.rgb` |
| `BLEND_MODE_ADD` | Additive blending | `src.rgb + dst.rgb` |
| `BLEND_MODE_ADD_ALPHA` | Add Alpha (Deprecated) | — |
| `BLEND_MODE_MULT` | Multiply | `src.rgb * dst.rgb` |
| `BLEND_MODE_SCREEN` | Screen | `src.rgb - dst.rgb * dst.rgb` |

**Omission rule**: Omit if `BLEND_MODE_ALPHA`.

```protobuf
blend_mode: BLEND_MODE_ADD
```

## TileLayer fields

### id (required) — `string`

Layer identifier. Used to reference the layer in scripts (e.g., `tilemap.get_tile()`, `tilemap.set_tile()`). Default: `"layer1"`.

```protobuf
id: "ground"
```

### z (required) — `float`

Z-order offset for this layer. Layers with higher z values render on top. Default: `0.0`.

```protobuf
z: 0.0
```

### is_visible (optional) — `uint32`

Layer visibility. `1` = visible, `0` = hidden. Default: `1`.

**Omission rule**: Omit if `1`.

```protobuf
is_visible: 1
```

### cell (repeated) — `TileCell`

Individual tile placements on this layer. Each cell is a `cell { ... }` block. See **TileCell fields** below.

## TileCell fields

### x (required) — `int32`

Horizontal grid position of the cell. Can be negative. Default: `0`.

```protobuf
x: 5
```

### y (required) — `int32`

Vertical grid position of the cell. Can be negative. Y increases upward. Default: `0`.

```protobuf
y: 3
```

### tile (required) — `uint32`

Tile index from the tile source. Tile `0` is typically empty/transparent. Tiles are numbered starting from 0 in the top-left of the tilesource, proceeding left-to-right, row-by-row. Default: `0`.

```protobuf
tile: 42
```

### h_flip (optional) — `uint32`

Horizontal flip. `0` = normal, `1` = flipped. Default: `0`.

**Omission rule**: Omit if `0`.

```protobuf
h_flip: 1
```

### v_flip (optional) — `uint32`

Vertical flip. `0` = normal, `1` = flipped. Default: `0`.

**Omission rule**: Omit if `0`.

```protobuf
v_flip: 1
```

### rotate90 (optional) — `uint32`

90-degree clockwise rotation. `0` = no rotation, `1` = rotated. Default: `0`.

**Omission rule**: Omit if `0`.

```protobuf
rotate90: 1
```

## Protobuf Text Format rules

1. **Default omission**: Omit fields that equal their proto default.
2. **Message blocks**: Use `field_name { ... }` with nested `key: value` pairs.
3. **Floats**: Always include decimal point: `1.0`, not `1`.
4. **Integers**: No decimal point: `4`, not `4.0`.
5. **Strings**: Always double-quoted.
6. **Enums**: Use the constant name without quotes.
7. **Repeated messages**: Each entry gets its own `field_name { ... }` block.
8. **Field order**: Follow the proto field number order.
9. **No trailing commas or semicolons**.
10. **Indentation**: 2 spaces per nesting level inside message blocks.

## Complete examples

### Simple single-layer tilemap

```protobuf
tile_set: "/assets/tiles/tileset.tilesource"
layers {
  id: "layer1"
  z: 0.0
  is_visible: 1
  cell {
    x: 0
    y: 0
    tile: 5
    h_flip: 0
    v_flip: 0
  }
  cell {
    x: 1
    y: 0
    tile: 6
    h_flip: 0
    v_flip: 0
  }
  cell {
    x: 2
    y: 0
    tile: 7
    h_flip: 0
    v_flip: 0
  }
}
material: "/builtins/materials/tile_map.material"
blend_mode: BLEND_MODE_ALPHA
```

### Multi-layer tilemap

```protobuf
tile_set: "/assets/tiles/platformer.tilesource"
layers {
  id: "background"
  z: 0.0
  is_visible: 1
  cell {
    x: 0
    y: 0
    tile: 10
    h_flip: 0
    v_flip: 0
  }
  cell {
    x: 1
    y: 0
    tile: 10
    h_flip: 0
    v_flip: 0
  }
}
layers {
  id: "ground"
  z: 0.1
  is_visible: 1
  cell {
    x: 0
    y: 0
    tile: 1
    h_flip: 0
    v_flip: 0
  }
  cell {
    x: 1
    y: 0
    tile: 2
    h_flip: 0
    v_flip: 0
  }
  cell {
    x: 2
    y: 0
    tile: 3
    h_flip: 0
    v_flip: 0
  }
}
layers {
  id: "decorations"
  z: 0.2
  is_visible: 1
  cell {
    x: 1
    y: 1
    tile: 44
    h_flip: 0
    v_flip: 0
  }
}
material: "/builtins/materials/tile_map.material"
blend_mode: BLEND_MODE_ALPHA
```

### Tilemap with flipped tiles

```protobuf
tile_set: "/assets/tiles/tileset.tilesource"
layers {
  id: "layer1"
  z: 0.0
  is_visible: 1
  cell {
    x: 0
    y: 0
    tile: 5
    h_flip: 0
    v_flip: 0
  }
  cell {
    x: 1
    y: 0
    tile: 5
    h_flip: 1
    v_flip: 0
  }
  cell {
    x: 0
    y: 1
    tile: 5
    h_flip: 0
    v_flip: 1
  }
  cell {
    x: 1
    y: 1
    tile: 5
    h_flip: 1
    v_flip: 1
  }
}
material: "/builtins/materials/tile_map.material"
blend_mode: BLEND_MODE_ALPHA
```

## Runtime manipulation

Tilemaps can be manipulated at runtime using the `tilemap` module:

```lua
-- Get a tile
local tile = tilemap.get_tile("/level#map", "ground", x, y)

-- Set a tile
tilemap.set_tile("/level#map", "ground", x, y, 4)

-- Set tile with transformations
tilemap.set_tile("/level#map", "ground", x, y, 4, true, false) -- h_flip, v_flip
```

### Runtime properties

These properties can be changed with `go.get()` and `go.set()`:

- `tile_source` — The tile source resource (`hash`)
- `material` — The material resource (`hash`)

### Material constants

The default tilemap material supports:

- `tint` — Color tint (`vector4`, RGBA)

```lua
go.set("#tilemap", "tint", vmath.vector4(1, 0, 0, 1))
go.animate("#tilemap", "tint", go.PLAYBACK_LOOP_PINGPONG, vmath.vector4(1, 0, 0, 1), go.EASING_LINEAR, 2)
```

## Workflow

### Creating a new tilemap

1. Determine the file path (must end with `.tilemap`).
2. Set `tile_set` to the tile source path.
3. Add at least one `layers` block with an `id`.
4. Add `cell` entries for each tile placement.
5. Set `material` if using a custom material (otherwise use default).
6. Set `blend_mode` if not using alpha blending.
7. Omit fields at their default values.

### Editing an existing tilemap

1. Read the current `.tilemap` file.
2. Modify only the requested changes (add/remove cells, change layers).
3. Preserve existing field values and order.
4. Apply omission rules for fields that become equal to their defaults.

### Tile coordinate system

- Origin (0, 0) is at the bottom-left of the tilemap grid.
- X increases to the right.
- Y increases upward.
- Coordinates can be negative for tiles placed to the left or below origin.

### Tile numbering (in tilesource)

Tiles in the tilesource are numbered starting from 0:
- Tile `0` is typically empty/transparent
- Numbering proceeds left-to-right, top-to-bottom in the source image

```
| 0  | 1  | 2  | 3  |
| 4  | 5  | 6  | 7  |
| 8  | 9  | 10 | 11 |
```
