# Editing Collision Objects

Creates and edits Defold `.collisionobject` component files using Protobuf Text Format.

## File format

Collision object files (`.collisionobject`) use **Protobuf Text Format** based on the `CollisionObjectDesc` message from `physics_ddf.proto`.

### Canonical example

```protobuf
type: COLLISION_OBJECT_TYPE_STATIC
mass: 0.0
friction: 0.1
restitution: 0.5
group: "ground"
mask: "entity"
mask: "world"
embedded_collision_shape {
  shapes {
    shape_type: TYPE_BOX
    position {
      y: 2.0
    }
    rotation {
      z: 0.25881904
      w: 0.9659258
    }
    index: 0
    count: 3
  }
  data: 2.5
  data: 2.5
  data: 2.5
}
```

## Fields reference

### collision_shape (optional) — `string`

Resource path to an external collision shape file (`.convexshape` or tilemap). Use this **instead of** `embedded_collision_shape` when the shape is defined externally.

**Omission rule**: Omit when using embedded shapes.

```protobuf
collision_shape: "/main/level.tilemap"
```

### type (required) — enum `CollisionObjectType`

| Value | Description |
|-------|-------------|
| `COLLISION_OBJECT_TYPE_DYNAMIC` | Simulated by the physics engine. Must have non-zero `mass`. |
| `COLLISION_OBJECT_TYPE_KINEMATIC` | Registers collisions but not simulated. Resolve collisions manually. |
| `COLLISION_OBJECT_TYPE_STATIC` | Never moves. Used for level geometry (ground, walls). Cannot be moved at runtime. |
| `COLLISION_OBJECT_TYPE_TRIGGER` | Detects overlaps but does not produce physical reactions. |

```protobuf
type: COLLISION_OBJECT_TYPE_DYNAMIC
```

**Parent transform caveat**: Dynamic collision objects (`COLLISION_OBJECT_TYPE_DYNAMIC`) must NOT be placed in a child game object whose parent has non-zero position or rotation. The physics engine overwrites the game object's world transform every frame with the simulated body's position/rotation, and parent transforms are not taken into account — the object will ignore its parent hierarchy entirely. Static, kinematic, and trigger objects are not affected by this limitation because their transforms are not driven by the physics engine.

### mass (required) — `float`

Physical mass of the object. Must be `0.0` for static and kinematic objects. Must be non-zero for dynamic objects.

```protobuf
mass: 1.0
```

### friction (required) — `float`

Friction coefficient. Usually `0.0` (slippery) to `1.0` (abrasive). Any positive value is valid. Combined between two shapes via geometric mean: `sqrt(F_A * F_B)`.

```protobuf
friction: 0.1
```

### restitution (required) — `float`

Bounciness. `0.0` = inelastic (no bounce), `1.0` = perfectly elastic. Combined between two shapes via `max(R_A, R_B)`.

```protobuf
restitution: 0.5
```

### group (required) — `string`

Collision group name this object belongs to. Up to 16 groups per project. Double-quoted.

```protobuf
group: "enemy"
```

### mask (repeated) — `string`

Other collision groups this object should collide with. Each group is a separate `mask:` line. Both objects must mutually list each other's group in their mask for collision to register. If empty, the object collides with nothing.

```protobuf
mask: "player"
mask: "world"
```

### embedded_collision_shape (optional) — `CollisionShape`

Inline collision shape definition containing one or more primitive shapes. Use this **instead of** `collision_shape` for embedded primitives.

See the **Embedded collision shape** section below.

### linear_damping (optional) — `float`

Reduces linear velocity. Values between `0.0` and `1.0`. Default: `0.0`.

**Omission rule**: Omit if `0.0`.

```protobuf
linear_damping: 0.1
```

### angular_damping (optional) — `float`

Reduces angular velocity. Values between `0.0` and `1.0`. Default: `0.0`.

**Omission rule**: Omit if `0.0`.

```protobuf
angular_damping: 0.1
```

### locked_rotation (optional) — `bool`

Disables rotation entirely. Default: `false`.

**Omission rule**: Omit if `false`.

```protobuf
locked_rotation: true
```

### bullet (optional) — `bool`

Enables continuous collision detection (CCD). Only applies when `type` is `COLLISION_OBJECT_TYPE_DYNAMIC`. Default: `false`.

