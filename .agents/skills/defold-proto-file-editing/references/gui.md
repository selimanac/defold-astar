
# Editing GUI Files

Defold GUI scene (`.gui`) — defines a 2D user interface component with nodes, fonts, textures, materials, layers, and an optional script.

## Overview

A GUI component is attached to a game object and rendered independently of the game view (on top by default). It has its own coordinate space and layout system. A `.gui` file defines:

- **Dependencies**: fonts, textures (atlases), materials, particle effects, resources
- **Layers**: control draw order for batching optimization
- **Nodes**: the visual elements (box, text, pie, template, particlefx, custom)
- **Layouts**: alternative node configurations for different screen sizes

GUI nodes are rendered in list order (first = behind, last = in front). Parent-child hierarchies inherit transforms. Layers override default draw order.

GUI scripts (`.gui_script`) have access to `gui` namespace but **not** `go` or `render`.

A `.gui` is added to a game object as a **component** — either as a file reference in `.go` or embedded in `.collection`. It is rendered independently of the game view; the game object's position has no effect on the GUI.

## File format

GUI files use **Protobuf Text Format** based on the `SceneDesc` message from `gamesys/gui_ddf.proto`.

## Canonical example

From `main/example.gui` — minimal GUI with a single box node:
```protobuf
nodes {
  size {
    x: 200.0
    y: 100.0
  }
  type: TYPE_BOX
  id: "box"
  inherit_alpha: true
  size_mode: SIZE_MODE_AUTO
}
material: "/builtins/materials/gui.material"
adjust_reference: ADJUST_REFERENCE_PARENT
```

Key observations from editor output:
- Fields at proto defaults are **omitted** (no `position`, `rotation`, `scale`, `color`, `blend_mode`, `alpha`, etc.)
- `background_color` is **not** output by the editor (deprecated)
- `max_nodes` is **not** output when at default (`512`)
- `material` and `adjust_reference` are always output

## SceneDesc fields reference

### script (optional) — `string`

Path to the `.gui_script` file that controls this GUI's behavior.

**Omission rule**: Omit if no script is attached.

```protobuf
script: "/main/hud.gui_script"
```

### fonts (repeated) — `FontDesc`

Font resources available to text nodes. Each entry maps a `name` (used in nodes) to a `font` resource path.

```protobuf
fonts {
  name: "system_font"
  font: "/builtins/fonts/default.font"
}
fonts {
  name: "title"
  font: "/assets/fonts/title.font"
}
```

### textures (repeated) — `TextureDesc`

Texture/atlas resources available to box and pie nodes. Each entry maps a `name` to a `texture` resource path.

```protobuf
textures {
  name: "main"
  texture: "/main/main.atlas"
}
```

Node `texture` field references these as `"<texture_name>/<animation_name>"` (e.g., `"main/logo"`).

### background_color (optional) — `dmMath.Vector4`

**Deprecated**. The editor no longer outputs this field. Omit it.

### nodes (repeated) — `NodeDesc`

The GUI nodes. See the **NodeDesc** section below for full field reference.

### layers (repeated) — `LayerDesc`

Named layers that control draw order. Nodes assigned to a layer are drawn in layer order rather than list order. Layers reduce draw calls by grouping same-type nodes.

```protobuf
layers {
  name: "graphics"
}
layers {
  name: "text"
}
```

### material (optional) — `string`

Default material for the GUI. Default: `"/builtins/materials/gui.material"`.

```protobuf
material: "/builtins/materials/gui.material"
```

### layouts (repeated) — `LayoutDesc`

Alternative node configurations for different screen sizes/orientations. Each layout has a `name` and a list of `nodes` that override the default nodes.

```protobuf
layouts {
  name: "Landscape"
  nodes {
    position {
      x: 568.0
      y: 320.0
    }
    size {
      x: 200.0
      y: 100.0
    }
    type: TYPE_BOX
    id: "box"
  }
}
```

