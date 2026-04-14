# .script Reference

Game object script — Lua file attached to a game object as a component. Defines behavior through lifecycle callbacks and exposes editable properties via `go.property()`.

## Available APIs

`go`, `msg`, `vmath`, `timer`, `sys`, `http`, `socket`, `json`, `zlib`, `html5`, `sound`, `sprite`, `label`, `particlefx`, `tilemap`, `factory`, `collectionfactory`, `collectionproxy`, `model`, `buffer`, `resource`, `crash`, `profiler`, `window`, `image`, `physics`, `b2d`, `camera`, `graphics` + Lua standard library + native extension globals

**Not available**: `gui`, `render`

For API details of specific namespaces use `defold-api-fetch`.

## Game object addressing

When referencing other game objects from a script (via `go.set_position`, `go.get_position`, `msg.post`, etc.), use absolute paths with a leading `/`: `hash("/sprite1")`, NOT `hash("sprite1")`. Without `/` the path is relative and only resolves to children of the current game object.

## go.property() reference

`go.property(name, default_value)` declares a script property that is:
- Visible and editable in the Defold editor Properties panel
- Accessible at runtime as `self.<name>`
- Overridable per-instance in `.go` files (via `PropertyDesc`) and `.collection` files (via `ComponentPropertyDesc`)
- Settable at spawn time via `factory.create()` / `collectionfactory.create()`
- Readable/writable externally via `go.get()` / `go.set()` / `go.animate()`

### Supported property types

| Lua constructor | Property type | Example declaration | Override type in `.go` |
|---|---|---|---|
| `number` literal | number | `go.property("speed", 200)` | `PROPERTY_TYPE_NUMBER` |
| `boolean` literal | boolean | `go.property("active", true)` | `PROPERTY_TYPE_BOOLEAN` |
| `hash("...")` | hash | `go.property("type", hash("enemy"))` | `PROPERTY_TYPE_HASH` |
| `msg.url()` | URL | `go.property("target", msg.url())` | `PROPERTY_TYPE_URL` |
| `vmath.vector3()` | vector3 | `go.property("dir", vmath.vector3(1, 0, 0))` | `PROPERTY_TYPE_VECTOR3` |
| `vmath.vector4()` | vector4 | `go.property("color", vmath.vector4(1, 1, 1, 1))` | `PROPERTY_TYPE_VECTOR4` |
| `vmath.quat()` | quaternion | `go.property("rot", vmath.quat())` | `PROPERTY_TYPE_QUAT` |
| `resource.atlas()` | resource | `go.property("my_atlas", resource.atlas("/main.atlas"))` | — |
| `resource.font()` | resource | `go.property("my_font", resource.font("/main.font"))` | — |
| `resource.material()` | resource | `go.property("my_mat", resource.material("/mat.material"))` | — |
| `resource.texture()` | resource | `go.property("my_tex", resource.texture("/tex.png"))` | — |
| `resource.tile_source()` | resource | `go.property("my_ts", resource.tile_source("/main.tilesource"))` | — |

### Unsupported types

- **`string`** — use `hash` instead
- **`table`** / **`array`** — not supported
- **`integer`** — use `number` (Lua has no integer type)
- **`nil`** — not a valid default

### No expression evaluation

`go.property()` default values are parsed statically at build time. **Expressions are not evaluated**. Only literal values and constructor calls are allowed.

```lua
-- CORRECT — literal values only
go.property("frame_time", 0.00833333)
go.property("half_pi", 1.5707963)
go.property("max_count", 10)

-- WRONG — expressions are NOT evaluated
go.property("frame_time", 1 / 120)        -- will NOT work
go.property("half_pi", math.pi / 2)       -- will NOT work
go.property("max_count", 5 + 5)           -- will NOT work
go.property("name", "enemy")              -- will NOT work (string not supported)
```

### Resource properties

Resource properties allow swapping assets per-instance in the editor. They show up as file browser fields in the Properties panel.

```lua
go.property("my_atlas", resource.atlas("/gfx/main.atlas"))
go.property("my_font", resource.font("/fonts/main.font"))
go.property("my_material", resource.material("/materials/sprite.material"))
go.property("my_texture", resource.texture("/textures/bg.png"))
go.property("my_tile_source", resource.tile_source("/tiles/main.tilesource"))

function init(self)
	go.set("#sprite", "image", self.my_atlas)
	go.set("#label", "font", self.my_font)
	go.set("#sprite", "material", self.my_material)
	go.set("#model", "texture0", self.my_texture)
	go.set("#tilemap", "tile_source", self.my_tile_source)
end
```

## Physics caveats

- **Raycast from inside an object**: `physics.raycast()` and `physics.raycast_async()` ignore any collision object that contains the ray's starting point. If the ray originates from within the object's own collision shape, that object will never appear in the results — no special filtering is needed.
- **Triggers vs raycasts**: Rays only intersect with dynamic, kinematic, and static collision objects. Trigger objects are invisible to raycasts.

## Property overrides in .go files

When a `.script` is referenced in a `.go` file, its `go.property` values can be overridden using `PropertyDesc` entries:

```protobuf
components {
  id: "script"
  component: "/main/enemy.script"
  properties {
    id: "speed"
    value: "200.0"
    type: PROPERTY_TYPE_NUMBER
  }
  properties {
    id: "type"
    value: "boss"
    type: PROPERTY_TYPE_HASH
  }
}
```

### Override value formats by type

| Property type | value format | Example |
|---|---|---|
| `PROPERTY_TYPE_NUMBER` | decimal string | `"200.0"` |
| `PROPERTY_TYPE_BOOLEAN` | `"true"` or `"false"` | `"true"` |
| `PROPERTY_TYPE_HASH` | bare string (hashed automatically) | `"enemy"` |
| `PROPERTY_TYPE_URL` | URL string | `"/level/spawner#script"` |
| `PROPERTY_TYPE_VECTOR3` | `"x, y, z"` | `"1.0, 0.0, 0.0"` |
| `PROPERTY_TYPE_VECTOR4` | `"x, y, z, w"` | `"1.0, 1.0, 1.0, 1.0"` |
| `PROPERTY_TYPE_QUAT` | `"x, y, z, w"` | `"0.0, 0.0, 0.0, 1.0"` |

### Override priority

1. Script default (`go.property("speed", 100)`) — lowest
2. `.go` file override (`PropertyDesc` in `ComponentDesc`) — medium
3. `.collection` file override (`ComponentPropertyDesc` in `InstanceDesc` / `EmbeddedInstanceDesc`) — high
4. `factory.create()` / `collectionfactory.create()` properties table — highest

