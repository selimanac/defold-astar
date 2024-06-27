#include <pather.h>

namespace micropather {

Graph *graph = nullptr;
MicroPather *pather;
uint8_t result;
uint16_t worldWidth, worldHeight, tileCount, worldSize;
uint8_t worldDirection = 8;
uint8_t typicalAdjacent = 6;
uint8_t mapType = GRID_CLASSIC;

int *world;
float totalCost;
bool cache = true;

Position pathFrom = {0, 0};
Position pathTo = {0, 0};
MPVector<void *> path;     // extern
MPVector<StateCost> nears; // extern
Tile *Costs;

// Entities
size_t entitiesSize;
uint8_t *entities;
bool getEntity = false;
bool getNearEntities = false;

int GetWordDirection() { return worldDirection; }

int GetWorldSize() { return worldSize; }

int GetPathSize() { return path.size(); }

int GetNearsSize() { return nears.size(); }

float GetTotalCost() { return totalCost; }

void SetTileCount(uint16_t _tileCount) { tileCount = _tileCount; }

void SetWorldSize(uint16_t _worldSize) { worldSize = _worldSize; }

void SetMapType(uint8_t type) { mapType = type; }

void SetPathFrom(int16_t x, int16_t y) {
  pathFrom.x = x;
  pathFrom.y = y;
}

void SetPathTo(int16_t x, int16_t y) {
  pathTo.x = x;
  pathTo.y = y;
}

void SetPathFromTo(int16_t from_x, int16_t from_y, int16_t to_x, int16_t to_y) {
  SetPathFrom(from_x, from_y);
  SetPathTo(to_x, to_y);
}

void Setup(uint16_t _worldWidth, uint16_t _worldHeight, uint8_t _worldDirection,
           uint16_t _allocate, uint16_t _typicalAdjacent, bool _cache) {

  if (pather != NULL) {
    Clear();
    pather = NULL;
  }

  worldWidth = _worldWidth;
  worldHeight = _worldHeight;
  worldDirection = _worldDirection;
  SetWorldSize(worldWidth * worldHeight);

  world = (int *)malloc(sizeof(int) * worldSize);

  memset(world, 0,
         sizeof(int) * worldSize); // Set all to 0

  pather = new MicroPather(graph, _allocate, _typicalAdjacent, _cache);
}

void SetCosts() {
  ResizePath();
  ResetPath();

  Costs = (Tile *)malloc(sizeof(Tile) * tileCount);

  for (int i = 0; i < tileCount; ++i) {
    Costs[i].costs = (float *)malloc(sizeof(float) * worldDirection);
    Costs[i].tile_id = 0;
  }
}

void AddCostTileID(uint16_t id, uint16_t tileID) { Costs[id].tile_id = tileID; }

void AddCost(uint16_t id, uint16_t costID, float cost) {
  Costs[id].costs[costID] = cost;
}

// NOT USING ANYMORE
void SetMap(int *_world) {
  ResetPath();
  memcpy(world, _world, sizeof(int) * GetWorldSize());
}

void SetTile(uint16_t id, int tile) { world[id] = tile; }

void MapVFlip() {
  for (uint16_t y = 0; y < worldHeight / 2; ++y) {
    for (uint16_t x = 0; x < worldWidth; ++x) {
      int temp = world[y * worldWidth + x];
      world[y * worldWidth + x] = world[(worldHeight - y - 1) * worldWidth + x];
      world[(worldHeight - y - 1) * worldWidth + x] = temp;
    }
  }
}

void MapHFlip() {
  for (uint16_t i = 0; i < worldHeight; ++i) {
    for (uint16_t j = 0; j < worldWidth / 2; ++j) {
      int temp = world[i * worldWidth + j];
      world[i * worldWidth + j] = world[i * worldWidth + (worldWidth - j - 1)];
      world[i * worldWidth + (worldWidth - j - 1)] = temp;
    }
  }
}

void UseEntities(bool toggle) {
  ResetPath();
  getEntity = toggle;
}

void PrintMap(uint8_t zero) {
  int row = 0 + zero;

  printf("cols/x:\n");
  for (int col = zero; col < worldWidth + zero; ++col) {
    printf("%i\t\t", col);
  }
  printf("\n");
  for (int col = 0; col < worldWidth; ++col) {
    printf("-\t\t");
  }
  printf("\n");
  for (int y = 0; y < worldHeight; y++) {
    for (int x = 0; x < worldWidth; x++) {
      printf("%d\t\t", world[y * worldWidth + x]);
    }

    printf("\t- row/y: %i\n", row);
    row++;
  }
}

void SetEntityCount(size_t size) {
  entitiesSize = size;
  entities = (uint8_t *)malloc(sizeof(uint8_t) * entitiesSize);
}
void SetEntity(uint8_t id, int16_t entityID) { entities[id] = entityID; }

// NOT USING ANYMORE
void SetEntities(uint8_t *_entities, size_t size) {
  ResetPath();

  entitiesSize = size;
  entities = (uint8_t *)malloc(sizeof(uint8_t) * size);

  memcpy(entities, _entities, sizeof(uint8_t) * size);
}

void ResizePath() { path.resize(0); }

void ResetPath() {
  if (pather != NULL) {
    pather->Reset();
  }
}

int WorldAt(int16_t x, int16_t y) {
  if (x >= 0 && x < worldWidth && y >= 0 && y < worldHeight) {
    return world[y * worldWidth + x];
  } else {
    return -1;
  }
}

void SetToWorldAt(int16_t x, int16_t y, int value) {
  if (x >= 0 && x < worldWidth && y >= 0 && y < worldHeight) {
    ResetPath();
    world[y * worldWidth + x] = value;
  }
}

void NodeToXY(void *node, int16_t *x, int16_t *y) {
  intptr_t index = (intptr_t)node;
  *y = (int)index / worldWidth;
  *x = (int)index - *y * worldWidth;
}

void *XYToNode(size_t x, size_t y) { return (void *)(y * worldWidth + x); }

float LeastCostEstimate(void *nodeStart, void *nodeEnd) {
  int16_t xStart, yStart, xEnd, yEnd;
  NodeToXY(nodeStart, &xStart, &yStart);
  NodeToXY(nodeEnd, &xEnd, &yEnd);

  int nx = abs(xStart - xEnd);
  int ny = abs(yStart - yEnd);

  if (worldDirection == DIRECTION_FOUR) {
    // Manhattan distance
    return (nx + ny);
  } else if (worldDirection == DIRECTION_EIGHT) {
    // Euclidean distance
    return (float)sqrt((double)(nx * nx) + (double)(ny * ny));
  }
  return 0.0f;
}

int Passable(int16_t nx, int16_t ny) {
  if (nx >= 0 && nx < worldWidth && ny >= 0 && ny < worldHeight) {
    int index = ny * worldWidth + nx;
    int c = world[index];

    if (getEntity) {
      for (int i = 0; i < entitiesSize; i++) {
        if (c == entities[i]) {
          return c;
        }
      }
    }

    for (unsigned int a = 0; a < tileCount; a++) {
      if (Costs[a].tile_id == c) {
        return c;
      }
    }
  }
  return -1;
}

void AdjacentCost(void *node, MPVector<StateCost> *neighbors) {
  int16_t x, y, nx, ny, pass;
  NodeToXY(node, &x, &y);
  StateCost nodeCost;

  for (unsigned int a = 0; a < tileCount; a = a + 1) {
    for (unsigned int b = 0; b < worldDirection; b = b + 1) {

      if (mapType == GRID_CLASSIC) {
        nx = x + dx[b];
        ny = y + dy[b];
      } else if (mapType == HEX_ODDR || mapType == HEX_EVENR) {
        if (y % 2 == 0) {
          nx = x + OffsetCoordinates[mapType][0][b];
          ny = y + OffsetCoordinates[mapType][1][b];
        } else {
          nx = x + OffsetCoordinates[mapType][2][b];
          ny = y + OffsetCoordinates[mapType][3][b];
        }
      } else if (mapType == HEX_ODDQ || mapType == HEX_EVENQ) {
        if (x % 2 == 0) {
          nx = x + OffsetCoordinates[mapType][0][b];
          ny = y + OffsetCoordinates[mapType][1][b];
        } else {
          nx = x + OffsetCoordinates[mapType][2][b];
          ny = y + OffsetCoordinates[mapType][3][b];
        }
      }

      pass = Passable(nx, ny);

      if (getEntity || getNearEntities) {

        for (int i = 0; i < entitiesSize; i++) {

          if (pass == entities[i]) {
            if (WorldAt(nx, ny) >= 0) {
              if (XYToNode(nx, ny) == XYToNode(pathTo.x, pathTo.y) &&
                  getEntity && getNearEntities == false) {

                nodeCost.state = XYToNode(nx, ny);
                nodeCost.cost = Costs[a].costs[b];
                neighbors->push_back(nodeCost);
              } else if (getNearEntities) {

                nodeCost.state = XYToNode(nx, ny);
                nodeCost.cost = Costs[a].costs[b];
                neighbors->push_back(nodeCost);
              }
            }
          }
        }
      }

      if (pass == Costs[a].tile_id) {
        if (WorldAt(nx, ny) >= 0) {
          nodeCost.state = XYToNode(nx, ny);
          nodeCost.cost = Costs[a].costs[b];
          neighbors->push_back(nodeCost);
        }
      }
    }
  }
}

int Solve() {

  if (pather == NULL) {
    dmLogError("You have to setup the astar using astar.setup()\n");
    return NO_SOLUTION;
  }
  if (Costs == NULL) {
    dmLogError("COSTS table is empty: Please set costs by using "
               "astar.set_costs(costs)\n");
    return NO_SOLUTION;
  }

  // Early exit if start/end x-y out of the map bounds
  if (WorldAt(pathTo.x, pathTo.y) < 0 || WorldAt(pathFrom.x, pathFrom.y) < 0) {
    ResizePath();
    result = NO_SOLUTION;
    return result;
  }

  result = pather->Solve(XYToNode(pathFrom.x, pathFrom.y),
                         XYToNode(pathTo.x, pathTo.y), &path, &totalCost);

  return result;
}

int SolveNear(float maxCost) {
  if (pather == NULL) {
    dmLogError("You have to setup the astar using astar.setup()\n");
    return NO_SOLUTION;
  }

  if (Costs == NULL) {
    dmLogError("COSTS table is empty: Please set costs by using  "
               "astar.set_costs(costs)\n");
    return NO_SOLUTION;
  }

  // Early exit if x-y out of the map bounds
  if (WorldAt(pathFrom.x, pathFrom.y) < 0) {
    ResizePath();
    result = NO_SOLUTION;
    return result;
  }

  getNearEntities = getEntity;
  result = pather->SolveForNearStates(XYToNode(pathFrom.x, pathFrom.y), &nears,
                                      maxCost);
  getNearEntities = false;
  return result;
}

void Clear() {
  ResetPath();
  if (Costs != NULL) {
    for (unsigned int a = 0; a < tileCount; a = a + 1) {
      free(Costs[a].costs);
    }
  }

  free(Costs);
  free(entities);
  free(world);

  if (pather != NULL) {
    delete pather;
    pather = NULL;
  }
}

} // namespace micropather