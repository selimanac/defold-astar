# .editor_script Reference

Editor script — Lua file that extends the Defold editor with custom menu commands, build lifecycle hooks, language servers, HTTP server routes, and preferences.

## Available APIs

`editor`, `editor.ui`, `editor.prefs`, `editor.tx`, `http` (editor version), `json`, `localization`, `tilemap.tiles`, `zip`, `zlib`, `io`, `os` (restricted), `string`, `table`, `math`, `coroutine`, `print`, `pprint`

**Not available**: all game engine namespaces (`go`, `gui`, `render`, `vmath`, `msg`, `timer`, `sprite`, `label`, `factory`, `collectionfactory`, `collectionproxy`, `physics`, `sound`, `particlefx`, `tilemap` engine API, `model`, `camera`, `buffer`, `resource`, `window`, `graphics`, `hash`, `crash`, `profiler`)

For `editor` namespace API details use `defold-api-fetch`. For editor scripting concepts and UI components use `defold-docs-fetch` (editor-scripts and editor-scripts-ui manuals).

## Key constraints

- Runs in the **editor**, not the game runtime
- Uses **Lua 5.2** (via luaj JVM runtime), not LuaJIT
- **No game engine APIs** — `go`, `gui`, `render`, `vmath`, `msg`, `timer`, etc. are not available
- **No lifecycle callbacks** (`init`, `update`, `on_message`, etc.) — uses module functions instead
- **No `go.property()`** or `self` table — no instance concept
- Only `/hooks.editor_script` at the project root receives lifecycle events
- All editor scripts reload via **Project → Reload Editor Scripts**

### Lua 5.2 runtime restrictions

- No `debug` package
- No `os.execute` — use `editor.execute()` instead
- No `os.tmpname`, `io.tmpfile` — scripts can only access files inside the project directory
- No `os.rename`, `os.exit`, `os.setlocale`

## Module structure

Every `.editor_script` **must return a module table**:

```lua
local M = {}

function M.get_commands()
	-- return array of command definitions
end

function M.get_language_servers()
	-- return array of language server definitions
end

function M.get_prefs_schema()
	-- return table of preference schemas
end

function M.get_http_server_routes()
	-- return array of HTTP route definitions
end

return M
```

All module functions are optional.

## Node properties (`editor.get` / `editor.tx.set`)

Use `editor.get(node, property)` to read and `editor.tx.set(node, property, value)` to write properties. Always check availability with `editor.can_get()` / `editor.can_set()` first.

### Property naming convention

Outline properties use **snake_case** names matching the proto field names (not hyphenated). To verify a property's editor script name, hover over its label in the Properties panel — the tooltip shows the exact name.

Examples: `"default_animation"`, `"blend_mode"`, `"size_mode"`, `"playback_rate"`, `"material"`.

**Sprite texture samplers** use a special naming pattern: `"__sampler__<sampler_name>__<index>"`. For the default sprite material, the atlas property is `"__sampler__texture_sampler__0"`. For multi-texture sprites, subsequent samplers use `__1`, `__2`, etc.

### Common properties

- `"path"` — resource path for file-backed resources (e.g. `"/main/game.script"`)
- `"text"` — text content of text-editable resources (scripts, JSON); reflects unsaved edits
- `"children"` — child resource paths for directory resources

### Component-type-specific list properties

- **Atlas**: `"images"` (list of image nodes), `"animations"` (list of animation nodes)
- **Atlas animation**: `"images"` (list of image nodes), `"id"` (animation name)
- **Atlas image**: `"image"` (resource path to PNG)
- **Tilemap**: `"layers"` (list of layer nodes)
- **Tilemap layer**: `"tiles"` (2D grid, see `tilemap.tiles.*`)
- **ParticleFX**: `"emitters"`, `"modifiers"`
- **ParticleFX emitter**: `"modifiers"`
- **Collision object**: `"shapes"` (list of shape nodes)
- **GUI**: `"layers"`, `"materials"`, `"fonts"`, `"textures"`, `"particlefxs"`, `"nodes"`
- **Game object** (`.go`): `"components"` (list of component nodes)
- **Collection**: `"children"` (list of game object / collection nodes)

