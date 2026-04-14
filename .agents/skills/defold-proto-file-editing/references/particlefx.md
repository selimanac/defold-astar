# Editing Particle Effects

Creates and edits Defold `.particlefx` files using Protobuf Text Format.

## Overview

A ParticleFX is a visual effect composed of one or more emitters and optional modifiers. Emitters are positioned shapes that emit particles with configurable spawn rate, life time, speed, size, color, rotation, and other properties animated via spline curves. Modifiers affect particle velocity through acceleration, drag, radial attraction/repulsion, or vortex forces. Modifiers can be placed at the effect level (affecting all emitters) or as children of a specific emitter.

## File format

ParticleFX files (`.particlefx`) use **Protobuf Text Format** based on the `ParticleFX` message from `particle_ddf.proto`.

The top-level message contains:
- `emitters` (repeated) — one or more particle emitters
- `modifiers` (repeated) — effect-level modifiers affecting all emitters

### Canonical example

```protobuf
emitters {
  id: "emitter"
  mode: PLAY_MODE_ONCE
  duration: 1.0
  space: EMISSION_SPACE_WORLD
  tile_source: "/assets/particles/particles.atlas"
  animation: "particle"
  material: "/builtins/materials/particlefx.material"
  max_particle_count: 64
  type: EMITTER_TYPE_CIRCLE
  properties {
    key: EMITTER_KEY_SPAWN_RATE
    points {
      y: 50.0
    }
  }
  properties {
    key: EMITTER_KEY_SIZE_X
    points {
      y: 0.0
    }
  }
  properties {
    key: EMITTER_KEY_SIZE_Y
    points {
      y: 0.0
    }
  }
  properties {
    key: EMITTER_KEY_SIZE_Z
    points {
      y: 0.0
    }
  }
  properties {
    key: EMITTER_KEY_PARTICLE_LIFE_TIME
    points {
      y: 1.0
    }
  }
  properties {
    key: EMITTER_KEY_PARTICLE_SPEED
    points {
      y: 200.0
    }
  }
  properties {
    key: EMITTER_KEY_PARTICLE_SIZE
    points {
      y: 20.0
    }
  }
  properties {
    key: EMITTER_KEY_PARTICLE_RED
    points {
      y: 1.0
    }
  }
  properties {
    key: EMITTER_KEY_PARTICLE_GREEN
    points {
      y: 1.0
    }
  }
  properties {
    key: EMITTER_KEY_PARTICLE_BLUE
    points {
      y: 1.0
    }
  }
  properties {
    key: EMITTER_KEY_PARTICLE_ALPHA
    points {
      y: 1.0
    }
  }
  properties {
    key: EMITTER_KEY_PARTICLE_ROTATION
    points {
      y: 0.0
    }
  }
  properties {
    key: EMITTER_KEY_PARTICLE_STRETCH_FACTOR_X
    points {
      y: 0.0
    }
  }
  properties {
    key: EMITTER_KEY_PARTICLE_STRETCH_FACTOR_Y
    points {
      y: 0.0
    }
  }
  properties {
    key: EMITTER_KEY_PARTICLE_ANGULAR_VELOCITY
    points {
      y: 0.0
    }
  }
  particle_properties {
    key: PARTICLE_KEY_SCALE
    points {
      y: 1.0
    }
  }
  particle_properties {
    key: PARTICLE_KEY_RED
    points {
      y: 1.0
    }
  }
  particle_properties {
    key: PARTICLE_KEY_GREEN
    points {
      y: 1.0
    }
  }
  particle_properties {
    key: PARTICLE_KEY_BLUE
    points {
      y: 1.0
    }
  }
  particle_properties {
    key: PARTICLE_KEY_ALPHA
    points {
      y: 1.0
    }
  }
  particle_properties {
    key: PARTICLE_KEY_ROTATION
    points {
      y: 0.0
    }
  }
  particle_properties {
    key: PARTICLE_KEY_STRETCH_FACTOR_X
    points {
      y: 0.0
    }
  }
  particle_properties {
    key: PARTICLE_KEY_STRETCH_FACTOR_Y
    points {
      y: 0.0
    }
  }
  particle_properties {
    key: PARTICLE_KEY_ANGULAR_VELOCITY
    points {
      y: 0.0
    }
  }
}
```

