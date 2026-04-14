---
name: monarch-screen-setup
description: "Organizes screens and popups in a Defold game using Monarch screen manager. Use when creating new screens, popups, or setting up navigation between them."
---

# Organizing Screens and Popups with Monarch

## Prerequisite: Verify Monarch Dependency

Before applying any guidance from this skill, you MUST confirm that the project actually uses Monarch. Check the `game.project` file in the project root for a dependency URL containing `britzl/monarch` (e.g. `dependencies#N = https://github.com/britzl/monarch/archive/...`). Alternatively, check for the presence of a `monarch/monarch.lua` file in the project tree.

If neither a Monarch dependency in `game.project` nor a local `monarch/monarch.lua` module is found, **do NOT apply this skill**. Inform the user that the project does not appear to use Monarch and suggest adding the dependency to `game.project` if they want to use it:
```
[project]
dependencies#N = https://github.com/britzl/monarch/archive/refs/tags/5.2.0.zip
```

---

This skill describes the conventions for creating and managing screens (via collectionproxy) and popups (via collectionfactory) in a Defold project using the Monarch library.

## Naming Rules

- All screen and popup names use `snake_case`.
- Screen and popup names MUST be unique across the entire project because Monarch registers them in a single shared registry (`screens` table in `monarch.lua`). Duplicate ids cause an assertion error.
- Recommended pattern: prefix popup names distinctly (e.g. `settings_popup`, `reward_popup`) so they never collide with screen names (e.g. `main_menu`, `gameplay`).

## Directory Layout

```
project/
├── main/
│   ├── main.collection          -- bootstrap collection (set in game.project [bootstrap])
│   └── main.script              -- main controller script: waits for registration, shows first screen
├── screens/
│   └── <screen_name>/
│       ├── <screen_name>.collection   -- IMPORTANT: collection name MUST be unique (see below)
│       ├── <screen_name>.gui
│       └── <screen_name>.gui_script
├── popups/
│   └── <popup_name>/
│       ├── <popup_name>.collection
│       ├── <popup_name>.gui
│       └── <popup_name>.gui_script
```

## Creating a Screen (collectionproxy)

A screen is a full-screen view loaded via `collectionproxy`. Only ONE screen should be active at a time. When switching screens, always use `{ clear = true }` to avoid stacking multiple screens.

### Required files for a screen (e.g. `gameplay`)

**1. `screens/gameplay/gameplay.collection`**
- The collection `name` property MUST be unique across the project (Defold uses it for URL addressing within the loaded world).
- Contains a game object with a camera component.

```protobuf
name: "gameplay"
scale_along_z: 0
embedded_instances {
  id: "camera"
  data: "embedded_components {\n"
  "  id: \"camera\"\n"
  "  type: \"camera\"\n"
  "  data: \"aspect_ratio: 1.0\\n"
  "fov: 0.7854\\n"
  "near_z: -1.0\\n"
  "far_z: 1.0\\n"
  "orthographic_projection: 1\\n"
  "orthographic_mode: ORTHO_MODE_AUTO_COVER\\n"
  "\"\n"
  "}\n"
  ""
}
```

### Registering a screen in `main.collection`

In `main.collection`, for each screen create a game object (e.g. id `gameplay`) with:

1. A **component** `screen_proxy` referencing `/monarch/screen_proxy.script` with properties:
   - `screen_id` = `gameplay` (hash, must match the id you use in `monarch.show()`)
   - `popup` = `false`
   - `popup_on_popup` = `false`
2. An **embedded component** `collectionproxy` of type `collectionproxy` pointing to `/screens/gameplay/gameplay.collection`.

The `screen_proxy.script` will call `monarch.register_proxy()` in its `init()`, registering the screen automatically.

### Showing a screen (single-screen stack pattern)

ALWAYS use `{ clear = true }` when showing a screen so the stack is cleared down to any existing instance and then replaced. This ensures only 1 screen is ever in the stack:

```lua
monarch.show("gameplay", { clear = true })
```

Do NOT call `monarch.show("gameplay")` without `clear = true` for screens -- that would push onto the stack and create confusing multi-screen stacks.

