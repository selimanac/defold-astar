# Editing Game Objects

Creates and edits Defold `.go` game object files using Protobuf Text Format.

## Overview

A game object (`.go`) is a container with an id, position, rotation and scale. It holds **components** that give it visual, audible, and logic representation. Components can be added by **file reference** (`components` block) or **embedded inline** (`embedded_components` block).

## File format

Game object files (`.go`) use **Protobuf Text Format** based on the `PrototypeDesc` message from `gameobject_ddf.proto`.

### Canonical example

```protobuf
components {
  id: "collisionobject"
  component: "/main/example.collisionobject"
}
embedded_components {
  id: "sprite"
  type: "sprite"
  data: "default_animation: \"logo\"\n"
  "material: \"/builtins/materials/sprite.material\"\n"
  "textures {\n"
  "  sampler: \"texture_sampler\"\n"
  "  texture: \"/main/main.atlas\"\n"
  "}\n"
  ""
}
```

## Top-level structure (`PrototypeDesc`)

The `.go` file is a `PrototypeDesc` message with two repeated fields:

- `components` — referenced component files (repeated `ComponentDesc`)
- `embedded_components` — inline component definitions (repeated `EmbeddedComponentDesc`)

A `.go` file may contain any combination of referenced and embedded components, or be empty (a bare position marker).

## Fields reference

### components (repeated) — `ComponentDesc`

A reference to an external component file. Each referenced component is a separate `components { ... }` block.

#### ComponentDesc fields

##### id (required) — `string`

Unique identifier for this component within the game object. Used for addressing: `go_id#component_id`.

```protobuf
id: "sprite"
```

##### component (required) — `string`

Absolute resource path to the component file. Supports any component type (`.script`, `.collisionobject`, `.gui`, `.particlefx`, `.tilemap`, `.sound`, `.label`, `.sprite`, `.model`, `.mesh`, `.camera`, `.factory`, `.collectionfactory`, `.collectionproxy`, etc.).

```protobuf
component: "/main/player.script"
```

##### position (optional) — `dmMath.Point3`

Local position offset of the component. Defaults: `x: 0.0`, `y: 0.0`, `z: 0.0`.

**Omission rule**: Omit the entire block if all components are at `0.0`. Only include components that differ from `0.0`.

```protobuf
position {
  x: 10.0
  y: 20.0
}
```

##### rotation (optional) — `dmMath.Quat`

Local rotation as quaternion. Defaults: `x: 0.0`, `y: 0.0`, `z: 0.0`, `w: 1.0`.

**Omission rule**: Omit the entire block if all components are at defaults. Only include components that differ from defaults.

```protobuf
rotation {
  z: 0.7071068
  w: 0.7071068
}
```

##### scale (optional) — `dmMath.Vector3One`

Local scale. Defaults: `x: 1.0`, `y: 1.0`, `z: 1.0`.

**Omission rule**: Omit the entire block if all components are `1.0`. Only include components that differ from `1.0`.

```protobuf
scale {
  x: 2.0
  y: 2.0
}
```

##### properties (repeated) — `PropertyDesc`

Overrides for script properties. Each override is a separate `properties { ... }` block.

See the **PropertyDesc** section below.

#### Full ComponentDesc example

```protobuf
components {
  id: "script"
  component: "/main/player.script"
  properties {
    id: "speed"
    value: "200.0"
    type: PROPERTY_TYPE_NUMBER
  }
}
```

### embedded_components (repeated) — `EmbeddedComponentDesc`

An inline component definition. Each embedded component is a separate `embedded_components { ... }` block.

#### EmbeddedComponentDesc fields

##### id (required) — `string`

Unique identifier for this component within the game object.

```protobuf
id: "sprite"
```

##### type (required) — `string`

Component type name. Common values: `"sprite"`, `"label"`, `"collisionobject"`, `"sound"`, `"particlefx"`, `"model"`, `"mesh"`, `"camera"`, `"factory"`, `"collectionfactory"`, `"collectionproxy"`, `"tilegrid"`.

**Important**: GUI components (`.gui`) CANNOT be embedded inline. They must always be added as **referenced components** using a `components` block pointing to a `.gui` file.

```protobuf
type: "sprite"
```

##### data (required) — `string`

The component's Protobuf Text Format content, encoded as a multi-line string. Each line of the component data is a separate quoted string, terminated with `\n`.

**Encoding rules**:
- Each logical line of the embedded component's protobuf text becomes a separate quoted string literal
- Lines end with `\n` inside the quotes
- Inner quotes are escaped as `\"`
- The last entry is an empty string `""`
- Indentation inside the data string reflects the component's own protobuf nesting (2 spaces per level)

```protobuf
data: "default_animation: \"idle\"\n"
"material: \"/builtins/materials/sprite.material\"\n"
"textures {\n"
"  sampler: \"texture_sampler\"\n"
"  texture: \"/main/main.atlas\"\n"
"}\n"
""
```

##### position (optional) — `dmMath.Point3`

Local position offset. Same defaults and omission rules as `ComponentDesc.position`.

##### rotation (optional) — `dmMath.Quat`

