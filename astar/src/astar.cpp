#define LIB_NAME "Astar"
#define MODULE_NAME "astar"
#define DLIB_LOG_DOMAIN "ASTAR"

#include <dmsdk/dlib/hashtable.h>
#include <dmsdk/sdk.h>
#include <pather.h>
#include <inttypes.h>

struct MapData
{
    micropather::Map map;
    float            pathTotalCost = 0.0f;
    bool             mapVFlip = false;
    uint8_t          zero = 1;
    uint8_t          pathSolved = 0;
    uint16_t         pathSize = 0;
    int16_t          pathX, pathY;
};

static const uint16_t                  defaultMapId = 0;
static uint16_t                        nextMapId = defaultMapId;
static dmHashTable<uint16_t, MapData*> maps;

//
static MapData* get_or_create_mapdata(uint16_t mapId)
{
    MapData** mapDataPtr = maps.Get(mapId);
    if (mapDataPtr == NULL)
    {
        if (maps.Full())
        {
            maps.SetCapacity(maps.Capacity() * 2, maps.Capacity() * 2); // for 1.9.7 sdk compability
        }
        MapData* mapData = new MapData();
        maps.Put(mapId, mapData);
        return mapData;
    }
    else
    {
        return *mapDataPtr;
    }
}

static uint16_t new_map_id()
{
    while (maps.Get(nextMapId) != NULL)
    {
        nextMapId++;
    }
    return nextMapId;
}

static MapData* get_map(lua_State* L, int nArg)
{
    const uint16_t _mapId = luaL_optinteger(L, nArg, defaultMapId);
    MapData**      mapData = maps.Get(_mapId);
    if (mapData == NULL)
    {
        dmLogError(
        "Map %" PRIu16
        " doesn't exist. You have to setup the map using astar.setup()\n",
        _mapId);
        return NULL;
    }
    return *mapData;
}

static void delete_map(uint16_t mapId)
{
    MapData** mapDataPtr = maps.Get(mapId);
    if (mapDataPtr != NULL)
    {
        MapData* mapData = *mapDataPtr;
        mapData->map.Clear();
        delete mapData;
        maps.Erase(mapId);
    }
}

static int astar_new_map_id(lua_State* L)
{
    uint16_t _mapId = new_map_id();
    lua_pushinteger(L, _mapId);
    return 1;
}

static int astar_delete_map(lua_State* L)
{
    uint16_t _mapId = luaL_checkinteger(L, 1);
    delete_map(_mapId);
    return 0;
}

static void use_zero(MapData* mapData, bool _zero)
{
    mapData->zero = (_zero) ? 0 : 1;
}

static int astar_setup(lua_State* L)
{
    uint16_t _worldWidth = luaL_checkint(L, 1);
    uint16_t _worldHeight = luaL_checkint(L, 2);
    uint8_t  _worldDirection = luaL_checkint(L, 3);
    uint16_t _allocate = luaL_checkint(L, 4);
    uint16_t _typicalAdjacent = luaL_checkint(L, 5);

    bool     _cache = true;
    if (lua_isboolean(L, 6))
    {
        _cache = lua_toboolean(L, 6);
    }

    bool _useZero = false;
    if (lua_isboolean(L, 7))
    {
        _useZero = lua_toboolean(L, 7);
    }

    bool _mapVFlip = false;
    if (lua_isboolean(L, 8))
    {
        bool _mapVFlip = lua_toboolean(L, 8);
    }

    uint16_t _mapId = luaL_optinteger(L, 9, defaultMapId);
    MapData* mapData = get_or_create_mapdata(_mapId);

    mapData->mapVFlip = _mapVFlip;

    use_zero(mapData, _useZero);

    mapData->map.Setup(_worldWidth, _worldHeight, _worldDirection, _allocate, _typicalAdjacent, _cache);

    return 0;
}

static int astar_map_vflip(lua_State* L)
{
    MapData* mapData = get_map(L, 1);
    if (mapData == NULL)
    {
        return 0;
    }
    mapData->map.MapVFlip();
    return 0;
}

