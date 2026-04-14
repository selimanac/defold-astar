# Editing Materials

Creates and edits Defold `.material` resource files using Protobuf Text Format.

## Overview

A Material defines how a graphical component (sprite, tilemap, font, GUI node, model, mesh, etc.) is rendered. It holds tags used by the render pipeline to select objects for rendering, references to vertex and fragment shader programs, shader constants (uniforms), texture samplers, and custom vertex attributes. Materials are referenced from components and listed in render resources.

## File format

Material files (`.material`) use **Protobuf Text Format** based on the `MaterialDesc` message from `render/material_ddf.proto`.

### Canonical example — sprite material with user constant

```protobuf
name: "sprite"
tags: "tile"
vertex_program: "/example/recolor.vp"
fragment_program: "/example/recolor.fp"
vertex_constants {
  name: "view_proj"
  type: CONSTANT_TYPE_VIEWPROJ
}
fragment_constants {
  name: "tint"
  type: CONSTANT_TYPE_USER
  value {
    x: 1.0
    y: 1.0
    z: 1.0
    w: 1.0
  }
}
samplers {
  name: "texture_sampler"
  wrap_u: WRAP_MODE_CLAMP_TO_EDGE
  wrap_v: WRAP_MODE_CLAMP_TO_EDGE
  filter_min: FILTER_MODE_MIN_DEFAULT
  filter_mag: FILTER_MODE_MAG_DEFAULT
}
```

### Canonical example — 3D model material with local vertex space

```protobuf
name: "unlit"
tags: "model"
vertex_program: "/example/unlit.vp"
fragment_program: "/example/unlit.fp"
vertex_space: VERTEX_SPACE_LOCAL
vertex_constants {
  name: "mtx_view"
  type: CONSTANT_TYPE_VIEW
}
vertex_constants {
  name: "mtx_proj"
  type: CONSTANT_TYPE_PROJECTION
}
fragment_constants {
  name: "tint"
  type: CONSTANT_TYPE_USER
  value {
    x: 1.0
    y: 1.0
    z: 1.0
    w: 1.0
  }
}
samplers {
  name: "texture0"
  wrap_u: WRAP_MODE_CLAMP_TO_EDGE
  wrap_v: WRAP_MODE_CLAMP_TO_EDGE
  filter_min: FILTER_MODE_MIN_LINEAR
  filter_mag: FILTER_MODE_MAG_LINEAR
  max_anisotropy: 0.0
}
```

## Fields reference

Fields are listed in proto field number order.

### name (required) — `string`

The identity of the material. This name is used in render resources and with `render.enable_material()`. Must be unique within the project.

```protobuf
name: "my_material"
```

### tags (repeated) — `string`

Tags used by `render.predicate()` to collect components for rendering. Each tag gets its own line. The maximum number of tags across a project is 32. Common tags: `"tile"` (sprites, tilemaps), `"model"` (3D models), `"gui"` (GUI), `"text"` (fonts/labels), `"particle"` (particle effects), `"debug"` (debug rendering).

```protobuf
tags: "tile"
```

Multiple tags:

```protobuf
tags: "model"
tags: "custom_pass"
```

**Omission rule**: Omit if no tags are needed (but materials almost always need at least one tag to be rendered).

### vertex_program (required) — `string`

Absolute resource path to a vertex shader program file (`.vp`).

```protobuf
vertex_program: "/builtins/materials/sprite.vp"
```

### fragment_program (required) — `string`

Absolute resource path to a fragment shader program file (`.fp`).

```protobuf
fragment_program: "/builtins/materials/sprite.fp"
```

### vertex_space (optional) — enum `VertexSpace`

Controls the coordinate space of vertex data passed to the vertex shader. Default: `VERTEX_SPACE_WORLD` (value `0`).

| Value | Description |
|-------|-------------|
| `VERTEX_SPACE_WORLD` | Vertices are in world space (default). Used for 2D components like sprites that are batched in world space. |
| `VERTEX_SPACE_LOCAL` | Vertices are in local (object) space. Used for 3D models where vertex transform is done in the shader. Required for instancing. |

**Omission rule**: Omit if `VERTEX_SPACE_WORLD`.

```protobuf
vertex_space: VERTEX_SPACE_LOCAL
```

