#define LIB_NAME "Astar"
#define MODULE_NAME "astar"
#define DLIB_LOG_DOMAIN "ASTAR"

#include <dmsdk/sdk.h>
#include <inttypes.h>
#include <pather.h>
#include <string_view>
#include <unordered_map>

struct MapData {
  micropather::Map map;
  float pathTotalCost = 0.0f;
  bool mapVFlip = false;
  uint8_t zero = 1;
  uint8_t pathSolved = 0;
  uint16_t pathSize = 0;
  int16_t pathX, pathY;
};

std::unordered_map<std::string_view, MapData> maps;

static void use_zero(std::string_view mapId, bool _zero) {
  maps.at(mapId).zero = (_zero) ? 0 : 1;
}

static int astar_setup(lua_State *L) {
  std::string_view _mapId = luaL_checkstring(L, 1);
  uint16_t _worldWidth = luaL_checkint(L, 2);
  uint16_t _worldHeight = luaL_checkint(L, 3);
  uint8_t _worldDirection = luaL_checkint(L, 4);
  uint16_t _allocate = luaL_checkint(L, 5);
  uint16_t _typicalAdjacent = luaL_checkint(L, 6);
  bool _cache = true;

  if (lua_isboolean(L, 7)) {
    _cache = lua_toboolean(L, 7);
  }

  auto [it, _] = maps.try_emplace(_mapId);
  auto &map_data = it->second;

  if (lua_isboolean(L, 8)) {
    use_zero(_mapId, lua_toboolean(L, 8));
  }

  if (lua_isboolean(L, 9)) {
    map_data.mapVFlip = lua_toboolean(L, 9);
  }

  map_data.map.Setup(_worldWidth, _worldHeight, _worldDirection, _allocate,
                     _typicalAdjacent, _cache);

  return 0;
}

static int astar_map_vflip(lua_State *L) {
  std::string_view _mapId = luaL_checkstring(L, 1);
  maps.at(_mapId).map.MapVFlip();
  return 0;
}

static int astar_map_hflip(lua_State *L) {
  std::string_view _mapId = luaL_checkstring(L, 1);
  maps.at(_mapId).map.MapHFlip();
  return 0;
}

static int astar_set_map_type(lua_State *L) {
  std::string_view _mapId = luaL_checkstring(L, 1);
  maps.at(_mapId).map.SetMapType(luaL_checkint(L, 2));
  return 0;
}

static int astar_set_map(lua_State *L) {
  std::string_view _mapId = luaL_checkstring(L, 1);
  auto &mapData = maps.at(_mapId);
  luaL_checktype(L, 2, LUA_TTABLE);

  for (int i = 0; i < mapData.map.GetWorldSize(); i++) {
    lua_pushinteger(L, i + 1);
    lua_gettable(L, -2);
    if (lua_isnumber(L, -1)) {
      mapData.map.SetTile(i, lua_tointeger(L, -1));
    }
    lua_pop(L, 1);
  }

  if (mapData.mapVFlip) {
    mapData.map.MapVFlip();
  }

  return 0;
}

static int astar_print_map(lua_State *L) {
  std::string_view _mapId = luaL_checkstring(L, 1);
  auto &map_data = maps.at(_mapId);
  maps.at(_mapId).map.PrintMap(map_data.zero);
  return 0;
}

static int astar_use_entities(lua_State *L) {
  std::string_view _mapId = luaL_checkstring(L, 1);
  maps.at(_mapId).map.UseEntities(lua_toboolean(L, 2));
  return 0;
}

static int astar_use_zero(lua_State *L) {
  std::string_view _mapId = luaL_checkstring(L, 1);
  bool _zero = lua_toboolean(L, 2);
  use_zero(_mapId, _zero);
  return 0;
}

static int astar_set_entities(lua_State *L) {
  std::string_view _mapId = luaL_checkstring(L, 1);
  auto &mapData = maps.at(_mapId);
  luaL_checktype(L, 2, LUA_TTABLE);

  size_t size = lua_objlen(L, 2);
  if (size == 0) {
    dmLogError("Entities table can not be empty");
  }

  mapData.map.SetEntityCount(size);

  for (int i = 0; i < size; i++) {
    lua_pushinteger(L, i + 1);
    lua_gettable(L, -2);
    if (lua_isnumber(L, -1)) {
      mapData.map.SetEntity(i, lua_tointeger(L, -1));
    }
    lua_pop(L, 1);
  }

  return 0;
}

