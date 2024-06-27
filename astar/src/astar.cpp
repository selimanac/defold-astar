#define LIB_NAME "Astar"
#define MODULE_NAME "astar"
#define DLIB_LOG_DOMAIN "ASTAR"

#include "micropather/micropather.h"
#include <dmsdk/dlib/log.h>
#include <dmsdk/sdk.h>
#include <inttypes.h>
#include <pather.h>

float pathTotalCost = 0.0f;
bool mapVFlip = false;
uint8_t zero = 1;
uint8_t pathSolved = 0;
uint16_t pathSize = 0;
int16_t pathX, pathY;

static void use_zero(bool _zero) { (_zero) ? zero = 0 : zero = 1; }

static int astar_setup(lua_State *L) {
  uint16_t _worldWidth = luaL_checkint(L, 1);
  uint16_t _worldHeight = luaL_checkint(L, 2);
  uint8_t _worldDirection = luaL_checkint(L, 3);
  uint16_t _allocate = luaL_checkint(L, 4);
  uint16_t _typicalAdjacent = luaL_checkint(L, 5);
  bool _cache = true;

  if (lua_isboolean(L, 6)) {
    _cache = lua_toboolean(L, 6);
  }

  if (lua_isboolean(L, 7)) {
    use_zero(lua_toboolean(L, 7));
  }

  if (lua_isboolean(L, 8)) {
    mapVFlip = lua_toboolean(L, 8);
  }

  micropather::Setup(_worldWidth, _worldHeight, _worldDirection, _allocate,
                     _typicalAdjacent, _cache);
  return 0;
}

static int astar_map_vflip(lua_State *L) {
  micropather::MapVFlip();
  return 0;
}

static int astar_map_hflip(lua_State *L) {
  micropather::MapHFlip();
  return 0;
}

static int astar_set_map_type(lua_State *L) {
  micropather::SetMapType(luaL_checkint(L, 1));
  return 0;
}

static int astar_set_map(lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);

  for (int i = 0; i < micropather::GetWorldSize(); i++) {
    lua_pushinteger(L, i + 1);
    lua_gettable(L, -2);
    if (lua_isnumber(L, -1)) {
      micropather::SetTile(i, lua_tointeger(L, -1));
    }
    lua_pop(L, 1);
  }

  if (mapVFlip) {
    micropather::MapVFlip();
  }

  return 0;
}

static int astar_print_map(lua_State *L) {
  micropather::PrintMap(zero);
  return 0;
}

static int astar_use_entities(lua_State *L) {
  micropather::UseEntities(lua_toboolean(L, 1));
  return 0;
}

static int astar_use_zero(lua_State *L) {
  bool _zero = lua_toboolean(L, 1);
  use_zero(_zero);
  return 0;
}

static int astar_set_entities(lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);

  size_t size = lua_objlen(L, 1);
  if (size == 0) {
    dmLogError("Entities table can not be empty");
  }

  micropather::SetEntityCount(size);

  for (int i = 0; i < size; i++) {
    lua_pushinteger(L, i + 1);
    lua_gettable(L, -2);
    if (lua_isnumber(L, -1)) {
      micropather::SetEntity(i, lua_tointeger(L, -1));
    }
    lua_pop(L, 1);
  }

  return 0;
}