### vertex_constants (repeated) — `Constant`

Shader uniforms passed to the vertex shader program. Each entry gets its own `vertex_constants { ... }` block.

```protobuf
vertex_constants {
  name: "view_proj"
  type: CONSTANT_TYPE_VIEWPROJ
}
```

**Omission rule**: Omit if no vertex constants are needed.

### fragment_constants (repeated) — `Constant`

Shader uniforms passed to the fragment shader program. Each entry gets its own `fragment_constants { ... }` block.

```protobuf
fragment_constants {
  name: "tint"
  type: CONSTANT_TYPE_USER
  value {
    x: 1.0
    y: 1.0
    z: 1.0
    w: 1.0
  }
}
```

**Omission rule**: Omit if no fragment constants are needed.

### textures (deprecated, repeated) — `string`

Legacy texture list. **Do not use.** Use `samplers` with a `texture` field instead.

**Omission rule**: Always omit.

### samplers (repeated) — `Sampler`

Texture sampler configurations. Each entry defines a sampler name (matching a `sampler2D` uniform in the shader), wrap modes, filter settings, optional max anisotropy, and an optional texture resource.

Sprite, tilemap, GUI, and particle effect components automatically bind the first `sampler2D` to the component's image. For model components, samplers must be configured explicitly in the material to allow texture assignment in the editor.

```protobuf
samplers {
  name: "texture_sampler"
  wrap_u: WRAP_MODE_CLAMP_TO_EDGE
  wrap_v: WRAP_MODE_CLAMP_TO_EDGE
  filter_min: FILTER_MODE_MIN_DEFAULT
  filter_mag: FILTER_MODE_MAG_DEFAULT
}
```

**Omission rule**: Omit if no samplers need explicit configuration (global graphics settings will apply).

### max_page_count (optional) — `uint32`

Maximum number of texture pages. Default: `0` (unlimited).

**Omission rule**: Omit if `0`.

```protobuf
max_page_count: 4
```

### attributes (repeated) — `dmGraphics.VertexAttribute`

Custom vertex attributes that provide additional per-vertex or per-instance data to the shader. Each entry defines an attribute name, optional semantic type, data type, vector type, step function, and default values.

```protobuf
attributes {
  name: "mycolor"
  double_values {
    v: 1.0
    v: 1.0
    v: 1.0
    v: 1.0
  }
}
```

**Omission rule**: Omit if no custom attributes are needed.

## Nested message: Constant

Each `vertex_constants` or `fragment_constants` entry contains:

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `name` | `string` | yes | The uniform name in the shader program |
| `type` | `ConstantType` | yes | The type of constant |
| `value` | repeated `dmMath.Vector4` | no | Initial values. Required for `CONSTANT_TYPE_USER` and `CONSTANT_TYPE_USER_MATRIX4`. Omit for engine-provided constants. |

### ConstantType enum

| Value | Description |
|-------|-------------|
| `CONSTANT_TYPE_USER` | A `vec4` constant for custom data. Mutable via `go.set()` / `go.animate()`. |
| `CONSTANT_TYPE_USER_MATRIX4` | A `mat4` constant for custom data. Mutable via `go.set()`. |
| `CONSTANT_TYPE_VIEWPROJ` | Combined view and projection matrix. |
| `CONSTANT_TYPE_WORLD` | World transform matrix. |
| `CONSTANT_TYPE_VIEW` | View (camera) matrix. |
| `CONSTANT_TYPE_PROJECTION` | Projection matrix. |
| `CONSTANT_TYPE_NORMAL` | Normal matrix (transpose inverse of world-view). |
| `CONSTANT_TYPE_WORLDVIEW` | Combined world and view matrix. |
| `CONSTANT_TYPE_WORLDVIEWPROJ` | Combined world, view, and projection matrix. |
| `CONSTANT_TYPE_TEXTURE` | Texture matrix. |

### Constant with user value

For `CONSTANT_TYPE_USER`, provide a `value` block with a `dmMath.Vector4` (components `x`, `y`, `z`, `w`, all default to `0.0`). Only include components that differ from `0.0`:

```protobuf
fragment_constants {
  name: "tint"
  type: CONSTANT_TYPE_USER
  value {
    x: 1.0
    y: 1.0
    z: 1.0
    w: 1.0
  }
}
```

