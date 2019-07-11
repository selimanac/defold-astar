#ifndef Map_hpp
#define Map_hpp

#include <micropather.h>
#include <stdio.h>
#include <math.h>

using namespace micropather;

typedef struct {
    int x;
    int y;
} Position;

typedef struct  {
    int tile_id;
    float* costs;
}Tile;


class Map : public Graph
{
private:
    Map(const Map&);
    void operator=(const Map&);
    
    MicroPather* pather;
    
    enum Direction
    {   DIRECTION_FOUR = 4,
        DIRECTION_EIGHT = 8
    };
    
    // E: 1/0
    // N: 0/1
    // W: -1/0
    // S: 0/-1
    
    // SW: -1/-1
    // SE : 1/-1
    // NW: -1/1
    // NE : 1/1
    
    //                  E  N   W   S     NE NW  SW  SE
    const int dx[8] = { 1, 0, -1,  0,    1, -1, -1,  1};
    const int dy[8] = { 0, 1,  0, -1,    1,  1, -1, -1};
    
    int result;
    
public:
    int worldWidth, worldHeight, tileCount, worldDirection;
    int *world;
    Position pathFrom = {0,0};
    Position pathTo = {5,5};
    MPVector<void*> path;
    MPVector<StateCost> near;
    Tile* Costs;
    
    Map(){};
    ~Map();
    
    enum
    {
        SOLVED,
        NO_SOLUTION,
        START_END_SAME,
    };
    
    void Setup(int _worldWidth, int _worldHeight, int _worldDirection, int _allocate, int _typicalAdjacent, bool _cache);
    void SetMap(int *_world);
    void ResetPath();
    void ClearPath();
    int WorldAt(int x, int y);
    void NodeToXY( void* node, int* x, int* y );
    void* XYToNode( size_t x, size_t y );
    
    virtual float LeastCostEstimate( void* nodeStart, void* nodeEnd );
    int Passable( int nx, int ny );
    virtual void AdjacentCost( void* node, MPVector<StateCost> *neighbors );
    virtual void PrintStateInfo( void* node ){};
    float totalCost;
    int Solve();
    int SolveNear(float maxCost);
    void Clear();
};


#endif /* Map_hpp */