## Emitter fields reference

Fields are listed in proto field number order. All emitter properties (`properties`) and particle properties (`particle_properties`) entries are required in practice — the Defold editor always writes all of them.

### id (optional) — `string`

Emitter identifier. Used when setting render constants for specific emitters. Default: `"emitter"`.

**Omission rule**: Omit if `"emitter"`.

```protobuf
id: "sparks"
```

### mode (required) — `PlayMode`

Controls how the emitter plays. `PLAY_MODE_ONCE` stops after reaching duration. `PLAY_MODE_LOOP` restarts after reaching duration.

```protobuf
mode: PLAY_MODE_LOOP
```

### duration (optional) — `float`

Number of seconds the emitter emits particles. Default: `0.0`. A value of `0` with `PLAY_MODE_LOOP` means the emitter runs indefinitely.

**Omission rule**: Omit if `0.0`.

```protobuf
duration: 2.0
```

### space (required) — `EmissionSpace`

Which geometrical space the spawned particles exist in. `EMISSION_SPACE_WORLD` moves particles independently of the emitter. `EMISSION_SPACE_EMITTER` moves particles relative to the emitter.

```protobuf
space: EMISSION_SPACE_WORLD
```

### position (optional) — `dmMath.Point3`

Transform position of the emitter relative to the ParticleFX component. Components: `x, y, z`, all default `0.0`.

**Omission rule**: Omit if all components are `0.0`.

```protobuf
position {
  x: 10.0
  y: 20.0
}
```

### rotation (optional) — `dmMath.Quat`

Transform rotation of the emitter relative to the ParticleFX component. Components: `x, y, z, w`, where `x/y/z` default to `0.0`, `w` defaults to `1.0`.

**Omission rule**: Omit if at default (identity quaternion).

```protobuf
rotation {
  z: 0.7071068
  w: 0.7071068
}
```

### tile_source (required) — `string`

Absolute resource path to the image file (Atlas or Tile Source) used for texturing and animating particles.

```protobuf
tile_source: "/assets/particles/particles.atlas"
```

### animation (required) — `string`

Animation name from the `tile_source` to use on particles.

```protobuf
animation: "spark"
```

### material (required) — `string`

Absolute resource path to the material used for shading particles. The built-in particle material is `/builtins/materials/particlefx.material`.

```protobuf
material: "/builtins/materials/particlefx.material"
```

### blend_mode (optional) — `BlendMode`

Blending mode for particle rendering. Default: `BLEND_MODE_ALPHA`.

**Omission rule**: Omit if `BLEND_MODE_ALPHA`.

```protobuf
blend_mode: BLEND_MODE_ADD
```

### particle_orientation (optional) — `ParticleOrientation`

How emitted particles are oriented. Default: `PARTICLE_ORIENTATION_DEFAULT`.

**Omission rule**: Omit if `PARTICLE_ORIENTATION_DEFAULT`.

```protobuf
particle_orientation: PARTICLE_ORIENTATION_MOVEMENT_DIRECTION
```

### inherit_velocity (optional) — `float`

Scale value of how much emitter velocity particles inherit. Default: `0.0`. Only works when `space` is `EMISSION_SPACE_WORLD`.

**Omission rule**: Omit if `0.0`.

```protobuf
inherit_velocity: 0.5
```

### max_particle_count (required) — `uint32`

Maximum number of particles from this emitter that can exist simultaneously.

```protobuf
max_particle_count: 128
```

### type (required) — `EmitterType`

Shape of the emitter. Controls how particles are distributed and their initial direction.

```protobuf
type: EMITTER_TYPE_BOX
```

### start_delay (optional) — `float`

Number of seconds the emitter waits before emitting. Default: `0.0`.

**Omission rule**: Omit if `0.0`.

```protobuf
start_delay: 0.5
```

### properties (repeated) — `Emitter.Property`

Keyed emitter properties animated over the emitter's play time. One entry per `EmitterKey` (except `EMITTER_KEY_COUNT`) is required. See [Emitter Property](#emitter-property) section.

### particle_properties (repeated) — `Emitter.ParticleProperty`

Keyed particle properties animated over each particle's life time. One entry per `ParticleKey` (except `PARTICLE_KEY_COUNT`) is required. See [Particle Property](#particle-property) section.

