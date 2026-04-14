
# Editing Collection Factory Files

Defold collection factory component (`.collectionfactory`) — spawns entire collections (hierarchies of game objects) dynamically at runtime.

## Overview

A collection factory creates copies of a `.collection` file at runtime. Unlike a regular factory (which spawns a single `.go`), a collection factory spawns all game objects in the collection while preserving parent-child relationships.

**Key differences from factory:**
- **Prototype**: references a `.collection` file (not a `.go`)
- **Return value**: `collectionfactory.create()` returns a table mapping collection-local ids to runtime ids
- **Properties**: passed per game object using `id`-`table` pairs
- **API namespace**: `collectionfactory.*` (not `factory.*`)

**Key concepts:**
- **Prototype**: The `.collection` file used as template
- **Dynamic loading**: Defer resource loading until first spawn or explicit `collectionfactory.load()`
- **Dynamic prototype**: Allow changing the collection prototype at runtime with `collectionfactory.set_prototype()`

## File format

Collection factory files use **Protobuf Text Format** based on the `CollectionFactoryDesc` message from `gamesys/gamesys_ddf.proto`.

## Canonical example

Minimal collection factory (most common):
```protobuf
prototype: "/game/enemy_group.collection"
```

Full collection factory with all options:
```protobuf
prototype: "/game/level_chunk.collection"
load_dynamically: true
dynamic_prototype: true
```

## Fields reference

### prototype
- **Required**: Yes
- **Type**: string (resource path)
- **Description**: Path to the `.collection` file used as template for spawned object hierarchies. Must be an absolute project path starting with `/`.
- **Omission rule**: Cannot be omitted (required field).

```protobuf
prototype: "/game/enemy_group.collection"
```

### load_dynamically
- **Required**: No
- **Type**: bool
- **Default**: `false`
- **Description**: When `false`, prototype resources are loaded when the collection factory's parent collection loads. When `true`, resources are loaded on first `collectionfactory.create()` (synchronously) or via explicit `collectionfactory.load()` (asynchronously).
- **Omission rule**: Omit when `false`.

```protobuf
prototype: "/game/enemy_group.collection"
load_dynamically: true
```

**Usage patterns**:
- `false` (default): Resources ready immediately, higher memory usage
- `true` with `collectionfactory.create()`: Synchronous load on first spawn (may cause hitch)
- `true` with `collectionfactory.load()`: Asynchronous pre-loading with callback

### dynamic_prototype
- **Required**: No
- **Type**: bool
- **Default**: `false`
- **Description**: When `true`, allows changing the prototype at runtime using `collectionfactory.set_prototype()`. Disables component count optimization — the collection uses default counts from `game.project`.
- **Omission rule**: Omit when `false`.

```protobuf
prototype: "/game/level_chunk.collection"
dynamic_prototype: true
```

## Common templates

### Basic collection factory (static, pre-loaded)
```protobuf
prototype: "/game/enemy_group.collection"
```

### Lazy-loaded collection factory (load on demand)
```protobuf
prototype: "/game/popup.collection"
load_dynamically: true
```

### Dynamic prototype collection factory (switchable at runtime)
```protobuf
prototype: "/game/level1.collection"
load_dynamically: true
dynamic_prototype: true
```

## Embedded collection factory in game object

Collection factories are typically embedded in `.go` or `.collection` files:

Minimal embedded collection factory:
```protobuf
embedded_components {
  id: "collectionfactory"
  type: "collectionfactory"
  data: "prototype: \"/game/enemy_group.collection\"\n"
  ""
}
```

With load_dynamically:
```protobuf
embedded_components {
  id: "collectionfactory"
  type: "collectionfactory"
  data: "prototype: \"/bubbles/popups/settings.collection\"\n"
  "load_dynamically: true\n"
  ""
}
```

