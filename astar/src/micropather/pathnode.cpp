#include <micropather/pathnode.h>

using namespace micropather;

void PathNode::Init(unsigned _frame, void* _state, float _costFromStart, float _estToGoal, PathNode* _parent)
{
    state = _state;
    costFromStart = _costFromStart;
    estToGoal = _estToGoal;
    CalcTotalCost();
    parent = _parent;
    frame = _frame;
    inOpen = 0;
    inClosed = 0;
}

void PathNode::Clear()
{
    memset(this, 0, sizeof(PathNode));
    numAdjacent = -1;
    cacheIndex = -1;
}