For `CONSTANT_TYPE_USER_MATRIX4`, provide four `value` blocks (one per matrix row).

### Constant with engine-provided value

For engine-provided types (`VIEWPROJ`, `WORLD`, `VIEW`, `PROJECTION`, `NORMAL`, `WORLDVIEW`, `WORLDVIEWPROJ`, `TEXTURE`), omit the `value` field:

```protobuf
vertex_constants {
  name: "mtx_view"
  type: CONSTANT_TYPE_VIEW
}
```

## Nested message: Sampler

Each `samplers` entry contains:

| Field | Type | Required | Default | Description |
|-------|------|----------|---------|-------------|
| `name` | `string` | yes | — | Sampler name matching the `sampler2D` uniform in the shader |
| `wrap_u` | `WrapMode` | yes | — | Horizontal wrap mode |
| `wrap_v` | `WrapMode` | yes | — | Vertical wrap mode |
| `filter_min` | `FilterModeMin` | yes | — | Minification filter |
| `filter_mag` | `FilterModeMag` | yes | — | Magnification filter |
| `max_anisotropy` | `float` | no | `1.0` | Anisotropic filtering level. Set to `0.0` to disable. |
| `texture` | `string` | no | `""` | Absolute resource path to a texture/image. Optional. |

### WrapMode enum

| Value | Description |
|-------|-------------|
| `WRAP_MODE_REPEAT` | Repeats texture data outside [0,1] range (default, value `0`) |
| `WRAP_MODE_MIRRORED_REPEAT` | Repeats with mirroring every second repetition |
| `WRAP_MODE_CLAMP_TO_EDGE` | Clamps to edge pixels |

### FilterModeMin enum

| Value | Description |
|-------|-------------|
| `FILTER_MODE_MIN_NEAREST` | Nearest texel (value `0`) |
| `FILTER_MODE_MIN_LINEAR` | Weighted linear average of 2×2 texels |
| `FILTER_MODE_MIN_NEAREST_MIPMAP_NEAREST` | Nearest texel within nearest mipmap |
| `FILTER_MODE_MIN_NEAREST_MIPMAP_LINEAR` | Nearest texel, linear between two mipmaps |
| `FILTER_MODE_MIN_LINEAR_MIPMAP_NEAREST` | Linear within nearest mipmap |
| `FILTER_MODE_MIN_LINEAR_MIPMAP_LINEAR` | Linear within and between mipmaps (trilinear) |
| `FILTER_MODE_MIN_DEFAULT` | Uses `game.project` Graphics settings |

### FilterModeMag enum

| Value | Description |
|-------|-------------|
| `FILTER_MODE_MAG_NEAREST` | Nearest texel (value `0`) |
| `FILTER_MODE_MAG_LINEAR` | Linear interpolation |
| `FILTER_MODE_MAG_DEFAULT` | Uses `game.project` Graphics settings |

### Sampler with default filter (for sprites)

```protobuf
samplers {
  name: "texture_sampler"
  wrap_u: WRAP_MODE_CLAMP_TO_EDGE
  wrap_v: WRAP_MODE_CLAMP_TO_EDGE
  filter_min: FILTER_MODE_MIN_DEFAULT
  filter_mag: FILTER_MODE_MAG_DEFAULT
}
```

### Sampler with linear filter (for 3D models)

```protobuf
samplers {
  name: "texture0"
  wrap_u: WRAP_MODE_CLAMP_TO_EDGE
  wrap_v: WRAP_MODE_CLAMP_TO_EDGE
  filter_min: FILTER_MODE_MIN_LINEAR
  filter_mag: FILTER_MODE_MAG_LINEAR
  max_anisotropy: 0.0
}
```

### Sampler with repeating wrap (for tiled textures)

```protobuf
samplers {
  name: "texture0"
  wrap_u: WRAP_MODE_REPEAT
  wrap_v: WRAP_MODE_REPEAT
  filter_min: FILTER_MODE_MIN_LINEAR
  filter_mag: FILTER_MODE_MAG_LINEAR
  max_anisotropy: 0.0
}
```

## Nested message: VertexAttribute

Each `attributes` entry contains:

