---
name: defold-scripts-editing
description: "Creates and edits Defold Lua script files (.script, .gui_script, .render_script, .editor_script) and plain Lua modules (.lua). Use when asked to create, modify, or configure any Defold script or Lua module."
---

# Editing Defold Script Files and Lua Modules

Defold has four Lua script types (each running in a different context with different APIs) and plain `.lua` modules for reusable logic.

**For API details** use `defold-api-fetch` skill. **For conceptual manuals** use `defold-docs-fetch` skill. This skill covers script structure, constraints, and templates.

## Lua modules (.lua)

Plain `.lua` files are Lua modules used to encapsulate reusable logic. **Extract frequently used universal logic into `.lua` modules** — avoid duplication and keep scripts lean. Modules are required via `require("path.to.module")` (dots as path separators).

Defold projects typically use **Lua shared state** (`shared_state` in `game.project`). When enabled, all scripts, GUI scripts, and the render script run in the **same Lua context**. A Lua module required from any script has the **same context and state** within that single Lua interpreter instance — module-level locals and `package.loaded` are shared across all users of the module. Stateful modules behave like singletons.

## Lua module structure (.lua)

Encapsulate data and functions in a local table, return it:

```lua
local M = {}

function M.hello()
    print("Hello")
end

return M
```

Avoid globals in modules. For stateful modules, internal state is shared between all callers (singleton-like). For stateless logic, pass state explicitly or use constructors that return new state tables. See [Modules manual](https://defold.com/llms/manuals/modules.md).

## Script types

| Extension | Context | Lua runtime | `go.property()` | Key namespaces |
|---|---|---|---|---|
| `.script` | Game object component | LuaJIT | Yes | `go`, `msg`, `vmath`, `timer`, `sys`, component namespaces (`sprite`, `label`, `factory`, `collectionfactory`, `collectionproxy`, `tilemap`, `model`, `sound`, `particlefx`, `buffer`, `resource`, `physics`) |
| `.gui_script` | GUI component | LuaJIT | No | `gui`, `msg`, `vmath`, `sys` |
| `.render_script` | Render pipeline (one per project) | LuaJIT | No | `render`, `graphics`, `camera`, `msg`, `vmath`, `sys` |
| `.editor_script` | Defold editor | Lua 5.2 (luaj) | No | `editor`, `editor.ui`, `editor.prefs`, `editor.tx`, `http`, `json`, `zip`, `zlib` |

## File format

All script types and `.lua` modules are plain Lua files (not Protobuf Text Format). Scripts use specific extensions (`.script`, `.gui_script`, etc.); modules use `.lua`.

## Runtime environment notes (.script, .gui_script, .render_script)

**Platform-specific**: `html5` module is only available on the **HTML5** platform.

**App Manifest exclusions**: Some built-in modules (`liveupdate`, `image`, `types`, `profiler`, `sound`, `physics`, etc.) can be excluded via the App Manifest to reduce binary size. If excluded, their APIs will not be available at runtime.

**No `utf8` module**: For working with non-ASCII strings, use the defold-utf8 dependency (`https://github.com/d954mas/defold-utf8/archive/refs/heads/master.zip`). API reference: [utf8.script_api](https://github.com/d954mas/defold-utf8/blob/master/utf8/api/utf8.script_api).

## Common runtime script patterns (.script, .gui_script, .render_script)

All runtime lifecycle callbacks receive `self` as the first parameter — a userdata that acts like a table for storing instance state.

### Script structure

1. **Module requires** (optional) — `local M = require("module")`
2. **Local helper functions** (optional) — must be at module scope, never inside other functions
3. **Declarations** (`.script` only: `go.property()` at top level)
4. **Lifecycle callbacks**

### Lifecycle callback availability

| Callback | `.script` | `.gui_script` | `.render_script` |
|---|---|---|---|
| `init(self)` | ✓ | ✓ | ✓ |
| `final(self)` | ✓ | ✓ | — |
| `fixed_update(self, dt)` | ✓ | — | — |
| `update(self, dt)` / `update(self)` | ✓ (dt) | ✓ (dt) | ✓ (no dt) |
| `late_update(self, dt)` | ✓ | — | — |
| `on_message(self, message_id, message, sender)` | ✓ | ✓ | ✓ (no sender) |
| `on_input(self, action_id, action)` | ✓ | ✓ | — |
| `on_reload(self)` | ✓ | ✓ | — |

### Key rules

- Store per-instance state in `self`, not in module-level locals (module-level locals are shared across all instances).
- Omit unused callbacks — especially `update()` and `fixed_update()` which cost a call per frame even if empty.
- Keep local helper functions at module scope, never inside other functions.

## Native extensions

Native extensions (C/C++/ObjC/Java) register their Lua functions into the global scope via the Lua C API (`lua_register`, `luaL_openlib`, etc.). This means extensions typically add a new global table (e.g., `myext`) with functions and constants accessible from any `.script` or `.gui_script`.

Extensions describe their API for editor auto-complete via `*.script_api` files (YAML format) located in their `api/` directory. The format:

```yaml
- name: extension_name
  type: table
  desc: Extension description
  members:
  - name: function_name
    type: function
    desc: Function description
    parameters:
    - name: param_name
      type: string
      desc: Parameter description
    returns:
    - name: return_name
      type: number
      desc: Return value description
```

Types: `table`, `string`, `boolean`, `number`, `function`. Multiple types: `[type1, type2]`.

## Lua preprocessing

Defold supports conditional compilation via the [Lua preprocessor extension](https://github.com/defold/extension-lua-preprocessor) (applies to all Lua files including `.lua` modules):

```lua
--#IF DEBUG
local lives = 999
--#ELSE
local lives = 3
--#ENDIF
```

Keywords: `RELEASE`, `DEBUG`, `HEADLESS`.

## Type-specific references

Consult the `references/` directory for constraints, templates, and patterns specific to each script type:

- `references/script.md` — `go.property()` reference (types, constraints, overrides), templates
- `references/gui_script.md` — GUI script constraints, template node access, templates
- `references/render_script.md` — render pipeline architecture, system messages, full working templates
- `references/editor_script.md` — module structure, commands, lifecycle hooks, execution modes, templates

## Workflow

### Creating a new Lua module (.lua)

1. Use when logic is reusable across multiple scripts or screens.
2. Create a local table, add functions, return it.
3. Keep modules stateless when possible; if stateful, document that state is shared.
4. Require with dot notation: `require("main.utils")`, `require("screens.game.helpers")`.

### Creating a new script

1. Determine the correct extension for the script type.
2. Read the corresponding type-specific reference in `references/` for constraints and templates.
3. Follow the structure pattern: requires → helpers → declarations → callbacks.
4. Only add lifecycle callbacks you actually need.

### Editing an existing script

1. Read the current file.
2. Preserve existing declarations (e.g., `go.property()` names) and callback signatures.
3. Add/modify callbacks as needed.
4. Changing a `go.property()` name or type may break overrides in `.go` and `.collection` files.
