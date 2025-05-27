/*
Copyright (c) 2000-2013 Lee Thomason (www.grinninglizard.com)
Micropather

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must
not claim that you wrote the original software. If you use this
software in a product, an acknowledgment in the product documentation
would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/

#ifndef GRINNINGLIZARD_MICROPATHER_INCLUDED
#define GRINNINGLIZARD_MICROPATHER_INCLUDED

/** @mainpage MicroPather

        MicroPather is a path finder and A* solver (astar or a-star) written in
   platform independent C++ that can be easily integrated into existing code.
   MicroPather focuses on being a path finding engine for video games but is a
   generic A* solver. MicroPather is open source, with a license suitable for
   open source or commercial use.
*/

// This probably works to remove, but isn't currently tested in STL mode.

#include <float.h>
#include <memory.h>
#include <micropather/closedset.h>
#include <micropather/coordinates.h>
#include <micropather/graph.h>
#include <micropather/mpvector.h>
#include <micropather/openqueue.h>
#include <micropather/pathcache.h>
#include <micropather/pathnode.h>
#include <micropather/pathnodepool.h>
#include <stdio.h>

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

namespace micropather {

struct CacheData {
  CacheData()
      : nBytesAllocated(0), nBytesUsed(0), memoryFraction(0), hit(0), miss(0),
        hitFraction(0) {}
  int nBytesAllocated;
  int nBytesUsed;
  float memoryFraction;
  int hit;
  int miss;
  float hitFraction;
};

class MicroPather {
  friend class micropather::PathNode;

public:
  MicroPather(Graph *graph, unsigned allocate = 250,
              unsigned typicalAdjacent = 6, bool cache = true);
  ~MicroPather();

  int Solve(void *startState, void *endState, MP_VECTOR<void *> *path,
            float *totalCost);

  int SolveForNearStates(void *startState, MP_VECTOR<StateCost> *near,
                         float maxCost);

  void Reset();

  void StatesInPool(MP_VECTOR<void *> *stateVec);
  void GetCacheData(CacheData *data);

private:
  MicroPather(const MicroPather &);  // undefined and unsupported
  void operator=(const MicroPather); // undefined and unsupported

  void GoalReached(PathNode *node, void *start, void *end,
                   MP_VECTOR<void *> *path);

  void GetNodeNeighbors(PathNode *node, MP_VECTOR<NodeCost> *neighborNode);

  PathNodePool pathNodePool;
  MP_VECTOR<StateCost> stateCostVec;
  MP_VECTOR<NodeCost> nodeCostVec;
  MP_VECTOR<float> costVec;

  Graph *graph;
  unsigned frame;

  PathCache *pathCache;
};
}; // namespace micropather

#endif
