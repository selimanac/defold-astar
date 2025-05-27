#include "dmsdk/dlib/log.h"
#include <cstdio>
#include <cstring>
#include <pather.h>

namespace micropather
{
    uint8_t Map::GetWordDirection()
    {
        return worldDirection;
    }

    uint16_t Map::GetWorldSize()
    {
        return worldSize;
    }

    uint16_t Map::GetPathSize()
    {
        return path.size();
    }

    uint16_t Map::GetNearsSize()
    {
        return nears.size();
    }

    float Map::GetTotalCost()
    {
        return totalCost;
    }

    void Map::SetTileCount(uint16_t _tileCount)
    {
        tileCount = _tileCount;
    }

    void Map::SetWorldSize(uint16_t _worldSize)
    {
        worldSize = _worldSize;
    }

    void Map::SetMapType(uint8_t type)
    {
        mapType = type;
    }

    void Map::SetPathFrom(int16_t x, int16_t y)
    {
        pathFrom.x = x;
        pathFrom.y = y;
    }

    void Map::SetPathTo(int16_t x, int16_t y)
    {
        pathTo.x = x;
        pathTo.y = y;
    }

    void Map::SetPathFromTo(int16_t from_x, int16_t from_y, int16_t to_x, int16_t to_y)
    {
        SetPathFrom(from_x, from_y);
        SetPathTo(to_x, to_y);
    }

    void Map::Setup(uint16_t _worldWidth, uint16_t _worldHeight, uint8_t _worldDirection, uint16_t _allocate, uint16_t _typicalAdjacent, bool _cache)
    {
        if (pather != NULL)
        {
            Clear();
            pather = NULL;
        }

        worldWidth = _worldWidth;
        worldHeight = _worldHeight;
        worldDirection = _worldDirection;

        SetWorldSize(worldWidth * worldHeight);

        world = (uint16_t*)malloc(sizeof(uint16_t) * worldSize);

        memset(world, 0, sizeof(uint16_t) * worldSize); // Set all to 0

        pather = new MicroPather(this, _allocate, _typicalAdjacent, _cache);
    }

    void Map::SetCosts()
    {
        ResizePath();
        ResetPath();

        Costs = (Tile*)malloc(sizeof(Tile) * tileCount);

        for (int i = 0; i < tileCount; ++i)
        {
            Costs[i].costs = (float*)malloc(sizeof(float) * worldDirection);
            Costs[i].tile_id = 0;
        }
    }

    void Map::AddCostTileID(uint16_t id, uint16_t tileID)
    {
        Costs[id].tile_id = tileID;
    }

    void Map::AddCost(uint16_t id, uint16_t costID, float cost)
    {
        Costs[id].costs[costID] = cost;
    }

    // NOT USING ANYMORE
    void Map::SetMap(int* _world)
    {
        ResetPath();
        memcpy(world, _world, sizeof(uint16_t) * GetWorldSize());
    }

    void Map::SetTile(uint16_t id, uint16_t tile)
    {
        world[id] = tile;
    }

    void Map::MapVFlip()
    {
        for (uint16_t y = 0; y < worldHeight / 2; ++y)
        {
            for (uint16_t x = 0; x < worldWidth; ++x)
            {
                uint16_t temp = world[y * worldWidth + x];
                world[y * worldWidth + x] = world[(worldHeight - y - 1) * worldWidth + x];
                world[(worldHeight - y - 1) * worldWidth + x] = temp;
            }
        }
    }

    void Map::MapHFlip()
    {
        for (uint16_t i = 0; i < worldHeight; ++i)
        {
            for (uint16_t j = 0; j < worldWidth / 2; ++j)
            {
                uint16_t temp = world[i * worldWidth + j];
                world[i * worldWidth + j] = world[i * worldWidth + (worldWidth - j - 1)];
                world[i * worldWidth + (worldWidth - j - 1)] = temp;
            }
        }
    }

    void Map::UseEntities(bool toggle)
    {
        ResetPath();
        getEntity = toggle;
    }

