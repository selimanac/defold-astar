# Editing Fonts

Creates and edits Defold `.font` resource files using Protobuf Text Format.

## Overview

Font resources define how a font file (TTF, OTF, or BMFont `.fnt`) is rasterized into glyph textures for rendering text on Label components and GUI text nodes. Two output formats are available: bitmap (default) and distance field.

## File format

Font files (`.font`) use **Protobuf Text Format** based on the `FontDesc` message from `render/font_ddf.proto`.

### Canonical example — Defold built-in default (`/builtins/fonts/default.font`)

```protobuf
font: "/builtins/fonts/vera_mo_bd.ttf"
material: "/builtins/fonts/font-df.material"
size: 14
antialias: 1
alpha: 1.0
shadow_alpha: 0.0
shadow_blur: 0
output_format: TYPE_DISTANCE_FIELD
```

Note: the Defold editor sometimes writes fields even when they equal the proto default (e.g. `antialias: 1`, `alpha: 1.0`). When **creating** files, omit default-valued fields. When **editing** existing files, preserve fields already present.

### Additional examples

Distance field font with outline, shadow, and multi-layer rendering:

```protobuf
font: "/example/assets/fonts/Nunito-Black.ttf"
material: "/builtins/fonts/font-df.material"
size: 50
outline_alpha: 1.0
outline_width: 3.0
shadow_alpha: 1.0
shadow_blur: 8
output_format: TYPE_DISTANCE_FIELD
render_mode: MODE_MULTI_LAYER
characters: " !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"
```

Minimal font with only required characters (score display):

```protobuf
font: "/example/assets/fonts/Nunito-Black.ttf"
material: "/builtins/fonts/font-df.material"
size: 50
outline_alpha: 1.0
outline_width: 3.0
output_format: TYPE_DISTANCE_FIELD
render_mode: MODE_MULTI_LAYER
characters: " +0123456789"
```

Font with explicit cache width and shadow offset:

```protobuf
font: "/example/assets/fonts/MPLUSRounded1c-Black.ttf"
material: "/builtins/fonts/font-df.material"
size: 50
outline_alpha: 1.0
outline_width: 4.0
shadow_alpha: 1.0
shadow_y: -2.5
output_format: TYPE_DISTANCE_FIELD
cache_width: 2048
render_mode: MODE_MULTI_LAYER
characters: " !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"
```

## Fields reference

### font (required) — `string`

Absolute resource path to the source font file (`.ttf`, `.otf`, or `.fnt`).

```protobuf
font: "/builtins/fonts/vera_mo_bd.ttf"
```

### material (required) — `string`

Absolute resource path to the `.material` file used when rendering this font.

Common built-in materials:
- `/builtins/fonts/font.material` — bitmap fonts
- `/builtins/fonts/font-df.material` — distance field fonts
- `/builtins/fonts/font-fnt.material` — BMFonts

The material **must** match the `output_format`:
- `TYPE_BITMAP` → `font.material`
- `TYPE_DISTANCE_FIELD` → `font-df.material`
- BMFont (`.fnt`) → `font-fnt.material`

```protobuf
material: "/builtins/fonts/font.material"
```

### size (required) — `uint32`

Target glyph size in pixels. Integer, no decimal point.

```protobuf
size: 24
```

### antialias (optional) — `uint32`

Antialiasing level. Default: `1`. Set to `0` for pixel-perfect font rendering.

**Omission rule**: Omit if `1`.

```protobuf
antialias: 0
```

### alpha (optional) — `float`

Transparency of the glyph face. Range: `0.0`–`1.0`. Default: `1.0` (opaque).

**Omission rule**: Omit if `1.0`.

```protobuf
alpha: 0.8
```

### outline_alpha (optional) — `float`

Transparency of the generated outline. Range: `0.0`–`1.0`. Default: `0.0` (transparent / no outline).

**Omission rule**: Omit if `0.0`.

```protobuf
outline_alpha: 1.0
```

### outline_width (optional) — `float`

Width of the generated outline in pixels. Default: `0.0` (no outline).

**Omission rule**: Omit if `0.0`.

```protobuf
outline_width: 2.0
```

### shadow_alpha (optional) — `float`

Transparency of the generated shadow. Range: `0.0`–`1.0`. Default: `0.0` (transparent / no shadow).

Shadow support is enabled by built-in font material shaders. If you don't need shadow support, keep this at `0.0` to avoid unnecessary memory usage.

**Omission rule**: Omit if `0.0`.

```protobuf
shadow_alpha: 0.5
```

### shadow_blur (optional) — `uint32`

Shadow blur amount. Default: `0`. For bitmap fonts, this is the number of blur kernel passes. For distance field fonts, this is the pixel width of the blur.

**Omission rule**: Omit if `0`.

```protobuf
shadow_blur: 2
```

### shadow_x (optional) — `float`

Horizontal shadow offset in pixels. Default: `0.0`. Only affects rendering when `render_mode` is `MODE_MULTI_LAYER`.

**Omission rule**: Omit if `0.0`.

```protobuf
shadow_x: 2.0
```

### shadow_y (optional) — `float`

Vertical shadow offset in pixels. Default: `0.0`. Only affects rendering when `render_mode` is `MODE_MULTI_LAYER`.

**Omission rule**: Omit if `0.0`.

```protobuf
shadow_y: -2.0
```

### extra_characters (deprecated) — `string`

Deprecated field. Default: `""`. Do not use — use `characters` instead.

**Omission rule**: Always omit.

### output_format (optional) — enum `FontTextureFormat`

