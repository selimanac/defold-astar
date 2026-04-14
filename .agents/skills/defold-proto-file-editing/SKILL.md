---
name: defold-proto-file-editing
description: "Creates and edits Defold resource and component files that use Protobuf Text Format (.collection, .go, .atlas, .sprite, .gui, .collisionobject, .convexshape, .label, .font, .material, .model, .mesh, .particlefx, .sound, .camera, .factory, .collectionfactory, .collectionproxy, .tilemap, .tilesource, .objectinterpolation). Use when asked to create, modify, or configure any Defold proto text format file."
---

# Editing Defold Proto Text Format Files

Creates and edits Defold resource and component files that use Protobuf Text Format.

## When to use

This skill covers all Defold file types that are serialized as Protobuf Text Format. It does NOT cover Lua script files (`.script`, `.gui_script`, `.render_script`, `.editor_script`).

## Supported file types

For detailed field references, consult the per-type reference file in `references/`:

- `references/collection.md` — `.collection` files (game levels, hierarchies)
- `references/gameobject.md` — `.go` files (game object prototypes)
- `references/gui.md` — `.gui` files (GUI scenes)
- `references/atlas.md` — `.atlas` files (texture atlases)
- `references/sprite.md` — `.sprite` files (sprite components)
- `references/collisionobject.md` — `.collisionobject` files (physics)
- `references/convexshape.md` — `.convexshape` files (external convex hull collision shapes)
- `references/label.md` — `.label` files (text in game space)
- `references/font.md` — `.font` files (font resources)
- `references/material.md` — `.material` files (render materials)
- `references/model.md` — `.model` files (3D models)
- `references/mesh.md` — `.mesh` files (custom 3D meshes from buffer data)
- `references/particlefx.md` — `.particlefx` files (particle effects)
- `references/sound.md` — `.sound` files (audio)
- `references/camera.md` — `.camera` files (camera components)
- `references/factory.md` — `.factory` files (object spawning)
- `references/collectionfactory.md` — `.collectionfactory` files (collection spawning)
- `references/collectionproxy.md` — `.collectionproxy` files (world loading)
- `references/tilemap.md` — `.tilemap` files (tile grids)
- `references/tilesource.md` — `.tilesource` files (tile source resources)
- `references/objectinterpolation.md` — `.objectinterpolation` files (extension: interpolation of fixed step movement)

For skill maintenance tasks (updating references, fetching proto schemas), use the `defold-skill-maintain` skill.

## Shaders and materials relationship

Shaders (`.vp`, `.fp`, `.glsl`) are GLSL files and are NOT covered by this skill — use the `defold-shaders-editing` skill for shader files. However, shaders and materials are tightly coupled:

### Data flow from material to shader

1. **Constants** declared in `vertex_constants` / `fragment_constants` become `uniform` variables in shaders. Engine-provided constants (`CONSTANT_TYPE_VIEW`, `CONSTANT_TYPE_PROJECTION`, etc.) are automatically populated. User constants (`CONSTANT_TYPE_USER`) can be animated via `go.set()` / `go.animate()`.

2. **Samplers** declared in `samplers` become `sampler2D` uniforms. The sampler `name` in the material must match the uniform name in the shader.

3. **Attributes** declared in `attributes` become vertex `in` variables. Semantic types like `SEMANTIC_TYPE_POSITION`, `SEMANTIC_TYPE_TEXCOORD` provide engine-generated data.

### Instancing with mtx_world and mtx_normal

For instanced rendering, two special vertex attributes are available **without declaring them in the material's `attributes` section**:

- `mtx_world` — `mat4` world transformation matrix (per-instance)
- `mtx_normal` — `mat4` normal transformation matrix (per-instance)

When these are declared as vertex `in` attributes in the shader, Defold automatically enables instanced rendering:

```glsl
// model_instanced.vp
in mediump mat4 mtx_world;
in mediump mat4 mtx_normal;

void main() {
    vec4 p = mtx_view * mtx_world * vec4(position.xyz, 1.0);
    var_normal = normalize((mtx_normal * vec4(normal, 0.0)).xyz);
    gl_Position = mtx_proj * p;
}
```

**Requirements for instancing**:
- Material must have `vertex_space: VERTEX_SPACE_LOCAL`
- Shader declares `mtx_world` and/or `mtx_normal` as `in` attributes
- No need to add these to the material's `attributes` list

### Reference examples

Built-in shader examples are in `.deps/builtins/materials/`:
- `sprite.vp` / `sprite.fp` — 2D sprite rendering (world space)
- `model.vp` / `model.fp` — 3D model rendering (local space, uniforms)
- `model_instanced.vp` — 3D model with instancing (uses `mtx_world`, `mtx_normal` as attributes)

## Bundled scripts

- `scripts/get_image_size.py` — Get image dimensions (width × height) from PNG/JPEG files. Pure Python, no external dependencies. Use this when creating collision object box shapes that should match sprite image sizes. See `references/collisionobject.md` → "Sizing box shapes from sprite images" for the full workflow.
- `scripts/gen_convexshape.py` — Generate a `.convexshape` file from a 2D image's non-transparent silhouette. Uses PIL/Pillow. Computes a convex hull, simplifies to ≤16 points (Box2D limit), centers at origin, and outputs Defold `.convexshape` format. See `references/convexshape.md` → "Generating from an image" for usage.
- `scripts/gen_silhouette_chain.py` — Generate a `.collisionobject` file with a chain of rotated TYPE_BOX shapes tracing the contour of any image silhouette (concave, with holes, multi-part). Uses PIL/Pillow. Extracts boundary contour loops, simplifies with RDP, and outputs a `.collisionobject` with thin rotated boxes along each edge. See `references/collisionobject.md` → "Silhouette chain from image contour" for usage.

