#define LIB_NAME "Astar"
#define MODULE_NAME "astar"
#define DLIB_LOG_DOMAIN "ASTAR"
#include <Map.hpp>
#include <dmsdk/sdk.h>
#include <dmsdk/dlib/log.h>
#include <stdio.h>
#include <cstdlib>
//test
Map map;
int x, y;
float maxCost;
unsigned size, pathResult;

static int astar_setup(lua_State *L)
{
    int _worldWidth = luaL_checkint(L, 1);
    int _worldHeight = luaL_checkint(L, 2);
    int _worldDirection = luaL_checkint(L, 3);
    int _allocate = luaL_checkint(L, 4);
    int _typicalAdjacent = luaL_checkint(L, 5);
    bool _cache = lua_toboolean(L, 6);

    map.Setup(_worldWidth, _worldHeight, _worldDirection, _allocate, _typicalAdjacent, _cache);

    return 0;
}

static int astar_setmap(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TTABLE);

    size = map.worldWidth * map.worldHeight;
    int _world[size];
    for (int i = 0; i < size; i++)
    {
        lua_pushinteger(L, i + 1);
        lua_gettable(L, -2);
        if (lua_isnumber(L, -1))
        {
            _world[i] = lua_tointeger(L, -1);
        }
        lua_pop(L, 1);
    }
    map.SetMap(_world);

    return 0;
}

static int astar_setcosts(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    map.ClearPath();
    map.ResetPath();

    int tile_count = 0;
    int id = 0;
    int cost_id = 0;

    lua_pushnil(L);
    while (lua_next(L, 1) != 0)
    {
        tile_count++;
        lua_pop(L, 1);
    }

    map.tileCount = tile_count;
    map.Costs = (Tile *)malloc(sizeof(Tile) * map.tileCount);

    lua_pushnil(L);
    while (lua_next(L, 1) != 0)
    {

        map.Costs[id].tile_id = lua_tointeger(L, -2);

        if (lua_istable(L, -1))
        {
            cost_id = 0;
            map.Costs[id].costs = (float *)malloc(sizeof(float) * map.worldDirection);

            lua_pushnil(L);
            while (lua_next(L, -2) != 0)
            {

                map.Costs[id].costs[cost_id] = lua_tonumber(L, -1);
                lua_pop(L, 1);
                cost_id++;

                if (cost_id > map.worldDirection)
                {
                    dmLogError("There are more costs than direction. Cost Count: %i, Direction: %i", cost_id, map.worldDirection);
                    return 0;
                }
            }
        }
        lua_pop(L, 1);
        id++;
    }
    return 0;
}

static int astar_resetcache(lua_State *L)
{
    map.ResetPath();
    return 0;
}

static int astar_clearpath(lua_State *L)
{
    map.ClearPath();
    return 0;
}

static int astar_reset(lua_State *L)
{
    map.Clear();
    return 0;
}


static int astar_solve(lua_State *L)
{
    int i = 3;
    map.pathFrom.x = luaL_checkint(L, 1) - 1;
    map.pathFrom.y = luaL_checkint(L, 2) - 1;
    map.pathTo.x = luaL_checkint(L, 3) - 1;
    map.pathTo.y = luaL_checkint(L, 4) - 1;

    pathResult = map.Solve();
    
    size = map.path.size();

     // Early exit if only found itself
    if (size == 1)
    {
        pathResult = Map::NO_SOLUTION;
        size = 0;
    }

    lua_pushinteger(L, pathResult);
    lua_pushinteger(L, size);
    lua_pushnumber(L, map.totalCost);

    if (size > 1)
    {
        i++;
        lua_createtable(L, size, 0);
        int newTable = lua_gettop(L);
        for (int ii = 0; ii < size; ii++)
        {
            map.NodeToXY(map.path[ii], &x, &y);

            lua_createtable(L, 2, 0);
            lua_pushstring(L, "x");
            lua_pushinteger(L, x + 1);
            lua_settable(L, -3);
            lua_pushstring(L, "y");
            lua_pushinteger(L, y + 1);
            lua_settable(L, -3);

            lua_rawseti(L, newTable, ii + 1);
        }
    }

    return i;
}

static int astar_solvenear(lua_State *L)
{
    int i = 2;
    map.pathFrom.x = luaL_checkint(L, 1);
    map.pathFrom.y = luaL_checkint(L, 2);
    maxCost = luaL_checknumber(L, 3);

    pathResult = map.SolveNear(maxCost);

    size = map.nears.size();

    // Early exit if only found itself
    if (size == 1)
    {
        pathResult = Map::NO_SOLUTION;
        size = 0;
    }

    lua_pushinteger(L, pathResult);
    lua_pushinteger(L, size);

    if (size > 1)
    {
        i++;
        lua_createtable(L, size, 0);
        int newTable = lua_gettop(L);
        for (int ii = 0; ii < size; ii++)
        {
            map.NodeToXY(map.nears[ii].state, &x, &y);

            lua_createtable(L, 2, 0);
            lua_pushstring(L, "x");
            lua_pushinteger(L, x);
            lua_settable(L, -3);
            lua_pushstring(L, "y");
            lua_pushinteger(L, y);
            lua_settable(L, -3);
            lua_pushstring(L, "cost");
            lua_pushnumber(L, map.nears[ii].cost);
            lua_settable(L, -3);

            lua_rawseti(L, newTable, ii + 1);
        }
    }
    return i;
}

static const luaL_reg Module_methods[] =
    {
        {"solve_near", astar_solvenear},
        {"solve", astar_solve},
        {"reset", astar_reset},
        {"clear_path", astar_clearpath},
        {"reset_cache", astar_resetcache},
        {"set_costs", astar_setcosts},
        {"set_map", astar_setmap},
        {"setup", astar_setup},
        {0, 0}

};

static void LuaInit(lua_State *L)
{
    int top = lua_gettop(L);

    // Register lua names
    luaL_register(L, MODULE_NAME, Module_methods);

#define SETCONSTANT(name)                     \
    lua_pushnumber(L, (lua_Number)Map::name); \
    lua_setfield(L, -2, #name);

    SETCONSTANT(SOLVED);
    SETCONSTANT(NO_SOLUTION);
    SETCONSTANT(START_END_SAME);
#undef SETCONSTANT

#define SETCONSTANT(name)                     \
    lua_pushnumber(L, (lua_Number)Map::name); \
    lua_setfield(L, -2, #name);

    SETCONSTANT(DIRECTION_FOUR);
    SETCONSTANT(DIRECTION_EIGHT);
#undef SETCONSTANT

    lua_pop(L, 1);
    assert(top == lua_gettop(L));
}

dmExtension::Result AppInitializeAstar(dmExtension::AppParams *params)
{
    return dmExtension::RESULT_OK;
}

dmExtension::Result InitializeAstar(dmExtension::Params *params)
{
    // Init Lua
    LuaInit(params->m_L);
    printf("Registered %s Extension\n", MODULE_NAME);
    return dmExtension::RESULT_OK;
}

dmExtension::Result AppFinalizeAstar(dmExtension::AppParams *params)
{
    return dmExtension::RESULT_OK;
}

dmExtension::Result FinalizeAstar(dmExtension::Params *params)
{
    return dmExtension::RESULT_OK;
}

DM_DECLARE_EXTENSION(astar, LIB_NAME, AppInitializeAstar, AppFinalizeAstar, InitializeAstar, 0, 0, FinalizeAstar)
