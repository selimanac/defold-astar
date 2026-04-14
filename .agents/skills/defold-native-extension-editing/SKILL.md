---
name: defold-native-extension-editing
description: "Defold native extension development. Use when creating or editing C/C++ (.c, .cpp, .h, .hpp), JavaScript (.js), or manifest files in native extension directories (src/, include/, lib/, api/)."
---

# Defold Native Extension Structure

A native extension is a folder containing `ext.manifest` and native code for extending the Defold engine.

## Required structure

```
my_extension/
├── ext.manifest           # Extension manifest (YAML)
├── src/                   # Source code (C/C++/ObjC/JS)
│   ├── extension.cpp      # Main extension entry point
│   └── ...
├── include/               # Public headers (optional)
├── lib/                   # Platform libraries (optional)
│   ├── x86_64-win32/
│   ├── x86_64-linux/
│   ├── arm64-ios/
│   └── ...
├── api/                   # Script API definitions (optional)
│   └── my_extension.script_api
└── res/                   # Platform resources (optional)
    └── android/
```

## ext.manifest format

```yaml
name: "MyExtension"
platforms:
  x86_64-win32:
    context:
      defines: ["MY_DEFINE"]
      libs: ["user32"]
  arm64-ios:
    context:
      frameworks: ["UIKit"]
```

## Extension entry point (C++)

```cpp
#define EXTENSION_NAME MyExtension
#define LIB_NAME "MyExtension"
#define MODULE_NAME "myextension"

#include <dmsdk/sdk.h>

static int MyFunction(lua_State* L)
{
    // Implementation
    return 0;
}

static const luaL_reg Module_methods[] =
{
    {"my_function", MyFunction},
    {0, 0}
};

static void LuaInit(lua_State* L)
{
    int top = lua_gettop(L);
    luaL_register(L, MODULE_NAME, Module_methods);
    lua_pop(L, 1);
    assert(top == lua_gettop(L));
}

static dmExtension::Result AppInitializeMyExtension(dmExtension::AppParams* params)
{
    return dmExtension::RESULT_OK;
}

static dmExtension::Result InitializeMyExtension(dmExtension::Params* params)
{
    LuaInit(params->m_L);
    return dmExtension::RESULT_OK;
}

static dmExtension::Result FinalizeMyExtension(dmExtension::Params* params)
{
    return dmExtension::RESULT_OK;
}

DM_DECLARE_EXTENSION(EXTENSION_NAME, LIB_NAME, AppInitializeMyExtension, 0, InitializeMyExtension, 0, 0, FinalizeMyExtension)
```

## HTML5/JavaScript extension

For web platform, use `.js` files in `src/`:

```javascript
var MyExtension = {
    MyFunction: function() {
        // Implementation
    }
};
mergeInto(LibraryManager.library, MyExtension);
```

## Platform library paths (lib/)

- `x86_64-win32/` - Windows 64-bit
- `x86-win32/` - Windows 32-bit
- `x86_64-linux/` - Linux 64-bit
- `arm64-osx/` - macOS ARM
- `x86_64-osx/` - macOS Intel
- `arm64-ios/` - iOS ARM64
- `armv7-android/`, `arm64-android/` - Android
- `js-web/`, `wasm-web/` - HTML5

## Prefer dmSDK over Lua API calls

**Always use direct C++ dmSDK functions instead of calling Lua API wrappers** (e.g. `lua_getglobal(L, "go")` + `lua_call`). The dmSDK provides efficient C++ equivalents for most Lua game object operations.

### Key dmSDK patterns

**Getting game object instances from Lua:**
```cpp
// Get the calling script's game object instance
dmGameObject::HInstance caller = dmScript::CheckGOInstance(L);

// Get the collection from any instance
dmGameObject::HCollection collection = dmGameObject::GetCollection(caller);

// Resolve a hash identifier (from Lua stack) to an instance
dmhash_t id = dmScript::CheckHash(L, index);
dmGameObject::HInstance target = dmGameObject::GetInstanceFromIdentifier(collection, id);
```

**Manipulating game objects directly in C++:**
```cpp
// Position (uses dmVMath::Point3)
dmVMath::Point3 pos = dmGameObject::GetPosition(instance);
dmGameObject::SetPosition(instance, dmVMath::Point3(x, y, z));

// Rotation (uses dmVMath::Quat)
dmVMath::Quat rot = dmGameObject::GetRotation(instance);
dmGameObject::SetRotation(instance, rot);

// Scale
float scale = dmGameObject::GetUniformScale(instance);
dmGameObject::SetScale(instance, scale);
dmGameObject::SetScale(instance, dmVMath::Vector3(sx, sy, sz));
```

