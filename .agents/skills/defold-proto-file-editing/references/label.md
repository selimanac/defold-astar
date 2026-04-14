# Editing Labels

Creates and edits Defold `.label` component files using Protobuf Text Format.

## File format

Label files (`.label`) use **Protobuf Text Format** based on the `LabelDesc` message from `label_ddf.proto`.

### Canonical example

```protobuf
size {
  x: 128.0
  y: 32.0
}
color {
  x: 0.93333334
  y: 0.93333334
  z: 0.93333334
}
outline {
  x: 0.2
  y: 0.2
  z: 0.2
}
shadow {
  x: 0.2
  y: 0.2
  z: 0.2
}
pivot: PIVOT_N
line_break: true
text: "Label"
font: "/builtins/fonts/default.font"
material: "/builtins/fonts/label-df.material"
```

## Fields reference

### size (required) — `dmMath.Vector4`

Bounding box of the text area. If `line_break` is enabled, `x` controls the wrap width.

- `x` — width (default: `0.0`)
- `y` — height (default: `0.0`)
- `z` — depth (default: `0.0`, rarely used)
- `w` — (default: `0.0`, rarely used)

Only include components that differ from `0.0`. Typically only `x` and `y` are set.

```protobuf
size {
  x: 256.0
  y: 64.0
}
```

### color (optional) — `dmMath.Vector4One`

Text RGBA color. Components default to `1.0` (white, fully opaque).

- `x` — red (default: `1.0`)
- `y` — green (default: `1.0`)
- `z` — blue (default: `1.0`)
- `w` — alpha (default: `1.0`)

**Omission rule**: Only include components that differ from `1.0`. If color is fully white (`1.0, 1.0, 1.0, 1.0`), omit the entire block.

```protobuf
color {
  x: 0.0
  y: 0.5
  z: 1.0
}
```

### outline (optional) — `dmMath.Vector4WOne`

Outline RGBA color. `x`, `y`, `z` default to `0.0`; `w` defaults to `1.0`.

**Omission rule**: Only include components that differ from their defaults (`x/y/z` from `0.0`, `w` from `1.0`). If all at defaults, omit the entire block.

```protobuf
outline {
  x: 0.2
  y: 0.2
  z: 0.2
}
```

### shadow (optional) — `dmMath.Vector4WOne`

Shadow RGBA color. Same defaults as `outline`.

Note: default material has shadow rendering disabled for performance. To see shadows, use a material that supports them.

```protobuf
shadow {
  x: 0.2
  y: 0.2
  z: 0.2
}
```

### leading (optional) — `float`

Line spacing multiplier. Default: `1.0`. Value `0` gives no line spacing.

**Omission rule**: Omit if `1.0`.

```protobuf
leading: 1.5
```

### tracking (optional) — `float`

Letter spacing multiplier. Default: `0.0`.

**Omission rule**: Omit if `0.0`.

```protobuf
tracking: 0.02
```

### pivot (optional) — enum `Pivot`

Text anchor point and alignment. Default: `PIVOT_CENTER`.

Valid values:

| Value | Meaning |
|-------|---------|
| `PIVOT_CENTER` | Center |
| `PIVOT_N` | North (top center) |
| `PIVOT_NE` | North East |
| `PIVOT_E` | East (center right) |
| `PIVOT_SE` | South East |
| `PIVOT_S` | South (bottom center) |
| `PIVOT_SW` | South West |
| `PIVOT_W` | West (center left) |
| `PIVOT_NW` | North West |

**Omission rule**: Omit if `PIVOT_CENTER`.

```protobuf
pivot: PIVOT_NW
```

### blend_mode (optional) — enum `BlendMode`

Blending mode for rendering. Default: `BLEND_MODE_ALPHA`.

| Value | Description |
|-------|-------------|
| `BLEND_MODE_ALPHA` | Normal alpha blending |
| `BLEND_MODE_ADD` | Additive blending (brightens) |
| `BLEND_MODE_MULT` | Multiply (darkens) |
| `BLEND_MODE_SCREEN` | Screen (inverse multiply) |

**Omission rule**: Omit if `BLEND_MODE_ALPHA`.

```protobuf
blend_mode: BLEND_MODE_ADD
```

### line_break (optional) — `bool`

Enable multi-line text wrapping at the bounding box width. Default: `false`.

**Omission rule**: Omit if `false`.

```protobuf
line_break: true
```

### text (optional) — `string`

Text content. Default: `""`. Always wrap in double quotes.

```protobuf
text: "Hello World"
```

### font (required) — `string`

Absolute resource path to a `.font` file. Must match the material type (bitmap, distance field, or BMFont).

```protobuf
font: "/builtins/fonts/default.font"
```

### material (required) — `string`

Absolute resource path to a `.material` file. Must match the font type.

Common built-in materials:
- `/builtins/fonts/label.material` — bitmap fonts
- `/builtins/fonts/label-df.material` — distance field fonts
- `/builtins/fonts/label-fnt.material` — BMFonts

```protobuf
material: "/builtins/fonts/label-df.material"
```

## Protobuf Text Format rules

1. **Default omission**: Omit fields that equal their proto default. This keeps files minimal and matches Defold editor behavior.
2. **Message blocks**: Use `field_name { ... }` with nested `key: value` pairs.
3. **Floats**: Always include decimal point: `1.0`, not `1`. Use standard float formatting.
4. **Strings**: Always double-quoted: `"text"`.
5. **Enums**: Use the enum constant name without quotes: `PIVOT_N`.
6. **Booleans**: `true` or `false`, no quotes.
7. **Field order**: Follow the proto field number order: `size`, `color`, `outline`, `shadow`, `leading`, `tracking`, `pivot`, `blend_mode`, `line_break`, `text`, `font`, `material`.
8. **No trailing commas or semicolons**.
9. **No field number tags** — use field names only.
10. **Newlines**: One empty line between the end of a message block `}` and the next field. No empty line between scalar fields.
11. **Indentation**: 2 spaces inside message blocks.

## Minimal label (only required fields + text)

```protobuf
size {
  x: 128.0
  y: 32.0
}
text: "Hello"
font: "/builtins/fonts/default.font"
material: "/builtins/fonts/label-df.material"
```

## Best practices

- **Font selection priority.** When choosing a `font` for a label:
  1. Use the font specified by the user in the current request, if provided.
  2. Search the project for existing `.font` files (e.g., `assets/fonts/`). If found, use the project font. Pick the most appropriate one if there are several (e.g., prefer distance field fonts).
  3. Fall back to `/builtins/fonts/default.font` only if no project fonts exist and the user did not specify one.
  Always match the `material` to the font type (`label-df.material` for distance field, `label.material` for bitmap, `label-fnt.material` for BMFont).
- **Use distance field fonts with base size 50.** The `.font` resource should use `size: 50` and `output_format: TYPE_DISTANCE_FIELD`. This provides crisp text at any visual size.
- **Control text size via game object scale, not font size.** Do not create separate `.font` files for different text sizes. Instead, scale the game object that contains the Label component. For example, to display text at visual size 25, keep `size: 50` in the `.font` and set the game object's scale to `0.5`.

## Workflow

### Creating a new label

1. Determine the file path (must end with `.label`).
2. Set required fields: `size`, `font`, `material`.
3. Set `text` with the desired content.
4. Add optional fields only if they differ from defaults.
5. Write the file using the field order from the reference above.

### Editing an existing label

1. Read the current `.label` file.
2. Modify only the requested fields.
3. Preserve existing field values and order.
4. Apply omission rules: remove fields that become equal to their defaults after editing.
