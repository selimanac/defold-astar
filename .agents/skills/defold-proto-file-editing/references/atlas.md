# Atlas (.atlas)

Proto message: `Atlas` from `atlas_ddf.proto`.

An atlas combines multiple images into larger texture pages for efficient rendering. Used as image sources for Sprite, ParticleFX, and other visual components.

## Canonical example

```protobuf
images {
  image: "/assets/images/logo.png"
}
margin: 4
extrude_borders: 2
```

## Top-level fields reference

### images (repeated) — `AtlasImage`

Individual images in the atlas. Each image is a separate `images { ... }` block.

```protobuf
images {
  image: "/assets/images/player.png"
}
images {
  image: "/assets/images/enemy.png"
}
```

### animations (repeated) — `AtlasAnimation`

Flipbook animation groups. Each animation is a separate `animations { ... }` block containing a sequence of images.

```protobuf
animations {
  id: "walk"
  images {
    image: "/assets/images/walk_01.png"
  }
  images {
    image: "/assets/images/walk_02.png"
  }
  fps: 12
  playback: PLAYBACK_LOOP_FORWARD
}
```

### margin (optional) — `uint32`

Pixels added between each image. Default: `0`. **Omit if `0`.**

### extrude_borders (optional) — `uint32`

Edge pixels repeatedly padded around each image. Prevents neighbor image bleeding. Default: `0`. **Omit if `0`.**

### inner_padding (optional) — `uint32`

Empty pixels padded around each image. Default: `0`. **Omit if `0`.**

### max_page_width (optional) — `uint32`

Maximum width of a page in a multi-page atlas (pixels). Default: `0` (no limit). **Omit if `0`.**

### max_page_height (optional) — `uint32`

Maximum height of a page in a multi-page atlas (pixels). Default: `0` (no limit). **Omit if `0`.**

### rename_patterns (optional) — `string`

Comma-separated `search=replace` patterns to rename animation IDs. **Omit if empty.**

## AtlasImage fields

### image (required) — `string`

Absolute resource path to a `.png` image file.

### sprite_trim_mode (optional) — enum `SpriteTrimmingMode`

How the sprite geometry is generated. Default: `SPRITE_TRIM_MODE_OFF`. **Omit if `SPRITE_TRIM_MODE_OFF`.**

Values: `SPRITE_TRIM_MODE_OFF`, `SPRITE_TRIM_MODE_4`, `SPRITE_TRIM_MODE_5`, `SPRITE_TRIM_MODE_6`, `SPRITE_TRIM_MODE_7`, `SPRITE_TRIM_MODE_8`, `SPRITE_TRIM_POLYGONS`.

Note: sprite trimming does not work with slice-9 sprites.

### pivot_x (optional) — `float`

Horizontal pivot. Default: `0.5` (center). **Omit if `0.5`.**

### pivot_y (optional) — `float`

Vertical pivot. Default: `0.5` (center). **Omit if `0.5`.**

## AtlasAnimation fields

### id (required) — `string`

Animation name. Used via `sprite.play_flipbook()`.

### images (repeated) — `AtlasImage`

Ordered list of frames.

### playback (optional) — enum `Playback`

Default: `PLAYBACK_ONCE_FORWARD`. **Omit if `PLAYBACK_ONCE_FORWARD`.**

Values: `PLAYBACK_NONE`, `PLAYBACK_ONCE_FORWARD`, `PLAYBACK_ONCE_BACKWARD`, `PLAYBACK_ONCE_PINGPONG`, `PLAYBACK_LOOP_FORWARD`, `PLAYBACK_LOOP_BACKWARD`, `PLAYBACK_LOOP_PINGPONG`.

### fps (optional) — `uint32`

Frames per second. Default: `30`. **Omit if `30`.**

### flip_horizontal (optional) — `uint32`

`0` = no flip, `1` = flip. Default: `0`. **Omit if `0`.**

### flip_vertical (optional) — `uint32`

`0` = no flip, `1` = flip. Default: `0`. **Omit if `0`.**

## Complete examples

### Atlas with animation

```protobuf
images {
  image: "/assets/images/idle.png"
}
animations {
  id: "walk"
  images {
    image: "/assets/images/walk_01.png"
  }
  images {
    image: "/assets/images/walk_02.png"
  }
  images {
    image: "/assets/images/walk_03.png"
  }
  images {
    image: "/assets/images/walk_04.png"
  }
  playback: PLAYBACK_LOOP_FORWARD
  fps: 12
}
margin: 2
extrude_borders: 2
```

### Multi-page atlas with trimming

```protobuf
images {
  image: "/assets/images/large_bg.png"
  sprite_trim_mode: SPRITE_TRIM_MODE_4
}
images {
  image: "/assets/images/ui_panel.png"
  sprite_trim_mode: SPRITE_TRIM_MODE_6
}
margin: 2
extrude_borders: 2
max_page_width: 2048
max_page_height: 2048
```