### adjust_reference (optional) — enum `AdjustReference`

Controls how each node's adjust mode is calculated. Default: `ADJUST_REFERENCE_LEGACY`.

| Value | Description |
|---|---|
| `ADJUST_REFERENCE_LEGACY` | Root-based (deprecated) |
| `ADJUST_REFERENCE_PARENT` | Per-node, adjusts against parent node or resized screen |
| `ADJUST_REFERENCE_DISABLED` | Turns off node adjust mode, all nodes keep set size |

```protobuf
adjust_reference: ADJUST_REFERENCE_PARENT
```

### max_nodes (optional) — `uint32`

Maximum number of nodes for this GUI. Default: `512`.

```protobuf
max_nodes: 512
```

### particlefxs (repeated) — `ParticleFXDesc`

Particle effect resources available to particlefx nodes.

```protobuf
particlefxs {
  name: "explosion"
  particlefx: "/effects/explosion.particlefx"
}
```

### resources (repeated) — `ResourceDesc`

Generic resources (e.g., buffers, custom data).

```protobuf
resources {
  name: "my_buffer"
  path: "/data/my_buffer.buffer"
}
```

### materials (repeated) — `MaterialDesc`

Additional materials available to individual nodes (besides the default scene material).

```protobuf
materials {
  name: "glow"
  material: "/materials/glow.material"
}
```

### max_dynamic_textures (optional) — `uint32`

Maximum number of textures that can be created using `gui.new_texture()`. Default: `128`.

### spine_scenes (repeated) — `SpineSceneDesc`

**Deprecated**. Spine scenes for spine nodes.

## NodeDesc fields reference

Each node is a `nodes { ... }` block inside `SceneDesc`. Nodes are rendered in list order.

### Common fields (all node types)

#### position (optional) — `dmMath.Vector4`

Position in pixels relative to parent (or scene origin). Components: `x`, `y`, `z`, `w`. Defaults: all `0.0`.

```protobuf
position {
  x: 320.0
  y: 568.0
}
```

#### rotation (optional) — `dmMath.Vector4`

Rotation as **Euler angles in degrees** (not quaternion). Components: `x`, `y`, `z`, `w`. Defaults: all `0.0`. Typically only `z` is used for 2D rotation.

```protobuf
rotation {
  z: 45.0
}
```

#### scale (optional) — `dmMath.Vector4One`

Scale factor. Components: `x`, `y`, `z`, `w`. Defaults: all `1.0`.

```protobuf
scale {
  x: 2.0
  y: 2.0
}
```

#### size (optional) — `dmMath.Vector4`

Node size in pixels. Components: `x`, `y`, `z`, `w`. Defaults: all `0.0`. Required for box and text nodes when `size_mode` is `SIZE_MODE_MANUAL`.

```protobuf
size {
  x: 200.0
  y: 100.0
}
```

#### color (optional) — `dmMath.Vector4One`

Tint color (RGBA). Components: `x` (R), `y` (G), `z` (B), `w` (A). Defaults: all `1.0` (white, fully opaque).

```protobuf
color {
  x: 1.0
  y: 0.0
  z: 0.0
}
```

#### type (optional) — enum `Type`

| Value | Description |
|---|---|
| `TYPE_BOX` | Rectangular node with color/texture |
| `TYPE_TEXT` | Text display node |
| `TYPE_PIE` | Circular/ellipsoid fill node |
| `TYPE_TEMPLATE` | Instance of another GUI scene |
| `TYPE_PARTICLEFX` | Particle effect node |
| `TYPE_CUSTOM` | Custom node type |

#### blend_mode (optional) — enum `BlendMode`

Default: `BLEND_MODE_ALPHA`.

| Value | Description |
|---|---|
| `BLEND_MODE_ALPHA` | Normal alpha blending |
| `BLEND_MODE_ADD` | Additive blending (linear dodge) |
| `BLEND_MODE_ADD_ALPHA` | Deprecated |
| `BLEND_MODE_MULT` | Multiply blending |
| `BLEND_MODE_SCREEN` | Screen blending |