**Omission rule**: Omit if `false`.

```protobuf
bullet: true
```

### event_collision (optional) — `bool`

Generate collision events. Default: `true`.

**Omission rule**: Omit if `true`.

```protobuf
event_collision: false
```

### event_contact (optional) — `bool`

Generate contact events. Default: `true`.

**Omission rule**: Omit if `true`.

```protobuf
event_contact: false
```

### event_trigger (optional) — `bool`

Generate trigger events. Default: `true`.

**Omission rule**: Omit if `true`.

```protobuf
event_trigger: false
```

## Embedded collision shape

The `embedded_collision_shape` block contains a `CollisionShape` message with `shapes` and `data`.

### Structure

```protobuf
embedded_collision_shape {
  shapes {
    shape_type: TYPE_BOX
    position {
    }
    rotation {
    }
    index: 0
    count: 3
  }
  data: 50.0
  data: 50.0
  data: 50.0
}
```

### Shape message fields

#### shape_type (required) — enum `Type`

| Value | Description | Data count |
|-------|-------------|------------|
| `TYPE_SPHERE` | Sphere shape | 1 (`radius`) |
| `TYPE_BOX` | Box shape | 3 (`ext_x`, `ext_y`, `ext_z` — half-extents) |
| `TYPE_CAPSULE` | Capsule shape (3D physics only) | 2 (`radius`, `height`) |
| `TYPE_HULL` | Convex hull | N (`x0, y0, z0, x1, y1, z1, ...`) |

#### position (required) — `dmMath.Point3`

Local position offset of the shape. Components default to `0.0`.

- `x` — (default: `0.0`)
- `y` — (default: `0.0`)
- `z` — (default: `0.0`)

**Omission rule**: Include the `position` block always (it is required), but only include components that differ from `0.0`.

```protobuf
position {
  y: 25.0
}
```

#### rotation (required) — `dmMath.Quat`

Local rotation of the shape as a quaternion. Defaults: `x: 0.0`, `y: 0.0`, `z: 0.0`, `w: 1.0`.

**Omission rule**: Include the `rotation` block always (it is required), but only include components that differ from their defaults.

```protobuf
rotation {
  z: 0.7071068
  w: 0.7071068
}
```

#### index (required) — `uint32`

Starting index into the `data` array for this shape's data.

#### count (required) — `uint32`

Number of `data` entries used by this shape.

#### id (optional) — `string`

Shape identifier for runtime shape manipulation via `physics.set_shape()`.

```protobuf
id: "my_box"
```

### Data array

The `data` entries are flat repeated floats at the `embedded_collision_shape` level. Each shape references its portion via `index` and `count`.

**Sphere** — 1 float: `radius`
```protobuf
data: 25.0
```

**Box** — 3 floats: `ext_x`, `ext_y`, `ext_z` (half-extents, so a box with data `50, 50, 50` is 100x100x100 in size)
```protobuf
data: 50.0
data: 50.0
data: 50.0
```

**Capsule** — 2 floats: `radius`, `height`
```protobuf
data: 10.0
data: 40.0
```

**Hull** — N floats: `x0, y0, z0, x1, y1, z1, ...` (points in counter-clockwise order for 2D)
```protobuf
data: 0.0
data: 0.0
data: 0.0
data: 100.0
data: 0.0
data: 0.0
data: 100.0
data: 100.0
data: 0.0
```

### Multiple shapes

When embedding multiple shapes, each shape references its slice of the shared `data` array via `index` and `count`.

```protobuf
embedded_collision_shape {
  shapes {
    shape_type: TYPE_SPHERE
    position {
    }
    rotation {
    }
    index: 0
    count: 1
  }
  shapes {
    shape_type: TYPE_BOX
    position {
      x: 30.0
    }
    rotation {
    }
    index: 1
    count: 3
  }
  data: 15.0
  data: 20.0
  data: 20.0
  data: 20.0
}
```

## Matching collision shape to sprite image

When creating a collision object for a sprite, choose the shape type based on the sprite's visual form:

| Approach | When to use | How |
|----------|-------------|-----|
| **Box shape** (embedded `TYPE_BOX`) | Rectangular sprites, platforms, walls, UI elements | Use `get_image_size.py` → half-extents → `TYPE_BOX` data |
| **Convex hull** (external `.convexshape`) | Characters, irregular convex shapes, sprites with transparency | Use `gen_convexshape.py` → `.convexshape` file → `collision_shape` property |
| **Silhouette chain** (embedded multi-box) | Concave shapes, race tracks, complex outlines, static level geometry | Use `gen_silhouette_chain.py` → `.collisionobject` file with rotated `TYPE_BOX` shapes along contour |

Ask the user which approach they prefer when the choice is ambiguous.

For convex hull generation from images, see `references/convexshape.md` → "Generating from an image".

### Box shape from sprite image

When creating a collision object that should match a sprite's visual size, determine the image dimensions first and use **half-extents** (half the pixel size) for the box shape data.

### Workflow

1. Find the image path from the atlas (`.atlas` file) referenced by the sprite. The atlas `images { image: "/assets/player.png" }` field gives the resource path.
2. Run the image size script (no external dependencies, pure Python stdlib):

```
python .agents/skills/defold-proto-file-editing/scripts/get_image_size.py <project_root_path>/<image_resource_path>
```

The `<image_resource_path>` is the path from the atlas file **without** the leading `/`. The script outputs: `<path> <width> <height>`.

Example:
```
python .agents/skills/defold-proto-file-editing/scripts/get_image_size.py assets/images/player.png
# Output: assets/images/player.png 64 128
```

