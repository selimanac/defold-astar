# Editing Object Interpolations

Creates and edits Defold `.objectinterpolation` component files using Protobuf Text Format.

## Overview

Object Interpolation is an **extension component** (not built-in). It interpolates position and rotation of a game object between fixed update steps. Typical use cases: smoothing movement of objects with `collisionobject` components in fixed time step mode, or smoothing movement driven by `fixed_update()`.

### Prerequisite

This component is only available when the `defold-object-interpolation` library is added as a dependency in `game.project`. Before creating `.objectinterpolation` files, verify the dependency exists:

```ini
[project]
dependencies#N = https://github.com/indiesoftby/defold-object-interpolation/archive/...zip
```

If the dependency is missing, inform the user that they need to add it to `game.project` and fetch libraries before the component can be used.

## File format

Object Interpolation files (`.objectinterpolation`) use **Protobuf Text Format** based on the `ObjectInterpolationDesc` message from the extension's `objectinterpolation_ddf.proto`.

### Canonical example

```protobuf
apply_transform: APPLY_TRANSFORM_TARGET
target_object: "player"
```

### Minimal file (all defaults)

An empty file is valid — `apply_transform` defaults to `APPLY_TRANSFORM_NONE` and `target_object` is optional.

```protobuf
```

### Embedded in a game object

When embedded in a `.go` file, the data appears inside an `embedded_components` block:

```protobuf
embedded_components {
  id: "objectinterpolation"
  type: "objectinterpolation"
  data: "apply_transform: APPLY_TRANSFORM_TARGET\n"
  "target_object: \"visual\"\n"
  ""
}
```

## Fields reference

### apply_transform (required) — `ApplyTransform` enum

The interpolation mode. Default: `APPLY_TRANSFORM_NONE`.

- `APPLY_TRANSFORM_NONE` — only calculates interpolated values, does not apply them to any object. Values are readable via `go.get()`.
- `APPLY_TRANSFORM_TARGET` — applies interpolated position and rotation to the game object specified in `target_object`.

**Omission rule**: Omit if `APPLY_TRANSFORM_NONE`.

```protobuf
apply_transform: APPLY_TRANSFORM_TARGET
```

### target_object (optional) — `string`

The game object identifier to apply interpolated transform to. Supports relative and absolute paths (like `go.get_id()`). Only meaningful when `apply_transform` is `APPLY_TRANSFORM_TARGET`.

**Omission rule**: Omit if empty or not needed (when `apply_transform` is `APPLY_TRANSFORM_NONE`).

```protobuf
target_object: "visual"
```

## Enum: ApplyTransform

| Constant | Display Name | Description |
|----------|-------------|-------------|
| `APPLY_TRANSFORM_NONE` | None | Only interpolates values for reading via `go.get()` |
| `APPLY_TRANSFORM_TARGET` | Target Object | Applies interpolated transform to `target_object` |

## Common templates

### Passive interpolation (read-only values)

Use when you want to read interpolated position/rotation from script but not automatically apply them.

```protobuf
```

### Target object interpolation

The most common setup — interpolate and apply to a visual representation object.

```protobuf
apply_transform: APPLY_TRANSFORM_TARGET
target_object: "visual"
```

## Best practice: collection with physics + visual objects

The recommended setup is a collection with two game objects:

1. **Physics object** — has `collisionobject` and `objectinterpolation` components. The `objectinterpolation` targets the visual object using a relative ID within the collection.
2. **Visual object** — has a `sprite` or `model` component that displays the physics object on screen with smooth interpolated movement.

The `target_object` field uses a **relative path** within the collection scope (e.g., `"visual"` refers to a sibling game object named `visual` in the same collection).

### Example `.objectinterpolation` for this setup

```protobuf
apply_transform: APPLY_TRANSFORM_TARGET
target_object: "visual"
```

Where `visual` is the ID of the sibling game object in the same collection that has the `sprite` or `model` component.

## Runtime API reference

### Properties (via `go.get()` / `go.set()`)

| Property | Type | Access | Description |
|----------|------|--------|-------------|
| `apply_transform` | number | read | Current mode (`object_interpolation.APPLY_TRANSFORM_NONE` or `object_interpolation.APPLY_TRANSFORM_TARGET`) |
| `target_object` | hash | read | Target game object identifier |
| `position` | vector3 | read/write | Interpolated position. Setting resets interpolation. |
| `rotation` | quat | read/write | Interpolated rotation. Setting resets interpolation. |

### Messages

- `set_apply_transform` — change mode and target at runtime:

```lua
msg.post("#objectinterpolation", "set_apply_transform", {
    apply_transform = object_interpolation.APPLY_TRANSFORM_TARGET,
    target_object = hash("/object_to_move")
})
```

### Lua API

- `object_interpolation.set_enabled(enabled)` — enable/disable interpolation globally.
- `object_interpolation.is_enabled()` — check if enabled.

### game.project settings

```ini
[object_interpolation]
max_count = 1024
```

## Protobuf Text Format rules

1. **Default omission**: Omit fields that equal their proto default.
2. **Strings**: Always double-quoted: `"text"`.
3. **Enums**: Use the constant name without quotes: `APPLY_TRANSFORM_TARGET`.
4. **Field order**: Follow proto field number order: `apply_transform`, `target_object`.
5. **No trailing commas or semicolons**.
6. **No empty lines between fields** (all fields are scalar).

## Workflow

### Creating a new object interpolation component

1. **Verify dependency**: Check that `game.project` includes the `defold-object-interpolation` library URL in its `dependencies` list. If missing, inform the user.
2. Determine the file path (must end with `.objectinterpolation`).
3. Choose the mode: `APPLY_TRANSFORM_NONE` (default, omit field) or `APPLY_TRANSFORM_TARGET`.
4. If using `APPLY_TRANSFORM_TARGET`, set `target_object` to the target game object path.
5. Write the file using the field order from the reference above.

### Editing an existing object interpolation component

1. Read the current `.objectinterpolation` file.
2. Modify only the requested fields.
3. Preserve existing field values and order.
4. Apply omission rules: remove fields that become equal to their defaults after editing.