    void Map::PrintMap(uint8_t zero)
    {
        int row = 0 + zero;

        printf("cols/x:\n");
        for (int16_t col = zero; col < worldWidth + zero; ++col)
        {
            printf("%i\t\t", col);
        }
        printf("\n");
        for (int16_t col = 0; col < worldWidth; ++col)
        {
            printf("-\t\t");
        }
        printf("\n");
        for (int16_t y = 0; y < worldHeight; y++)
        {
            for (int16_t x = 0; x < worldWidth; x++)
            {
                printf("%u\t\t", world[y * worldWidth + x]);
            }

            printf("\t- row/y: %i\n", row);
            row++;
        }
    }

    void Map::SetEntityCount(size_t size)
    {
        entitiesSize = size;
        entities = (uint8_t*)malloc(sizeof(uint8_t) * entitiesSize);
    }
    void Map::SetEntity(uint8_t id, int16_t entityID)
    {
        entities[id] = entityID;
    }

    // NOT USING ANYMORE
    void Map::SetEntities(uint8_t* _entities, size_t size)
    {
        ResetPath();

        entitiesSize = size;
        entities = (uint8_t*)malloc(sizeof(uint8_t) * size);

        memcpy(entities, _entities, sizeof(uint8_t) * size);
    }

    void Map::ResizePath()
    {
        path.resize(0);
    }

    void Map::ResetPath()
    {
        if (pather != NULL)
        {
            pather->Reset();
        }
    }

    int Map::WorldAt(int16_t x, int16_t y)
    {
        if (x >= 0 && x < worldWidth && y >= 0 && y < worldHeight)
        {
            return world[y * worldWidth + x];
        }
        else
        {
            return -1;
        }
    }

    void Map::SetToWorldAt(int16_t x, int16_t y, uint16_t value)
    {
        if (x >= 0 && x < worldWidth && y >= 0 && y < worldHeight)
        {
            ResetPath();
            world[y * worldWidth + x] = value;
        }
    }

    void Map::NodeToXY(void* node, int16_t* x, int16_t* y)
    {
        intptr_t index = (intptr_t)node;
        *y = (int)index / worldWidth;
        *x = (int)index - *y * worldWidth;
    }

    void* Map::XYToNode(size_t x, size_t y)
    {
        return (void*)(y * worldWidth + x);
    }

    float Map::LeastCostEstimate(void* nodeStart, void* nodeEnd)
    {
        int16_t xStart, yStart, xEnd, yEnd;
        NodeToXY(nodeStart, &xStart, &yStart);
        NodeToXY(nodeEnd, &xEnd, &yEnd);

        int nx = abs(xStart - xEnd);
        int ny = abs(yStart - yEnd);

        if (worldDirection == DIRECTION_FOUR)
        {
            // Manhattan distance
            return (nx + ny);
        }
        else if (worldDirection == DIRECTION_EIGHT)
        {
            // Euclidean distance
            return (float)sqrt((double)(nx * nx) + (double)(ny * ny));
        }
        return 0.0f;
    }

    int Map::Passable(int16_t nx, int16_t ny)
    {
        if (nx >= 0 && nx < worldWidth && ny >= 0 && ny < worldHeight)
        {
            int      index = ny * worldWidth + nx;
            uint16_t c = world[index];

            if (getEntity)
            {
                for (int i = 0; i < entitiesSize; i++)
                {
                    if (c == entities[i])
                    {
                        return c;
                    }
                }
            }

            for (unsigned int a = 0; a < tileCount; a++)
            {
                if (Costs[a].tile_id == c)
                {
                    return c;
                }
            }
        }
        return -1;
    }

    void Map::PushNeighbors(StateCost* nodeCost, MPVector<StateCost>* neighbors, int16_t* nx, int16_t* ny, unsigned int a, unsigned int b)
    {
        nodeCost->state = XYToNode(*nx, *ny);
        nodeCost->cost = Costs[a].costs[b];
        neighbors->push_back(*nodeCost);
    }