With position and rotation:
```protobuf
embedded_components {
  id: "spawner"
  type: "collectionfactory"
  data: "prototype: \"/game/enemy_group.collection\"\n"
  "load_dynamically: true\n"
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

## Runtime API

### Creating objects

```lua
-- Basic spawn at collection factory position
local ids = collectionfactory.create("#enemy_factory")

-- Spawn at specific position
local pos = vmath.vector3(100, 200, 0)
local ids = collectionfactory.create("#enemy_factory", pos)

-- Spawn with rotation
local rot = vmath.quat_rotation_z(math.pi / 4)
local ids = collectionfactory.create("#enemy_factory", pos, rot)

-- Spawn with per-object script properties
local props = {}
props[hash("/enemy")] = { speed = 100, health = 50 }
props[hash("/weapon")] = { damage = 10 }
local ids = collectionfactory.create("#enemy_factory", pos, nil, props)

-- Spawn with scale (uniform)
local ids = collectionfactory.create("#enemy_factory", pos, nil, nil, 2.0)
```

### Accessing spawned objects

`collectionfactory.create()` returns a table mapping collection-local hash ids to runtime ids:

```lua
local ids = collectionfactory.create("#bean_factory")
-- ids = {
--   hash("/bean") = hash("/collection0/bean"),
--   hash("/shield") = hash("/collection0/shield"),
-- }

-- Access a specific spawned object by its collection-local id
local bean_id = ids[hash("/bean")]
go.set_scale(0.5, bean_id)

-- Send message to a spawned object
msg.post(ids[hash("/enemy")], "activate")

-- Access component on a spawned object
local sprite_url = msg.url(nil, ids[hash("/enemy")], "sprite")
sprite.play_flipbook(sprite_url, hash("run"))
```

### Dynamic loading

```lua
-- Asynchronous loading with callback
local function on_loaded(self, url, result)
    if result then
        local ids = collectionfactory.create(url)
    end
end

function init(self)
    collectionfactory.load("#enemy_factory", on_loaded)
end

function final(self)
    -- Unload when done
    collectionfactory.unload("#enemy_factory")
end
```

### Changing prototype (requires dynamic_prototype: true)

```lua
-- Unload current resources
collectionfactory.unload("#factory")

-- Set new prototype (uses .collectionc — compiled collection)
collectionfactory.set_prototype("#factory", "/main/levels/level1.collectionc")

-- Create uses new prototype
local ids = collectionfactory.create("#factory")
```

### Tracking and deleting spawned collections

```lua
function init(self)
    self.spawned_groups = {}
end

function spawn_enemy_group(self, pos)
    local ids = collectionfactory.create("#enemy_factory", pos)
    table.insert(self.spawned_groups, ids)
    return ids
end

function cleanup(self)
    -- Delete all spawned groups
    for _, ids in ipairs(self.spawned_groups) do
        go.delete(ids)
    end
    self.spawned_groups = {}
end
```

## Instance limits

The `max_instances` setting in `game.project` (Collection related settings) limits total game objects in a world. Each collection factory spawn creates multiple objects — all count against this limit.

## Protobuf Text Format rules

1. **Default omission**: Omit fields that equal their proto default
2. **Floats**: Always include decimal point: `1.0`, not `1`
3. **Strings**: Always double-quoted
4. **Booleans**: `true` or `false`, no quotes
5. **Field order**: `prototype`, `load_dynamically`, `dynamic_prototype`
6. **No trailing commas or semicolons**
7. **Embedded data**: Multi-line strings with escaped quotes and `\n`

## Workflow

### Creating a new collection factory

1. Create the prototype `.collection` file first (with its game objects and hierarchy)
2. Create `.collectionfactory` file or embed in parent `.go` / `.collection`
3. Set `prototype` to the `.collection` path
4. Enable `load_dynamically` if resources should load on demand
5. Enable `dynamic_prototype` if prototype will change at runtime

### Editing an existing collection factory

1. Read the current file content
2. Modify fields as needed
3. Ensure required `prototype` field is present
4. Omit optional fields that equal defaults