static int astar_set_costs(lua_State *L) {
  std::string_view _mapId = luaL_checkstring(L, 1);
  auto &mapData = maps.at(_mapId);
  luaL_checktype(L, 2, LUA_TTABLE);

  mapData.map.ResizePath();
  mapData.map.ResetPath();

  uint16_t tileCount = 0;
  uint16_t id = 0;
  uint16_t costID = 0;

  lua_pushnil(L);
  while (lua_next(L, 2) != 0) {
    tileCount++;
    lua_pop(L, 1);
  }

  mapData.map.SetTileCount(tileCount);
  mapData.map.SetCosts();

  lua_pushnil(L);
  while (lua_next(L, 2) != 0) {
    mapData.map.AddCostTileID(id, lua_tointeger(L, -2));

    if (lua_istable(L, -1)) {
      costID = 0;
      lua_pushnil(L);

      while (lua_next(L, -2) != 0) {
        mapData.map.AddCost(id, costID, lua_tonumber(L, -1));
        lua_pop(L, 1);
        costID++;

        if (costID > mapData.map.GetWordDirection()) {
          dmLogError("There are more costs than direction. Cost Count: %i, "
                     "Direction: %i",
                     costID, mapData.map.GetWordDirection());
          return 0;
        }
      }
    }
    lua_pop(L, 1);
    id++;
  }

  return 0;
}

static int astar_reset_cache(lua_State *L) {
  std::string_view _mapId = luaL_checkstring(L, 1);
  maps.at(_mapId).map.ResetPath();
  return 0;
}

static int astar_resize_path(lua_State *L) {
  std::string_view _mapId = luaL_checkstring(L, 1);
  maps.at(_mapId).map.ResizePath();
  return 0;
}

static int astar_reset(lua_State *L) {
  std::string_view _mapId = luaL_checkstring(L, 1);
  maps.at(_mapId).map.Clear();
  return 0;
}

static int astar_get_at(lua_State *L) {
  std::string_view _mapId = luaL_checkstring(L, 1);
  auto &mapData = maps.at(_mapId);
  mapData.pathX = luaL_checkint(L, 2) - mapData.zero;
  mapData.pathY = luaL_checkint(L, 3) - mapData.zero;
  lua_pushinteger(L, mapData.map.WorldAt(mapData.pathX, mapData.pathY));
  return 1;
}

static int astar_set_at(lua_State *L) {
  std::string_view _mapId = luaL_checkstring(L, 1);
  auto &mapData = maps.at(_mapId);
  mapData.pathX = luaL_checkint(L, 2) - mapData.zero;
  mapData.pathY = luaL_checkint(L, 3) - mapData.zero;
  mapData.map.SetToWorldAt(mapData.pathX, mapData.pathY, luaL_checkint(L, 4));
  return 0;
}

static int astar_solve(lua_State *L) {
  std::string_view _mapId = luaL_checkstring(L, 1);
  auto &mapData = maps.at(_mapId);

  uint8_t i = 3;

  mapData.map.SetPathFromTo(
      luaL_checkint(L, 2) - mapData.zero, luaL_checkint(L, 3) - mapData.zero,
      luaL_checkint(L, 4) - mapData.zero, luaL_checkint(L, 5) - mapData.zero);

  mapData.pathSolved = mapData.map.Solve();
  mapData.pathSize = mapData.map.GetPathSize();
  mapData.pathTotalCost = mapData.map.GetTotalCost();

  // Early exit if only found itself
  if (mapData.pathSize == 1) {
    mapData.pathSolved = micropather::NO_SOLUTION;
    mapData.pathSize = 0;
  }

  lua_pushinteger(L, mapData.pathSolved);
  lua_pushinteger(L, mapData.pathSize);
  lua_pushnumber(L, mapData.pathTotalCost);

  if (mapData.pathSize > 1) {

    i++;
    lua_createtable(L, mapData.pathSize, 0);
    int newTable = lua_gettop(L);

    for (int ii = 0; ii < mapData.pathSize; ii++) {
      mapData.map.NodeToXY(mapData.map.path[ii], &mapData.pathX,
                           &mapData.pathY);

      lua_createtable(L, 2, 0);
      lua_pushstring(L, "x");
      lua_pushinteger(L, mapData.pathX + mapData.zero);
      lua_settable(L, -3);
      lua_pushstring(L, "y");
      lua_pushinteger(L, mapData.pathY + mapData.zero);
      lua_settable(L, -3);
      lua_pushstring(L, "id");
      lua_pushinteger(L, mapData.map.WorldAt(mapData.pathX, mapData.pathY));
      lua_settable(L, -3);

      lua_rawseti(L, newTable, ii + 1);
    }
  }

  return i;
}