Local rotation. Same defaults and omission rules as `ComponentDesc.rotation`.

##### scale (optional) — `dmMath.Vector3One`

Local scale. Same defaults and omission rules as `ComponentDesc.scale`.

#### Full EmbeddedComponentDesc example

```protobuf
embedded_components {
  id: "label"
  type: "label"
  data: "size {\n"
  "  x: 128.0\n"
  "  y: 32.0\n"
  "}\n"
  "text: \"Hello World\"\n"
  "font: \"/builtins/fonts/default.font\"\n"
  "material: \"/builtins/fonts/label-df.material\"\n"
  ""
  position {
    y: 50.0
  }
}
```

## PropertyDesc

Used to override script properties on referenced components.

### id (required) — `string`

The script property name.

### value (required) — `string`

The property value as a string. Numbers, hashes, URLs, vectors, quaternions, and booleans are all represented as strings.

### type (required) — enum `PropertyType`

| Value | Description |
|-------|-------------|
| `PROPERTY_TYPE_NUMBER` | Numeric value |
| `PROPERTY_TYPE_HASH` | Hash value |
| `PROPERTY_TYPE_URL` | URL value |
| `PROPERTY_TYPE_VECTOR3` | Vector3 value (format: `"x, y, z"`) |
| `PROPERTY_TYPE_VECTOR4` | Vector4 value (format: `"x, y, z, w"`) |
| `PROPERTY_TYPE_QUAT` | Quaternion value |
| `PROPERTY_TYPE_BOOLEAN` | Boolean value (`"true"` or `"false"`) |

```protobuf
properties {
  id: "speed"
  value: "200.0"
  type: PROPERTY_TYPE_NUMBER
}
```

## Common templates

### Empty game object (position marker)

```protobuf
```

An empty file is valid — it creates a game object with no components.

### Game object with a script reference

```protobuf
components {
  id: "script"
  component: "/main/player.script"
}
```

### Game object with an embedded sprite

```protobuf
embedded_components {
  id: "sprite"
  type: "sprite"
  data: "default_animation: \"idle\"\n"
  "material: \"/builtins/materials/sprite.material\"\n"
  "textures {\n"
  "  sampler: \"texture_sampler\"\n"
  "  texture: \"/main/main.atlas\"\n"
  "}\n"
  ""
}
```

### Game object with a script and collision object references

```protobuf
components {
  id: "script"
  component: "/main/enemy.script"
}
components {
  id: "collisionobject"
  component: "/main/enemy.collisionobject"
}
```

### Game object with referenced and embedded components

```protobuf
components {
  id: "script"
  component: "/main/player.script"
}
embedded_components {
  id: "sprite"
  type: "sprite"
  data: "default_animation: \"idle\"\n"
  "material: \"/builtins/materials/sprite.material\"\n"
  "textures {\n"
  "  sampler: \"texture_sampler\"\n"
  "  texture: \"/main/main.atlas\"\n"
  "}\n"
  ""
}
```

### Game object with a GUI reference

```protobuf
components {
  id: "gui"
  component: "/screens/gameplay/gameplay.gui"
}
```

### Game object with script property overrides

```protobuf
components {
  id: "script"
  component: "/main/player.script"
  properties {
    id: "speed"
    value: "200.0"
    type: PROPERTY_TYPE_NUMBER
  }
  properties {
    id: "health"
    value: "100.0"
    type: PROPERTY_TYPE_NUMBER
  }
}
```

## Protobuf Text Format rules

1. **Default omission**: Omit optional fields that equal their proto default.
2. **Message blocks**: Use `field_name { ... }` with nested `key: value` pairs.
3. **Floats**: Always include decimal point: `1.0`, not `1`.
4. **Integers**: No decimal point: `4`, not `4.0`.
5. **Strings**: Always double-quoted.
6. **Enums**: Use the constant name without quotes.
7. **Booleans**: `true` or `false`, no quotes.
8. **Repeated fields**: Each entry gets its own `field_name { ... }` block.
9. **Field order**: Follow the proto field number order: `components` before `embedded_components`.
10. **No trailing commas or semicolons**.
11. **Indentation**: 2 spaces per nesting level inside message blocks.
12. **Newlines**: One empty line between the end of a message block `}` and the next field. No empty line between consecutive scalar fields.
13. **Embedded data strings**: Each line is a separate quoted string. Escape inner quotes with `\"`. End lines with `\n`. Terminate with empty `""`.

## Workflow

### Creating a new game object

1. Determine the file path (must end with `.go`).
2. Decide which components to include (referenced files vs. embedded).
3. For referenced components: set `id` and `component` path.
4. For embedded components: set `id`, `type`, and encode the component data as a multi-line escaped string.
5. Add optional `position`, `rotation`, `scale` only if they differ from defaults.
6. Add `properties` overrides for script components if needed.
7. Write the file with `components` blocks first, then `embedded_components` blocks.

### Editing an existing game object

1. Read the current `.go` file.
2. Modify only the requested fields or components.
3. Preserve existing component order and field values.
4. Apply omission rules: remove fields that become equal to their defaults after editing.
