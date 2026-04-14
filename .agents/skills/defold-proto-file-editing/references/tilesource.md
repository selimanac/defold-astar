
# Editing Tile Sources

Creates and edits Defold `.tilesource` resource files using Protobuf Text Format.

## Overview

A Tile Source defines a grid of uniformly-sized tiles from a single image. Tile sources are used by Tilemap components to paint tiles onto a grid, or as image sources for Sprite and ParticleFX components. They support flipbook animations and collision shape definitions for physics.

## File format

Tile source files (`.tilesource`) use **Protobuf Text Format** based on the `TileSet` message from `tile_ddf.proto`.

### Canonical example

```protobuf
image: "/assets/images/tileset.png"
tile_width: 32
tile_height: 32
animations {
  id: "walk"
  start_tile: 1
  end_tile: 4
  playback: PLAYBACK_LOOP_FORWARD
  fps: 12
}
extrude_borders: 2
```

## Top-level fields reference

### image (required) — `string`

Absolute resource path to the source image containing the tile grid. The image must contain tiles arranged in a uniform grid.

```protobuf
image: "/assets/images/tileset.png"
```

### tile_width (required) — `uint32`

Width of each tile in pixels. Default: `0`.

```protobuf
tile_width: 32
```

### tile_height (required) — `uint32`

Height of each tile in pixels. Default: `0`.

```protobuf
tile_height: 32
```

### tile_margin (optional) — `uint32`

Number of pixels surrounding each tile in the source image (margin around the tile). Default: `0`.

**Omission rule**: Omit if `0`.

```protobuf
tile_margin: 1
```

### tile_spacing (optional) — `uint32`

Number of pixels between each tile in the source image. Default: `0`.

**Omission rule**: Omit if `0`.

```protobuf
tile_spacing: 2
```

### collision (optional) — `string`

Absolute resource path to an image used to automatically generate collision shapes for tiles. Often the same image as the main tile image. When specified, the engine generates convex hull collision shapes from non-transparent pixels.

**Omission rule**: Omit if empty/unused.

```protobuf
collision: "/assets/images/tileset.png"
```

### material_tag (optional) — `string`

Tag used in render scripts to identify this tile source's material. Default: `"tile"`.

**Omission rule**: Omit if `"tile"`.

```protobuf
material_tag: "tile"
```

### convex_hulls (repeated) — `ConvexHull`

Collision shapes for tiles. Each entry defines a convex hull for one tile. See **ConvexHull fields** below. These are typically auto-generated from the collision image.

```protobuf
convex_hulls {
  index: 0
  count: 4
  collision_group: "ground"
}
```

### collision_groups (repeated) — `string`

List of collision group names used by tiles. Each name appears once in this list and is referenced by `ConvexHull.collision_group`.

```protobuf
collision_groups: "ground"
collision_groups: "danger"
```

### animations (repeated) — `Animation`

Flipbook animations defined from consecutive tiles. See **Animation fields** below.

```protobuf
animations {
  id: "run"
  start_tile: 1
  end_tile: 8
  playback: PLAYBACK_LOOP_FORWARD
  fps: 12
}
```

### extrude_borders (optional) — `uint32`

Number of times edge pixels are replicated around each tile in the compiled texture. Prevents texture bleeding at tile edges. Default: `0`.

**Omission rule**: Omit if `0`.

```protobuf
extrude_borders: 2
```

### inner_padding (optional) — `uint32`

Empty pixels added around each tile in the compiled texture. Default: `0`.

**Omission rule**: Omit if `0`.

```protobuf
inner_padding: 1
```

### sprite_trim_mode (optional) — enum `SpriteTrimmingMode`

How sprite geometry is generated when used with Sprite components. Trimming transparent pixels can reduce overdraw. Default: `SPRITE_TRIM_MODE_OFF`.

| Value | Description |
|-------|-------------|
| `SPRITE_TRIM_MODE_OFF` | Rectangular quad (default) |
| `SPRITE_TRIM_MODE_4` | 4 vertices |
| `SPRITE_TRIM_MODE_5` | 5 vertices |
| `SPRITE_TRIM_MODE_6` | 6 vertices |
| `SPRITE_TRIM_MODE_7` | 7 vertices |
| `SPRITE_TRIM_MODE_8` | 8 vertices |
| `SPRITE_TRIM_POLYGONS` | Polygon-based trimming |

**Omission rule**: Omit if `SPRITE_TRIM_MODE_OFF`.

```protobuf
sprite_trim_mode: SPRITE_TRIM_MODE_6
```

## Animation fields

### id (required) — `string`

Animation name. Must be unique within the tile source. Used to reference the animation in code via `sprite.play_flipbook()`.

```protobuf
id: "walk"
```

### start_tile (required) — `uint32`

First tile of the animation. Numbering starts at 1 in the top-left corner and proceeds left-to-right, row-by-row.

```protobuf
start_tile: 1
```

### end_tile (required) — `uint32`

Last tile of the animation (inclusive).

```protobuf
end_tile: 8
```

### playback (optional) — enum `Playback`

Animation playback mode. Default: `PLAYBACK_ONCE_FORWARD`.

