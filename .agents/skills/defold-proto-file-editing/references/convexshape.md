# Convex Shape (.convexshape)

Proto message: `CollisionShape` from `physics_ddf.proto`.

A convex shape file defines an external collision shape that can be referenced by a collision object's `collision_shape` property. It uses the same `CollisionShape` message as the `embedded_collision_shape` block inside `.collisionobject` files, but as a standalone file.

## When to use

- When you need a convex hull collision shape (more than primitive box/sphere/capsule).
- When you want to share a collision shape across multiple collision objects.
- When you need a polygon-based collision boundary that matches a sprite's outline.

## File format

The file uses **Protobuf Text Format** with top-level `shape_type`, `data`, and optionally `shapes` blocks.

### Canonical example — convex hull (2D rectangle)

```protobuf
shape_type: TYPE_HULL
data: 200.0
data: 100.0
data: 0.0
data: 400.0
data: 100.0
data: 0.0
data: 400.0
data: 300.0
data: 0.0
data: 200.0
data: 300.0
data: 0.0
```

This defines a rectangle with corners at (200,100), (400,100), (400,300), (200,300):

```
 200x300   400x300
    4---------3
    |         |
    |         |
    1---------2
 200x100   400x100
```

## Fields reference

### shape_type (required) — enum `Type`

The shape type. For `.convexshape` files, typically `TYPE_HULL`.

| Value | Description | Data format |
|-------|-------------|-------------|
| `TYPE_SPHERE` | Sphere shape | 1 float: `radius` |
| `TYPE_BOX` | Box shape | 3 floats: `ext_x`, `ext_y`, `ext_z` (half-extents) |
| `TYPE_CAPSULE` | Capsule shape (3D only) | 2 floats: `radius`, `height` |
| `TYPE_HULL` | Convex hull | N floats: `x0, y0, z0, x1, y1, z1, ...` (vertices) |

```protobuf
shape_type: TYPE_HULL
```

### data (repeated) — `float`

Flat array of floats defining the shape geometry. Each value is on its own `data:` line.