### modifiers (repeated) — `Modifier`

Emitter-level modifiers that affect only this emitter's particles. See [Modifier](#modifier) section.

### size_mode (optional) — `SizeMode`

Controls how flipbook animations are sized. `SIZE_MODE_MANUAL` uses the particle size property. `SIZE_MODE_AUTO` uses the source image frame size, ignoring the size property. Default: `SIZE_MODE_MANUAL`.

**Omission rule**: Omit if `SIZE_MODE_MANUAL`.

```protobuf
size_mode: SIZE_MODE_AUTO
```

### start_delay_spread (optional) — `float`

Random variation for `start_delay`. Default: `0.0`.

**Omission rule**: Omit if `0.0`.

```protobuf
start_delay_spread: 0.2
```

### duration_spread (optional) — `float`

Random variation for `duration`. Default: `0.0`.

**Omission rule**: Omit if `0.0`.

```protobuf
duration_spread: 0.5
```

### stretch_with_velocity (optional) — `bool`

Whether to scale particle stretch in the direction of movement. Default: `false`.

**Omission rule**: Omit if `false`.

```protobuf
stretch_with_velocity: true
```

### start_offset (optional) — `float`

Number of seconds to prewarm the simulation. The emitter starts as if it had been running for this duration. Default: `0.0`.

**Omission rule**: Omit if `0.0`.

```protobuf
start_offset: 2.0
```

### pivot (optional) — `dmMath.Point3`

Pivot point for the emitter. Components: `x, y, z`, all default `0.0`.

**Omission rule**: Omit if all components are `0.0`.

```protobuf
pivot {
  x: 0.5
  y: 0.5
}
```

### attributes (repeated) — `dmGraphics.VertexAttribute`

Custom vertex attribute overrides. See the `dmGraphics.VertexAttribute` message in `graphics/graphics_ddf.proto`.

**Omission rule**: Omit if no custom attributes are needed.

## Emitter Property

Each `properties` entry inside an emitter represents a keyed property animated over the emitter's play time.

```protobuf
properties {
  key: EMITTER_KEY_SPAWN_RATE
  points {
    y: 100.0
  }
  spread: 10.0
}
```

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `key` | `EmitterKey` | yes | Which property this entry controls |
| `points` | repeated `SplinePoint` | yes | Spline curve points defining the value over time |
| `spread` | `float` | no | Random variation (value ± spread). Default: `0.0` |

All `EmitterKey` values (except `EMITTER_KEY_COUNT`) must have a corresponding `properties` entry. Order follows the enum value order.

## Particle Property

Each `particle_properties` entry represents a property animated over each particle's life time.

```protobuf
particle_properties {
  key: PARTICLE_KEY_ALPHA
  points {
    y: 1.0
  }
  points {
    x: 0.5
    y: 0.8
    t_x: 1.0
  }
  points {
    x: 1.0
    y: 0.0
    t_x: 1.0
  }
}
```

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `key` | `ParticleKey` | yes | Which property this entry controls |
| `points` | repeated `SplinePoint` | yes | Spline curve points defining the value over particle life |

All `ParticleKey` values (except `PARTICLE_KEY_COUNT`) must have a corresponding `particle_properties` entry. Order follows the enum value order.

Note: `ParticleProperty` does not have a `spread` field (unlike `Emitter.Property`).

## SplinePoint format

Spline points define values over time using cubic Hermite spline curves.

| Field | Type | Required | Default | Description |
|-------|------|----------|---------|-------------|
| `x` | `float` | no | — | Time position along the curve (0.0 = start, 1.0 = end) |
| `y` | `float` | yes | — | Value at this point |
| `t_x` | `float` | no | `1.0` | Tangent X (controls curve shape) |
| `t_y` | `float` | no | `0.0` | Tangent Y (controls curve shape) |

### Constant value (no curve)

For a constant property, use a single point with only `y`:

```protobuf
points {
  y: 50.0
}
```

### Curve with multiple points

For values animated over time, use multiple points. `x` ranges from `0.0` (start) to `1.0` (end):

```protobuf
points {
  y: 1.0
}
points {
  x: 0.7
  y: 1.0
  t_x: 1.0
}
points {
  x: 1.0
  y: 0.0
  t_x: 1.0
}
```