| Field | Type | Required | Default | Description |
|-------|------|----------|---------|-------------|
| `name` | `string` | yes | — | Attribute name matching the shader `attribute` declaration |
| `semantic_type` | `SemanticType` | no | `SEMANTIC_TYPE_NONE` | Semantic meaning of the attribute |
| `normalize` | `bool` | no | `false` | Whether GPU normalizes values |
| `data_type` | `DataType` | no | `TYPE_FLOAT` | Backing data type |
| `coordinate_space` | `CoordinateSpace` | no | `COORDINATE_SPACE_LOCAL` | Coordinate space for position/normal semantics |
| `step_function` | `VertexStepFunction` | no | `VERTEX_STEP_FUNCTION_VERTEX` | Per-vertex or per-instance stepping |
| `vector_type` | `VectorType` | no | `VECTOR_TYPE_VEC4` | Vector dimensionality |
| `double_values` | `DoubleValues` | no | — | Float attribute values (repeated `v` entries) |
| `long_values` | `LongValues` | no | — | Integer attribute values (repeated `v` entries) |

**Omission rules for attribute sub-fields**: Omit `semantic_type` if `SEMANTIC_TYPE_NONE`. Omit `normalize` if `false`. Omit `data_type` if `TYPE_FLOAT`. Omit `coordinate_space` if `COORDINATE_SPACE_LOCAL`. Omit `step_function` if `VERTEX_STEP_FUNCTION_VERTEX`. Omit `vector_type` if `VECTOR_TYPE_VEC4`. Omit value block if no default values are needed.

### SemanticType enum

| Value | Description |
|-------|-------------|
| `SEMANTIC_TYPE_NONE` | No special semantic (default) |
| `SEMANTIC_TYPE_POSITION` | Per-vertex position data |
| `SEMANTIC_TYPE_TEXCOORD` | Per-vertex texture coordinates |
| `SEMANTIC_TYPE_PAGE_INDEX` | Per-vertex page indices |
| `SEMANTIC_TYPE_COLOR` | Color data (shows color picker in editor) |
| `SEMANTIC_TYPE_NORMAL` | Per-vertex normal data |
| `SEMANTIC_TYPE_TANGENT` | Per-vertex tangent data |
| `SEMANTIC_TYPE_WORLD_MATRIX` | Per-vertex world matrix |
| `SEMANTIC_TYPE_NORMAL_MATRIX` | Per-vertex normal matrix |

### DataType enum

| Value | Description |
|-------|-------------|
| `TYPE_BYTE` | Signed 8-bit |
| `TYPE_UNSIGNED_BYTE` | Unsigned 8-bit |
| `TYPE_SHORT` | Signed 16-bit |
| `TYPE_UNSIGNED_SHORT` | Unsigned 16-bit |
| `TYPE_INT` | Signed 32-bit |
| `TYPE_UNSIGNED_INT` | Unsigned 32-bit |
| `TYPE_FLOAT` | Floating point (default) |

### VectorType enum

| Value | Description |
|-------|-------------|
| `VECTOR_TYPE_SCALAR` | Single scalar |
| `VECTOR_TYPE_VEC2` | 2D vector |
| `VECTOR_TYPE_VEC3` | 3D vector |
| `VECTOR_TYPE_VEC4` | 4D vector (default) |
| `VECTOR_TYPE_MAT2` | 2×2 matrix |
| `VECTOR_TYPE_MAT3` | 3×3 matrix |
| `VECTOR_TYPE_MAT4` | 4×4 matrix |

### Simple custom color attribute

```protobuf
attributes {
  name: "newcolor"
  semantic_type: SEMANTIC_TYPE_COLOR
  double_values {
    v: 0.3882
    v: 0.6078
    v: 1.0
    v: 1.0
  }
}
```

### Position attribute with vec2 type

```protobuf
attributes {
  name: "position_local"
  semantic_type: SEMANTIC_TYPE_POSITION
  vector_type: VECTOR_TYPE_VEC2
}
```

### Per-instance attribute (for instancing)

```protobuf
attributes {
  name: "instance_color"
  step_function: VERTEX_STEP_FUNCTION_INSTANCE
  double_values {
    v: 1.0
    v: 1.0
    v: 1.0
    v: 1.0
  }
}
```