For **TYPE_HULL**, data contains vertex positions as triplets `(x, y, z)`. In 2D physics, `z` is typically `0.0`. Points must be in **counter-clockwise order** for 2D physics. An abstract point cloud is used for 3D physics.

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
data: 0.0
data: 100.0
data: 0.0
```

### shapes (optional, repeated) — `Shape`

When present, defines multiple shapes with position/rotation offsets. Each shape references a slice of the `data` array via `index` and `count`. This uses the same structure as `embedded_collision_shape.shapes` in `.collisionobject` files — see `collisionobject.md` for the full `Shape` field reference.

For simple single-shape convex hulls, the `shapes` block is typically omitted.

## Common templates

### Simple convex hull — triangle (2D)

```protobuf
shape_type: TYPE_HULL
data: 0.0
data: 0.0
data: 0.0
data: 64.0
data: 0.0
data: 0.0
data: 32.0
data: 64.0
data: 0.0
```

### Convex hull — pentagon (2D)

```protobuf
shape_type: TYPE_HULL
data: 32.0
data: 0.0
data: 0.0
data: 64.0
data: 24.0
data: 0.0
data: 50.0
data: 64.0
data: 0.0
data: 14.0
data: 64.0
data: 0.0
data: 0.0
data: 24.0
data: 0.0
```

### Centered rectangle matching a sprite (2D)

For a sprite of size `W × H`, center the hull at origin using half-extents. Points in counter-clockwise order:

```protobuf
shape_type: TYPE_HULL
data: -32.0
data: -32.0
data: 0.0
data: 32.0
data: -32.0
data: 0.0
data: 32.0
data: 32.0
data: 0.0
data: -32.0
data: 32.0
data: 0.0
```

This defines a 64×64 hull centered at origin.

## Usage with collision objects

Reference the `.convexshape` file from a collision object's `collision_shape` field:

```protobuf
collision_shape: "/main/player.convexshape"
type: COLLISION_OBJECT_TYPE_KINEMATIC
mass: 0.0
friction: 0.1
restitution: 0.5
group: "player"
mask: "ground"
mask: "enemy"
```

When using `collision_shape`, do **not** include `embedded_collision_shape` — they are mutually exclusive.

## Protobuf Text Format rules

1. **Floats**: Always include decimal point: `1.0`, not `1`.
2. **Enums**: Use the constant name without quotes: `TYPE_HULL`.
3. **Repeated fields**: Each `data:` value gets its own line.
4. **No trailing commas or semicolons**.
5. **Field order**: `shape_type`, then `data` entries.

## Workflow

### Creating a new convex shape

1. Determine the vertices of the convex hull.
2. For 2D physics, order points **counter-clockwise** and set `z` to `0.0`.
3. Set `shape_type: TYPE_HULL`.
4. Add each vertex coordinate as a separate `data:` line (x, y, z per vertex).
5. Reference the file from the collision object's `collision_shape` property.

### Sizing from a sprite image (rectangular)

To create a rectangular convex hull matching a sprite's dimensions:

1. Get the image size using the bundled script:
   ```
   python .agents/skills/defold-proto-file-editing/scripts/get_image_size.py <image_path>
   ```
2. Calculate half-extents: `hw = width / 2`, `hh = height / 2`.
3. Define 4 vertices centered at origin (counter-clockwise):
   - `(-hw, -hh, 0)`, `(hw, -hh, 0)`, `(hw, hh, 0)`, `(-hw, hh, 0)`

## Generating from an image

For sprites with non-rectangular shapes (characters, objects with transparency), use the `gen_convexshape.py` script to automatically generate a convex hull that tightly fits the visible (non-transparent) pixels.

### How it works

1. Reads the image and extracts the alpha channel
2. Finds boundary pixels of the non-transparent silhouette
3. Computes a convex hull via Graham scan (Andrew's monotone chain)
4. Simplifies to ≤16 points using Visvalingam-Whyatt area-based simplification (16 is the Box2D vertex limit in Defold)
5. Centers all points at the image origin (0,0) and flips Y axis to match Defold's coordinate system
6. Ensures counter-clockwise winding order (required by Defold 2D physics)
7. Outputs a ready-to-use `.convexshape` file

### Usage

```
python .agents/skills/defold-proto-file-editing/scripts/gen_convexshape.py <image_path> [--output <path>] [--max-points N] [--alpha-threshold T] [--inset P]
```

Arguments:
- `image_path` — path to PNG or JPEG image (relative to project root)
- `--output`, `-o` — output `.convexshape` file path (default: prints to stdout)
- `--max-points`, `-m` — maximum hull vertices (default: 16)
- `--alpha-threshold`, `-a` — alpha value threshold for "non-transparent" pixels, 0-255 (default: 1)
- `--inset`, `-i` — inset percentage to shrink the shape toward its centroid, 0-100 (default: 0). Useful to make the collision shape slightly smaller than the sprite's visible outline.

### Examples

Generate and write directly to a file:
```
python .agents/skills/defold-proto-file-editing/scripts/gen_convexshape.py assets/images/player.png -o main/player.convexshape
```

Preview to stdout first:
```
python .agents/skills/defold-proto-file-editing/scripts/gen_convexshape.py assets/images/player.png
```

Use fewer points for simpler shapes:
```
python .agents/skills/defold-proto-file-editing/scripts/gen_convexshape.py assets/images/coin.png -o main/coin.convexshape -m 8
```

### Choosing between box shape and convex hull

When creating collision shapes for sprites, choose the approach based on the sprite's shape:

| Approach | When to use | How |
|----------|-------------|-----|
| **Box shape** (embedded in `.collisionobject`) | Simple rectangular sprites, UI elements, platforms, walls | Use `get_image_size.py` to get dimensions, calculate half-extents, set as `TYPE_BOX` data in the collision object |
| **Convex hull** (`.convexshape` file) | Characters, irregular objects, sprites with significant transparency around edges | Use `gen_convexshape.py` to generate a `.convexshape` file, reference it via `collision_shape` in the collision object |

Ask the user which approach they prefer when the choice is ambiguous.
