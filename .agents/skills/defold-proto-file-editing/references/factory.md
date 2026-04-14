
# Editing Factory Files

Defold factory component (`.factory`) — spawns game objects dynamically at runtime from a prototype.

## Overview

A factory component creates copies of a game object prototype during runtime. Each `factory.create()` call instantiates a new game object with optional position, rotation, scale, and script properties.

**Key concepts**:
- **Prototype**: The `.go` file that serves as a template for spawned objects
- **Dynamic loading**: Optionally defer resource loading until first spawn or explicit `factory.load()` call
- **Dynamic prototype**: Allow changing the prototype at runtime with `factory.set_prototype()`

## File format

Factory files use **Protobuf Text Format** based on the `FactoryDesc` message from `gamesys/gamesys_ddf.proto`.

## Canonical example

Minimal factory (most common):
```protobuf
prototype: "/game/enemy.go"
```

Full factory with all options:
```protobuf
prototype: "/game/bullet.go"
load_dynamically: true
dynamic_prototype: true
```

## Fields reference

### prototype
- **Required**: Yes
- **Type**: string (resource path)
- **Description**: Path to the `.go` file used as template for spawned objects. Must be an absolute project path starting with `/`.
- **Omission rule**: Cannot be omitted (required field).

```protobuf
prototype: "/game/enemy.go"
```

### load_dynamically
- **Required**: No
- **Type**: bool
- **Default**: `false`
- **Description**: When `false`, prototype resources are loaded when the factory's parent collection loads. When `true`, resources are loaded on first `factory.create()` (synchronously) or via explicit `factory.load()` (asynchronously).
- **Omission rule**: Omit when `false`.

```protobuf
prototype: "/game/enemy.go"
load_dynamically: true
```

**Usage patterns**:
- `false` (default): Resources ready immediately, slight memory overhead
- `true` with `factory.create()`: Synchronous load on first spawn (may cause hitch)
- `true` with `factory.load()`: Asynchronous pre-loading with callback

### dynamic_prototype
- **Required**: No
- **Type**: bool
- **Default**: `false`
- **Description**: When `true`, allows changing the prototype at runtime using `factory.set_prototype()`. Disables component count optimization — the collection uses default counts from `game.project`.
- **Omission rule**: Omit when `false`.

```protobuf
prototype: "/game/bullet.go"
dynamic_prototype: true
```

## Common templates

### Basic factory (static, pre-loaded)
```protobuf
prototype: "/game/enemy.go"
```

### Lazy-loaded factory (load on demand)
```protobuf
prototype: "/game/powerup.go"
load_dynamically: true
```

### Dynamic prototype factory (switchable at runtime)
```protobuf
prototype: "/game/bullet_fire.go"
load_dynamically: true
dynamic_prototype: true
```

## Embedded factory in game object

Factories are typically embedded in a `.go` file rather than as separate `.factory` files:

```protobuf
embedded_components {
  id: "enemy_factory"
  type: "factory"
  data: "prototype: \"/game/enemy.go\"\n"
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

With load_dynamically:
```protobuf
embedded_components {
  id: "bulletfactory"
  type: "factory"
  data: "prototype: \"/example/bullet.go\"\n"
  "load_dynamically: false\n"
  "dynamic_prototype: false\n"
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

Minimal embedded factory:
```protobuf
embedded_components {
  id: "carrotfactory"
  type: "factory"
  data: "prototype: \"/example/debris.go\"\n"
  ""
}
```

## Runtime API

### Creating objects

```lua
-- Basic spawn at factory position
local id = factory.create("#enemy_factory")

-- Spawn at specific position
local pos = vmath.vector3(100, 200, 0)
local id = factory.create("#enemy_factory", pos)

-- Spawn with rotation
local rot = vmath.quat_rotation_z(math.pi / 4)
local id = factory.create("#enemy_factory", pos, rot)

-- Spawn with script properties
local id = factory.create("#enemy_factory", pos, nil, { speed = 100, health = 50 })

-- Spawn with scale (uniform)
local id = factory.create("#enemy_factory", pos, nil, nil, 2.0)

-- Spawn with non-uniform scale
local id = factory.create("#enemy_factory", pos, nil, nil, vmath.vector3(1, 2, 1))
```

### Dynamic loading

```lua
-- Asynchronous loading with callback
local function on_loaded(self, url, result)
    if result then
        local id = factory.create(url)
    end
end

function init(self)
    factory.load("#enemy_factory", on_loaded)
end

function final(self)
    -- Unload when done
    factory.unload("#enemy_factory")
end
```

### Changing prototype (requires dynamic_prototype: true)

```lua
-- Unload current resources
factory.unload("#bulletfactory")

-- Set new prototype
factory.set_prototype("#bulletfactory", "/game/bullet_ice.goc")

-- Create uses new prototype
local id = factory.create("#bulletfactory")
```

### Tracking spawned objects

```lua
function init(self)
    self.spawned = {}
end

function spawn_enemy(self, pos)
    local id = factory.create("#enemy_factory", pos)
    table.insert(self.spawned, id)
    return id
end

function cleanup(self)
    -- Delete all spawned objects
    go.delete(self.spawned)
    self.spawned = {}
end
```

### Addressing spawned objects

```lua
-- Send message to spawned object
local id = factory.create("#enemy_factory")
msg.post(id, "set_target", { target = player_id })

-- Access component on spawned object
local sprite_url = msg.url(nil, id, "sprite")
sprite.play_flipbook(sprite_url, hash("run"))
```

## Instance limits

The `max_instances` setting in `game.project` (Collection related settings) limits total game objects in a world. All placed and spawned objects count against this limit. Deleting objects frees slots for new spawns.

## Protobuf Text Format rules

1. **Default omission**: Omit fields that equal their proto default
2. **Floats**: Always include decimal point: `1.0`, not `1`
3. **Strings**: Always double-quoted
4. **Booleans**: `true` or `false`, no quotes
5. **Field order**: `prototype`, `load_dynamically`, `dynamic_prototype`
6. **No trailing commas or semicolons**
7. **Embedded data**: Multi-line strings with escaped quotes and `\n`

## Workflow

### Creating a new factory

1. Create the prototype `.go` file first
2. Create `.factory` file or embed in parent `.go`
3. Set `prototype` to the `.go` path
4. Enable `load_dynamically` if resources should load on demand
5. Enable `dynamic_prototype` if prototype will change at runtime

### Editing an existing factory

1. Read the current file content
2. Modify fields as needed
3. Ensure required `prototype` field is present
4. Omit optional fields that equal defaults