To navigate between screens:
```lua
-- from main_menu to gameplay:
monarch.show("gameplay", { clear = true })

-- from gameplay to main_menu:
monarch.show("main_menu", { clear = true })
```

Do NOT use `monarch.back()` for screen-to-screen navigation. `back()` is for closing popups.

## Creating a Popup (collectionfactory)

A popup is an overlay that pauses/dims screens beneath it. Popups are created via `collectionfactory` so multiple popup instances can coexist (popup on popup).

### Required files for a popup (e.g. `settings_popup`)

**1. `popups/settings_popup/settings_popup.collection`**
- Contains a game object with a **referenced** GUI component.

```protobuf
name: "settings_popup"
scale_along_z: 0
embedded_instances {
  id: "go"
  data: "components {\n"
  "  id: \"gui\"\n"
  "  component: \"/popups/settings_popup/settings_popup.gui\"\n"
  "}\n"
  ""
}
```

**2. `popups/settings_popup/settings_popup.gui`**
- Standard Defold GUI file with the popup's UI nodes.

**3. `popups/settings_popup/settings_popup.gui_script`**
- To close itself, call `monarch.back()`.

```lua
local monarch = require("monarch.monarch")

function init(self)
end

function final(self)
end

function on_message(self, message_id, message, sender)
end

function on_input(self, action_id, action)
    if action_id == hash("touch") and action.pressed then
        -- example: close popup on a button press
        -- (check node picking for your close button here)
        monarch.back()
    end
end
```

### Registering a popup in `main.collection`

In `main.collection`, for each popup create a game object (e.g. id `settings_popup`) with:

1. A **component** `screen_factory` referencing `/monarch/screen_factory.script` with properties:
   - `screen_id` = `settings_popup` (hash)
   - `popup` = `true`
   - `popup_on_popup` = `true` (set to `true` if this popup can appear on top of another popup)
   - `screen_factory` = URL to the collectionfactory component (see step 2)
2. An **embedded component** `collectionfactory` of type `collectionfactory` pointing to `/popups/settings_popup/settings_popup.collection`.

The `screen_factory.script` will call `monarch.register_factory()` in its `init()`, registering the popup automatically.

### Showing and closing a popup

```lua
-- Open a popup (it stacks on top of the current screen):
monarch.show("settings_popup")

-- Open a popup on top of another popup (requires popup_on_popup = true):
monarch.show("confirm_popup")

-- Close the topmost popup:
monarch.back()
```

### Popup timestep behavior

When a popup is shown on top of a screen, Monarch automatically sets `timestep_below_popup` on the underlying screen's proxy. The default value is `1` (normal speed). Set it to `0` in the screen's `screen_proxy.script` properties to pause the screen beneath a popup:
- `timestep_below_popup` = `0`

## Waiting for All Screens to Register Before Starting

Do NOT call `monarch.show()` directly in `init()` of `main.script` -- at that point other scripts' `init()` has not yet run and screens are not registered. This causes an assertion error.

The solution: post a message to self from `init()`. By the time the message is processed, all `init()` functions in the collection have completed and all screens/popups are registered.

### Example: `main/main.script`

```lua
local monarch = require("monarch.monarch")

function init(self)
    msg.post(".", "acquire_input_focus")
    msg.post("#", "start")
end

function on_message(self, message_id, message, sender)
    if message_id == hash("start") then
        monarch.show("main_menu", { clear = true })
    end
end
```

## Quick Reference

| Action | Code |
|---|---|
| Show a screen (clear stack) | `monarch.show("screen_name", { clear = true })` |
| Show a popup | `monarch.show("popup_name")` |
| Close topmost popup | `monarch.back()` |
| Check screen registered | `monarch.screen_exists("screen_name")` |
| Check if busy (transitioning) | `monarch.is_busy()` |
| Get screen data | `monarch.data("screen_name")` |
| Pass data to screen | `monarch.show("screen_name", { clear = true }, { key = "value" })` |
| Pass data to popup | `monarch.show("popup_name", nil, { key = "value" })` |
| Check if popup | `monarch.is_popup("popup_name")` |
| Get current top screen | `monarch.top()` |