#### id (optional) — `string`

Unique identifier within the GUI scene. Used by `gui.get_node()` in scripts.

```protobuf
id: "my_button"
```

#### xanchor (optional) — enum `XAnchor`

Default: `XANCHOR_NONE`.

| Value | Description |
|---|---|
| `XANCHOR_NONE` | Position relative to center of parent |
| `XANCHOR_LEFT` | Anchored to left edge |
| `XANCHOR_RIGHT` | Anchored to right edge |

#### yanchor (optional) — enum `YAnchor`

Default: `YANCHOR_NONE`.

| Value | Description |
|---|---|
| `YANCHOR_NONE` | Position relative to center of parent |
| `YANCHOR_TOP` | Anchored to top edge |
| `YANCHOR_BOTTOM` | Anchored to bottom edge |

#### pivot (optional) — enum `Pivot`

Default: `PIVOT_CENTER`. The origin/anchor point of the node. Rotation, scaling, and size changes happen around this point. For text nodes, also controls text alignment (West = left, Center = center, East = right).

| Value | Description |
|---|---|
| `PIVOT_CENTER` | Center |
| `PIVOT_N` | North (top center) |
| `PIVOT_NE` | North East |
| `PIVOT_E` | East (center right) |
| `PIVOT_SE` | South East |
| `PIVOT_S` | South (bottom center) |
| `PIVOT_SW` | South West |
| `PIVOT_W` | West (center left) |
| `PIVOT_NW` | North West |

#### adjust_mode (optional) — enum `AdjustMode`

Default: `ADJUST_MODE_FIT`. Controls how node content scales when scene boundaries are stretched.

| Value | Description |
|---|---|
| `ADJUST_MODE_FIT` | Scale to fit inside stretched bounds (preserves aspect) |
| `ADJUST_MODE_ZOOM` | Scale to cover stretched bounds (preserves aspect) |
| `ADJUST_MODE_STRETCH` | Stretch to fill bounds (breaks aspect) |

#### parent (optional) — `string`

ID of the parent node. Empty or omitted means the node is a root node.

```protobuf
parent: "panel"
```

#### layer (optional) — `string`

Layer assignment. Must match a layer name defined in `layers`. Unset layer inherits from parent; root nodes with no layer go to the implicit "null" layer (drawn first).

```protobuf
layer: "graphics"
```

#### inherit_alpha (optional) — `bool`

Default: `false`. When `true`, node alpha is multiplied with parent's alpha.

#### alpha (optional) — `float`

Default: `1.0`. Node translucency (0.0 = transparent, 1.0 = opaque). Animatable.

#### enabled (optional) — `bool`

Default: `true`. When `false`, node is not rendered, not animated, and not pickable.

#### visible (optional) — `bool`

Default: `true`. When `false`, node is not rendered but can still be animated and picked.

#### material (optional) — `string`

Per-node material override. References a material name from the `materials` list, or empty to use the scene default.

### Text node fields

These fields apply when `type: TYPE_TEXT`.

#### text (optional) — `string`

The display text.

```protobuf
text: "Score: 0"
```

#### font (optional) — `string`

Font name from the `fonts` list.

```protobuf
font: "system_font"
```

#### line_break (optional) — `bool`

Default: `false`. When `true`, text wraps at node width.

#### outline (optional) — `dmMath.Vector4WOne`

Outline color (RGBA). Defaults: x/y/z: `0.0`, w: `1.0`.

```protobuf
outline {
  x: 0.0
  y: 0.0
  z: 0.0
  w: 1.0
}
```

#### shadow (optional) — `dmMath.Vector4WOne`

Shadow color (RGBA). Same defaults as `outline`.

#### outline_alpha (optional) — `float`

Default: `1.0`. Outline translucency.

#### shadow_alpha (optional) — `float`

