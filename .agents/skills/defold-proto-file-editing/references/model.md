# Editing Models

Creates and edits Defold `.model` component files using Protobuf Text Format.

## Overview

A Model component displays a 3D mesh (glTF `.gltf` or `.glb`) with one or more materials and textures. It optionally supports skeletal animation through a skeleton file and animation set. Models can reference built-in or custom materials depending on whether they are static, instanced, skinned, or both.

## File format

Model files (`.model`) use **Protobuf Text Format** based on the `ModelDesc` message from `gamesys/model_ddf.proto`.

### Canonical example — static model

```protobuf
mesh: "/assets/models/crate.glb"
name: "{{NAME}}"
materials {
  name: "colormap"
  material: "/builtins/materials/model.material"
  textures {
    sampler: "tex0"
    texture: "/assets/models/crate_texture.png"
  }
}
```

### Canonical example — skinned model with animations

```protobuf
mesh: "/assets/models/Knight.glb"
skeleton: "/assets/models/Knight.glb"
animations: "/assets/models/Knight.glb"
default_animation: "T-Pose"
name: "{{NAME}}"
materials {
  name: "knight_texture"
  material: "/builtins/materials/model_skinned.material"
  textures {
    sampler: "tex0"
    texture: "/assets/models/knight_texture.png"
  }
}
```

## Fields reference

