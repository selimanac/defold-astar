# Editing Collections

Creates and edits Defold `.collection` files using Protobuf Text Format.

## Overview

A collection (`.collection`) is the primary structural unit in Defold. Collections organize game objects and other collections into tree hierarchies. They are used for game levels, groups of entities, UI screens, and any reusable structural grouping. The bootstrap collection (specified in `game.project`) is the entry point of the game.

Collections can contain:
- **Referenced game objects** (`instances`) — pointing to external `.go` files
- **Embedded game objects** (`embedded_instances`) — inline game object definitions
- **Sub-collections** (`collection_instances`) — references to other `.collection` files

Game objects within a collection can form parent-child hierarchies via the `children` field.

## File format

Collection files (`.collection`) use **Protobuf Text Format** based on the `CollectionDesc` message from `gameobject/gameobject_ddf.proto`.

### Canonical example

```protobuf
name: "main"
instances {
  id: "player"
  prototype: "/main/player.go"
}
instances {
  id: "enemy"
  prototype: "/main/enemy.go"
  position {
    x: 200.0
    y: 100.0
  }
}
scale_along_z: 0
embedded_instances {
  id: "go"
  data: "components {\n"
  "  id: \"main\"\n"
  "  component: \"/main/main.script\"\n"
  "}\n"
  ""
  position {
    x: 640.0
    y: 360.0
  }
}
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

## Top-level structure (`CollectionDesc`)

| Field # | Field | Type | Required | Default |
|---------|-------|------|----------|---------|
| 1 | `name` | `string` | required | — |
| 2 | `instances` | repeated `InstanceDesc` | — | — |
| 3 | `collection_instances` | repeated `CollectionInstanceDesc` | — | — |
| 4 | `scale_along_z` | `uint32` | optional | `0` |
| 5 | `embedded_instances` | repeated `EmbeddedInstanceDesc` | — | — |
| 6 | `property_resources` | repeated `string` | — | — |
| 7 | `component_types` | repeated `ComponenTypeDesc` | — | — |

Fields #6 and #7 are `runtime_only` — they are never written in `.collection` files and are only used internally by the engine.

**Field order**: `name`, `instances`, `collection_instances`, `scale_along_z`, `embedded_instances`.

## Fields reference

### name (required) — `string`

The name identifier of the collection. Typically matches the file name without extension.

```protobuf
name: "main"
```

### instances (repeated) — `InstanceDesc`

References to external game object files. Each referenced game object is a separate `instances { ... }` block.

See the **InstanceDesc** section below.

### collection_instances (repeated) — `CollectionInstanceDesc`

References to external collection files (sub-collections). Each sub-collection is a separate `collection_instances { ... }` block.

See the **CollectionInstanceDesc** section below.

### scale_along_z (optional) — `uint32`

Deprecated field. Default: `0`. The Defold editor always writes this field with value `0`.

**Omission rule**: Although the proto default is `0`, the Defold editor always writes this field explicitly. Always include `scale_along_z: 0` to match editor output.

```protobuf
scale_along_z: 0
```

### embedded_instances (repeated) — `EmbeddedInstanceDesc`

Inline game object definitions. Each embedded game object is a separate `embedded_instances { ... }` block.

See the **EmbeddedInstanceDesc** section below.

## InstanceDesc

A reference to an external `.go` file placed in the collection.

### InstanceDesc fields

#### id (required) — `string`

Unique identifier for this game object within the collection. Used for addressing (e.g., `go.get_position("player")`).

```protobuf
id: "player"
```

#### prototype (required) — `string`

Absolute resource path to the `.go` file.

```protobuf
prototype: "/main/player.go"
```

#### children (repeated) — `string`

IDs of game objects that are children of this instance (parent-child hierarchy). Each child is a separate `children:` line.

**Omission rule**: Omit if no children.

```protobuf
children: "child1"
children: "child2"
```

#### position (optional) — `dmMath.Point3`

Position of the game object in the collection. Defaults: `x: 0.0`, `y: 0.0`, `z: 0.0`.

**Omission rule**: Omit the entire block if all components are `0.0`. Only include components that differ from `0.0`.

```protobuf
position {
  x: 200.0
  y: 100.0
}
```

#### rotation (optional) — `dmMath.Quat`

Rotation as quaternion. Defaults: `x: 0.0`, `y: 0.0`, `z: 0.0`, `w: 1.0`.

**Omission rule**: Omit the entire block if all components are at defaults. Only include components that differ.

```protobuf
rotation {
  z: 0.7071068
  w: 0.7071068
}
```

#### component_properties (repeated) — `ComponentPropertyDesc`

Overrides for script properties on components of this game object. Each override targets a specific component by its id.

See the **ComponentPropertyDesc** section below.

**Omission rule**: Omit if no property overrides.

#### scale (optional) — `float`

Uniform scale factor. Default: `1.0`.

**Omission rule**: Omit if `1.0`. Note: this is a scalar float, not a message block. Deprecated in favor of `scale3`.

```protobuf
scale: 2.0
```

#### scale3 (optional) — `dmMath.Vector3One`

Non-uniform scale. Defaults: `x: 1.0`, `y: 1.0`, `z: 1.0`.

**Omission rule**: Omit the entire block if all components are `1.0`. Only include components that differ from `1.0`.

```protobuf
scale3 {
  x: 2.0
  y: 2.0
}
```

### Full InstanceDesc example

```protobuf
instances {
  id: "enemy"
  prototype: "/main/enemy.go"
  children: "weapon"
  position {
    x: 200.0
    y: 100.0
  }
  component_properties {
    id: "script"
    properties {
      id: "speed"
      value: "150.0"
      type: PROPERTY_TYPE_NUMBER
    }
  }
}
```

## EmbeddedInstanceDesc

An inline game object definition within the collection. The game object content (its `PrototypeDesc`) is encoded as a multi-line string in the `data` field.

### EmbeddedInstanceDesc fields

#### id (required) — `string`

Unique identifier for this game object within the collection.

```protobuf
id: "go"
```

#### children (repeated) — `string`

IDs of child game objects. Same format as `InstanceDesc.children`.

**Omission rule**: Omit if no children.

#### data (required) — `string`

The game object's `PrototypeDesc` content encoded as a multi-line string. This contains the same content that would be in a `.go` file (`components` and `embedded_components` blocks).

**Encoding rules**:
- Each logical line of the game object's protobuf text becomes a separate quoted string literal
- Lines end with `\n` inside the quotes
- Inner quotes are escaped as `\"`
- Double-nested data (embedded components inside the game object) uses `\\n` for line breaks and `\\\"` for quotes
- The last entry is an empty string `""`

