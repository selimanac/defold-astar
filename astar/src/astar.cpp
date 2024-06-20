#define LIB_NAME "Astar"
#define MODULE_NAME "astar"
#define DLIB_LOG_DOMAIN "ASTAR"
#include <Map.hpp>
#include <cstdlib>
#include <dmsdk/dlib/log.h>
#include <dmsdk/sdk.h>
#include <stdio.h>
// test
Map map;
int x, y;
float maxCost;
unsigned size, pathResult;
unsigned short int zero = 0;
bool flipMap = false;

static int astar_setup(lua_State *L) {
  int _worldWidth = luaL_checkint(L, 1);
  int _worldHeight = luaL_checkint(L, 2);
  int _worldDirection = luaL_checkint(L, 3);
  int _allocate = luaL_checkint(L, 4);
  int _typicalAdjacent = luaL_checkint(L, 5);
  bool _cache = true;

  if (lua_isboolean(L, 6)) {
    _cache = lua_toboolean(L, 6);
  }

  if (lua_isboolean(L, 7)) {
    flipMap = lua_toboolean(L, 7);
  }

  map.Setup(_worldWidth, _worldHeight, _worldDirection, _allocate,
            _typicalAdjacent, _cache);
  return 0;
}

static int astar_toogle_zero(lua_State *L) {
  bool _zero = lua_toboolean(L, 1);
  (_zero) ? zero = 1 : zero = 0;
  return 0;
}

static int astar_set_entities(lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);

  size_t size = lua_objlen(L, 1);
  if (size == 0) {
    dmLogError("Entities table can not be empty");
  }

  int _entities[size];

  for (int i = 0; i < size; i++) {
    lua_pushinteger(L, i + 1);
    lua_gettable(L, -2);
    if (lua_isnumber(L, -1)) {
      _entities[i] = lua_tointeger(L, -1);
    }
    lua_pop(L, 1);
  }
  map.SetEntities(_entities, size);
  return 0;
}

static int astar_setmap(lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);

  size = map.worldWidth * map.worldHeight;
  int _world[size];

  for (int i = 0; i < size; i++) {
    lua_pushinteger(L, i + 1);
    lua_gettable(L, -2);
    if (lua_isnumber(L, -1)) {
      _world[i] = lua_tointeger(L, -1);
    }
    lua_pop(L, 1);
  }

  // FLIP TO MAP FOR DEFOLD COORD STYLE
  if (flipMap) {
    for (int y = 0; y < map.worldHeight / 2; ++y) {
      for (int x = 0; x < map.worldWidth; ++x) {
        int temp = _world[y * map.worldWidth + x];
        _world[y * map.worldWidth + x] =
            _world[(map.worldHeight - y - 1) * map.worldWidth + x];
        _world[(map.worldHeight - y - 1) * map.worldWidth + x] = temp;
      }
    }
  }

  /*
  //DEBUG MAP
  for (int i = 0; i < size; i++) {
    printf("%d - %d\n", i, _world[i]);
  }*/

  map.SetMap(_world);

  return 0;
}

static int astar_setcosts(lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  map.ClearPath();
  map.ResetPath();

  int tile_count = 0;
  int id = 0;
  int cost_id = 0;

  lua_pushnil(L);
  while (lua_next(L, 1) != 0) {
    tile_count++;
    lua_pop(L, 1);
  }

  map.tileCount = tile_count;
  map.Costs = (Tile *)malloc(sizeof(Tile) * map.tileCount);

  lua_pushnil(L);
  while (lua_next(L, 1) != 0) {

    map.Costs[id].tile_id = lua_tointeger(L, -2);

    if (lua_istable(L, -1)) {
      cost_id = 0;
      map.Costs[id].costs = (float *)malloc(sizeof(float) * map.worldDirection);

      lua_pushnil(L);
      while (lua_next(L, -2) != 0) {

        map.Costs[id].costs[cost_id] = lua_tonumber(L, -1);
        lua_pop(L, 1);
        cost_id++;

        if (cost_id > map.worldDirection) {
          dmLogError("There are more costs than direction. Cost Count: %i, "
                     "Direction: %i",
                     cost_id, map.worldDirection);
          return 0;
        }
      }
    }
    lua_pop(L, 1);
    id++;
  }
  return 0;
}

static int astar_resetcache(lua_State *L) {
  map.ResetPath();
  return 0;
}

static int astar_clearpath(lua_State *L) {
  map.ClearPath();
  return 0;
}

static int astar_reset(lua_State *L) {
  map.Clear();
  return 0;
}