## Modifier

Modifiers affect particle velocity. They can be placed at the effect level (inside the top-level `ParticleFX`) or as children of an emitter.

```protobuf
modifiers {
  type: MODIFIER_TYPE_ACCELERATION
  properties {
    key: MODIFIER_KEY_MAGNITUDE
    points {
      y: -100.0
    }
  }
}
```

### Modifier fields

| Field | Type | Required | Default | Description |
|-------|------|----------|---------|-------------|
| `type` | `ModifierType` | yes | — | Type of velocity modification |
| `use_direction` | `uint32` | no | `0` | Whether the modifier uses a direction vector |
| `position` | `dmMath.Point3` | no | `0, 0, 0` | Position relative to parent |
| `rotation` | `dmMath.Quat` | no | identity | Rotation relative to parent |
| `properties` | repeated `Modifier.Property` | yes | — | Modifier properties (magnitude, max distance) |

### Modifier.Property

| Field | Type | Required | Default | Description |
|-------|------|----------|---------|-------------|
| `key` | `ModifierKey` | yes | — | `MODIFIER_KEY_MAGNITUDE` or `MODIFIER_KEY_MAX_DISTANCE` |
| `points` | repeated `SplinePoint` | yes | — | Spline curve points |
| `spread` | `float` | no | `0.0` | Random variation |

`MODIFIER_KEY_MAX_DISTANCE` is only used for `MODIFIER_TYPE_RADIAL` and `MODIFIER_TYPE_VORTEX`.

## Enum: EmitterType

| Constant | Value | Description |
|----------|-------|-------------|
| `EMITTER_TYPE_CIRCLE` | 0 | Emits from a circle; particles directed outward. Size X = diameter |
| `EMITTER_TYPE_2DCONE` | 1 | Emits from a flat cone (triangle); particles directed out the top. Size X = width, Y = height |
| `EMITTER_TYPE_BOX` | 2 | Emits from a box; particles directed up along local Y. Size X/Y/Z = width/height/depth |
| `EMITTER_TYPE_SPHERE` | 3 | Emits from a sphere; particles directed outward. Size X = diameter |
| `EMITTER_TYPE_CONE` | 4 | Emits from a 3D cone; particles directed out the top disc. Size X = diameter, Y = height |

## Enum: PlayMode

| Constant | Value | Description |
|----------|-------|-------------|
| `PLAY_MODE_ONCE` | 0 | Stops after reaching duration |
| `PLAY_MODE_LOOP` | 1 | Restarts after reaching duration |

## Enum: EmissionSpace

| Constant | Value | Description |
|----------|-------|-------------|
| `EMISSION_SPACE_WORLD` | 0 | Particles move independently of the emitter |
| `EMISSION_SPACE_EMITTER` | 1 | Particles move relative to the emitter |

## Enum: BlendMode

| Constant | Value | Description |
|----------|-------|-------------|
| `BLEND_MODE_ALPHA` | 0 | Normal alpha blending |
| `BLEND_MODE_ADD` | 1 | Additive blending (brighten) |
| `BLEND_MODE_ADD_ALPHA` | 2 | **Deprecated** — do not use |
| `BLEND_MODE_MULT` | 3 | Multiply blending (darken) |
| `BLEND_MODE_SCREEN` | 4 | Screen blending (brighten) |

## Enum: SizeMode

| Constant | Value | Description |
|----------|-------|-------------|
| `SIZE_MODE_MANUAL` | 0 | Particle size controlled by size property |
| `SIZE_MODE_AUTO` | 1 | Particle size matches source image frame size |

## Enum: ParticleOrientation

| Constant | Value | Description |
|----------|-------|-------------|
| `PARTICLE_ORIENTATION_DEFAULT` | 0 | Unit orientation |
| `PARTICLE_ORIENTATION_INITIAL_DIRECTION` | 1 | Keeps initial orientation |
| `PARTICLE_ORIENTATION_MOVEMENT_DIRECTION` | 2 | Oriented according to velocity |
| `PARTICLE_ORIENTATION_ANGULAR_VELOCITY` | 3 | Oriented by angular velocity |

## Enum: EmitterKey

