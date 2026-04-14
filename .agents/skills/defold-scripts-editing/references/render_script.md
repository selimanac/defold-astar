# .render_script Reference

Render script — Lua file that controls the rendering pipeline: what is drawn, when, and where. One active render script per project.

## Available APIs

`render`, `graphics`, `camera`, `msg`, `vmath`, `sys`, `hash`, `pprint`, `print`, `socket`, `json`, `zlib`, `http`, `html5`, `image`, `window`, `timer` + Lua standard library

**Not available**: `go`, `gui`, `sprite`, `label`, `tilemap`, `factory`, `collectionfactory`, `collectionproxy`, `model`, `particlefx`, `sound`, `physics`, `buffer`, `resource`, `crash`, `profiler`

For `render`, `graphics`, and `camera` namespace API details use `defold-api-fetch`. For rendering concepts use `defold-docs-fetch` (render manual).

## Key constraints

- Has access to `render`, `graphics`, `camera` namespaces, but **not** `go`, `gui`, or component-specific namespaces
- **No `go.property()`**
- **Only three callbacks**: `init()`, `update()`, `on_message()` — no `final`, `fixed_update`, `late_update`, `on_input`, `on_reload`
- **`update()` runs at the end of the frame** — after all game object updates and transforms are complete
- **`on_message` has no `sender` parameter** — only `(self, message_id, message)`
- Receives messages via the **`@render:` socket** (e.g., `msg.post("@render:", "clear_color", {...})`)

## Render file connection

A render script must be referenced by a `.render` file:

```
script: "/render/my.render_script"
render_resources {
  name: "my_material"
  path: "/materials/my.material"
}
```

The `.render` file is set in `game.project`:

```ini
[bootstrap]
render = /render/my.render
```