static int astar_get_at(lua_State *L) {
  int x = luaL_checkint(L, 1) - zero;
  int y = luaL_checkint(L, 2) - zero;
  int ret = map.WorldAt(x, y);

  lua_pushinteger(L, ret);
  return 1;
}

static int astar_set_at(lua_State *L) {
  int x = luaL_checkint(L, 1) - zero;
  int y = luaL_checkint(L, 2) - zero;
  int value = luaL_checkint(L, 3);
  map.SetToWorldAt(x, y, value);
  return 0;
}

static int astar_solve(lua_State *L) {
  int i = 3;

  map.pathFrom.x = luaL_checkint(L, 1) - zero;
  map.pathFrom.y = luaL_checkint(L, 2) - zero;
  map.pathTo.x = luaL_checkint(L, 3) - zero;
  map.pathTo.y = luaL_checkint(L, 4) - zero;

  if (lua_isboolean(L, 5)) {
    map.getEntity = lua_toboolean(L, 5);
  }

  pathResult = map.Solve();

  size = map.path.size();

  // Early exit if only found itself
  if (size == 1) {
    pathResult = Map::NO_SOLUTION;
    size = 0;
  }

  lua_pushinteger(L, pathResult);
  lua_pushinteger(L, size);
  lua_pushnumber(L, map.totalCost);

  if (size > 1) {
    i++;
    lua_createtable(L, size, 0);
    int newTable = lua_gettop(L);
    for (int ii = 0; ii < size; ii++) {
      map.NodeToXY(map.path[ii], &x, &y);

      lua_createtable(L, 2, 0);
      lua_pushstring(L, "x");
      lua_pushinteger(L, x + zero);
      lua_settable(L, -3);
      lua_pushstring(L, "y");
      lua_pushinteger(L, y + zero);
      lua_settable(L, -3);

      lua_rawseti(L, newTable, ii + 1);
    }
  }

  map.getEntity = false;

  return i;
}

static int astar_solvenear(lua_State *L) {
  int i = 2;
  map.pathFrom.x = luaL_checkint(L, 1) - zero;
  map.pathFrom.y = luaL_checkint(L, 2) - zero;
  maxCost = luaL_checknumber(L, 3);

  pathResult = map.SolveNear(maxCost);

  size = map.nears.size();

  // Early exit if only found itself
  if (size == 1) {
    pathResult = Map::NO_SOLUTION;
    size = 0;
  }

  lua_pushinteger(L, pathResult);
  lua_pushinteger(L, size);

  if (size > 1) {
    i++;
    lua_createtable(L, size, 0);
    int newTable = lua_gettop(L);
    for (int ii = 0; ii < size; ii++) {
      map.NodeToXY(map.nears[ii].state, &x, &y);

      lua_createtable(L, 2, 0);
      lua_pushstring(L, "x");
      lua_pushinteger(L, x + zero);
      lua_settable(L, -3);
      lua_pushstring(L, "y");
      lua_pushinteger(L, y + zero);
      lua_settable(L, -3);
      lua_pushstring(L, "cost");
      lua_pushnumber(L, map.nears[ii].cost);
      lua_settable(L, -3);

      lua_rawseti(L, newTable, ii + 1);
    }
  }
  return i;
}

static const luaL_reg Module_methods[] = {{"solve_near", astar_solvenear},
                                          {"solve", astar_solve},
                                          {"reset", astar_reset},
                                          {"clear_path", astar_clearpath},
                                          {"reset_cache", astar_resetcache},
                                          {"set_costs", astar_setcosts},
                                          {"set_map", astar_setmap},
                                          {"setup", astar_setup},
                                          {"get_at", astar_get_at},
                                          {"set_at", astar_set_at},
                                          {"toogle_zero", astar_toogle_zero},
                                          {"set_entities", astar_set_entities},
                                          {0, 0}

};

static void LuaInit(lua_State *L) {
  int top = lua_gettop(L);

  // Register lua names
  luaL_register(L, MODULE_NAME, Module_methods);

#define SETCONSTANT(name)                                                      \
  lua_pushnumber(L, (lua_Number)Map::name);                                    \
  lua_setfield(L, -2, #name);

  SETCONSTANT(SOLVED);
  SETCONSTANT(NO_SOLUTION);
  SETCONSTANT(START_END_SAME);
#undef SETCONSTANT

#define SETCONSTANT(name)                                                      \
  lua_pushnumber(L, (lua_Number)Map::name);                                    \
  lua_setfield(L, -2, #name);

  SETCONSTANT(DIRECTION_FOUR);
  SETCONSTANT(DIRECTION_EIGHT);
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
