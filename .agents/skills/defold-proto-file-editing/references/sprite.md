# Editing Sprites

Creates and edits Defold `.sprite` component files using Protobuf Text Format.

## Overview

A Sprite component displays a simple image or flipbook animation on screen. It uses an Atlas or Tile Source for its graphics and supports slice-9 texturing, multiple texture samplers, blend modes, and custom material attributes.

## File format

Sprite files (`.sprite`) use **Protobuf Text Format** based on the `SpriteDesc` message from `gamesys/sprite_ddf.proto`.

### Canonical example

```protobuf
default_animation: "logo"
material: "/builtins/materials/sprite.material"
size {
  x: 100.0
  y: 100.0
}
size_mode: SIZE_MODE_MANUAL
textures {
  sampler: "texture_sampler"
  texture: "/main/example.atlas"
}
```

## Fields reference

### tile_set (deprecated) — `string`

Legacy field for single-texture sprites. **Do not use**. Use the `textures` repeated field instead.

**Omission rule**: Always omit.

### default_animation (required) — `string`

The animation (or image) id to display. The animation data is taken from the first atlas or tilesource.

```protobuf
default_animation: "idle"
```

### material (optional) — `string`

Absolute resource path to a `.material` file. Default: `"/builtins/materials/sprite.material"`.

**Omission rule**: Omit if `"/builtins/materials/sprite.material"`.

```protobuf
material: "/builtins/materials/sprite.material"
```

### blend_mode (optional) — enum `BlendMode`

Blending mode for rendering. Default: `BLEND_MODE_ALPHA`.

| Value | Description |
|-------|-------------|
| `BLEND_MODE_ALPHA` | Normal alpha blending |
| `BLEND_MODE_ADD` | Additive blending (brightens) |
| `BLEND_MODE_ADD_ALPHA` | Add Alpha (Deprecated) |
| `BLEND_MODE_MULT` | Multiply (darkens) |
| `BLEND_MODE_SCREEN` | Screen (inverse multiply) |

**Omission rule**: Omit if `BLEND_MODE_ALPHA`.

```protobuf
blend_mode: BLEND_MODE_ADD
```

### slice9 (optional) — `dmMath.Vector4`

Slice-9 margins in pixels to preserve edges when the sprite is resized. Components default to `0.0`. Values are set clockwise: left (`x`), top (`y`), right (`z`), bottom (`w`).

- `x` — left margin (default: `0.0`)
- `y` — top margin (default: `0.0`)
- `z` — right margin (default: `0.0`)
- `w` — bottom margin (default: `0.0`)

When using slice-9, the Sprite Trim Mode in the atlas must be set to Off.

**Omission rule**: Omit entire block if all components are `0.0`.

```protobuf
slice9 {
  x: 16.0
  y: 16.0
  z: 16.0
  w: 16.0
}
```

### size (optional) — `dmMath.Vector4`

Sprite size in pixels. **Only has effect when `size_mode` is `SIZE_MODE_MANUAL`**. When `size_mode` is `SIZE_MODE_AUTO` (the default), the engine determines the size automatically from the image — do NOT set `size` and do NOT look up the image dimensions. Components default to `0.0`.

- `x` — width (default: `0.0`)
- `y` — height (default: `0.0`)
- `z` — depth (default: `0.0`, rarely used)
- `w` — (default: `0.0`, rarely used)

**Omission rule**: Omit entire block if `size_mode` is `SIZE_MODE_AUTO` (default) or if all components are `0.0`.

```protobuf
size {
  x: 128.0
  y: 64.0
}
```

### size_mode (optional) — enum `SizeMode`

Controls whether the sprite size is set automatically from the image or manually. Default: `SIZE_MODE_AUTO`.

| Value | Description |
|-------|-------------|
| `SIZE_MODE_MANUAL` | Size is set manually via the `size` field |
| `SIZE_MODE_AUTO` | Size is determined automatically from the image |

**Omission rule**: Omit if `SIZE_MODE_AUTO`.

```protobuf
size_mode: SIZE_MODE_MANUAL
```

### offset (optional) — `float`

The normalized initial value of the animation cursor when the animation starts playing. Default: `0.0`. Range: `0.0` to `1.0`.

