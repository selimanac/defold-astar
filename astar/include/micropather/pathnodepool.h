#pragma once
#include <micropather/mpvector.h>
#include <micropather/nodecost.h>
#include <micropather/pathnode.h>

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#include <stdlib.h>
typedef uintptr_t MP_UPTR;
#elif defined(__GNUC__) && (__GNUC__ >= 3)
#include <stdint.h>
#include <stdlib.h>
typedef uintptr_t MP_UPTR;
#else
// Assume not 64 bit pointers. Get a new compiler.
typedef unsigned MP_UPTR;
#endif

#include <memory.h>

namespace micropather
{
    /* Memory manager for the PathNodes. */
    class PathNodePool
    {
        public:
        PathNodePool(unsigned allocate, unsigned typicalAdjacent);
        ~PathNodePool();

        // Free all the memory except the first block. Resets all memory.
        void Clear();

        // Essentially:
        // pNode = Find();
        // if ( !pNode )
        //		pNode = New();
        //
        // Get the PathNode associated with this state. If the PathNode already
        // exists (allocated and is on the current frame), it will be returned.
        // Else a new PathNode is allocated and returned. The returned object
        // is always fully initialized.
        //
        // NOTE: if the pathNode exists (and is current) all the initialization
        //       parameters are ignored.
        PathNode* GetPathNode(unsigned frame, void* _state, float _costFromStart, float _estToGoal, PathNode* _parent);

        // Get a pathnode that is already in the pool.
        PathNode* FetchPathNode(void* state);

        // Store stuff in cache
        bool PushCache(const NodeCost* nodes, int nNodes, int* start);

        // Get neighbors from the cache
        // Note - always access this with an offset. Can get re-allocated.
        void GetCache(int start, int nNodes, NodeCost* nodes);

        // Return all the allocated states. Useful for visuallizing what
        // the pather is doing.
        void AllStates(unsigned frame, MP_VECTOR<void*>* stateVec);

        private:
        struct Block
        {
            Block*   nextBlock;
            PathNode pathNode[1];
        };

        unsigned Hash(void* voidval);
        unsigned HashSize() const
        {
            return 1 << hashShift;
        }
        unsigned HashMask() const
        {
            return ((1 << hashShift) - 1);
        }
        void       AddPathNode(unsigned key, PathNode* p);
        Block*     NewBlock();
        PathNode*  Alloc();

        PathNode** hashTable;
        Block*     firstBlock;
        Block*     blocks;

        NodeCost*  cache;
        int        cacheCap;
        int        cacheSize;

        PathNode   freeMemSentinel;
        unsigned   allocate;   // how big a block of pathnodes to allocate at once
        unsigned   nAllocated; // number of pathnodes allocated (from Alloc())
        unsigned   nAvailable; // number available for allocation

        unsigned   hashShift;
        unsigned   totalCollide;
    };
}; // namespace micropather