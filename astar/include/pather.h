#pragma once

#include <math.h>
#include <micropather/micropather.h>

namespace micropather
{
    struct Position
    {
        int16_t x;
        int16_t y;
    };

    struct Tile
    {
        uint16_t tile_id;
        float*   costs;
    };

    enum Direction
    {
        DIRECTION_FOUR = 4,
        DIRECTION_EIGHT = 8
    };

    enum MapType
    {
        HEX_ODDR = 0,
        HEX_EVENR = 1,
        HEX_ODDQ = 2,
        HEX_EVENQ = 3,
        GRID_CLASSIC = 5
    };

    // GRID_CLASSIC
    const int dx[8] = {
        1,  // E
        0,  // N
        -1, //  W
        0,  // S
        1,  // NE
        -1, // NW
        -1, // SW
        1   // SE
    };
    const int dy[8] = {
        0,  // E
        1,  // N
        0,  //  W
        -1, // S
        1,  // NE
        1,  // NW
        -1, // SW
        -1  // SE
    };

    class Map : Graph
    {
        public:
        MicroPather*        pather;
        uint8_t             result;
        uint16_t            worldWidth, worldHeight, tileCount, worldSize;
        uint8_t             worldDirection = 8;
        uint8_t             typicalAdjacent = 6;
        uint8_t             mapType = GRID_CLASSIC;

        uint16_t*           world;
        float               totalCost;
        bool                cache = true;

        Position            pathFrom = { 0, 0 };
        Position            pathTo = { 0, 0 };
        MPVector<void*>     path;  // extern
        MPVector<StateCost> nears; // extern
        Tile*               Costs;

        // Entities
        size_t   entitiesSize;
        uint8_t* entities;
        bool     getEntity = false;
        bool     getNearEntities = false;

        void     Initialize();

        void     Setup(uint16_t _worldWidth, uint16_t _worldHeight, uint8_t _worldDirection = 8, uint16_t _allocate = 250, uint16_t _typicalAdjacent = 6, bool _cache = true);

        // Helper Get/Set
        uint8_t  GetWordDirection();
        uint16_t GetWorldSize();

        uint16_t GetPathSize();
        uint16_t GetNearsSize();
        float    GetTotalCost();

        void     SetTileCount(uint16_t _tileCount);
        void     SetWorldSize(uint16_t _worldSize);
        void     SetPathFrom(int16_t x, int16_t y);
        void     SetPathTo(int16_t x, int16_t y);
        void     SetPathFromTo(int16_t from_x, int16_t from_y, int16_t to_x, int16_t to_y);

        void     SetCosts();
        void     AddCostTileID(uint16_t id, uint16_t tileID);
        void     AddCost(uint16_t id, uint16_t costID, float cost);
        void     SetMapType(uint8_t type);
        void     SetMap(int* _world); // NOT USING ANYMORE
        void     MapVFlip();
        void     MapHFlip();
        void     PrintMap(uint8_t zero);
        void     SetTile(uint16_t id, uint16_t tile);
        void     SetEntities(uint8_t* _entities, size_t size); // NOT USING ANYMORE
        void     SetEntityCount(size_t size);
        void     SetEntity(uint8_t id, int16_t entityID);
        void     UseEntities(bool toggle);

        int      WorldAt(int16_t x, int16_t y);
        void     SetToWorldAt(int16_t x, int16_t y, uint16_t value);
        void     NodeToXY(void* node, int16_t* x, int16_t* y);
        void*    XYToNode(size_t x, size_t y);
        void     PushNeighbors(StateCost* nodeCost, MPVector<StateCost>* neighbors, int16_t* nx, int16_t* ny, unsigned int a, unsigned int b);

        float    LeastCostEstimate(void* nodeStart, void* nodeEnd);
        void     AdjacentCost(void* node, MPVector<StateCost>* neighbors);
        void     PrintStateInfo(void* state);

        int      Passable(int16_t nx, int16_t ny);
        int      Solve();
        int      SolveNear(float maxCost);
        void     Clear();

        void     ResetPath();
        void     ResizePath();
    };
} // namespace micropather