| Constant | Value | Description |
|----------|-------|-------------|
| `EMITTER_KEY_SPAWN_RATE` | 0 | Particles emitted per second |
| `EMITTER_KEY_SIZE_X` | 1 | Emitter shape size X |
| `EMITTER_KEY_SIZE_Y` | 2 | Emitter shape size Y |
| `EMITTER_KEY_SIZE_Z` | 3 | Emitter shape size Z |
| `EMITTER_KEY_PARTICLE_LIFE_TIME` | 4 | Particle lifespan in seconds |
| `EMITTER_KEY_PARTICLE_SPEED` | 5 | Initial particle speed |
| `EMITTER_KEY_PARTICLE_SIZE` | 6 | Initial particle size |
| `EMITTER_KEY_PARTICLE_RED` | 7 | Initial red color component |
| `EMITTER_KEY_PARTICLE_GREEN` | 8 | Initial green color component |
| `EMITTER_KEY_PARTICLE_BLUE` | 9 | Initial blue color component |
| `EMITTER_KEY_PARTICLE_ALPHA` | 10 | Initial alpha component |
| `EMITTER_KEY_PARTICLE_ROTATION` | 11 | Initial rotation (degrees) |
| `EMITTER_KEY_PARTICLE_STRETCH_FACTOR_X` | 12 | Initial stretch X (units) |
| `EMITTER_KEY_PARTICLE_STRETCH_FACTOR_Y` | 13 | Initial stretch Y (units) |
| `EMITTER_KEY_PARTICLE_ANGULAR_VELOCITY` | 14 | Initial angular velocity (degrees/second) |

## Enum: ParticleKey

| Constant | Value | Description |
|----------|-------|-------------|
| `PARTICLE_KEY_SCALE` | 0 | Scale over particle life |
| `PARTICLE_KEY_RED` | 1 | Red tint over particle life |
| `PARTICLE_KEY_GREEN` | 2 | Green tint over particle life |
| `PARTICLE_KEY_BLUE` | 3 | Blue tint over particle life |
| `PARTICLE_KEY_ALPHA` | 4 | Alpha over particle life |
| `PARTICLE_KEY_ROTATION` | 5 | Rotation over particle life (degrees) |
| `PARTICLE_KEY_STRETCH_FACTOR_X` | 6 | Stretch X over particle life (units) |
| `PARTICLE_KEY_STRETCH_FACTOR_Y` | 7 | Stretch Y over particle life (units) |
| `PARTICLE_KEY_ANGULAR_VELOCITY` | 8 | Angular velocity over particle life (degrees/second) |

## Enum: ModifierType

| Constant | Value | Description |
|----------|-------|-------------|
| `MODIFIER_TYPE_ACCELERATION` | 0 | Acceleration in a general direction |
| `MODIFIER_TYPE_DRAG` | 1 | Reduces acceleration proportional to velocity |
| `MODIFIER_TYPE_RADIAL` | 2 | Attracts or repels from a position |
| `MODIFIER_TYPE_VORTEX` | 3 | Circular/spiral movement around a position |

## Enum: ModifierKey

| Constant | Value | Description |
|----------|-------|-------------|
| `MODIFIER_KEY_MAGNITUDE` | 0 | Amount of effect on particles |
| `MODIFIER_KEY_MAX_DISTANCE` | 1 | Maximum distance for effect (Radial and Vortex only) |

## Protobuf Text Format rules

1. **Default omission**: Omit optional scalar fields that equal their proto default.
2. **Message blocks**: Use `field_name { ... }` with nested `key: value` pairs.
3. **Floats**: Always include decimal point: `1.0`, not `1`.
4. **Integers**: No decimal point: `64`, not `64.0`.
5. **Strings**: Always double-quoted: `"text"`.
6. **Enums**: Use the enum constant name without quotes.
7. **Booleans**: `true` or `false`, no quotes.
8. **Repeated messages**: Each entry gets its own `field_name { ... }` block.
9. **Field order**: Follow the proto field number order.
10. **No trailing commas or semicolons**.
11. **No field number tags** — use field names only.
12. **Indentation**: 2 spaces per nesting level inside message blocks.
13. **All `properties` and `particle_properties` entries are required** — include one for each enum key (except `_COUNT` values), even if the value is `0.0` or `1.0`.

