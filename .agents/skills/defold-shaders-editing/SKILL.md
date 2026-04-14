---
name: defold-shaders-editing
description: "Creates and edits Defold shader files (.vp, .fp, .glsl). Use when asked to create, modify, or configure any Defold vertex shader, fragment shader, or GLSL include file."
---

# Editing Defold Shaders

Creates and edits Defold shader files: vertex programs (`.vp`), fragment programs (`.fp`), and GLSL include snippets (`.glsl`).

## When to use

This skill covers GLSL shader files used in Defold's rendering pipeline. It does NOT cover `.material` files — for those, use the `defold-proto-file-editing` skill (see `references/material.md`).

## Shader pipeline and GLSL version

Defold supports two shader pipelines:

1. **Legacy pipeline** — shaders written in OpenGL ES 2.0 compatible GLSL (no `#version` directive). **Deprecated since Defold 1.9.2.**
2. **Modern pipeline** — shaders written in SPIR-V compatible GLSL with `#version 140` or higher. **This is the current standard.**

**Always write shaders using `#version 140`** (OpenGL 3.1) at the top of the file. This directive selects the modern pipeline during the build process. If no `#version` is found, Defold falls back to the legacy pipeline.

```glsl
#version 140
```

### Cross-compilation and platform targets

Defold compiles shaders for multiple graphics APIs from a single GLSL source:

- **OpenGL 3.x / 4.x** (desktop: Windows, macOS, Linux)
- **OpenGL ES 2.0 / 3.0** (mobile: Android, iOS)
- **WebGL 1.0 / 2.0** (HTML5)
- **SPIR-V** (Vulkan on Android, desktop)
- **Metal** (iOS, macOS — via SPIR-V cross-compilation)

Because of this cross-compilation, **not all GLSL features are available everywhere**. Some functions (e.g., `dFdx`, `dFdy`, `fwidth`) require extensions on ES 2.0 / WebGL 1.0 targets but are built-in on ES 3.0+ / desktop GL. Use `#extension` and preprocessor guards for conditional features:

```glsl
#ifdef GL_OES_standard_derivatives
#extension GL_OES_standard_derivatives : enable
#endif

#if !defined(GL_ES) || __VERSION__ >= 300 || defined(GL_OES_standard_derivatives)
    // Use derivative functions
#else
    // Provide fallback
#endif
```

**Key limitations to keep in mind:**
- Dynamic loops with variable bounds may not work on ES 2.0 / WebGL 1.0
- Integer operations are limited on ES 2.0
- `sampler2DArray` requires ES 3.0+ / WebGL 2.0+
- Storage buffers and compute shaders require Vulkan / Metal

## File types

### Vertex program (`.vp`)

Runs once per vertex. Transforms vertex positions from model/world space to screen space. Outputs `gl_Position` and passes data to the fragment shader via `out` variables.

### Fragment program (`.fp`)

Runs once per fragment (pixel). Computes the final color. Outputs to a user-defined `out vec4` variable (not `gl_FragColor`, which is deprecated in `#version 140`).

### GLSL include snippet (`.glsl`)

Reusable GLSL code included by `.vp` or `.fp` files via `#include`. Does not run standalone. Use header guards to prevent double-inclusion.

## Modern GLSL syntax rules (#version 140)

### Attributes → `in`

In vertex shaders, use `in` instead of `attribute`:

```glsl
in highp vec4 position;
in mediump vec2 texcoord0;
```

Fragment shaders do NOT have vertex attributes.

### Varyings → `out` / `in`

In vertex shaders, use `out` instead of `varying`. In fragment shaders, use `in`:

```glsl
// vertex shader
out mediump vec2 var_texcoord0;

// fragment shader
in mediump vec2 var_texcoord0;
```

### Fragment output → `out vec4`

Use a declared `out` variable instead of the deprecated `gl_FragColor`:

```glsl
out vec4 out_fragColor;

void main()
{
    out_fragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
```

### Uniform blocks

Non-opaque uniforms (matrices, vectors, floats) must be placed in a **uniform block**. Use `vs_uniforms` for vertex shaders and `fs_uniforms` for fragment shaders (by convention):

```glsl
uniform vs_uniforms
{
    highp mat4 view_proj;
};
```

Opaque uniforms (samplers, images) remain standalone:

```glsl
uniform mediump sampler2D texture_sampler;
```

Members of the uniform block are accessed directly by name (no block prefix):

```glsl
gl_Position = view_proj * vec4(position.xyz, 1.0);
```

### Texture sampling → `texture()`

Use `texture()` instead of the deprecated `texture2D()` / `texture2DArray()`:

```glsl
vec4 color = texture(texture_sampler, var_texcoord0.xy);
```

