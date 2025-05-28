#pragma once

#include <float.h>
#include <memory.h>

namespace micropather
{
    /*
    Every state (void*) is represented by a PathNode in MicroPather. There
    can only be one PathNode for a given state.
    */

    class PathNode
    {
        public:
        void Init(unsigned _frame, void* _state, float _costFromStart, float _estToGoal, PathNode* _parent);

        void Clear();
        void InitSentinel()
        {
            Clear();
            Init(0, 0, FLT_MAX, FLT_MAX, 0);
            prev = next = this;
        }

        void*     state;         // the client state
        float     costFromStart; // exact
        float     estToGoal;     // estimated
        float     totalCost;     // could be a function, but save some math.
        PathNode* parent;        // the parent is used to reconstruct the path
        unsigned  frame;         // unique id for this path, so the solver can distinguish
                                 // correct from stale values

        int       numAdjacent; // -1  is unknown & needs to be queried
        int       cacheIndex;  // position in cache

        PathNode* child[2];    // Binary search in the hash table. [left, right]
        PathNode *next, *prev; // used by open queue

        bool      inOpen;
        bool      inClosed;

        void      Unlink()
        {
            next->prev = prev;
            prev->next = next;
            next = prev = 0;
        }
        void AddBefore(PathNode* addThis)
        {
            addThis->next = this;
            addThis->prev = prev;
            prev->next = addThis;
            prev = addThis;
        }

        void CalcTotalCost()
        {
            if (costFromStart < FLT_MAX && estToGoal < FLT_MAX)
                totalCost = costFromStart + estToGoal;
            else
                totalCost = FLT_MAX;
        }

        private:
        void operator=(const PathNode&);
    };
}; // namespace micropather