Default: `1.0`. Shadow translucency.

#### text_leading (optional) — `float`

Default: `1.0`. Line spacing multiplier. `0` = no spacing, `1` = normal.

#### text_tracking (optional) — `float`

Default: `0.0`. Letter spacing adjustment.

### Box node fields

These fields apply when `type: TYPE_BOX`.

#### texture (optional) — `string`

Reference to a texture/animation in format `"<texture_name>/<animation_id>"` or empty for a solid color box.

```protobuf
texture: "main/button"
```

#### slice9 (optional) — `dmMath.Vector4`

9-slice margins in pixels. Components: `x` (left), `y` (top), `z` (right), `w` (bottom). Defaults: all `0.0`. Preserves edge pixel size when node is resized.

```protobuf
slice9 {
  x: 12.0
  y: 12.0
  z: 12.0
  w: 12.0
}
```

#### size_mode (optional) — enum `SizeMode`

Default: `SIZE_MODE_MANUAL`.

| Value | Description |
|---|---|
| `SIZE_MODE_MANUAL` | Size set manually |
| `SIZE_MODE_AUTO` | Size determined automatically from texture |

#### clipping_mode (optional) — enum `ClippingMode`

Default: `CLIPPING_MODE_NONE`.

| Value | Description |
|---|---|
| `CLIPPING_MODE_NONE` | No clipping |
| `CLIPPING_MODE_STENCIL` | Node acts as stencil mask for children |

#### clipping_visible (optional) — `bool`

Default: `true`. Whether the clipping node itself is rendered.

#### clipping_inverted (optional) — `bool`

Default: `false`. Inverts the stencil mask.

### Pie node fields

These fields apply when `type: TYPE_PIE`. Pie nodes also support `texture`, `size_mode`, and clipping fields (same as box).

#### outerBounds (optional) — enum `PieBounds`

Default: `PIEBOUNDS_ELLIPSE`.

| Value | Description |
|---|---|
| `PIEBOUNDS_RECTANGLE` | Rectangular outer bounds |
| `PIEBOUNDS_ELLIPSE` | Elliptical outer bounds |

#### innerRadius (optional) — `float`

Default: `0`. Inner radius along X axis (creates a ring when > 0).

#### perimeterVertices (optional) — `int32`

Default: `32`. Number of segments building the shape.

#### pieFillAngle (optional) — `float`

Default: `360`. Fill angle in degrees (partial pie when < 360).

### Template node fields

These fields apply when `type: TYPE_TEMPLATE`.

#### template (optional) — `string`

Path to another `.gui` file used as template.

```protobuf
template: "/gui/button.gui"
```

#### template_node_child (optional) — `bool`

Internal flag — nodes that belong to a template instance have this set to `true`.

#### overridden_fields (repeated) — `uint32`

List of proto field numbers that are overridden from the template. Used internally by the editor to track which properties have been customized.

### ParticleFX node fields

These fields apply when `type: TYPE_PARTICLEFX`.

#### particlefx (optional) — `string`

Name from the `particlefxs` list.

```protobuf
particlefx: "explosion"
```

### Custom node fields

#### custom_type (optional) — `uint32`

Default: `0`. The custom type identifier.

## Templates (GUI composition)

GUI scenes can include other GUI scenes as **template nodes** (`TYPE_TEMPLATE`). This is the primary way to build reusable UI components (buttons, dialogs, HUD elements).

**Important caveat**: Only the **main GUI's** `.gui_script` executes. If a template GUI has its own `.gui_script` attached, that script is **ignored** — it does NOT run. All logic for template nodes must be handled by the parent GUI's script.

Template node children are accessible via `gui.get_node()` with the template node ID as prefix: `gui.get_node("template_id/child_node_id")`.

```protobuf
nodes {
  type: TYPE_TEMPLATE
  id: "play_button"
  template: "/gui/button.gui"
  inherit_alpha: true
}
```

