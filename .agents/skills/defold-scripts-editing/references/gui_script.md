# .gui_script Reference

GUI script — Lua file attached to a `.gui` component via its `script` field. Controls UI behavior, node manipulation, and input handling.

## Available APIs

`gui`, `msg`, `vmath`, `sys`, `http`, `socket`, `json`, `zlib`, `html5`, `image`, `timer`, `sound`, `resource`, `camera` + Lua standard library + native extension globals

**Not available**: `go`, `render`, `sprite`, `label`, `tilemap`, `factory`, `collectionfactory`, `collectionproxy`, `model`, `particlefx`, `buffer`, `graphics`, `physics`

For `gui` namespace API details use `defold-api-fetch`.

## Key constraints (differences from .script)

- Has access to `gui` namespace, but **not** `go` or `render`
- **No `go.property()`** — gui scripts cannot declare exposed properties
- **No `fixed_update()` or `late_update()`** callbacks
- **No raw gamepad input** — `on_input` does not receive raw gamepad data (mapped button actions still work)
- **One script per GUI** — each `.gui` component has exactly one `.gui_script` (or none)
- **Template scripts are ignored** — if a GUI is included as a template in another GUI, only the parent GUI's script runs
- Prefer reactive/message-driven logic over `update()` polling
- Cache node references in `init()` — calling `gui.get_node()` every frame is wasteful

## Template node access

For nodes inside a template, prefix with the template node ID:

```lua
local btn_bg = gui.get_node("play_button/background")
local btn_text = gui.get_node("play_button/label")
```

## Addressing the GUI component

From game object scripts:

```lua
msg.post("hud_go#gui", "set_score", { score = 100 })
```

From the GUI script, address game objects:

```lua
msg.post("/player#script", "heal", { amount = 10 })
```

