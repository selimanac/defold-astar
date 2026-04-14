---
name: xmath-usage
description: "Provides xmath API reference and in-place math optimization patterns for Defold. Use when writing performance-critical math code, optimizing vector/quaternion/matrix operations, or when the user mentions xmath, zero-allocation math, or reducing Lua GC pressure."
---

# Using xmath for Zero-Allocation Math in Defold

## Prerequisite: Verify xmath Dependency

Before applying any guidance from this skill, you MUST confirm that the project uses xmath. Check the `game.project` file for a dependency URL containing `thejustinwalsh/defold-xmath` (e.g. `dependencies#N = https://github.com/thejustinwalsh/defold-xmath/archive/...`). Alternatively, check for the presence of `xmath/` in the `.deps/` directory.

If neither an xmath dependency in `game.project` nor a local xmath module is found, **do NOT apply this skill**. Inform the user that the project does not use xmath and suggest adding the dependency:
```
[project]
dependencies#N = https://github.com/thejustinwalsh/defold-xmath/archive/refs/heads/main.zip
```

---

## Core Concept: In-Place Mutation to Eliminate Heap Allocations

Standard `vmath` creates a **new Lua object on every operation**, causing constant GC pressure in hot loops:

```lua
-- BAD: vmath allocates 3 new objects every frame
function update(self, dt)
    local v = self.dir * 5 * dt    -- alloc #1
    local pos = go.get_position()  -- alloc #2
    local result = pos + v         -- alloc #3
    go.set_position(result)
end
```

xmath **mutates an existing variable in place** — the result is written into the first argument. You allocate once, reuse forever:

```lua
-- GOOD: xmath reuses pre-allocated variables, zero allocations per frame
go.property("dir", vmath.vector3(0, 1, 0))

local v = vmath.vector3()  -- allocate ONCE at module scope

function update(self, dt)
    local pos = go.get_position()
    xmath.mul(v, self.dir, 5 * dt)  -- writes into v
    xmath.add(v, pos, v)            -- writes into v
    go.set_position(v)
end
```

## Key Rules

1. **Pre-allocate scratch variables at module scope or in `init()`** — never inside `update()` or `on_message()`.
2. **The output variable is always the first argument** — this is the fundamental calling convention difference from `vmath`.
3. **Functions return nothing** — you cannot chain calls. Use a scratch variable at each step.
4. **Use `vmath` to create initial objects** — `vmath.vector3()`, `vmath.vector4()`, `vmath.quat()`, `vmath.matrix4()` to allocate scratch buffers, then use `xmath` to operate on them.
5. **Type polymorphism** — functions like `xmath.lerp` work for `vector3`, `vector4`, and `quaternion` based on the output argument type.

## Optimization Pattern

```lua
-- Scratch variables — allocated once
local temp_v = vmath.vector3()
local temp_q = vmath.quat()

function update(self, dt)
    -- Instead of: local dir = vmath.normalize(target - pos)
    xmath.sub(temp_v, self.target, self.pos)
    xmath.normalize(temp_v, temp_v)  -- can use same variable as both input and output

    -- Instead of: local rot = vmath.quat_rotation_z(angle)
    xmath.quat_rotation_z(temp_q, self.angle)

    -- Instead of: local rotated = vmath.rotate(rot, dir)
    xmath.rotate(temp_v, temp_q, temp_v)
end
```

---

## Full API Reference

All functions write the result into the first argument. No return values.

### Vector Operations (vector3 / vector4)

| Function | Equivalent | Description |
|---|---|---|
| `xmath.add(out, v1, v2)` | `out = v1 + v2` | Add two vectors |
| `xmath.sub(out, v1, v2)` | `out = v1 - v2` | Subtract two vectors |
| `xmath.mul(out, v, n)` | `out = v * n` | Multiply vector by scalar |
| `xmath.div(out, v, n)` | `out = v / n` | Divide vector by scalar |
| `xmath.cross(out, v1, v2)` | `out = cross(v1, v2)` | Cross product (vector3 only) |
| `xmath.mul_per_elem(out, v1, v2)` | `out.x = v1.x * v2.x, ...` | Element-wise multiplication |
| `xmath.normalize(out, v)` | `out = normalize(v)` | Normalize vector |
| `xmath.rotate(out, q, v)` | `out = rotate(q, v)` | Rotate vector3 by quaternion |
| `xmath.vector(out)` | `out = (0,0,0)` | Reset to zero vector |

### Interpolation (vector3 / vector4 / quaternion)

| Function | Equivalent | Description |
|---|---|---|
| `xmath.lerp(out, t, v1, v2)` | `out = lerp(t, v1, v2)` | Linear interpolation |
| `xmath.slerp(out, t, v1, v2)` | `out = slerp(t, v1, v2)` | Spherical interpolation |

### Quaternion Operations

| Function | Description |
|---|---|
| `xmath.quat(out)` | Reset to identity `(0, 0, 0, 1)` |
| `xmath.conj(out, q)` | Conjugate of quaternion |
| `xmath.quat_axis_angle(out, axis, angle)` | Quaternion from axis + angle |
| `xmath.quat_basis(out, x, y, z)` | Quaternion from 3 basis vectors (vector3) |
| `xmath.quat_from_to(out, v1, v2)` | Rotation quaternion from v1 to v2 |
| `xmath.quat_rotation_x(out, angle)` | Rotation around X axis |
| `xmath.quat_rotation_y(out, angle)` | Rotation around Y axis |
| `xmath.quat_rotation_z(out, angle)` | Rotation around Z axis |

### Matrix Operations (matrix4)

| Function | Description |
|---|---|
| `xmath.matrix(out [, m1])` | Reset to identity or copy from m1 |
| `xmath.matrix_axis_angle(out, axis, angle)` | Rotation matrix from axis + angle |
| `xmath.matrix_from_quat(out, q)` | Matrix from quaternion |
| `xmath.matrix_frustum(out, left, right, bottom, top, near, far)` | Frustum projection matrix |
| `xmath.matrix_inv(out, m)` | Matrix inverse |
| `xmath.matrix_look_at(out, eye, look_at, up)` | View matrix |
| `xmath.matrix4_orthographic(out, left, right, bottom, top, near, far)` | Orthographic projection |
| `xmath.matrix_ortho_inv(out, m)` | Orthographic inverse |
| `xmath.matrix4_perspective(out, fov, aspect, near, far)` | Perspective projection |
| `xmath.matrix_rotation_x(out, angle)` | Rotation around X axis |
| `xmath.matrix_rotation_y(out, angle)` | Rotation around Y axis |
| `xmath.matrix_rotation_z(out, angle)` | Rotation around Z axis |
| `xmath.matrix_translation(out, position)` | Translation matrix from vector3/vector4 |
