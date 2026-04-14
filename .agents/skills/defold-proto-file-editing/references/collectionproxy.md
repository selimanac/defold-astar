
# Editing Collection Proxy Files

Defold collection proxy component (`.collectionproxy`) — dynamically loads and unloads separate game worlds from collection files.

## Overview

A collection proxy loads the contents of a `.collection` file into a **separate game world** with its own physics simulation. Unlike collection factories (which spawn objects into the current world), proxies create isolated worlds accessed through a named socket.

**Use cases:**
- Level switching
- GUI screens / popup overlays
- Loading/unloading narrative scenes
- Mini-games within a game

**Key behaviors:**
- Each loaded collection creates a separate physics world — no cross-world physics interactions
- The collection's `Name` property becomes the socket name for addressing (`"mylevel:/object"`)
- Socket names must be unique — loading two collections with the same name causes an error
- Input propagates through the proxy only if the parent game object has acquired input focus

## File format

Collection proxy files use **Protobuf Text Format** based on the `CollectionProxyDesc` message from `gamesys/gamesys_ddf.proto`.

## Canonical example

Minimal collection proxy (most common):
```protobuf
collection: "/levels/level1.collection"
```

With exclude for Live Update:
```protobuf
collection: "/levels/level1.collection"
exclude: true
```

## Fields reference

### collection
- **Required**: Yes
- **Type**: string (resource path)
- **Description**: Path to the `.collection` file to load as a separate game world. Must be an absolute project path starting with `/`.
- **Omission rule**: Cannot be omitted (required field).

```protobuf
collection: "/levels/level1.collection"
```

### exclude
- **Required**: No
- **Type**: bool
- **Default**: `false`
- **Description**: When `true`, excludes the collection's content from the game bundle. The content must then be downloaded separately using the Live Update feature. Used for on-demand content delivery.
- **Omission rule**: Omit when `false`.

```protobuf
collection: "/levels/level1.collection"
exclude: true
```

## Common templates

### Standard level proxy
```protobuf
collection: "/levels/level1.collection"
```

### Live Update proxy (excluded content)
```protobuf
collection: "/levels/bonus_level.collection"
exclude: true
```

## Embedded collection proxy in game object

Collection proxies are typically embedded in `.go` or `.collection` files:

Minimal embedded:
```protobuf
embedded_components {
  id: "levelproxy"
  type: "collectionproxy"
  data: "collection: \"/levels/level1.collection\"\n"
  ""
}
```

With exclude:
```protobuf
embedded_components {
  id: "menuproxy"
  type: "collectionproxy"
  data: "collection: \"/example/menu.collection\"\n"
  "exclude: false\n"
  ""
  position {
    x: 0.0
    y: 0.0
    z: 0.0
  }
  rotation {
    x: 0.0
    y: 0.0
    z: 0.0
    w: 1.0
  }
}
```

Multiple proxies on same game object (level switcher pattern):
```protobuf
embedded_components {
  id: "menuproxy"
  type: "collectionproxy"
  data: "collection: \"/example/menu.collection\"\n"
  ""
}
embedded_components {
  id: "level1proxy"
  type: "collectionproxy"
  data: "collection: \"/example/level1.collection\"\n"
  ""
}
embedded_components {
  id: "level2proxy"
  type: "collectionproxy"
  data: "collection: \"/example/level2.collection\"\n"
  ""
}
```

## Runtime API

### Loading lifecycle

```lua
-- Step 1: Load the collection (async)
msg.post("#levelproxy", "async_load")
-- or synchronous (blocks until loaded):
msg.post("#levelproxy", "load")

-- Step 2: Handle proxy_loaded callback
function on_message(self, message_id, message, sender)
    if message_id == hash("proxy_loaded") then
        -- Step 3: Initialize and enable the world
        msg.post(sender, "init")
        msg.post(sender, "enable")
    end
end
```

### Unloading lifecycle

```lua
-- Full explicit unload
msg.post("#levelproxy", "disable")
msg.post("#levelproxy", "final")
msg.post("#levelproxy", "unload")

-- Or simplified — unload auto-disables and finalizes
msg.post("#levelproxy", "unload")

-- Handle unload completion
function on_message(self, message_id, message, sender)
    if message_id == hash("proxy_unloaded") then
        -- World fully unloaded
    end
end
```

### Level switching pattern

```lua
local function show(self, proxy)
    if self.current_proxy then
        msg.post(self.current_proxy, "unload")
        self.current_proxy = nil
    end
    msg.post(proxy, "async_load")
end

function init(self)
    msg.post(".", "acquire_input_focus")
    self.current_proxy = nil
    show(self, "#menuproxy")
end

function on_message(self, message_id, message, sender)
    if message_id == hash("proxy_loaded") then
        self.current_proxy = sender
        msg.post(sender, "enable")
    elseif message_id == hash("proxy_unloaded") then
        print("Unloaded", sender)
    end
end
```

### Time step control

```lua
-- Pause the loaded world (factor=0)
msg.post("#levelproxy", "set_time_step", { factor = 0, mode = 0 })

-- Resume at normal speed
msg.post("#levelproxy", "set_time_step", { factor = 1, mode = 1 })

-- Slow motion (20% speed, discrete)
msg.post("#levelproxy", "set_time_step", { factor = 0.2, mode = 1 })

-- Double speed (continuous)
msg.post("#levelproxy", "set_time_step", { factor = 2, mode = 0 })
```

Time step modes:
- `0` — `TIME_STEP_MODE_CONTINUOUS`: dt is scaled continuously
- `1` — `TIME_STEP_MODE_DISCRETE`: dt alternates between 0 and full frame dt (useful for factors < 1.0)

### Cross-world addressing

```lua
-- From loaded collection, address bootstrap world
msg.post("main:/loader#script", "load_level", { level_id = 2 })

-- From bootstrap world, address loaded collection
msg.post("mylevel:/myobject", "hello")
```

**Limitation**: `go.get_position()` and similar functions can only access objects within the same collection.

### Input propagation

The game object containing the proxy must acquire input focus for input to reach the loaded collection:

```lua
function init(self)
    msg.post(".", "acquire_input_focus")
    msg.post("#levelproxy", "async_load")
end
```

## Caveats

- **Separate physics worlds**: Objects in different proxied collections cannot physically interact (no collisions, triggers, or ray-casts across worlds)
- **Memory overhead**: Each loaded collection creates a full game world — avoid loading many simultaneously
- **Unique names**: Each loaded collection must have a unique `Name` property (set in the collection file)
- **For spawning objects**: Use collection factories instead of proxies when spawning many instances into the same world

## Protobuf Text Format rules

1. **Default omission**: Omit fields that equal their proto default
2. **Strings**: Always double-quoted
3. **Booleans**: `true` or `false`, no quotes
4. **Field order**: `collection`, `exclude`
5. **No trailing commas or semicolons**
6. **Embedded data**: Multi-line strings with escaped quotes and `\n`

## Workflow

### Creating a new collection proxy

1. Create the target `.collection` file first
2. Ensure the collection's `Name` property is unique across all loaded worlds
3. Create `.collectionproxy` file or embed in parent `.go` / `.collection`
4. Set `collection` to the `.collection` path
5. Set `exclude: true` only if using Live Update

### Editing an existing collection proxy

1. Read the current file content
2. Modify fields as needed
3. Ensure required `collection` field is present
4. Omit optional fields that equal defaults