| Value | Description |
|-------|-------------|
| `PLAYBACK_NONE` | No playback, shows first frame |
| `PLAYBACK_ONCE_FORWARD` | Play once, first to last |
| `PLAYBACK_ONCE_BACKWARD` | Play once, last to first |
| `PLAYBACK_ONCE_PINGPONG` | Play once forward then backward |
| `PLAYBACK_LOOP_FORWARD` | Loop, first to last |
| `PLAYBACK_LOOP_BACKWARD` | Loop, last to first |
| `PLAYBACK_LOOP_PINGPONG` | Loop forward then backward |

**Omission rule**: Omit if `PLAYBACK_ONCE_FORWARD`.

```protobuf
playback: PLAYBACK_LOOP_FORWARD
```

### fps (optional) — `uint32`

Playback speed in frames per second. Default: `30`.

**Omission rule**: Omit if `30`.

```protobuf
fps: 12
```

### flip_horizontal (optional) — `uint32`

Flip animation horizontally. `0` = no flip, `1` = flip. Default: `0`.

**Omission rule**: Omit if `0`.

### flip_vertical (optional) — `uint32`

Flip animation vertically. `0` = no flip, `1` = flip. Default: `0`.

**Omission rule**: Omit if `0`.

### cues (repeated) — `Cue`

Animation cues/events triggered at specific frames. See **Cue fields** below.

## Cue fields

### id (required) — `string`

Cue identifier.

### frame (required) — `uint32`

Frame number when the cue triggers (0-indexed within animation).

### value (optional) — `float`

Optional value associated with the cue. Default: `0.0`.

**Omission rule**: Omit if `0.0`.

## ConvexHull fields

### index (required) — `uint32`

Index into the convex hull points array. Default: `0`.

### count (required) — `uint32`

Number of points in this convex hull. Default: `0`.

### collision_group (required) — `string`

Collision group name for this tile's shape. Must match an entry in `collision_groups`. Default: `"tile"`. Use empty string `""` for tiles without collision.

```protobuf
convex_hulls {
  index: 0
  count: 4
  collision_group: "ground"
}
```

## Protobuf Text Format rules

1. **Default omission**: Omit fields that equal their proto default.
2. **Message blocks**: Use `field_name { ... }` with nested `key: value` pairs.
3. **Floats**: Always include decimal point: `1.0`, not `1`.
4. **Integers**: No decimal point: `4`, not `4.0`.
5. **Strings**: Always double-quoted.
6. **Enums**: Use the constant name without quotes.
7. **Repeated messages**: Each entry gets its own `field_name { ... }` block.
8. **Repeated scalars**: Each value gets its own line with the field name.
9. **Field order**: Follow the proto field number order.
10. **No trailing commas or semicolons**.
11. **Indentation**: 2 spaces per nesting level inside message blocks.

## Complete examples

### Simple tile source for sprites

```protobuf
image: "/assets/images/character_sheet.png"
tile_width: 96
tile_height: 128
animations {
  id: "run"
  start_tile: 37
  end_tile: 44
  playback: PLAYBACK_LOOP_FORWARD
  fps: 10
}
extrude_borders: 2
```

### Tile source for tilemap with collisions

```protobuf
image: "/assets/images/platformer_tiles.png"
tile_width: 64
tile_height: 64
collision: "/assets/images/platformer_tiles.png"
convex_hulls {
  index: 0
  count: 4
  collision_group: "ground"
}
convex_hulls {
  index: 4
  count: 4
  collision_group: "ground"
}
convex_hulls {
  index: 8
  count: 6
  collision_group: "danger"
}
collision_groups: "ground"
collision_groups: "danger"
extrude_borders: 1
```

### Tile source with multiple animations

```protobuf
image: "/assets/images/character.png"
tile_width: 32
tile_height: 32
animations {
  id: "idle"
  start_tile: 1
  end_tile: 4
  playback: PLAYBACK_LOOP_FORWARD
  fps: 8
}
animations {
  id: "walk"
  start_tile: 5
  end_tile: 12
  playback: PLAYBACK_LOOP_FORWARD
  fps: 12
}
animations {
  id: "jump"
  start_tile: 13
  end_tile: 16
  fps: 15
}
extrude_borders: 2
```

## Workflow

### Creating a new tile source

1. Determine the file path (must end with `.tilesource`).
2. Set `image` to the source image path.
3. Set `tile_width` and `tile_height` to match the tile grid.
4. Set `tile_margin` and `tile_spacing` if the source image has gaps between tiles.
5. Add `animations` blocks for flipbook animations if needed.
6. If using for tilemaps with physics:
   - Set `collision` to the collision mask image.
   - Add `collision_groups` for each collision group used.
   - Convex hulls are typically auto-generated by the editor.
7. Set `extrude_borders` (recommended: 1-2) to prevent edge bleeding.
8. Omit fields at their default values.

### Editing an existing tile source

1. Read the current `.tilesource` file.
2. Modify only the requested fields (add/remove animations, change properties).
3. Preserve existing field values and order.
4. Apply omission rules for fields that become equal to their defaults.

### Tile numbering

Tiles are numbered starting from 1 in the top-left corner, proceeding left-to-right, then top-to-bottom:

```
| 1  | 2  | 3  | 4  |
| 5  | 6  | 7  | 8  |
| 9  | 10 | 11 | 12 |
```