## Protobuf Text Format rules

1. **Default omission**: Omit fields that equal their proto default. This keeps files minimal and matches Defold editor behavior.
2. **Message blocks**: Use `field_name { ... }` with nested `key: value` pairs.
3. **Floats**: Always include decimal point: `1.0`, not `1`.
4. **Integers**: No decimal point: `4`, not `4.0`.
5. **Strings**: Always double-quoted: `"text"`.
6. **Enums**: Use the enum constant name without quotes.
7. **Booleans**: `true` or `false`, no quotes.
8. **Repeated fields**: Each value gets its own line with the field name.
9. **Repeated messages**: Each entry gets its own `field_name { ... }` block.
10. **Field order**: Follow the proto field number order: `name`, `tags`, `vertex_program`, `fragment_program`, `vertex_space`, `vertex_constants`, `fragment_constants`, `samplers`, `max_page_count`, `attributes`.
11. **No trailing commas or semicolons**.
12. **No field number tags** — use field names only.
13. **Indentation**: 2 spaces per nesting level inside message blocks.
14. **Newlines**: One empty line between the end of a message block `}` and the next field. No empty line between consecutive scalar fields.

## Common templates

### Sprite material (default-like)

```protobuf
name: "sprite"
tags: "tile"
vertex_program: "/assets/materials/sprite.vp"
fragment_program: "/assets/materials/sprite.fp"
vertex_constants {
  name: "view_proj"
  type: CONSTANT_TYPE_VIEWPROJ
}
fragment_constants {
  name: "tint"
  type: CONSTANT_TYPE_USER
  value {
    x: 1.0
    y: 1.0
    z: 1.0
    w: 1.0
  }
}
samplers {
  name: "texture_sampler"
  wrap_u: WRAP_MODE_CLAMP_TO_EDGE
  wrap_v: WRAP_MODE_CLAMP_TO_EDGE
  filter_min: FILTER_MODE_MIN_DEFAULT
  filter_mag: FILTER_MODE_MAG_DEFAULT
}
```

### Unlit 3D model material

```protobuf
name: "unlit"
tags: "model"
vertex_program: "/assets/materials/unlit.vp"
fragment_program: "/assets/materials/unlit.fp"
vertex_space: VERTEX_SPACE_LOCAL
vertex_constants {
  name: "mtx_view"
  type: CONSTANT_TYPE_VIEW
}
vertex_constants {
  name: "mtx_proj"
  type: CONSTANT_TYPE_PROJECTION
}
fragment_constants {
  name: "tint"
  type: CONSTANT_TYPE_USER
  value {
    x: 1.0
    y: 1.0
    z: 1.0
    w: 1.0
  }
}
samplers {
  name: "texture0"
  wrap_u: WRAP_MODE_CLAMP_TO_EDGE
  wrap_v: WRAP_MODE_CLAMP_TO_EDGE
  filter_min: FILTER_MODE_MIN_LINEAR
  filter_mag: FILTER_MODE_MAG_LINEAR
  max_anisotropy: 0.0
}
```

### Model material with multiple samplers

```protobuf
name: "multi_tex"
tags: "model"
vertex_program: "/assets/materials/multi_tex.vp"
fragment_program: "/assets/materials/multi_tex.fp"
vertex_space: VERTEX_SPACE_LOCAL
vertex_constants {
  name: "mtx_world"
  type: CONSTANT_TYPE_WORLD
}
vertex_constants {
  name: "mtx_view"
  type: CONSTANT_TYPE_VIEW
}
vertex_constants {
  name: "mtx_proj"
  type: CONSTANT_TYPE_PROJECTION
}
samplers {
  name: "texture0"
  wrap_u: WRAP_MODE_CLAMP_TO_EDGE
  wrap_v: WRAP_MODE_CLAMP_TO_EDGE
  filter_min: FILTER_MODE_MIN_LINEAR
  filter_mag: FILTER_MODE_MAG_LINEAR
  max_anisotropy: 0.0
}
samplers {
  name: "texture_pattern"
  wrap_u: WRAP_MODE_REPEAT
  wrap_v: WRAP_MODE_REPEAT
  filter_min: FILTER_MODE_MIN_LINEAR
  filter_mag: FILTER_MODE_MAG_LINEAR
  max_anisotropy: 0.0
}
```