static int astar_solve_near(lua_State *L) {
  std::string_view _mapId = luaL_checkstring(L, 1);
  auto &mapData = maps.at(_mapId);

  uint8_t i = 2;

  mapData.map.SetPathFrom(luaL_checkint(L, 2) - mapData.zero,
                          luaL_checkint(L, 3) - mapData.zero);

  mapData.pathSolved = mapData.map.SolveNear(luaL_checknumber(L, 4));
  mapData.pathSize = mapData.map.GetNearsSize();

  // Early exit if only found itself
  if (mapData.pathSize == 1) {
    mapData.pathSolved = micropather::NO_SOLUTION;
    mapData.pathSize = 0;
  }

  lua_pushinteger(L, mapData.pathSolved);
  lua_pushinteger(L, mapData.pathSize);

  if (mapData.pathSize > 1) {

    i++;
    lua_createtable(L, mapData.pathSize, 0);
    int newTable = lua_gettop(L);

    for (int ii = 0; ii < mapData.pathSize; ii++) {

      mapData.map.NodeToXY(mapData.map.nears[ii].state, &mapData.pathX,
                           &mapData.pathY);

      lua_createtable(L, 2, 0);
      lua_pushstring(L, "x");
      lua_pushinteger(L, mapData.pathX + mapData.zero);
      lua_settable(L, -3);
      lua_pushstring(L, "y");
      lua_pushinteger(L, mapData.pathY + mapData.zero);
      lua_settable(L, -3);
      lua_pushstring(L, "cost");
      lua_pushnumber(L, mapData.map.nears[ii].cost);
      lua_settable(L, -3);
      lua_pushstring(L, "id");
      lua_pushinteger(L, mapData.map.WorldAt(mapData.pathX, mapData.pathY));
      lua_settable(L, -3);

      lua_rawseti(L, newTable, ii + 1);
    }
  }

  return i;
}

static const luaL_reg Module_methods[] = {{"solve_near", astar_solve_near},
                                          {"solve", astar_solve},
                                          {"reset", astar_reset},
                                          {"clear_path", astar_resize_path},
                                          {"reset_cache", astar_reset_cache},
                                          {"set_costs", astar_set_costs},
                                          {"set_map_type", astar_set_map_type},
                                          {"set_map", astar_set_map},
                                          {"setup", astar_setup},
                                          {"get_at", astar_get_at},
                                          {"set_at", astar_set_at},
                                          {"use_zero", astar_use_zero},
                                          {"set_entities", astar_set_entities},
                                          {"use_entities", astar_use_entities},
                                          {"print_map", astar_print_map},
                                          {"map_vflip", astar_map_vflip},
                                          {"map_hflip", astar_map_hflip},
                                          {0, 0}

};

static void LuaInit(lua_State *L) {
  int top = lua_gettop(L);

  // Register lua names
  luaL_register(L, MODULE_NAME, Module_methods);

#define SETCONSTANT(name)                                                      \
  lua_pushnumber(L, (lua_Number)micropather::name);                            \
  lua_setfield(L, -2, #name);

  SETCONSTANT(SOLVED);
  SETCONSTANT(NO_SOLUTION);
  SETCONSTANT(START_END_SAME);
#undef SETCONSTANT

#define SETCONSTANT(name)                                                      \
  lua_pushnumber(L, (lua_Number)micropather::name);                            \
  lua_setfield(L, -2, #name);

  SETCONSTANT(DIRECTION_FOUR);
  SETCONSTANT(DIRECTION_EIGHT);
#undef SETCONSTANT

#define SETCONSTANT(name)                                                      \
  lua_pushnumber(L, (lua_Number)micropather::name);                            \
  lua_setfield(L, -2, #name);

  SETCONSTANT(HEX_ODDR);
  SETCONSTANT(HEX_EVENR);
  SETCONSTANT(HEX_ODDQ);
  SETCONSTANT(HEX_EVENQ);
  SETCONSTANT(GRID_CLASSIC);
#undef SETCONSTANT

  lua_pop(L, 1);
  assert(top == lua_gettop(L));
}

dmExtension::Result AppInitializeAstar(dmExtension::AppParams *params) {
  return dmExtension::RESULT_OK;
}

dmExtension::Result InitializeAstar(dmExtension::Params *params) {

  LuaInit(params->m_L);
  printf("Registered %s Extension\n", MODULE_NAME);
  return dmExtension::RESULT_OK;
}

DM_DECLARE_EXTENSION(astar, LIB_NAME, AppInitializeAstar, 0, InitializeAstar, 0,
                     0, 0)