Type of font data generated. Default: `TYPE_BITMAP`.

| Value | Description |
|-------|-------------|
| `TYPE_BITMAP` | Bitmap texture (default). Color channels encode face, outline, and shadow. |
| `TYPE_DISTANCE_FIELD` | Distance field texture. Requires a DF material. Better for upscaling. |

**Omission rule**: Omit if `TYPE_BITMAP`.

```protobuf
output_format: TYPE_DISTANCE_FIELD
```

### all_chars (optional) — `bool`

Include all glyphs available in the source font file. Default: `false`.

**Omission rule**: Omit if `false`.

```protobuf
all_chars: true
```

### cache_width (optional) — `uint32`

Width of the glyph cache bitmap in pixels. Default: `0` (automatic, grows up to 2048).

**Omission rule**: Omit if `0`.

```protobuf
cache_width: 512
```

### cache_height (optional) — `uint32`

Height of the glyph cache bitmap in pixels. Default: `0` (automatic, grows up to 4096).

**Omission rule**: Omit if `0`.

```protobuf
cache_height: 512
```

### render_mode (optional) — enum `FontRenderMode`

Glyph rendering mode. Default: `MODE_SINGLE_LAYER`.

| Value | Description |
|-------|-------------|
| `MODE_SINGLE_LAYER` | Single quad per character (default). |
| `MODE_MULTI_LAYER` | Separate quads for face, outline, and shadow. Prevents overlapping glyphs and enables proper shadow offset via `shadow_x`/`shadow_y`. |

**Omission rule**: Omit if `MODE_SINGLE_LAYER`.

```protobuf
render_mode: MODE_MULTI_LAYER
```

### characters (optional) — `string`

Characters to include in the font. Default: `""`. Typically set to ASCII printable range (codes 32–126). Always wrap in double quotes.

For runtime fonts, this text acts as a cache prewarming hint.

```protobuf
characters: " !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"
```

## Protobuf Text Format rules

1. **Default omission**: Omit fields that equal their proto default. This keeps files minimal and matches Defold editor behavior.
2. **Floats**: Always include decimal point: `1.0`, not `1`.
3. **Integers**: No decimal point: `4`, not `4.0`.
4. **Strings**: Always double-quoted: `"text"`.
5. **Enums**: Use the enum constant name without quotes: `TYPE_DISTANCE_FIELD`.
6. **Booleans**: `true` or `false`, no quotes.
7. **Field order**: Follow the proto field number order: `font`, `material`, `size`, `antialias`, `alpha`, `outline_alpha`, `outline_width`, `shadow_alpha`, `shadow_blur`, `shadow_x`, `shadow_y`, `output_format`, `all_chars`, `cache_width`, `cache_height`, `render_mode`, `characters`.
8. **No trailing commas or semicolons**.
9. **No field number tags** — use field names only.
10. **No empty lines** between scalar fields (font files have no message blocks).

## Best practices

- **Use base size 50** for all fonts. This provides enough detail for distance field generation and crisp rendering at any display size.
- **Always use distance field** (`output_format: TYPE_DISTANCE_FIELD` with `material: "/builtins/fonts/font-df.material"`). Distance field fonts look sharp at any scale — both upscaled and downscaled — unlike bitmap fonts that become blocky when enlarged.
- **Scale labels via game object**, not font size. Keep `size: 50` in the `.font` file and use the game object's `scale` property to adjust the visual size of Label components on the scene. This way a single font resource works for all text sizes in the game.
- **Always include `~` and space in `characters`**. The space character is required for word spacing — without it, spaces won't render. The `~` character is used as a fallback substitute for missing glyphs.

## Common templates

### Recommended: distance field font (ASCII)

```protobuf
font: "/assets/fonts/my_font.ttf"
material: "/builtins/fonts/font-df.material"
size: 50
output_format: TYPE_DISTANCE_FIELD
characters: " !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"
```

### Distance field font with outline and shadow (multi-layer)

```protobuf
font: "/assets/fonts/my_font.ttf"
material: "/builtins/fonts/font-df.material"
size: 50
outline_alpha: 1.0
outline_width: 3.0
shadow_alpha: 1.0
shadow_blur: 8
output_format: TYPE_DISTANCE_FIELD
render_mode: MODE_MULTI_LAYER
characters: " !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"
```

### All characters included

```protobuf
font: "/assets/fonts/my_font.ttf"
material: "/builtins/fonts/font-df.material"
size: 50
output_format: TYPE_DISTANCE_FIELD
all_chars: true
```

### BMFont

```protobuf
font: "/assets/fonts/my_bmfont.fnt"
material: "/builtins/fonts/font-fnt.material"
size: 50
characters: " !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"
```

### Bitmap font (no antialiasing, pixel-perfect)

```protobuf
font: "/assets/fonts/pixel_font.ttf"
material: "/builtins/fonts/font.material"
size: 16
antialias: 0
characters: " !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"
```

## Workflow

### Creating a new font

1. Determine the file path (must end with `.font`).
2. Set required fields: `font` (source font path), `material` (matching material), `size`.
3. Set `characters` with desired glyphs or enable `all_chars`.
4. Choose `output_format` and matching `material` if not using default bitmap.
5. Add optional fields (outline, shadow, cache) only if they differ from defaults.
6. Write the file using the field order from the reference above.

### Editing an existing font

1. Read the current `.font` file.
2. Modify only the requested fields.
3. Preserve existing field values and order.
4. Apply omission rules: remove fields that become equal to their defaults after editing.
5. If changing `output_format`, ensure `material` is updated to match.