### Sprite material with custom vertex attributes

```protobuf
name: "sprite"
tags: "tile"
vertex_program: "/assets/materials/custom_sprite.vp"
fragment_program: "/assets/materials/custom_sprite.fp"
vertex_constants {
  name: "view_proj"
  type: CONSTANT_TYPE_VIEWPROJ
}
samplers {
  name: "texture_sampler"
  wrap_u: WRAP_MODE_CLAMP_TO_EDGE
  wrap_v: WRAP_MODE_CLAMP_TO_EDGE
  filter_min: FILTER_MODE_MIN_DEFAULT
  filter_mag: FILTER_MODE_MAG_DEFAULT
}
attributes {
  name: "mycolor"
  semantic_type: SEMANTIC_TYPE_COLOR
  double_values {
    v: 1.0
    v: 1.0
    v: 1.0
    v: 1.0
  }
}
```

### Sprite material with local UV coordinates

```protobuf
name: "sprite"
tags: "tile"
vertex_program: "/assets/materials/sprite_local_uv.vp"
fragment_program: "/assets/materials/sprite_local_uv.fp"
vertex_constants {
  name: "view_proj"
  type: CONSTANT_TYPE_VIEWPROJ
}
fragment_constants {
  name: "tint"
  type: CONSTANT_TYPE_USER
  value {
    x: 1.0
    y: 1.0
    z: 1.0
    w: 1.0
  }
}
samplers {
  name: "texture_sampler"
  wrap_u: WRAP_MODE_CLAMP_TO_EDGE
  wrap_v: WRAP_MODE_CLAMP_TO_EDGE
  filter_min: FILTER_MODE_MIN_DEFAULT
  filter_mag: FILTER_MODE_MAG_DEFAULT
}
attributes {
  name: "position_local"
  semantic_type: SEMANTIC_TYPE_POSITION
  vector_type: VECTOR_TYPE_VEC2
}
attributes {
  name: "sprite_size"
  double_values {
    v: 64.0
    v: 64.0
  }
  vector_type: VECTOR_TYPE_VEC2
}
```

### Instanced 3D model material

```protobuf
name: "instanced_model"
tags: "model"
vertex_program: "/assets/materials/instanced.vp"
fragment_program: "/assets/materials/instanced.fp"
vertex_space: VERTEX_SPACE_LOCAL
vertex_constants {
  name: "mtx_view"
  type: CONSTANT_TYPE_VIEW
}
vertex_constants {
  name: "mtx_proj"
  type: CONSTANT_TYPE_PROJECTION
}
samplers {
  name: "texture0"
  wrap_u: WRAP_MODE_CLAMP_TO_EDGE
  wrap_v: WRAP_MODE_CLAMP_TO_EDGE
  filter_min: FILTER_MODE_MIN_LINEAR
  filter_mag: FILTER_MODE_MAG_LINEAR
  max_anisotropy: 0.0
}
attributes {
  name: "instance_color"
  step_function: VERTEX_STEP_FUNCTION_INSTANCE
  double_values {
    v: 1.0
    v: 1.0
    v: 1.0
    v: 1.0
  }
}
```

## Workflow

### Creating a new material

1. Determine the file path (must end with `.material`).
2. Set the required `name` field — should be unique and descriptive.
3. Add at least one `tags` entry to ensure the material is included in a render predicate.
4. Set `vertex_program` and `fragment_program` to the shader file paths.
5. Set `vertex_space: VERTEX_SPACE_LOCAL` if creating a material for 3D models (required for instancing). Omit for 2D components (sprites, tilemaps) which use world space by default.
6. Add `vertex_constants` and/or `fragment_constants` for shader uniforms. Engine-provided constants need only `name` and `type`. User constants also need a `value` block.
7. Add `samplers` entries for texture configuration. Match sampler names to `sampler2D` uniforms in the shaders.
8. Add `attributes` if custom vertex attributes are needed.
9. Write the file using the field order from the reference above.

### Editing an existing material

1. Read the current `.material` file.
2. Modify only the requested fields.
3. Preserve existing field values and order.
4. Apply omission rules: remove fields that become equal to their defaults after editing.