## Common templates

### Simple one-shot burst

```protobuf
emitters {
  id: "burst"
  mode: PLAY_MODE_ONCE
  duration: 0.2
  space: EMISSION_SPACE_WORLD
  tile_source: "/assets/particles/particles.atlas"
  animation: "particle"
  material: "/builtins/materials/particlefx.material"
  blend_mode: BLEND_MODE_ADD
  max_particle_count: 32
  type: EMITTER_TYPE_CIRCLE
  properties {
    key: EMITTER_KEY_SPAWN_RATE
    points {
      y: 200.0
    }
  }
  properties {
    key: EMITTER_KEY_SIZE_X
    points {
      y: 0.0
    }
  }
  properties {
    key: EMITTER_KEY_SIZE_Y
    points {
      y: 0.0
    }
  }
  properties {
    key: EMITTER_KEY_SIZE_Z
    points {
      y: 0.0
    }
  }
  properties {
    key: EMITTER_KEY_PARTICLE_LIFE_TIME
    points {
      y: 0.5
    }
  }
  properties {
    key: EMITTER_KEY_PARTICLE_SPEED
    points {
      y: 300.0
    }
  }
  properties {
    key: EMITTER_KEY_PARTICLE_SIZE
    points {
      y: 15.0
    }
  }
  properties {
    key: EMITTER_KEY_PARTICLE_RED
    points {
      y: 1.0
    }
  }
  properties {
    key: EMITTER_KEY_PARTICLE_GREEN
    points {
      y: 0.8
    }
  }
  properties {
    key: EMITTER_KEY_PARTICLE_BLUE
    points {
      y: 0.2
    }
  }
  properties {
    key: EMITTER_KEY_PARTICLE_ALPHA
    points {
      y: 1.0
    }
  }
  properties {
    key: EMITTER_KEY_PARTICLE_ROTATION
    points {
      y: 0.0
    }
  }
  properties {
    key: EMITTER_KEY_PARTICLE_STRETCH_FACTOR_X
    points {
      y: 0.0
    }
  }
  properties {
    key: EMITTER_KEY_PARTICLE_STRETCH_FACTOR_Y
    points {
      y: 0.0
    }
  }
  properties {
    key: EMITTER_KEY_PARTICLE_ANGULAR_VELOCITY
    points {
      y: 0.0
    }
  }
  particle_properties {
    key: PARTICLE_KEY_SCALE
    points {
      y: 1.0
    }
  }
  particle_properties {
    key: PARTICLE_KEY_RED
    points {
      y: 1.0
    }
  }
  particle_properties {
    key: PARTICLE_KEY_GREEN
    points {
      y: 1.0
    }
  }
  particle_properties {
    key: PARTICLE_KEY_BLUE
    points {
      y: 1.0
    }
  }
  particle_properties {
    key: PARTICLE_KEY_ALPHA
    points {
      y: 1.0
    }
    points {
      x: 1.0
      y: 0.0
      t_x: 1.0
    }
  }
  particle_properties {
    key: PARTICLE_KEY_ROTATION
    points {
      y: 0.0
    }
  }
  particle_properties {
    key: PARTICLE_KEY_STRETCH_FACTOR_X
    points {
      y: 0.0
    }
  }
  particle_properties {
    key: PARTICLE_KEY_STRETCH_FACTOR_Y
    points {
      y: 0.0
    }
  }
  particle_properties {
    key: PARTICLE_KEY_ANGULAR_VELOCITY
    points {
      y: 0.0
    }
  }
}
```

### Looping emitter with gravity modifier