static int astar_map_hflip(lua_State* L)
{
    MapData* mapData = get_map(L, 1);
    if (mapData == NULL)
    {
        return 0;
    }
    mapData->map.MapHFlip();
    return 0;
}

static int astar_set_map_type(lua_State* L)
{
    uint8_t  _mapType = luaL_checkint(L, 1);
    MapData* mapData = get_map(L, 2);
    if (mapData == NULL)
    {
        return 0;
    }
    mapData->map.SetMapType(_mapType);
    return 0;
}

static int astar_set_map(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    MapData* mapData = get_map(L, 2);
    if (mapData == NULL)
    {
        return 0;
    }

    for (int i = 0; i < mapData->map.GetWorldSize(); i++)
    {
        lua_pushinteger(L, i + 1);
        lua_gettable(L, 1);
        if (lua_isnumber(L, -1))
        {
            mapData->map.SetTile(i, lua_tointeger(L, -1));
        }
        lua_pop(L, 1);
    }

    if (mapData->mapVFlip)
    {
        mapData->map.MapVFlip();
    }

    return 0;
}

static int astar_print_map(lua_State* L)
{
    MapData* mapData = get_map(L, 1);
    if (mapData == NULL)
    {
        return 0;
    }
    mapData->map.PrintMap(mapData->zero);
    return 0;
}

static int astar_use_entities(lua_State* L)
{
    bool     _useEntities = lua_toboolean(L, 1);
    MapData* mapData = get_map(L, 2);
    if (mapData == NULL)
    {
        return 0;
    }
    mapData->map.UseEntities(_useEntities);
    return 0;
}

static int astar_use_zero(lua_State* L)
{
    bool     _zero = lua_toboolean(L, 1);
    MapData* mapData = get_map(L, 2);
    if (mapData == NULL)
    {
        return 0;
    }
    use_zero(mapData, _zero);
    return 0;
}

static int astar_set_entities(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TTABLE);

    size_t size = lua_objlen(L, 1);
    if (size == 0)
    {
        dmLogError("Entities table can not be empty");
    }

    MapData* mapData = get_map(L, 2);
    if (mapData == NULL)
    {
        return 0;
    }

    mapData->map.SetEntityCount(size);

    for (int i = 0; i < size; i++)
    {
        lua_pushinteger(L, i + 1);
        lua_gettable(L, 1);
        if (lua_isnumber(L, -1))
        {
            mapData->map.SetEntity(i, lua_tointeger(L, -1));
        }
        lua_pop(L, 1);
    }

    return 0;
}

static int astar_set_costs(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TTABLE);

    MapData* mapData = get_map(L, 2);
    if (mapData == NULL)
    {
        return 0;
    }

    mapData->map.ResizePath();
    mapData->map.ResetPath();

    uint16_t tileCount = 0;
    uint16_t id = 0;
    uint16_t costID = 0;

    lua_pushnil(L);
    while (lua_next(L, 1) != 0)
    {
        tileCount++;
        lua_pop(L, 1);
    }

    mapData->map.SetTileCount(tileCount);
    mapData->map.SetCosts();

    lua_pushnil(L);
    while (lua_next(L, 1) != 0)
    {
        mapData->map.AddCostTileID(id, lua_tointeger(L, -2));

        if (lua_istable(L, -1))
        {
            costID = 0;
            lua_pushnil(L);

            while (lua_next(L, -2) != 0)
            {
                mapData->map.AddCost(id, costID, lua_tonumber(L, -1));
                lua_pop(L, 1);
                costID++;

                if (costID > mapData->map.GetWordDirection())
                {
                    dmLogError(
                    "There are more costs than direction. Cost Count: %i, "
                    "Direction: %i",
                    costID,
                    mapData->map.GetWordDirection());
                    return 0;
                }
            }
        }
        lua_pop(L, 1);
        id++;
    }

    return 0;
}

static int astar_reset_cache(lua_State* L)
{
    MapData* mapData = get_map(L, 1);
    if (mapData == NULL)
    {
        return 0;
    }
    mapData->map.ResetPath();
    return 0;
}

static int astar_resize_path(lua_State* L)
{
    MapData* mapData = get_map(L, 1);
    if (mapData == NULL)
    {
        return 0;
    }
    mapData->map.ResizePath();
    return 0;
}