To override properties of template child nodes, add them after the template node with `template_node_child: true` and the `overridden_fields` list indicating which fields are customized.

## GUI as a component in .go / .collection

A `.gui` file is referenced as a component in game objects, just like `.script` or `.sprite`:

In a `.go` file:
```protobuf
components {
  id: "example"
  component: "/main/example.gui"
}
```

In a `.collection` embedded instance data:
```protobuf
embedded_instances {
  id: "go"
  data: "components {\n"
  "  id: \"gui\"\n"
  "  component: \"/main/hud.gui\"\n"
  "}\n"
  ""
}
```

The game object's position has **no effect** on the GUI — GUI rendering is independent of the game view.

## Common templates

### Empty GUI (placeholder)

```protobuf
script: "/main/empty.gui_script"
material: "/builtins/materials/gui.material"
adjust_reference: ADJUST_REFERENCE_PARENT
```

### GUI with a centered text label

```protobuf
script: "/main/hud.gui_script"
fonts {
  name: "system_font"
  font: "/builtins/fonts/default.font"
}
nodes {
  position {
    x: 320.0
    y: 568.0
  }
  size {
    x: 400.0
    y: 40.0
  }
  type: TYPE_TEXT
  text: "Score: 0"
  font: "system_font"
  id: "score"
  pivot: PIVOT_N
  yanchor: YANCHOR_TOP
  inherit_alpha: true
}

material: "/builtins/materials/gui.material"
adjust_reference: ADJUST_REFERENCE_PARENT
```

### GUI with layered button (box + text)

```protobuf
script: "/main/menu.gui_script"
fonts {
  name: "system_font"
  font: "/builtins/fonts/default.font"
}
textures {
  name: "ui"
  texture: "/assets/ui.atlas"
}
nodes {
  position {
    x: 320.0
    y: 400.0
  }
  size {
    x: 200.0
    y: 60.0
  }
  type: TYPE_BOX
  texture: "ui/button"
  id: "btn_play"
  layer: "graphics"
  inherit_alpha: true
}

nodes {
  position {
    x: 320.0
    y: 400.0
  }
  size {
    x: 200.0
    y: 60.0
  }
  type: TYPE_TEXT
  text: "PLAY"
  font: "system_font"
  id: "btn_play_text"
  parent: "btn_play"
  layer: "text"
  inherit_alpha: true
}

layers {
  name: "graphics"
}
layers {
  name: "text"
}
material: "/builtins/materials/gui.material"
adjust_reference: ADJUST_REFERENCE_PARENT
```

### Box node with 9-slice

```protobuf
nodes {
  position {
    x: 320.0
    y: 300.0
  }
  size {
    x: 400.0
    y: 200.0
  }
  type: TYPE_BOX
  texture: "ui/panel"
  id: "panel"
  slice9 {
    x: 16.0
    y: 16.0
    z: 16.0
    w: 16.0
  }
  inherit_alpha: true
}
```

### Stencil clipping (mask + content)

```protobuf
nodes {
  position {
    x: 320.0
    y: 400.0
  }
  size {
    x: 200.0
    y: 200.0
  }
  type: TYPE_BOX
  texture: "ui/circle_mask"
  id: "mask"
  clipping_mode: CLIPPING_MODE_STENCIL
  clipping_visible: true
  inherit_alpha: true
}

nodes {
  position {
    x: 320.0
    y: 400.0
  }
  size {
    x: 300.0
    y: 300.0
  }
  type: TYPE_BOX
  texture: "ui/photo"
  id: "content"
  parent: "mask"
  inherit_alpha: true
}
```

### Pie node (health ring)

```protobuf
nodes {
  position {
    x: 80.0
    y: 80.0
  }
  size {
    x: 100.0
    y: 100.0
  }
  color {
    x: 0.0
    y: 1.0
    z: 0.0
  }
  type: TYPE_PIE
  id: "health_ring"
  outerBounds: PIEBOUNDS_ELLIPSE
  innerRadius: 40.0
  perimeterVertices: 64
  pieFillAngle: 270.0
  inherit_alpha: true
}
```