Fields are listed in proto field number order. Deprecated fields (`material` #3, `textures` #4, `name` #10) are documented but should not be used in new files — use `materials` instead.

### mesh (required) — `string`

Absolute resource path to the glTF mesh file (`.gltf` or `.glb`). If the file contains multiple meshes, only the first one is read.

```protobuf
mesh: "/assets/models/character.glb"
```

### material (deprecated) — `string`

Legacy single-material field. **Do not use in new files.** Use the `materials` repeated field instead.

**Omission rule**: Always omit.

### textures (deprecated, repeated) — `string`

Legacy texture list for the single deprecated material. **Do not use in new files.** Use `textures` inside `materials` entries instead.

**Omission rule**: Always omit.

### skeleton (optional) — `string`

Absolute resource path to the glTF file (`.gltf` or `.glb`) containing the skeleton for animation. The skeleton must have a single root bone. Typically the same file as `mesh`.

**Omission rule**: Omit if the model has no skeleton/animation.

```protobuf
skeleton: "/assets/models/character.glb"
```

### animations (optional) — `string`

Absolute resource path to an Animation Set File that contains the animations for this model. Often the same file as `mesh` and `skeleton`.

**Omission rule**: Omit if the model has no animations.

```protobuf
animations: "/assets/models/character.glb"
```

### default_animation (optional) — `string`

The animation id (from the animation set) that plays automatically when the model loads.

**Omission rule**: Omit if no default animation is needed.

```protobuf
default_animation: "idle"
```

### name (optional) — `string`

Internal name for the model instance. Typically set to `"{{NAME}}"` (a Defold editor placeholder that resolves to the component name at build time) or a custom descriptive name.

**Omission rule**: Omit if not needed. The Defold editor typically auto-generates this as `"{{NAME}}"`.

```protobuf
name: "{{NAME}}"
```

### materials (repeated) — `Material`

Material bindings for the model. Each entry maps a material name (from the glTF file) to a Defold `.material` resource and its textures. A model can have multiple materials if its mesh uses multiple material slots.

```protobuf
materials {
  name: "default"
  material: "/builtins/materials/model.material"
  textures {
    sampler: "tex0"
    texture: "/assets/textures/diffuse.png"
  }
}
```

### create_go_bones (optional) — `bool`

Whether to create a game object for every bone in the model skeleton. This allows attaching other game objects (e.g., weapons) to bone positions using `model.get_go()`. Default: `true`.

**Omission rule**: Omit if `true` (the default).

```protobuf
create_go_bones: false
```

## Nested message: Material

Each `materials` entry contains the following fields:

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `name` | `string` | yes | The material name as defined in the glTF model file |
| `material` | `string` | yes | Absolute resource path to a `.material` file |
| `textures` | repeated `Texture` | no | Texture bindings for this material |
| `attributes` | repeated `dmGraphics.VertexAttribute` | no | Custom vertex attribute overrides |

### Built-in materials

| Material | Use case |
|----------|----------|
| `/builtins/materials/model.material` | Static non-instanced models |
| `/builtins/materials/model_instances.material` | Static instanced models |
| `/builtins/materials/model_skinned.material` | Skinned (animated) non-instanced models |
| `/builtins/materials/model_skinned_instances.material` | Skinned (animated) instanced models |

## Nested message: Texture

Each `textures` entry inside a `Material` contains:

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `sampler` | `string` | yes | The sampler name from the material (e.g., `"tex0"`) |
| `texture` | `string` | yes | Absolute resource path to the texture image file |

```protobuf
textures {
  sampler: "tex0"
  texture: "/assets/textures/diffuse.png"
}
```

## Nested message: VertexAttribute

Custom vertex attributes can override values from the material. See the `dmGraphics.VertexAttribute` message in `graphics/graphics_ddf.proto` for all fields. Common usage:

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

## Protobuf Text Format rules

1. **Default omission**: Omit fields that equal their proto default. This keeps files minimal and matches Defold editor behavior.
2. **Message blocks**: Use `field_name { ... }` with nested `key: value` pairs.
3. **Floats**: Always include decimal point: `1.0`, not `1`.
4. **Integers**: No decimal point: `4`, not `4.0`.
5. **Strings**: Always double-quoted: `"text"`.
6. **Enums**: Use the enum constant name without quotes.
7. **Booleans**: `true` or `false`, no quotes.
8. **Repeated messages**: Each entry gets its own `field_name { ... }` block.
9. **Field order**: Follow the proto field number order: `mesh`, `skeleton`, `animations`, `default_animation`, `name`, `materials`, `create_go_bones`.
10. **No trailing commas or semicolons**.
11. **No field number tags** — use field names only.
12. **Indentation**: 2 spaces per nesting level inside message blocks.
13. **Newlines**: One empty line between the end of a message block `}` and the next field. No empty line between consecutive scalar fields.

## Common templates

### Static model (non-instanced)

```protobuf
mesh: "/assets/models/crate.glb"
name: "{{NAME}}"
materials {
  name: "default"
  material: "/builtins/materials/model.material"
  textures {
    sampler: "tex0"
    texture: "/assets/textures/crate.png"
  }
}
```

### Static instanced model

```protobuf
mesh: "/assets/models/tree.glb"
name: "{{NAME}}"
materials {
  name: "default"
  material: "/builtins/materials/model_instances.material"
  textures {
    sampler: "tex0"
    texture: "/assets/textures/tree.png"
  }
}
```

### Skinned model with animations

```protobuf
mesh: "/assets/models/character.glb"
skeleton: "/assets/models/character.glb"
animations: "/assets/models/character.glb"
default_animation: "idle"
name: "{{NAME}}"
materials {
  name: "body"
  material: "/builtins/materials/model_skinned.material"
  textures {
    sampler: "tex0"
    texture: "/assets/textures/character.png"
  }
}
```

### Model without textures (vertex colors only)

```protobuf
mesh: "/assets/models/shape.glb"
name: "{{NAME}}"
materials {
  name: "default"
  material: "/assets/materials/vertex_color.material"
}
```

### Multi-material model

```protobuf
mesh: "/assets/models/vehicle.glb"
name: "{{NAME}}"
materials {
  name: "body"
  material: "/builtins/materials/model.material"
  textures {
    sampler: "tex0"
    texture: "/assets/textures/vehicle_body.png"
  }
}
materials {
  name: "wheels"
  material: "/builtins/materials/model.material"
  textures {
    sampler: "tex0"
    texture: "/assets/textures/vehicle_wheels.png"
  }
}
```

### Skinned instanced model without bone game objects

```protobuf
mesh: "/assets/models/crowd_npc.glb"
skeleton: "/assets/models/crowd_npc.glb"
animations: "/assets/models/crowd_npc.glb"
default_animation: "walk"
name: "{{NAME}}"
materials {
  name: "default"
  material: "/builtins/materials/model_skinned_instances.material"
  textures {
    sampler: "tex0"
    texture: "/assets/textures/npc.png"
  }
}
create_go_bones: false
```

## Workflow

### Creating a new model

1. Determine the file path (must end with `.model`).
2. Set the required `mesh` field to the glTF resource path.
3. Add at least one `materials` entry with the material name from the glTF file, a Defold material path, and texture bindings.
4. Choose the correct built-in material:
   - Static: `model.material` or `model_instances.material`
   - Skinned: `model_skinned.material` or `model_skinned_instances.material`
5. If the model is animated, set `skeleton`, `animations`, and optionally `default_animation`.
6. Set `name` to `"{{NAME}}"` or a descriptive name.
7. Set `create_go_bones: false` only if bone game objects are not needed (the default is `true`).
8. Write the file using the field order from the reference above.

### Editing an existing model

1. Read the current `.model` file.
2. Modify only the requested fields.
3. Preserve existing field values and order.
4. Apply omission rules: remove fields that become equal to their defaults after editing.