### Precision qualifiers

Explicit precision (`lowp`, `mediump`, `highp`) is **optional** in `#version 140` — the pipeline sets precision automatically for platforms that need it. However, you may still use them for clarity or to match existing code style in the project. Follow the convention of surrounding files.

## Editor-specific code (`EDITOR` define)

When shaders are rendered in the Defold Editor viewport, the preprocessor define `EDITOR` is available. Use `#ifdef EDITOR` to write code that behaves differently in the editor vs the game:

```glsl
#ifdef EDITOR
    // Simplified rendering for editor preview
    out_fragColor = texture(texture_sampler, var_texcoord0.xy);
#else
    // Full rendering with effects for the game
    out_fragColor = apply_effects(texture(texture_sampler, var_texcoord0.xy));
#endif
```

Common use cases:
- Disable expensive effects (RGSS, post-processing) in editor for performance
- Show debug/fallback visuals for materials that don't preview well
- Skip features that depend on runtime-only data (e.g., skinning)

## Including snippets (`#include`)

Shader files can include `.glsl` snippets using `#include` with project-relative or file-relative paths:

```glsl
// Absolute (project-relative) path
#include "/main/my-snippet.glsl"

// Relative to current file
#include "my-snippet.glsl"
#include "../shared/utils.glsl"
```

**Rules:**
- Only `.glsl` files can be included
- Paths must be within the project (or library dependencies)
- Absolute paths start with `/`
- `#include` cannot be used inline within a statement

### Header guards

Use header guards in `.glsl` snippets to prevent double-inclusion:

```glsl
#ifndef MY_SNIPPET_GLSL
#define MY_SNIPPET_GLSL

// ... snippet code ...

#endif // MY_SNIPPET_GLSL
```

## Relationship with materials

Shaders and materials (`.material`) are tightly coupled. The material file defines **what data** the shader receives. For material file editing, use the `defold-proto-file-editing` skill (reference: `references/material.md`).

### Constants (uniforms)

Constants declared in the material's `vertex_constants` / `fragment_constants` become uniform variables in the shader. Place them inside the uniform block:

| Material constant type | Shader uniform type | Description |
|------------------------|---------------------|-------------|
| `CONSTANT_TYPE_VIEWPROJ` | `mat4` | Combined view × projection matrix |
| `CONSTANT_TYPE_WORLD` | `mat4` | World transform matrix |
| `CONSTANT_TYPE_VIEW` | `mat4` | View (camera) matrix |
| `CONSTANT_TYPE_PROJECTION` | `mat4` | Projection matrix |
| `CONSTANT_TYPE_WORLDVIEW` | `mat4` | World × view matrix |
| `CONSTANT_TYPE_WORLDVIEWPROJ` | `mat4` | World × view × projection matrix |
| `CONSTANT_TYPE_NORMAL` | `mat4` | Normal matrix — `transpose(inverse(view * world))`. **Produces view-space normals, not world-space.** |
| `CONSTANT_TYPE_USER` | `vec4` | Custom data, mutable via `go.set()` / `go.animate()` |
| `CONSTANT_TYPE_USER_MATRIX4` | `mat4` | Custom matrix data, mutable via `go.set()` |

The `name` in the material must match the variable name in the uniform block.

### Samplers

Samplers declared in the material's `samplers` section become `sampler2D` uniforms. The sampler `name` in the material must match the uniform name in the shader:

```glsl
uniform mediump sampler2D texture_sampler;  // Matches samplers { name: "texture_sampler" ... }
```

For sprites, tilemaps, GUI, and particles — the first `sampler2D` is automatically bound to the component's image.

### Vertex attributes

Attributes declared in the material's `attributes` section (or default attributes provided by the engine) become `in` variables in the vertex shader.

**Default attributes by component type:**

| Component | Attributes |
|-----------|------------|
| Sprite | `position`, `texcoord0` |
| Tilemap | `position`, `texcoord0` |
| GUI node | `position`, `texcoord0`, `color` |
| ParticleFX | `position`, `texcoord0`, `color` |
| Model | `position`, `texcoord0`, `normal` |
| Font | `position`, `texcoord0`, `face_color`, `outline_color`, `shadow_color` |

### Vertex space

The material's `vertex_space` setting affects how position data arrives:

- `VERTEX_SPACE_WORLD` (default) — positions are pre-transformed to world space. Used for 2D components (sprites, tilemaps). Use `view_proj` (or `CONSTANT_TYPE_VIEWPROJ`) to go directly to screen space.
- `VERTEX_SPACE_LOCAL` — positions are in local/object space. Used for 3D models. You must transform through `world → view → projection` in the shader.

### Instancing

