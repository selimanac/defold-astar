#include "Map.hpp"

Map::~Map()
{
    delete[] world;
    delete pather;
}

int Map::WorldAt(int x, int y)
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

void Map::ResetPath()
{
    if (pather != NULL)
    {
        pather->Reset();
    }
}

void Map::ClearPath()
{
    path.resize(0);
}

void Map::NodeToXY(void *node, int *x, int *y)
{
    intptr_t index = (intptr_t)node;
    *y = (int)index / worldWidth;
    *x = (int)index - *y * worldWidth;
}

void *Map::XYToNode(size_t x, size_t y)
{
    return (void *)(y * worldWidth + x);
}

int Map::Passable(int nx, int ny)
{
    if (nx >= 0 && nx < worldWidth && ny >= 0 && ny < worldHeight)
    {
        int index = ny * worldWidth + nx;
        int c = world[index];
        for (unsigned int a = 0; a < tileCount; a = a + 1)
        {
            if (Costs[a].tile_id == c)
            {
                return c;
            }
        }
    }
    return -1;
}

float Map::LeastCostEstimate(void *nodeStart, void *nodeEnd)
{
    int xStart, yStart, xEnd, yEnd;
    NodeToXY(nodeStart, &xStart, &yStart);
    NodeToXY(nodeEnd, &xEnd, &yEnd);

    int nx = abs(xStart - xEnd);
    int ny = abs(yStart - yEnd);

    if (worldDirection == DIRECTION_FOUR)
    {
        //Manhattan distance
        return (nx + ny);
    }
    else if (worldDirection == DIRECTION_EIGHT)
    {
        //Euclidean distance
        return (float)sqrt((double)(nx * nx) + (double)(ny * ny));
    }
    else
    {
        //Diagonal distance <- Not using
        // 1.41 -> cost
        return (nx + ny) + (1.41 - 2 * 1) * fmin(nx, ny);
    }
}

void Map::AdjacentCost(void *node, MPVector<StateCost> *neighbors)
{
    int x, y, nx, ny, pass;
    NodeToXY(node, &x, &y);
    StateCost nodeCost;

    for (unsigned int a = 0; a < tileCount; a = a + 1)
    {
        for (unsigned int b = 0; b < worldDirection; b = b + 1)
        {
            nx = x + dx[b];
            ny = y + dy[b];
            pass = Passable(nx, ny);

            if (pass == Costs[a].tile_id)
            {
                if (WorldAt(nx, ny) >= 0)
                {
                    //   nodeCost = { XYToNode( nx, ny ), Costs[a].costs[b] };
                    nodeCost.state = XYToNode(nx, ny);
                    nodeCost.cost = Costs[a].costs[b];
                    neighbors->push_back(nodeCost);
                }
            }
        }
    }
}

void Map::Setup(int _worldWidth, int _worldHeight, int _worldDirection = 8, int _allocate = 250, int _typicalAdjacent = 6, bool _cache = true)
{
    if (pather != NULL)
    {
        delete pather;
        pather = NULL;
    }

    worldWidth = _worldWidth;
    worldHeight = _worldHeight;
    worldDirection = _worldDirection;

    world = (int *)malloc(sizeof(int) * worldWidth * worldHeight);
    memset(world, 0, sizeof(int) * worldWidth * worldHeight); // Set all to 0 -> Walkable

    pather = new MicroPather(this, _allocate, _typicalAdjacent, _cache);
}

void Map::SetMap(int *_world)
{
    ResetPath();
    memcpy(world, _world, sizeof(int) * worldWidth * worldHeight);
}

void Map::Clear()
{
    ResetPath();
    if (Costs != NULL)
    {
        for (unsigned int a = 0; a < tileCount; a = a + 1)
        {
            delete[] Costs[a].costs;
        }
        delete[] Costs;
    }

    Costs = NULL;

    delete[] world;

    if (pather != NULL)
    {
        delete pather;
        pather = NULL;
    }
}

int Map::Solve()
{
    if (Costs == NULL)
    {
        dmLogError("COSTS is null: Please set costs by using astar.set_costs(costs)\n");
        return NO_SOLUTION;
    }

    // Early exit if start/end x-y out of the map bounds
    if (WorldAt(pathTo.x, pathTo.y) < 0 || WorldAt(pathFrom.x, pathFrom.y) < 0)
    {
        ClearPath();
        result = NO_SOLUTION;
        return result;
    }

    result = pather->Solve(XYToNode(pathFrom.x, pathFrom.y), XYToNode(pathTo.x, pathTo.y), &path, &totalCost);

    return result;
}

int Map::SolveNear(float maxCost)
{
    if (Costs == NULL)
    {
        dmLogError("COSTS is null: Please set costs by using astar.set_costs(costs)\n");
        return NO_SOLUTION;
    }

    // Early exit if x-y out of the map bounds
    if (WorldAt(pathFrom.x, pathFrom.y) < 0)
    {
        ClearPath();
        result = NO_SOLUTION;
        return result;
    }

    result = pather->SolveForNearStates(XYToNode(pathFrom.x, pathFrom.y), &nears, maxCost);

    return result;
}