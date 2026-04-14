# Editing Meshes

Creates and edits Defold `.mesh` component files using Protobuf Text Format.

## Overview

A Mesh component renders custom 3D geometry defined by a buffer file (`.buffer`). Unlike the Model component which loads glTF files, Mesh works with raw vertex data streams and can be manipulated at runtime via `buffer.create()`, `buffer.get_stream()`, and `resource.set_buffer()`. Meshes are NOT automatically frustum-culled; AABB metadata must be set manually via `buffer.set_metadata()`. If the material's Vertex Space is set to World Space, the position and normal streams are needed for the engine to transform vertices correctly.

## File format

Mesh files (`.mesh`) use **Protobuf Text Format** based on the `MeshDesc` message from `gamesys/mesh_ddf.proto`.

### Canonical example

```protobuf
material: "/builtins/materials/debug.material"
vertices: "/assets/meshes/plane.buffer"
textures: "/assets/textures/diffuse.png"
```

## Fields reference

Fields are listed in proto field number order.

### material (required) — `string`

Absolute resource path to the material used for rendering the mesh.

```protobuf
material: "/builtins/materials/debug.material"
```

### vertices (required) — `string`

Absolute resource path to a buffer file (`.buffer`) describing the mesh vertex data per stream.

```protobuf
vertices: "/assets/meshes/plane.buffer"
```

### textures (repeated) — `string`

Absolute resource paths to textures used by the mesh (tex0..tex7). Each texture gets its own `textures:` line. Up to 8 textures can be bound.

**Omission rule**: Omit if the mesh does not use any textures.

```protobuf
textures: "/assets/textures/diffuse.png"
textures: "/assets/textures/normal.png"
```

### primitive_type (optional) — `PrimitiveType`

How the vertex data is interpreted for rendering. Default: `PRIMITIVE_TRIANGLES`.

**Omission rule**: Omit if `PRIMITIVE_TRIANGLES`.

```protobuf
primitive_type: PRIMITIVE_LINES
```

### position_stream (optional) — `string`

Name of the position stream in the buffer. Automatically provided as input to the vertex shader. Required when the material's Vertex Space is World Space.

**Omission rule**: Omit if not needed (empty string default).

```protobuf
position_stream: "position"
```

### normal_stream (optional) — `string`

Name of the normal stream in the buffer. Automatically provided as input to the vertex shader. Required when the material's Vertex Space is World Space.

**Omission rule**: Omit if not needed (empty string default).

```protobuf
normal_stream: "normal"
```

## Enum: PrimitiveType

| Constant | Value | Description |
|----------|-------|-------------|
| `PRIMITIVE_LINES` | 1 | Vertices are interpreted as line segments |
| `PRIMITIVE_TRIANGLES` | 4 | Vertices are interpreted as triangles (default) |
| `PRIMITIVE_TRIANGLE_STRIP` | 5 | Vertices are interpreted as a triangle strip |

## Protobuf Text Format rules

1. **Default omission**: Omit fields that equal their proto default. This keeps files minimal and matches Defold editor behavior.
2. **Floats**: Always include decimal point: `1.0`, not `1`.
3. **Integers**: No decimal point: `4`, not `4.0`.
4. **Strings**: Always double-quoted: `"text"`.
5. **Enums**: Use the enum constant name without quotes.
6. **Repeated strings**: Each value gets its own `field_name: "value"` line.
7. **Field order**: Follow the proto field number order: `material`, `vertices`, `textures`, `primitive_type`, `position_stream`, `normal_stream`.
8. **No trailing commas or semicolons**.
9. **No field number tags** — use field names only.
10. **No empty lines between fields** (all fields are scalar or repeated scalar).

## Common templates

### Basic mesh with texture

```protobuf
material: "/builtins/materials/debug.material"
vertices: "/assets/meshes/plane.buffer"
textures: "/assets/textures/diffuse.png"
```

### Basic mesh without texture

```protobuf
material: "/builtins/materials/debug.material"
vertices: "/assets/meshes/cube.buffer"
```

### Mesh with lines primitive type

```protobuf
material: "/builtins/materials/debug.material"
vertices: "/assets/meshes/wireframe.buffer"
primitive_type: PRIMITIVE_LINES
```

### Mesh with explicit stream names

```protobuf
material: "/assets/materials/custom_mesh.material"
vertices: "/assets/meshes/terrain.buffer"
textures: "/assets/textures/terrain.png"
position_stream: "position"
normal_stream: "normal"
```

## Workflow

### Creating a new mesh

1. Determine the file path (must end with `.mesh`).
2. Set the required `material` field to the material resource path.
3. Set the required `vertices` field to the buffer file resource path.
4. Add `textures` lines for each texture the mesh needs (up to 8).
5. Set `primitive_type` only if not using the default `PRIMITIVE_TRIANGLES`.
6. Set `position_stream` and `normal_stream` if the material's Vertex Space is World Space.
7. Write the file using the field order from the reference above.

### Editing an existing mesh

1. Read the current `.mesh` file.
2. Modify only the requested fields.
3. Preserve existing field values and order.
4. Apply omission rules: remove fields that become equal to their defaults after editing.