**Empty game object**: Use `data: ""` for a game object with no components (position marker). This is a required field, so it must always be present even if empty.

**Single-nesting example** (game object with referenced component):

```protobuf
data: "components {\n"
"  id: \"script\"\n"
"  component: \"/main/main.script\"\n"
"}\n"
""
```

**Double-nested example** (game object with embedded component):

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

Note: multi-line string concatenation blocks end with an empty `""` terminator. A single-line `data: ""` does not need an additional terminator.

**Common mistake — closing quote of inner `data`**: The line that closes the inner `data` string (the `\"` that ends the embedded component's data value) must use single-escaped newline `"\"\n"`, NOT double-escaped `"\"\\n"`. The closing quote is the boundary between nesting levels — after it, you are back at the outer (game object) level, so the newline is single `\n`. Using `\\n` here corrupts the game object data and causes a load error.

#### position (optional) — `dmMath.Point3`

Position of the game object. Same defaults and omission rules as `InstanceDesc.position`.

#### rotation (optional) — `dmMath.Quat`

Rotation quaternion. Same defaults and omission rules as `InstanceDesc.rotation`.

#### component_properties (repeated) — `ComponentPropertyDesc`

Property overrides. Same as `InstanceDesc.component_properties`.

#### scale (optional) — `float`

Uniform scale. Default: `1.0`. Same omission rules as `InstanceDesc.scale`.

#### scale3 (optional) — `dmMath.Vector3One`

Non-uniform scale. Same defaults and omission rules as `InstanceDesc.scale3`.

### Full EmbeddedInstanceDesc example

```protobuf
embedded_instances {
  id: "go"
  data: "components {\n"
  "  id: \"main\"\n"
  "  component: \"/main/main.script\"\n"
  "}\n"
  "embedded_components {\n"
  "  id: \"logo\"\n"
  "  type: \"sprite\"\n"
  "  data: \"default_animation: \\\"logo\\\"\\n"
  "material: \\\"/builtins/materials/sprite.material\\\"\\n"
  "textures {\\n"
  "  sampler: \\\"texture_sampler\\\"\\n"
  "  texture: \\\"/main/main.atlas\\\"\\n"
  "}\\n"
  "\"\n"
  "}\n"
  ""
  position {
    x: 640.0
    y: 360.0
  }
}
```

## CollectionInstanceDesc

A reference to an external `.collection` file (sub-collection).

### CollectionInstanceDesc fields

#### id (required) — `string`

Unique identifier for this sub-collection within the parent collection. Used as a path prefix when addressing game objects inside (e.g., `"level/enemy"`).

```protobuf
id: "level"
```

#### collection (required) — `string`

Absolute resource path to the `.collection` file.

```protobuf
collection: "/main/level1.collection"
```

#### position (optional) — `dmMath.Point3`

Position of the sub-collection. Defaults: `x: 0.0`, `y: 0.0`, `z: 0.0`.

**Omission rule**: Omit the entire block if all components are `0.0`.

```protobuf
position {
  x: 500.0
  y: 200.0
}
```

#### rotation (optional) — `dmMath.Quat`

Rotation quaternion. Defaults: `x: 0.0`, `y: 0.0`, `z: 0.0`, `w: 1.0`.

**Omission rule**: Omit the entire block if all components are at defaults.

#### scale (optional) — `float`

Uniform scale. Default: `1.0`.

**Omission rule**: Omit if `1.0`.

#### instance_properties (repeated) — `InstancePropertyDesc`

Property overrides for game objects inside the sub-collection. Each entry targets a specific game object by its id and overrides properties on its components.

See the **InstancePropertyDesc** section below.

**Omission rule**: Omit if no property overrides.

#### scale3 (optional) — `dmMath.Vector3One`

Non-uniform scale. Defaults: `x: 1.0`, `y: 1.0`, `z: 1.0`.

**Omission rule**: Omit the entire block if all components are `1.0`.

### Full CollectionInstanceDesc example

```protobuf
collection_instances {
  id: "level"
  collection: "/main/level1.collection"
  position {
    x: 500.0
    y: 200.0
  }
  instance_properties {
    id: "enemy"
    properties {
      id: "script"
      properties {
        id: "speed"
        value: "200.0"
        type: PROPERTY_TYPE_NUMBER
      }
    }
  }
}
```

## ComponentPropertyDesc

Used to override script properties on a specific component of a game object.

### id (required) — `string`

The component id within the game object (e.g., `"script"`).

### properties (repeated) — `PropertyDesc`

Property overrides. Each is a `properties { ... }` block.

```protobuf
component_properties {
  id: "script"
  properties {
    id: "speed"
    value: "200.0"
    type: PROPERTY_TYPE_NUMBER
  }
}
```

## InstancePropertyDesc

Used within `CollectionInstanceDesc` to override properties on game objects inside a sub-collection.

### id (required) — `string`

The game object id inside the sub-collection.

### properties (repeated) — `ComponentPropertyDesc`

Component property overrides for that game object. Each is a `properties { ... }` block containing component id and property overrides.

```protobuf
instance_properties {
  id: "enemy"
  properties {
    id: "script"
    properties {
      id: "health"
      value: "50.0"
      type: PROPERTY_TYPE_NUMBER
    }
  }
}
```

## PropertyDesc

### id (required) — `string`

The script property name.

### value (required) — `string`

The property value as a string.

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
| `PROPERTY_TYPE_MATRIX4` | Matrix4 value |

## Parent-child hierarchies

Game objects within a collection can form parent-child hierarchies. The parent game object lists its children using the `children` field. This affects the transform hierarchy at runtime — child positions, rotations, and scales are relative to the parent.

To create a hierarchy:
1. The parent game object (either `instances` or `embedded_instances`) includes `children: "child_id"` for each child.
2. The child game object must exist in the same collection (as `instances` or `embedded_instances`).
3. Multiple children are listed as separate `children:` lines.

```protobuf
instances {
  id: "parent"
  prototype: "/main/parent.go"
  children: "child1"
  children: "child2"
}
instances {
  id: "child1"
  prototype: "/main/child.go"
  position {
    x: 50.0
  }
}
instances {
  id: "child2"
  prototype: "/main/child.go"
  position {
    x: -50.0
  }
}
```

## Data string encoding rules

The `data` field in `EmbeddedInstanceDesc` contains a game object's `PrototypeDesc` as an escaped string. Understanding the encoding is essential:

### Single nesting level (game object with referenced components)

The game object content is escaped once:
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
"\"\\n"
"}\n"
""
```

Note: After the opening `data: \"...\\n"` line inside the double-nested data, subsequent lines of that inner data do NOT have leading whitespace (they start at column 0 of the quoted string).

## Protobuf Text Format rules

1. **Default omission**: Omit optional fields that equal their proto default (exception: `scale_along_z` is always written by the editor).
2. **Message blocks**: Use `field_name { ... }` with nested `key: value` pairs.
3. **Floats**: Include decimal point for clarity: `1.0`, not `1`. Inside embedded `data` strings, Defold may write integer-like values for `uint32` fields (e.g., `orthographic_projection: 1`).
4. **Integers**: No decimal point: `0`, not `0.0`.
5. **Strings**: Always double-quoted.
6. **Enums**: Use the constant name without quotes.
7. **Booleans**: `true` or `false`, no quotes.
8. **Repeated fields**: Each value gets its own line with the field name.
9. **Repeated messages**: Each entry gets its own `field_name { ... }` block.
10. **Field order**: Follow the proto field number order.
11. **No trailing commas or semicolons**.
12. **Indentation**: 2 spaces per nesting level inside message blocks.
13. **No blank lines** between fields or blocks (match Defold editor output).
14. **Embedded data strings**: Each line is a separate quoted string. Escape inner quotes with `\"`. End lines with `\n`. Terminate multi-line concatenations with empty `""`.
15. **When editing existing files**, preserve the existing formatting style.

## Common templates

### Minimal collection (empty)

```protobuf
name: "empty"
scale_along_z: 0
```

### Collection with a single embedded game object

```protobuf
name: "level"
scale_along_z: 0
embedded_instances {
  id: "go"
  data: "components {\n"
  "  id: \"script\"\n"
  "  component: \"/main/level.script\"\n"
  "}\n"
  ""
}
```

### Collection with referenced game objects

```protobuf
name: "level"
instances {
  id: "player"
  prototype: "/main/player.go"
  position {
    x: 320.0
    y: 240.0
  }
}
instances {
  id: "enemy"
  prototype: "/main/enemy.go"
  position {
    x: 600.0
    y: 240.0
  }
}
scale_along_z: 0
```

### Collection with a sub-collection

```protobuf
name: "main"
collection_instances {
  id: "level"
  collection: "/main/level1.collection"
}
scale_along_z: 0
```

### Collection with parent-child hierarchy

```protobuf
name: "level"
scale_along_z: 0
embedded_instances {
  id: "parent"
  children: "child"
  data: "components {\n"
  "  id: \"script\"\n"
  "  component: \"/main/parent.script\"\n"
  "}\n"
  ""
  position {
    x: 320.0
    y: 240.0
  }
}
embedded_instances {
  id: "child"
  data: "embedded_components {\n"
  "  id: \"sprite\"\n"
  "  type: \"sprite\"\n"
  "  data: \"default_animation: \\\"idle\\\"\\n"
  "material: \\\"/builtins/materials/sprite.material\\\"\\n"
  "textures {\\n"
  "  sampler: \\\"texture_sampler\\\"\\n"
  "  texture: \\\"/main/main.atlas\\\"\\n"
  "}\\n"
  "\"\n"
  "}\n"
  ""
  position {
    x: 50.0
  }
}
```

### Collection with a GUI component

GUI components (`.gui`) CANNOT be embedded inline. They must always be added as **referenced components** using a `components` block pointing to a `.gui` file.

```protobuf
name: "gameplay"
scale_along_z: 0
embedded_instances {
  id: "go"
  data: "components {\n"
  "  id: \"gui\"\n"
  "  component: \"/screens/gameplay/gameplay.gui\"\n"
  "}\n"
  ""
}
```

### Collection with embedded camera (typical 2D setup)

```protobuf
name: "main"
scale_along_z: 0
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

## Workflow

### Creating a new collection

1. Determine the file path (must end with `.collection`).
2. Set the `name` field (typically matching the file name without extension).
3. Add game objects as `instances` (file references) or `embedded_instances` (inline).
4. Add sub-collections as `collection_instances` if needed.
5. Set `scale_along_z: 0` (always include this field).
6. Set `position`, `rotation`, `scale3` on instances only if they differ from defaults.
7. Establish parent-child hierarchies with `children` fields if needed.
8. For embedded instances, encode the game object data following the data string encoding rules.
9. Write fields in proto field number order: `name`, `instances`, `collection_instances`, `scale_along_z`, `embedded_instances`.

### Editing an existing collection

1. Read the current `.collection` file.
2. Modify only the requested game objects, sub-collections, or properties.
3. Preserve existing instance order and field values.
4. When adding new instances, place them after existing instances of the same type.
5. Apply omission rules: remove fields that become equal to their defaults after editing.
6. Maintain parent-child relationships: if removing a game object, also remove it from any parent's `children` list.