### Outline property types supported

`strings`, `booleans`, `numbers`, `vec2`/`vec3`/`vec4`, `resources`, `curves`

Set a resource property to `nil` by passing `""`.

## Commands

Define custom menu items via `get_commands()`. Each command table:

- `label` (required) — text shown in the menu
- `locations` (required) — array of: `"Edit"`, `"View"`, `"Project"`, `"Debug"`, `"Assets"`, `"Bundle"`, `"Scene"`, `"Outline"`
- `query` (optional) — `{selection = {type = "resource"|"outline"|"scene", cardinality = "one"|"many"}}` or `{argument = ...}` (Bundle only)
- `id` (optional) — identifier for shortcut assignment (e.g., `"my-ext.do-stuff"`)
- `active` (optional) — function returning boolean. Runs in **immediate** mode — must be fast
- `run` (optional) — function executed when selected. Runs in **long-running** mode

```lua
function M.get_commands()
	return {
		{
			label = "Remove Comments",
			locations = {"Edit", "Assets"},
			query = {
				selection = {type = "resource", cardinality = "one"}
			},
			active = function(opts)
				local path = editor.get(opts.selection, "path")
				return path:match("%.lua$") ~= nil or path:match("%.script$") ~= nil
			end,
			run = function(opts)
				local text = editor.get(opts.selection, "text")
				local new_text = text:gsub("%-%-[^\n]*", "")
				editor.transact({
					editor.tx.set(opts.selection, "text", new_text)
				})
			end
		}
	}
end
```

Alternatively, use `editor.command({...})` at the top level of the module.

## Execution modes

**Immediate mode** — `active` callbacks and top-level script code. Must be fast.

**Long-running mode** — `run` callbacks and lifecycle hooks. Can take time.

Long-running-only functions (error if called in immediate mode):
- `editor.create_directory()`, `editor.create_resources()`, `editor.delete_directory()`
- `editor.save()`, `os.remove()`, `file:write()`
- `editor.execute()`
- `editor.transact()`

## Lifecycle hooks

**Only** `/hooks.editor_script` (at the project root, next to `game.project`) receives lifecycle events. Other editor scripts do NOT receive them. Hooks are editor-only — NOT executed by Bob.

```lua
local M = {}

function M.on_build_started(opts)
	-- opts.platform — e.g., "x86_64-win32"
	-- Raising error aborts the build
end

function M.on_build_finished(opts)
	-- opts.platform, opts.success (boolean)
end

function M.on_bundle_started(opts)
	-- opts.output_directory, opts.platform, opts.variant ("debug"|"release"|"headless")
	-- Raising error aborts the bundle
end

function M.on_bundle_finished(opts)
	-- opts.output_directory, opts.platform, opts.variant, opts.success
end

function M.on_target_launched(opts)
	-- opts.url — e.g., "http://127.0.0.1:35405"
end

function M.on_target_terminated(opts)
	-- opts.url
end

return M
```

## Language servers

Define LSP-compatible language servers via `get_language_servers()`:

```lua
function M.get_language_servers()
	local command = "build/plugins/my-ext/plugins/bin/" .. editor.platform .. "/lua-lsp"
	if editor.platform == "x86_64-win32" then
		command = command .. ".exe"
	end
	return {
		{
			languages = {"lua"},
			watched_files = {
				{pattern = "**/.luacheckrc"}
			},
			command = {command, "--stdio"}
		}
	}
end
```

- `languages` (required) — array of language identifiers
- `command` (required) — array of command and arguments
- `watched_files` (optional) — array of `{pattern = "glob"}`

## Editor scripts in libraries

Editor scripts in libraries are automatically picked up. Lifecycle hooks cannot be in libraries — they must be in `/hooks.editor_script`. Library hooks should be exposed as Lua functions for the user to `require` in their own `/hooks.editor_script`.