static int astar_set_costs(lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);

  micropather::ResizePath();
  micropather::ResetPath();

  uint16_t tileCount = 0;
  uint16_t id = 0;
  uint16_t costID = 0;

  lua_pushnil(L);
  while (lua_next(L, 1) != 0) {
    tileCount++;
    lua_pop(L, 1);
  }

  micropather::SetTileCount(tileCount);
  micropather::SetCosts();

  lua_pushnil(L);
  while (lua_next(L, 1) != 0) {
    micropather::AddCostTileID(id, lua_tointeger(L, -2));

    if (lua_istable(L, -1)) {
      costID = 0;
      lua_pushnil(L);

      while (lua_next(L, -2) != 0) {
        micropather::AddCost(id, costID, lua_tonumber(L, -1));
        lua_pop(L, 1);
        costID++;

        if (costID > micropather::GetWordDirection()) {
          dmLogError("There are more costs than direction. Cost Count: %i, "
                     "Direction: %i",
                     costID, micropather::GetWordDirection());
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
  micropather::ResetPath();
  return 0;
}

static int astar_resize_path(lua_State *L) {
  micropather::ResizePath();
  return 0;
}

static int astar_reset(lua_State *L) {
  micropather::Clear();
  return 0;
}

static int astar_get_at(lua_State *L) {
  pathX = luaL_checkint(L, 1) - zero;
  pathY = luaL_checkint(L, 2) - zero;
  lua_pushinteger(L, micropather::WorldAt(pathX, pathY));
  return 1;
}

static int astar_set_at(lua_State *L) {
  pathX = luaL_checkint(L, 1) - zero;
  pathY = luaL_checkint(L, 2) - zero;
  micropather::SetToWorldAt(pathX, pathY, luaL_checkint(L, 3));
  return 0;
}

static int astar_solve(lua_State *L) {
  uint8_t i = 3;

  micropather::SetPathFromTo(
      luaL_checkint(L, 1) - zero, luaL_checkint(L, 2) - zero,
      luaL_checkint(L, 3) - zero, luaL_checkint(L, 4) - zero);

  pathSolved = micropather::Solve();
  pathSize = micropather::GetPathSize();
  pathTotalCost = micropather::GetTotalCost();

  // Early exit if only found itself
  if (pathSize == 1) {
    pathSolved = micropather::NO_SOLUTION;
    pathSize = 0;
  }

  lua_pushinteger(L, pathSolved);
  lua_pushinteger(L, pathSize);
  lua_pushnumber(L, pathTotalCost);

  if (pathSize > 1) {

    i++;
    lua_createtable(L, pathSize, 0);
    int newTable = lua_gettop(L);

    for (int ii = 0; ii < pathSize; ii++) {
      micropather::NodeToXY(micropather::path[ii], &pathX, &pathY);

      lua_createtable(L, 2, 0);
      lua_pushstring(L, "x");
      lua_pushinteger(L, pathX + zero);
      lua_settable(L, -3);
      lua_pushstring(L, "y");
      lua_pushinteger(L, pathY + zero);
      lua_settable(L, -3);
      lua_pushstring(L, "id");
      lua_pushinteger(L, micropather::WorldAt(pathX, pathY));
      lua_settable(L, -3);

      lua_rawseti(L, newTable, ii + 1);
    }
  }

  return i;
}

static int astar_solve_near(lua_State *L) {
  uint8_t i = 2;

  micropather::SetPathFrom(luaL_checkint(L, 1) - zero,
                           luaL_checkint(L, 2) - zero);

  pathSolved = micropather::SolveNear(luaL_checknumber(L, 3));
  pathSize = micropather::GetNearsSize();

  // Early exit if only found itself
  if (pathSize == 1) {
    pathSolved = micropather::NO_SOLUTION;
    pathSize = 0;
  }

  lua_pushinteger(L, pathSolved);
  lua_pushinteger(L, pathSize);

  if (pathSize > 1) {

    i++;
    lua_createtable(L, pathSize, 0);
    int newTable = lua_gettop(L);

    for (int ii = 0; ii < pathSize; ii++) {

      micropather::NodeToXY(micropather::nears[ii].state, &pathX, &pathY);

      lua_createtable(L, 2, 0);
      lua_pushstring(L, "x");
      lua_pushinteger(L, pathX + zero);
      lua_settable(L, -3);
      lua_pushstring(L, "y");
      lua_pushinteger(L, pathY + zero);
      lua_settable(L, -3);
      lua_pushstring(L, "cost");
      lua_pushnumber(L, micropather::nears[ii].cost);
      lua_settable(L, -3);
      lua_pushstring(L, "id");
      lua_pushinteger(L, micropather::WorldAt(pathX, pathY));
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