For instanced rendering (Model components), declare `mtx_world` and `mtx_normal` as `in` attributes (not uniforms):

```glsl
in mediump mat4 mtx_world;
in mediump mat4 mtx_normal;
```

The material must have `vertex_space: VERTEX_SPACE_LOCAL`. These attributes are automatically configured for per-instance step function.

### Normal matrix (`mtx_normal`) — view-space vs world-space

The built-in `CONSTANT_TYPE_NORMAL` computes `transpose(inverse(view * world))` on the CPU. This produces normals in **view-space** (camera-space), not world-space.

**View-space normals are fine when:**
- All lighting calculations are done in view-space
- Camera position is implicitly at origin `(0,0,0)` (simplifies specular)

**World-space normals are needed for:**
- Cubemap reflections, environment mapping
- World-space lighting, world-space effects

To get world-space normals, compute the normal matrix from `mtx_world` in the vertex shader using the adjugate matrix trick (cheaper than full `inverse()` — 3 cross products instead of cofactor expansion):

```glsl
// transpose(adjugate(M)) for upper-left 3x3 of mat4.
// Equivalent to transpose(inverse(M)) up to a uniform scale factor,
// which is eliminated by normalize().
mat3 adjoint(mat4 m)
{
    return mat3(
        cross(m[1].xyz, m[2].xyz),
        cross(m[2].xyz, m[0].xyz),
        cross(m[0].xyz, m[1].xyz)
    );
}
```

Usage in vertex shader:

```glsl
var_world_normal = normalize(adjoint(mtx_world) * normal.xyz);
```

| Goal | Method |
|------|--------|
| View-space normals | Use built-in `mtx_normal` (`CONSTANT_TYPE_NORMAL`) |
| World-space normals | Compute `normalize(adjoint(mtx_world) * normal.xyz)` in shader |

**Key rule:** Never mix coordinate spaces — if light direction is in world-space, normals must also be in world-space.

## Canonical examples

### Builtin reference shaders

Use `.deps/builtins/materials/` as reference for standard shaders:

- `sprite.vp` / `sprite.fp` — 2D sprite (world space, `view_proj`, `tint`)
- `model.vp` / `model.fp` — 3D model with lighting (local space, `mtx_worldview`, `mtx_normal`, `light`)
- `model_instanced.vp` — 3D model with instancing (`mtx_world`, `mtx_normal` as `in` attributes)
- `gui.vp` / `gui.fp` — GUI nodes (world space, `color` attribute, premultiplied alpha)
- `particlefx.vp` / `particlefx.fp` — Particle effects
- `tile_map.vp` / `tile_map.fp` — Tilemaps
- `skinning.glsl` — Skeletal animation include (uses `#ifdef EDITOR` for fallback)

### GLSL include snippet (with header guards and extension)

```glsl
#ifndef MY_UTILS_GLSL
#define MY_UTILS_GLSL

#ifdef GL_OES_standard_derivatives
#extension GL_OES_standard_derivatives : enable
#endif

// Utility function available on platforms that support derivatives
#if !defined(GL_ES) || __VERSION__ >= 300 || defined(GL_OES_standard_derivatives)
mediump float edge_smoothing(mediump float dist)
{
    return smoothstep(0.0, fwidth(dist), dist);
}
#else
mediump float edge_smoothing(mediump float dist)
{
    return step(0.0, dist);
}
#endif

#endif // MY_UTILS_GLSL
```

## Workflow

### Creating a new shader pair (.vp + .fp)

1. Determine the component type (sprite, model, GUI, etc.) to know which attributes and vertex space to use.
2. Check the corresponding `.material` file (or plan one) to know which constants, samplers, and attributes the shader will receive. Use the `defold-proto-file-editing` skill for material editing.
3. Start both files with `#version 140`.
4. In the `.vp`: declare `in` attributes, `out` varyings, uniform block with constants, and compute `gl_Position`.
5. In the `.fp`: declare `in` varyings, `out vec4` for color output, samplers, uniform block, and compute the output color.
6. Ensure uniform names match the material's constant names exactly.
7. Ensure sampler names match the material's sampler names exactly.

### Creating a GLSL include snippet

1. Create a `.glsl` file.
2. Add header guards (`#ifndef` / `#define` / `#endif`).
3. Add `#extension` directives with preprocessor guards if using features not available on all targets.
4. Write reusable functions or constants.

### Editing an existing shader

1. Read the current shader file and its corresponding `.material` file.
2. Modify only the requested parts.
3. Keep the existing code style (precision qualifiers, naming, spacing).
4. Ensure any new uniforms are also added to the material file (use `defold-proto-file-editing` skill).
5. Ensure any removed uniforms are also removed from the material file.