    inline void GetNextCoordinate(uint8_t mapType, int16_t x, int16_t y, uint8_t direction, bool isOddX, bool isOddY, int16_t* nx, int16_t* ny)
    {
        if (mapType == GRID_CLASSIC)
        {
            *nx = x + dx[direction];
            *ny = y + dy[direction];
        }
        else if (mapType == HEX_ODDR || mapType == HEX_EVENR)
        {
            int idx = !isOddY ? 0 : 2;
            *nx = x + OffsetCoordinates[mapType][idx][direction];
            *ny = y + OffsetCoordinates[mapType][idx + 1][direction];
        }
        else if (mapType == HEX_ODDQ || mapType == HEX_EVENQ)
        {
            int idx = !isOddX ? 0 : 2;
            *nx = x + OffsetCoordinates[mapType][idx][direction];
            *ny = y + OffsetCoordinates[mapType][idx + 1][direction];
        }
    }

    void Map::AdjacentCost(void* node, MPVector<StateCost>* neighbors)
    {
        int16_t x, y, nx, ny, pass;
        NodeToXY(node, &x, &y);
        StateCost nodeCost;

        bool      isOddY = (y % 2 != 0);
        bool      isOddX = (x % 2 != 0);

        for (unsigned int a = 0; a < tileCount; ++a)
        {
            for (unsigned int b = 0; b < worldDirection; ++b)
            {
                GetNextCoordinate(mapType, x, y, b, isOddX, isOddY, &nx, &ny);

                pass = Passable(nx, ny);

                if (getEntity || getNearEntities)
                {
                    for (int i = 0; i < entitiesSize; i++)
                    {
                        if (pass == entities[i])
                        {
                            if (WorldAt(nx, ny) >= 0)
                            {
                                if (XYToNode(nx, ny) == XYToNode(pathTo.x, pathTo.y) &&
                                    getEntity && getNearEntities == false)
                                {
                                    PushNeighbors(&nodeCost, neighbors, &nx, &ny, a, b);
                                }
                                else if (getNearEntities)
                                {
                                    PushNeighbors(&nodeCost, neighbors, &nx, &ny, a, b);
                                }
                            }
                        }
                    }
                }

                if (pass == Costs[a].tile_id)
                {
                    if (WorldAt(nx, ny) >= 0)
                    {
                        PushNeighbors(&nodeCost, neighbors, &nx, &ny, a, b);
                    }
                }
            }
        }
    }

    int Map::Solve()
    {
        if (pather == NULL)
        {
            dmLogError("You have to setup the astar using astar.setup()\n");
            return NO_SOLUTION;
        }
        if (Costs == NULL)
        {
            dmLogError(
            "COSTS table is empty: Please set costs by using "
            "astar.set_costs(costs)\n");
            return NO_SOLUTION;
        }

        // Early exit if start/end x-y out of the map bounds
        if (WorldAt(pathTo.x, pathTo.y) < 0 || WorldAt(pathFrom.x, pathFrom.y) < 0)
        {
            ResizePath();
            result = NO_SOLUTION;
            return result;
        }

        result = pather->Solve(XYToNode(pathFrom.x, pathFrom.y),
                               XYToNode(pathTo.x, pathTo.y),
                               &path,
                               &totalCost);

        return result;
    }

    int Map::SolveNear(float maxCost)
    {
        if (pather == NULL)
        {
            dmLogError("You have to setup the astar using astar.setup()\n");
            return NO_SOLUTION;
        }

        if (Costs == NULL)
        {
            dmLogError(
            "COSTS table is empty: Please set costs by using  "
            "astar.set_costs(costs)\n");
            return NO_SOLUTION;
        }

        // Early exit if x-y out of the map bounds
        if (WorldAt(pathFrom.x, pathFrom.y) < 0)
        {
            ResizePath();
            result = NO_SOLUTION;
            return result;
        }

        getNearEntities = getEntity;
        result = pather->SolveForNearStates(XYToNode(pathFrom.x, pathFrom.y), &nears, maxCost);
        getNearEntities = false;
        return result;
    }

    void Map::Clear()
    {
        ResetPath();
        if (Costs != NULL)
        {
            for (unsigned int a = 0; a < tileCount; a = a + 1)
            {
                free(Costs[a].costs);
            }
        }

        free(Costs);
        free(entities);
        free(world);

        if (pather != NULL)
        {
            delete pather;
            pather = NULL;
        }
    }

    void Map::PrintStateInfo(void* state) {};
} // namespace micropather