**Math types — create once, reuse in loops:**
```cpp
dmVMath::Point3 pos(0.0f, 0.0f, 0.0f);
for (int i = 0; i < count; ++i)
{
    pos.setX(computed_x);
    pos.setY(computed_y);
    dmGameObject::SetPosition(instance, pos);
}
```

**Extracting Lua types via dmScript:**
```cpp
dmVMath::Vector3* v = dmScript::CheckVector3(L, index);
dmVMath::Vector4* v4 = dmScript::CheckVector4(L, index);
dmVMath::Quat* q = dmScript::CheckQuat(L, index);
dmVMath::Matrix4* m = dmScript::CheckMatrix4(L, index);
dmhash_t hash = dmScript::CheckHash(L, index);
dmhash_t hash = dmScript::CheckHashOrString(L, index);
```

### Async Lua callbacks via dmScript

When a native extension needs to asynchronously invoke a user-provided Lua callback (e.g. after a platform event, timer, or async operation), use `dmScript::LuaCallbackInfo`. Reference: https://github.com/indiesoftby/defold-page-visibility/tree/main/page_visibility/src

**Storing a callback from Lua:**
```cpp
static dmScript::LuaCallbackInfo* g_Callback = 0;

static int SetCallback(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);

    // Destroy previous callback to avoid leaks
    if (g_Callback)
    {
        dmScript::DestroyCallback(g_Callback);
        g_Callback = 0;
    }

    if (lua_isfunction(L, 1))
    {
        g_Callback = dmScript::CreateCallback(L, 1);
    }

    return 0;
}
```

**Invoking the callback later (from any native event):**
```cpp
static void OnAsyncEvent(int result_code)
{
    if (!g_Callback || !dmScript::IsCallbackValid(g_Callback))
        return;

    lua_State* L = dmScript::GetCallbackLuaContext(g_Callback);
    DM_LUA_STACK_CHECK(L, 0);

    if (!dmScript::SetupCallback(g_Callback))
    {
        dmScript::DestroyCallback(g_Callback);
        g_Callback = 0;
        return;
    }

    // Push arguments after self (self is already on stack from SetupCallback)
    lua_pushnumber(L, result_code);

    dmScript::PCall(L, 2, 0); // 2 = self + 1 user argument

    dmScript::TeardownCallback(g_Callback);
}
```

**Cleanup in Finalize (prevent leaks on extension unload):**
```cpp
static dmExtension::Result FinalizeMyExtension(dmExtension::Params* params)
{
    if (g_Callback)
    {
        dmScript::DestroyCallback(g_Callback);
        g_Callback = 0;
    }
    return dmExtension::RESULT_OK;
}
```

**Lifecycle:** `CreateCallback` → `SetupCallback` → `PCall` → `TeardownCallback` → `DestroyCallback` (when done).

For one-shot callbacks, call `DestroyCallback` right after `TeardownCallback`. For persistent listeners, keep the callback and only destroy on replacement or finalize.

### When Lua API calls are acceptable

Only fall back to `lua_getglobal`/`lua_call` patterns when:
- The functionality has **no dmSDK C++ equivalent** (e.g. `go.animate`, `msg.post` to arbitrary URLs)
- You need to call a **user-defined Lua callback** without the `dmScript::LuaCallbackInfo` pattern (rare)

## Defold engine source as reference

The Defold engine source at https://github.com/defold/defold/tree/dev/engine contains extensive dmSDK usage examples. When implementing native extensions, actively study the engine source to find correct API usage patterns, especially:

- `engine/gameobject/` — game object manipulation (`dmGameObject::*`)
- `engine/gamesys/` — component systems (sprite, collision, factory, etc.)
- `engine/script/` — script bridge utilities (`dmScript::*`)
- `engine/dlib/` — math, hashing, logging, buffers (`dmVMath::*`, `dmLog*`, `dmBuffer::*`)
- `engine/render/` — render pipeline (`dmRender::*`)

Additionally, the Spine extension at https://github.com/defold/extension-spine/tree/main/defold-spine is an excellent real-world reference for dmSDK usage — it demonstrates component registration, game object manipulation, rendering, resource management, and script bindings.

Use the Librarian tool or `defold-api-fetch` skill to fetch specific API docs. Browse engine source and extension-spine on GitHub for real-world usage when the API docs are insufficient.

## Code formatting

When working with native extensions, ensure `.clang-format` exists in project root for consistent C/C++ formatting. If missing, fetch from Defold repository:

```
https://raw.githubusercontent.com/defold/defold/refs/heads/dev/.clang-format
```

**Important**: This `.clang-format` is from the official Defold repository and ensures code style consistency with the engine.

## API reference

For C++ SDK documentation, use `defold-api-fetch` skill with C++ Native Extension APIs section (dmExtension, dmScript, dmBuffer, etc.).