3. Calculate half-extents: `ext_x = width / 2`, `ext_y = height / 2`, `ext_z = width / 2` (or `10.0` for 2D games where z doesn't matter).
4. Use the half-extents in the `data` fields of the box shape.

### Example

For a 64×128 pixel sprite image:

```protobuf
embedded_collision_shape {
  shapes {
    shape_type: TYPE_BOX
    position {
    }
    rotation {
    }
    index: 0
    count: 3
  }
  data: 32.0
  data: 64.0
  data: 10.0
}
```

### Multiple images in one command

The script accepts multiple paths:

```
python .agents/skills/defold-proto-file-editing/scripts/get_image_size.py assets/a.png assets/b.jpg
# Output:
# assets/a.png 64 64
# assets/b.jpg 128 256
```

## Common templates

### Static ground (box)

```protobuf
type: COLLISION_OBJECT_TYPE_STATIC
mass: 0.0
friction: 0.1
restitution: 0.5
group: "ground"
mask: "player"
mask: "enemy"
embedded_collision_shape {
  shapes {
    shape_type: TYPE_BOX
    position {
    }
    rotation {
    }
    index: 0
    count: 3
  }
  data: 500.0
  data: 10.0
  data: 10.0
}
```

### Dynamic ball (sphere)

```protobuf
type: COLLISION_OBJECT_TYPE_DYNAMIC
mass: 1.0
friction: 0.3
restitution: 0.8
group: "ball"
mask: "ground"
mask: "wall"
embedded_collision_shape {
  shapes {
    shape_type: TYPE_SPHERE
    position {
    }
    rotation {
    }
    index: 0
    count: 1
  }
  data: 16.0
}
```

### Kinematic player (box)

```protobuf
type: COLLISION_OBJECT_TYPE_KINEMATIC
mass: 0.0
friction: 0.0
restitution: 0.0
group: "player"
mask: "ground"
mask: "enemy"
mask: "pickup"
embedded_collision_shape {
  shapes {
    shape_type: TYPE_BOX
    position {
    }
    rotation {
    }
    index: 0
    count: 3
  }
  data: 8.0
  data: 16.0
  data: 8.0
}
```

### Trigger zone (sphere)

```protobuf
type: COLLISION_OBJECT_TYPE_TRIGGER
mass: 0.0
friction: 0.0
restitution: 0.0
group: "trigger"
mask: "player"
embedded_collision_shape {
  shapes {
    shape_type: TYPE_SPHERE
    position {
    }
    rotation {
    }
    index: 0
    count: 1
  }
  data: 64.0
}
```

## Protobuf Text Format rules

1. **Default omission**: Omit optional fields that equal their proto default.
2. **Message blocks**: Use `field_name { ... }` with nested `key: value` pairs.
3. **Floats**: Always include decimal point: `1.0`, not `1`.
4. **Strings**: Always double-quoted.
5. **Enums**: Use the constant name without quotes.
6. **Booleans**: `true` or `false`, no quotes.
7. **Repeated fields**: Each value gets its own line with the field name (e.g., `mask:`, `data:`).
8. **Field order**: Follow the proto field number order.
9. **No trailing commas or semicolons**.
10. **Indentation**: 2 spaces per nesting level inside message blocks.

## Workflow

### Creating a new collision object

1. Determine object type (`STATIC`, `DYNAMIC`, `KINEMATIC`, `TRIGGER`).
2. Set `mass` (non-zero for dynamic, `0.0` for others).
3. Set `friction` and `restitution`.
4. Set `group` and `mask` entries.
5. Define shapes in `embedded_collision_shape` or reference an external `collision_shape`.
6. Add optional fields (`linear_damping`, `angular_damping`, etc.) only if they differ from defaults.

### Editing an existing collision object

1. Read the current `.collisionobject` file.
2. Modify only the requested fields.
3. Preserve existing field values and order.
4. Apply omission rules for fields that become equal to their defaults.

## Silhouette chain from image contour

For static level geometry with complex, concave, or multi-part shapes (e.g., race tracks, terrain outlines, irregular platforms), use the `gen_silhouette_chain.py` script to generate a `.collisionobject` file with thin, rotated `TYPE_BOX` shapes that trace the contour polygon of the image silhouette.

This simulates concave collision in Box2D (which only supports convex primitives) by placing narrow boxes along every edge of the simplified boundary polygon.

### How it works

1. Reads the image and extracts the alpha channel
2. Extracts directed boundary edges between opaque and transparent regions on the pixel grid
3. Chains edges into closed contour loops (handles shapes touching image edges, multiple components, and holes)
4. Simplifies each contour with Ramer-Douglas-Peucker (controlled by `--epsilon`)
5. For each edge of the simplified polygon, emits a thin `TYPE_BOX` shape:
   - **position** = edge midpoint in image-centred, Y-up Defold coordinates
   - **rotation** = quaternion aligning the box along the edge angle
   - **half-extents** = `(half_edge_length, thickness, 10.0)`
6. Outputs a ready-to-use `.collisionobject` with `COLLISION_OBJECT_TYPE_STATIC`

### Usage

```
python .agents/skills/defold-proto-file-editing/scripts/gen_silhouette_chain.py <image_path> [options]
```

Arguments:
- `image_path` — path to PNG or JPEG image
- `--output`, `-o` — output `.collisionobject` file path (default: prints to stdout)
- `--epsilon`, `-e` — RDP simplification tolerance in pixels (default: 2.0). Lower = more edges, higher fidelity
- `--thickness`, `-t` — half-thickness of wall boxes in pixels (default: 2.0)
- `--alpha-threshold`, `-a` — alpha threshold for "non-transparent" pixels, 0-255 (default: 1)
- `--group`, `-g` — collision group (default: `"geometry"`)
- `--mask` — collision mask group, repeatable (default: `"default"`)
- `--friction` — friction coefficient (default: 0.1)
- `--restitution` — restitution / bounciness (default: 0.5)

### Examples

Generate collision for a race track:
```
python .agents/skills/defold-proto-file-editing/scripts/gen_silhouette_chain.py assets/images/track.png -o main/track.collisionobject
```

Higher fidelity contour (smaller epsilon):
```
python .agents/skills/defold-proto-file-editing/scripts/gen_silhouette_chain.py assets/images/terrain.png -o main/terrain.collisionobject -e 1.0
```

With custom physics properties:
```
python .agents/skills/defold-proto-file-editing/scripts/gen_silhouette_chain.py assets/images/wall.png -o main/wall.collisionobject --group "ground" --mask "player" --mask "enemy" --friction 0.3
```

Thicker walls for more forgiving collision:
```
python .agents/skills/defold-proto-file-editing/scripts/gen_silhouette_chain.py assets/images/border.png -o main/border.collisionobject -t 4.0
```

### Choosing the right epsilon

| Epsilon | Result |
|---------|--------|
| 0.5–1.0 | High fidelity, many boxes. Use for small detailed sprites. |
| 2.0–4.0 | Good balance. Default is 2.0. |
| 8.0+ | Very simplified contour, few boxes. Use for large coarse shapes. |