static void clearMaps(void* context, const uint16_t* key, MapData** mapDataPtr)
{
    if (mapDataPtr != NULL)
    {
        MapData* mapData = *mapDataPtr;

        mapData->map.Clear();
        delete mapData;
        *mapDataPtr = NULL;
    }
}

static int astar_reset(lua_State* L)
{
    maps.Iterate(&clearMaps, (void*)NULL);
    maps.Clear();

    return 0;
}

static int astar_get_at(lua_State* L)
{
    int16_t  pathX = luaL_checkint(L, 1);
    int16_t  pathY = luaL_checkint(L, 2);
    MapData* mapData = get_map(L, 3);
    if (mapData == NULL)
    {
        return 0;
    }
    mapData->pathX = pathX - mapData->zero;
    mapData->pathY = pathY - mapData->zero;
    lua_pushinteger(L, mapData->map.WorldAt(mapData->pathX, mapData->pathY));
    return 1;
}

static int astar_set_at(lua_State* L)
{
    int16_t  pathX = luaL_checkint(L, 1);
    int16_t  pathY = luaL_checkint(L, 2);
    int      value = luaL_checkint(L, 3);
    MapData* mapData = get_map(L, 4);
    if (mapData == NULL)
    {
        return 0;
    }
    mapData->pathX = pathX - mapData->zero;
    mapData->pathY = pathY - mapData->zero;
    mapData->map.SetToWorldAt(mapData->pathX, mapData->pathY, value);
    return 0;
}

static int astar_solve(lua_State* L)
{
    uint8_t  i = 3;

    int16_t  fromX = luaL_checkint(L, 1);
    int16_t  fromY = luaL_checkint(L, 2);
    int16_t  toX = luaL_checkint(L, 3);
    int16_t  toY = luaL_checkint(L, 4);
    MapData* mapData = get_map(L, 5);
    if (mapData == NULL)
    {
        return 0;
    }

    mapData->map.SetPathFromTo(fromX - mapData->zero, fromY - mapData->zero, toX - mapData->zero, toY - mapData->zero);

    mapData->pathSolved = mapData->map.Solve();
    mapData->pathSize = mapData->map.GetPathSize();
    mapData->pathTotalCost = mapData->map.GetTotalCost();

    // Early exit if only found itself
    if (mapData->pathSize == 1)
    {
        mapData->pathSolved = micropather::NO_SOLUTION;
        mapData->pathSize = 0;
    }

    lua_pushinteger(L, mapData->pathSolved);
    lua_pushinteger(L, mapData->pathSize);
    lua_pushnumber(L, mapData->pathTotalCost);

    if (mapData->pathSize > 1)
    {
        i++;
        lua_createtable(L, mapData->pathSize, 0);
        int newTable = lua_gettop(L);

        for (int ii = 0; ii < mapData->pathSize; ii++)
        {
            mapData->map.NodeToXY(mapData->map.path[ii], &mapData->pathX, &mapData->pathY);

            lua_createtable(L, 2, 0);
            lua_pushstring(L, "x");
            lua_pushinteger(L, mapData->pathX + mapData->zero);
            lua_settable(L, -3);
            lua_pushstring(L, "y");
            lua_pushinteger(L, mapData->pathY + mapData->zero);
            lua_settable(L, -3);
            lua_pushstring(L, "id");
            lua_pushinteger(L, mapData->map.WorldAt(mapData->pathX, mapData->pathY));
            lua_settable(L, -3);

            lua_rawseti(L, newTable, ii + 1);
        }
    }

    return i;
}

