# Editing Cameras

Creates and edits Defold `.camera` component files using Protobuf Text Format.

## Overview

A Camera component changes the viewport and projection of the game world. It provides view and projection matrices to the render script, supporting both perspective (3D) and orthographic (2D) projections.

**Important**: Camera components are almost never created as separate `.camera` files. The standard practice is to embed them directly inside a game object within a `.collection` file as an `embedded_components` block. All examples in this project follow this pattern.

## File format

Camera files (`.camera`) use **Protobuf Text Format** based on the `CameraDesc` message from `gamesys/camera_ddf.proto`.

### Canonical example (standalone `.camera` file)

```protobuf
aspect_ratio: 1.0
fov: 0.7854
near_z: -1.0
far_z: 1.0
orthographic_projection: 1
orthographic_mode: ORTHO_MODE_AUTO_COVER
```

### Embedded in a collection (typical usage)

Camera components are typically embedded as part of a dedicated game object in a `.collection` file. This is the most common way to use cameras in Defold:

```protobuf
embedded_instances {
  id: "camera"
  data: "embedded_components {\n"
  "  id: \"camera\"\n"
  "  type: \"camera\"\n"
  "  data: \"aspect_ratio: 1.0\\n"
  "fov: 0.7854\\n"
  "near_z: -1.0\\n"
  "far_z: 1.0\\n"
  "orthographic_projection: 1\\n"
  "orthographic_mode: ORTHO_MODE_AUTO_COVER\\n"
  "\"\n"
  "}\n"
  ""
  position {
    x: 640.0
    y: 360.0
  }
}
```

## Fields reference

### aspect_ratio (required) — `float`

The ratio between the frustum width and height. Used when calculating the projection of a perspective camera. `1.0` means a quadratic view, `1.33` for 4:3, `1.78` for 16:9. Ignored if `auto_aspect_ratio` is set.

```protobuf
aspect_ratio: 1.0
```

### fov (required) — `float`

Vertical camera field of view expressed in radians. Only used for perspective projection. The wider the field of view, the more the camera sees. Common values: `0.7854` (45°), `1.0472` (60°).

```protobuf
fov: 0.7854
```

### near_z (required) — `float`

Z-value of the near clipping plane. For orthographic 2D cameras, typically set to `-1.0`.

```protobuf
near_z: -1.0
```

### far_z (required) — `float`

Z-value of the far clipping plane. For orthographic 2D cameras, typically set to `1.0`. For perspective 3D cameras, use larger values like `1000.0`.

```protobuf
far_z: 1.0
```

### auto_aspect_ratio (optional) — `uint32`

Automatically calculate the aspect ratio based on the window dimensions. `0` = disabled (default), `1` = enabled. Only used for perspective cameras.

**Omission rule**: Omit if `0`.

```protobuf
auto_aspect_ratio: 1
```

### orthographic_projection (optional) — `uint32`

Switch the camera to orthographic projection. `0` = perspective (default), `1` = orthographic. For 2D games, this is almost always set to `1`.

**Omission rule**: Omit if `0`.

```protobuf
orthographic_projection: 1
```

### orthographic_zoom (optional) — `float`

Zoom level for orthographic projection. Default: `1.0`. Values `> 1.0` zoom in, values `< 1.0` zoom out. Only meaningful when `orthographic_projection` is `1`.

**Omission rule**: Omit if `1.0`.

```protobuf
orthographic_zoom: 2.0
```

### orthographic_mode (optional) — `OrthoZoomMode`

Controls how the orthographic camera determines zoom relative to window size and design resolution. Default: `ORTHO_MODE_FIXED`. Only meaningful when `orthographic_projection` is `1`.

**Omission rule**: Omit if `ORTHO_MODE_FIXED`.

```protobuf
orthographic_mode: ORTHO_MODE_AUTO_COVER
```

## Enum: OrthoZoomMode

| Value | Description |
|-------|-------------|
| `ORTHO_MODE_FIXED` | Uses the current `orthographic_zoom` value as-is. |
| `ORTHO_MODE_AUTO_FIT` | Automatically adjusts zoom so the full design area fits inside the window (contain). May show extra content. |
| `ORTHO_MODE_AUTO_COVER` | Automatically adjusts zoom so the design area covers the entire window (cover). May crop content. |

## Runtime properties

These properties can be read/written at runtime via `go.get()` / `go.set()`:

| Property | Type | Read/Write |
|----------|------|------------|
| `fov` | `float` | Read/Write |
| `near_z` | `float` | Read/Write |
| `far_z` | `float` | Read/Write |
| `orthographic_zoom` | `float` | Read/Write |
| `aspect_ratio` | `float` | Read/Write |
| `view` | `matrix4` | Read only |
| `projection` | `matrix4` | Read only |

## Protobuf Text Format rules

1. **Default omission**: Omit fields that equal their proto default.
2. **Floats**: Always include decimal point: `1.0`, not `1`.
3. **Integers**: No decimal point: `1`, not `1.0`.
4. **Enums**: Use the constant name without quotes: `ORTHO_MODE_AUTO_COVER`.
5. **Field order**: Follow the proto field number order: `aspect_ratio`, `fov`, `near_z`, `far_z`, `auto_aspect_ratio`, `orthographic_projection`, `orthographic_zoom`, `orthographic_mode`.
6. **No trailing commas or semicolons**.
7. **No empty lines between fields** (all fields are scalar).

## Common templates

### 2D orthographic camera (auto cover)

The most common setup for 2D games. Position the game object at the center of the design resolution.

```protobuf
aspect_ratio: 1.0
fov: 0.7854
near_z: -1.0
far_z: 1.0
orthographic_projection: 1
orthographic_mode: ORTHO_MODE_AUTO_COVER
```

### 2D orthographic camera (auto fit)

Shows the full design area, may reveal extra content at edges.

```protobuf
aspect_ratio: 1.0
fov: 0.7854
near_z: -1.0
far_z: 1.0
orthographic_projection: 1
orthographic_mode: ORTHO_MODE_AUTO_FIT
```

### 2D orthographic camera (fixed zoom)

Manual zoom control; `orthographic_mode` is omitted because `ORTHO_MODE_FIXED` is the default.

```protobuf
aspect_ratio: 1.0
fov: 0.7854
near_z: -1.0
far_z: 1.0
orthographic_projection: 1
```

### 3D perspective camera

```protobuf
aspect_ratio: 1.78
fov: 0.7854
near_z: 0.1
far_z: 1000.0
auto_aspect_ratio: 1
```

## Workflow

### Creating a new camera

1. Determine whether the camera should be a standalone `.camera` file or embedded in a collection (embedded is the standard practice).
2. Set the four required fields: `aspect_ratio`, `fov`, `near_z`, `far_z`.
3. For 2D games, set `orthographic_projection: 1` and choose an `orthographic_mode`.
4. Add optional fields only if they differ from defaults.
5. If embedded in a collection, position the game object at the center of the design resolution (e.g., `x: 640.0, y: 360.0` for 1280×720).

### Editing an existing camera

1. Read the current `.camera` file or the embedded camera data in the collection.
2. Modify only the requested fields.
3. Preserve existing field values and order.
4. Apply omission rules: remove fields that become equal to their defaults after editing.