**Omission rule**: Omit if `0.0`.

```protobuf
offset: 0.5
```

### playback_rate (optional) — `float`

The rate at which the animation plays. Must be positive. Default: `1.0`.

**Omission rule**: Omit if `1.0`.

```protobuf
playback_rate: 2.0
```

### attributes (optional, repeated) — `dmGraphics.VertexAttribute`

Custom vertex attributes that override values from the material. Each entry specifies an attribute name and its values.

```protobuf
attributes {
  name: "tint"
  double_values {
    v: 1.0
    v: 0.0
    v: 0.0
    v: 1.0
  }
}
```

### textures (repeated) — `SpriteTexture`

Texture bindings. Each entry maps a material sampler name to an atlas or tilesource resource. For the default sprite material, the sampler name is `"texture_sampler"`.

When using multiple textures, the animation frame ids must match across all textures. Use the atlas `Rename patterns` to ensure matching.

```protobuf
textures {
  sampler: "texture_sampler"
  texture: "/main/my_atlas.atlas"
}
```

#### SpriteTexture fields

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `sampler` | `string` | yes | The sampler name from the material |
| `texture` | `string` | yes | Absolute resource path to `.atlas` or `.tilesource` |

## Protobuf Text Format rules

1. **Default omission**: Omit fields that equal their proto default. This keeps files minimal and matches Defold editor behavior.
2. **Message blocks**: Use `field_name { ... }` with nested `key: value` pairs.
3. **Floats**: Always include decimal point: `1.0`, not `1`. Use standard float formatting.
4. **Strings**: Always double-quoted: `"text"`.
5. **Enums**: Use the enum constant name without quotes: `SIZE_MODE_MANUAL`.
6. **Booleans**: `true` or `false`, no quotes.
7. **Field order**: Follow the proto field number order: `default_animation`, `material`, `blend_mode`, `slice9`, `size`, `size_mode`, `offset`, `playback_rate`, `attributes`, `textures`.
8. **No trailing commas or semicolons**.
9. **No field number tags** — use field names only.
10. **Newlines**: One empty line between the end of a message block `}` and the next field. No empty line between consecutive scalar fields.
11. **Indentation**: 2 spaces inside message blocks.
12. **Repeated messages**: Each entry gets its own `field_name { ... }` block.

## Common templates

### Simple sprite (auto-sized)

```protobuf
default_animation: "idle"
textures {
  sampler: "texture_sampler"
  texture: "/assets/sprites.atlas"
}
```

### Manual-sized sprite

```protobuf
default_animation: "background"
size {
  x: 320.0
  y: 240.0
}
size_mode: SIZE_MODE_MANUAL
textures {
  sampler: "texture_sampler"
  texture: "/assets/backgrounds.atlas"
}
```

### Slice-9 sprite (for UI panels)

```protobuf
default_animation: "panel"
slice9 {
  x: 12.0
  y: 12.0
  z: 12.0
  w: 12.0
}
size {
  x: 200.0
  y: 100.0
}
size_mode: SIZE_MODE_MANUAL
textures {
  sampler: "texture_sampler"
  texture: "/assets/ui.atlas"
}
```

### Multi-texture sprite

```protobuf
default_animation: "hero_idle"
material: "/assets/materials/multi_tex.material"
textures {
  sampler: "diffuse"
  texture: "/assets/hero_diffuse.atlas"
}
textures {
  sampler: "normal"
  texture: "/assets/hero_normal.atlas"
}
```

## Workflow

### Creating a new sprite

1. Determine the file path (must end with `.sprite`).
2. Set the required `default_animation` field.
3. Add at least one `textures` entry with the sampler name and atlas/tilesource path.
4. Set `material` only if not using the default sprite material.
5. If manual sizing is needed, set `size_mode: SIZE_MODE_MANUAL` and provide `size`. Otherwise, do NOT set `size` and do NOT look up image dimensions — the engine handles sizing automatically.
6. Add optional fields only if they differ from defaults.
7. Write the file using the field order from the reference above.

### Editing an existing sprite

1. Read the current `.sprite` file.
2. Modify only the requested fields.
3. Preserve existing field values and order.
4. Apply omission rules: remove fields that become equal to their defaults after editing.