```protobuf
emitters {
  id: "fire"
  mode: PLAY_MODE_LOOP
  space: EMISSION_SPACE_WORLD
  tile_source: "/assets/particles/particles.atlas"
  animation: "flame"
  material: "/builtins/materials/particlefx.material"
  blend_mode: BLEND_MODE_ADD
  max_particle_count: 100
  type: EMITTER_TYPE_BOX
  properties {
    key: EMITTER_KEY_SPAWN_RATE
    points {
      y: 30.0
    }
  }
  properties {
    key: EMITTER_KEY_SIZE_X
    points {
      y: 20.0
    }
  }
  properties {
    key: EMITTER_KEY_SIZE_Y
    points {
      y: 0.0
    }
  }
  properties {
    key: EMITTER_KEY_SIZE_Z
    points {
      y: 0.0
    }
  }
  properties {
    key: EMITTER_KEY_PARTICLE_LIFE_TIME
    points {
      y: 1.5
    }
  }
  properties {
    key: EMITTER_KEY_PARTICLE_SPEED
    points {
      y: 50.0
    }
  }
  properties {
    key: EMITTER_KEY_PARTICLE_SIZE
    points {
      y: 30.0
    }
  }
  properties {
    key: EMITTER_KEY_PARTICLE_RED
    points {
      y: 1.0
    }
  }
  properties {
    key: EMITTER_KEY_PARTICLE_GREEN
    points {
      y: 0.6
    }
  }
  properties {
    key: EMITTER_KEY_PARTICLE_BLUE
    points {
      y: 0.1
    }
  }
  properties {
    key: EMITTER_KEY_PARTICLE_ALPHA
    points {
      y: 1.0
    }
  }
  properties {
    key: EMITTER_KEY_PARTICLE_ROTATION
    points {
      y: 0.0
    }
  }
  properties {
    key: EMITTER_KEY_PARTICLE_STRETCH_FACTOR_X
    points {
      y: 0.0
    }
  }
  properties {
    key: EMITTER_KEY_PARTICLE_STRETCH_FACTOR_Y
    points {
      y: 0.0
    }
  }
  properties {
    key: EMITTER_KEY_PARTICLE_ANGULAR_VELOCITY
    points {
      y: 0.0
    }
  }
  particle_properties {
    key: PARTICLE_KEY_SCALE
    points {
      y: 1.0
    }
    points {
      x: 1.0
      y: 0.5
      t_x: 1.0
    }
  }
  particle_properties {
    key: PARTICLE_KEY_RED
    points {
      y: 1.0
    }
  }
  particle_properties {
    key: PARTICLE_KEY_GREEN
    points {
      y: 1.0
    }
    points {
      x: 1.0
      y: 0.3
      t_x: 1.0
    }
  }
  particle_properties {
    key: PARTICLE_KEY_BLUE
    points {
      y: 1.0
    }
  }
  particle_properties {
    key: PARTICLE_KEY_ALPHA
    points {
      y: 1.0
    }
    points {
      x: 0.8
      y: 0.5
      t_x: 1.0
    }
    points {
      x: 1.0
      y: 0.0
      t_x: 1.0
    }
  }
  particle_properties {
    key: PARTICLE_KEY_ROTATION
    points {
      y: 0.0
    }
  }
  particle_properties {
    key: PARTICLE_KEY_STRETCH_FACTOR_X
    points {
      y: 0.0
    }
  }
  particle_properties {
    key: PARTICLE_KEY_STRETCH_FACTOR_Y
    points {
      y: 0.0
    }
  }
  particle_properties {
    key: PARTICLE_KEY_ANGULAR_VELOCITY
    points {
      y: 0.0
    }
  }
  modifiers {
    type: MODIFIER_TYPE_ACCELERATION
    properties {
      key: MODIFIER_KEY_MAGNITUDE
      points {
        y: 50.0
      }
    }
  }
}
```

## Workflow

### Creating a new particle effect

1. Determine the file path (must end with `.particlefx`).
2. Add at least one `emitters` entry with all required fields: `mode`, `space`, `tile_source`, `animation`, `material`, `max_particle_count`, `type`.
3. Include all 15 `properties` entries (one per `EmitterKey`, excluding `EMITTER_KEY_COUNT`), in enum value order.
4. Include all 9 `particle_properties` entries (one per `ParticleKey`, excluding `PARTICLE_KEY_COUNT`), in enum value order.
5. Set optional emitter fields (`blend_mode`, `duration`, `start_delay`, `size_mode`, etc.) only if they differ from defaults.
6. Add modifiers as needed, either at the emitter level or effect level.
7. Follow proto field number order for all fields.

### Editing an existing particle effect

1. Read the current `.particlefx` file.
2. Modify only the requested fields or property values.
3. Preserve existing field values, order, and all `properties`/`particle_properties` entries.
4. Apply omission rules for optional scalar fields only — never omit `properties` or `particle_properties` entries.