static int astar_solve_near(lua_State* L)
{
    uint8_t  i = 2;

    int16_t  x = luaL_checkint(L, 1);
    int16_t  y = luaL_checkint(L, 2);
    float    maxCost = luaL_checknumber(L, 3);
    MapData* mapData = get_map(L, 4);
    if (mapData == NULL)
    {
        return 0;
    }

    mapData->map.SetPathFrom(x - mapData->zero, y - mapData->zero);

    mapData->pathSolved = mapData->map.SolveNear(maxCost);
    mapData->pathSize = mapData->map.GetNearsSize();

    // Early exit if only found itself
    if (mapData->pathSize == 1)
    {
        mapData->pathSolved = micropather::NO_SOLUTION;
        mapData->pathSize = 0;
    }

    lua_pushinteger(L, mapData->pathSolved);
    lua_pushinteger(L, mapData->pathSize);

    if (mapData->pathSize > 1)
    {
        i++;
        lua_createtable(L, mapData->pathSize, 0);
        int newTable = lua_gettop(L);

        for (int ii = 0; ii < mapData->pathSize; ii++)
        {
            mapData->map.NodeToXY(mapData->map.nears[ii].state, &mapData->pathX, &mapData->pathY);

            lua_createtable(L, 2, 0);
            lua_pushstring(L, "x");
            lua_pushinteger(L, mapData->pathX + mapData->zero);
            lua_settable(L, -3);
            lua_pushstring(L, "y");
            lua_pushinteger(L, mapData->pathY + mapData->zero);
            lua_settable(L, -3);
            lua_pushstring(L, "cost");
            lua_pushnumber(L, mapData->map.nears[ii].cost);
            lua_settable(L, -3);
            lua_pushstring(L, "id");
            lua_pushinteger(L, mapData->map.WorldAt(mapData->pathX, mapData->pathY));
            lua_settable(L, -3);

            lua_rawseti(L, newTable, ii + 1);
        }
    }

    return i;
}

static const luaL_reg Module_methods[] = { { "solve_near", astar_solve_near },
                                           { "solve", astar_solve },
                                           { "reset", astar_reset },
                                           { "clear_path", astar_resize_path },
                                           { "reset_cache", astar_reset_cache },
                                           { "set_costs", astar_set_costs },
                                           { "set_map_type", astar_set_map_type },
                                           { "set_map", astar_set_map },
                                           { "setup", astar_setup },
                                           { "get_at", astar_get_at },
                                           { "set_at", astar_set_at },
                                           { "use_zero", astar_use_zero },
                                           { "set_entities", astar_set_entities },
                                           { "use_entities", astar_use_entities },
                                           { "print_map", astar_print_map },
                                           { "map_vflip", astar_map_vflip },
                                           { "map_hflip", astar_map_hflip },
                                           { "new_map_id", astar_new_map_id },
                                           { "delete_map", astar_delete_map },
                                           { 0, 0 }

};

static void LuaInit(lua_State* L)
{
    int top = lua_gettop(L);

    // Register lua names
    luaL_register(L, MODULE_NAME, Module_methods);

#define SETCONSTANT(name) \
    lua_pushnumber(L, (lua_Number)micropather::name); \
    lua_setfield(L, -2, #name);

    SETCONSTANT(SOLVED);
    SETCONSTANT(NO_SOLUTION);
    SETCONSTANT(START_END_SAME);
#undef SETCONSTANT

#define SETCONSTANT(name) \
    lua_pushnumber(L, (lua_Number)micropather::name); \
    lua_setfield(L, -2, #name);

    SETCONSTANT(DIRECTION_FOUR);
    SETCONSTANT(DIRECTION_EIGHT);
#undef SETCONSTANT

#define SETCONSTANT(name) \
    lua_pushnumber(L, (lua_Number)micropather::name); \
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

dmExtension::Result AppInitializeAstar(dmExtension::AppParams* params)
{
    maps.SetCapacity(1, 1); // for 1.9.7 sdk compability
    return dmExtension::RESULT_OK;
}

dmExtension::Result AppFinalizeAstar(dmExtension::AppParams* params)
{
    maps.Iterate(&clearMaps, (void*)NULL);
    maps.Clear();
    return dmExtension::RESULT_OK;
}

dmExtension::Result InitializeAstar(dmExtension::Params* params)
{
    LuaInit(params->m_L);
    printf("Registered %s Extension\n", MODULE_NAME);
    return dmExtension::RESULT_OK;
}

DM_DECLARE_EXTENSION(astar, LIB_NAME, AppInitializeAstar, 0, InitializeAstar, 0, 0, 0)