## Layout, anchoring, and adjust mode

**Pivot + Anchor interaction**: The pivot sets the node's origin point. When an anchor is active, the pivot edge stays at a fixed percentage from the corresponding screen/parent edge. For edge-aligned elements, set the pivot to the same side as the anchor (e.g., `PIVOT_W` + `XANCHOR_LEFT`).

**Adjust mode**: Controls how node content scales when the scene is stretched to fit the screen:
- `ADJUST_MODE_FIT` — content fits inside bounds (may leave empty space)
- `ADJUST_MODE_ZOOM` — content covers bounds (may crop)
- `ADJUST_MODE_STRETCH` — content fills bounds (may distort)

**Layers and draw calls**: Nodes are batched into draw calls when they share the same type, atlas, blend mode, and font. Layers let you group same-type nodes to minimize draw calls. Without layers, alternating node types in the hierarchy breaks batching.

## Protobuf Text Format rules

1. **Default omission**: Omit fields that equal their proto default.
2. **Message blocks**: Use `field_name { ... }` with nested `key: value` pairs.
3. **Floats**: Always include decimal point: `1.0`, not `1`.
4. **Integers**: No decimal point: `32`, not `32.0`.
5. **Strings**: Always double-quoted.
6. **Enums**: Use the constant name without quotes.
7. **Booleans**: `true` or `false`, no quotes.
8. **Repeated fields**: Each entry gets its own `field_name { ... }` block.
9. **Field order**: Follow the proto field number order.
10. **No trailing commas or semicolons**.
11. **Indentation**: 2 spaces per nesting level inside message blocks.
12. **Newlines**: One empty line between the end of a message block `}` and the next field. No empty line between consecutive scalar fields.
13. **Vector blocks**: Only include components that differ from defaults. Omit the block entirely if all components are at defaults (for optional fields).

## Best practices

- **Font selection priority.** When choosing a font for GUI text nodes:
  1. Use the font specified by the user in the current request, if provided.
  2. Search the project for existing `.font` files (e.g., `assets/fonts/`). If found, use the project font. Pick the most appropriate one if there are several (e.g., prefer distance field fonts).
  3. Fall back to `/builtins/fonts/default.font` only if no project fonts exist and the user did not specify one.
- **Use distance field fonts with base size 50.** The `.font` resource should use `size: 50` and `output_format: TYPE_DISTANCE_FIELD`. This provides crisp text at any visual size.
- **Control text size via node scale, not font size.** Do not create separate `.font` files for different text sizes. Instead, set the `scale` of the text node. For example, to display text at visual size 25, keep `size: 50` in the `.font` and set the node's scale to `0.5`.

## Workflow

### Creating a new GUI

1. Determine the file path (must end with `.gui`).
2. Add `fonts` entries for any fonts needed by text nodes.
3. Add `textures` entries for any atlases needed by box/pie nodes.
4. Add `layers` if you need draw order optimization.
5. Add `materials` if nodes need non-default materials.
6. Add `particlefxs` if using particle effect nodes.
7. Add `nodes` — each node needs at minimum: `type`, `id`, and type-specific fields (`text`+`font` for text, `texture` for textured box, etc.).
8. Set `parent` field on child nodes to establish hierarchy.
9. Set `script` to the `.gui_script` path.
10. Set `adjust_reference` (prefer `ADJUST_REFERENCE_PARENT`).
11. Set `material` (usually keep the default).

### Editing an existing GUI

1. Read the current `.gui` file.
2. When adding nodes, place them after existing nodes.
3. When adding dependencies (fonts, textures, etc.), check if the dependency already exists before adding a duplicate.
4. Preserve existing node IDs — they may be referenced in `.gui_script`.
5. When modifying template nodes, only override the fields you need to change.