## Embedded component type names

When embedding components in `.go` or `.collection` files, these are the `type` string values:

- `"sprite"` — Sprite component
- `"label"` — Label component
- `"collisionobject"` — Collision object
- `"sound"` — Sound component
- `"particlefx"` — Particle effect
- `"model"` — 3D model
- `"mesh"` — Mesh
- `"camera"` — Camera
- `"factory"` — Factory
- `"collectionfactory"` — Collection factory
- `"collectionproxy"` — Collection proxy
- `"tilegrid"` — Tilemap (note: type is `"tilegrid"`, not `"tilemap"`)
- `"objectinterpolation"` — Object interpolation (extension)

GUI components (`.gui`) CANNOT be embedded inline — they must be added as referenced components.

## Protobuf Text Format rules

These rules apply to ALL Defold proto text format files:

1. **Default omission**: Omit fields that equal their proto default. This keeps files minimal and matches Defold editor output.
2. **Message blocks**: Use `field_name { ... }` with nested `key: value` pairs.
3. **Floats**: Always include decimal point: `1.0`, not `1`.
4. **Integers**: No decimal point: `4`, not `4.0`.
5. **Strings**: Always double-quoted: `"text"`.
6. **Enums**: Use the constant name without quotes: `BLEND_MODE_ALPHA`.
7. **Booleans**: `true` or `false`, no quotes.
8. **Repeated fields**: Each scalar value gets its own line with the field name.
9. **Repeated messages**: Each entry gets its own `field_name { ... }` block.
10. **Field order**: Follow the proto field number order.
11. **No trailing commas or semicolons**.
12. **Indentation**: 2 spaces per nesting level inside message blocks.
13. **Newlines**: One empty line between the end of a message block `}` and the next field. No empty line between consecutive scalar fields.
14. **No blank lines** between fields or blocks within the same nesting level (match Defold editor output).

## Vector and math type conventions

These types from `ddf_math.proto` appear across many file types:

### dmMath.Vector3 / dmMath.Point3
Components: `x, y, z` (and `d`, rarely used). All default to `0.0`.

### dmMath.Vector3One
Components: `x, y, z` (and `d`, rarely used). All default to `1.0`.

### dmMath.Vector4
Components: `x, y, z, w`. All default to `0.0`.

### dmMath.Vector4One
Components: `x, y, z, w`. All default to `1.0`.

### dmMath.Vector4WOne
Components: `x, y, z, w`. `x/y/z` default to `0.0`, `w` defaults to `1.0`.

### dmMath.Quat
Components: `x, y, z, w`. `x/y/z` default to `0.0`, `w` defaults to `1.0`.

### Omission rules for vector/math blocks

- Only include components that differ from their default value.
- If all components are at defaults, omit the entire block (for optional fields).
- If the field is required, include the block but leave it empty: `position { }`.

```protobuf
position {
  x: 200.0
  y: 100.0
}
rotation {
  z: 0.7071068
  w: 0.7071068
}
```

## Data string encoding rules

Game objects embedded in `.collection` files and components embedded in `.go` files encode their content as multi-line strings in a `data` field.

### Single nesting level (game object with referenced components)

The content is escaped once:
- Quotes become `\"`
- Newlines become `\n`

```protobuf
data: "components {\n"
"  id: \"script\"\n"
"  component: \"/main/main.script\"\n"
"}\n"
""
```

### Double nesting level (game object with embedded components)

The embedded component's `data` field requires double escaping:
- Outer quotes: `\"`
- Inner quotes (inside component data): `\\\"`
- Outer newlines: `\n`
- Inner newlines (inside component data): `\\n`

```protobuf
data: "embedded_components {\n"
"  id: \"camera\"\n"
"  type: \"camera\"\n"
"  data: \"aspect_ratio: 1.0\\n"
"fov: 0.7854\\n"
"near_z: -1.0\\n"
"far_z: 1.0\\n"
"orthographic_projection: 1\\n"
"\"\n"
"}\n"
""
```

**Common mistake — closing quote of inner `data`**: The line that closes the inner `data` string (the `\"` that ends the embedded component's data value) must use single-escaped newline `"\"\n"`, NOT double-escaped `"\"\\n"`. The closing quote `\"` is the boundary between nesting levels — after it, you are back at the outer (game object) level, so the newline is single `\n`. Using `\\n` here corrupts the game object data and causes a load error.

Note: After the opening `data: \"...\\n"` line inside double-nested data, subsequent lines of that inner data do NOT have leading whitespace (they start at column 0 of the quoted string).

Multi-line string concatenation blocks end with an empty `""` terminator. A single-line `data: ""` does not need an additional terminator.

## Workflow

### Creating a new file

1. Determine the file type and path.
2. Consult the relevant reference file in `references/` for the field structure.
3. Set all required fields.
4. Set optional fields only if they differ from defaults.
5. Follow proto field number order.
6. Apply all Protobuf Text Format rules above.

### Editing an existing file

1. Read the current file.
2. Modify only the requested fields.
3. Preserve existing field values and order.
4. Apply omission rules: remove fields that become equal to their defaults after editing.
5. When editing existing files, preserve the existing formatting style.
