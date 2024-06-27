/*
Copyright (c) 2000-2009 Lee Thomason (www.grinninglizard.com)

Grinning Lizard Utilities.

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

#ifdef _MSC_VER
#pragma warning(disable : 4786) // Debugger truncating names.
#pragma warning(disable : 4530) // Exception handler isn't used
#endif

#include "micropather/micropather.h"

using namespace micropather;

MicroPather::MicroPather(Graph *_graph, unsigned allocate,
                         unsigned typicalAdjacent, bool cache)
    : pathNodePool(allocate, typicalAdjacent), graph(_graph), frame(0) {
  pathCache = 0;
  if (cache) {
    pathCache = new PathCache(allocate * 4); // untuned arbitrary constant
  }
}

MicroPather::~MicroPather() { delete pathCache; }

void MicroPather::Reset() {
  pathNodePool.Clear();
  if (pathCache) {
    pathCache->Reset();
  }
  frame = 0;
}

void MicroPather::GoalReached(PathNode *node, void *start, void *end,
                              MP_VECTOR<void *> *_path) {
  MP_VECTOR<void *> &path = *_path;
  path.clear();

  // We have reached the goal.
  // How long is the path? Used to allocate the vector which is returned.
  int count = 1;
  PathNode *it = node;
  while (it->parent) {
    ++count;
    it = it->parent;
  }

  // Now that the path has a known length, allocate
  // and fill the vector that will be returned.
  if (count < 3) {
    // Handle the short, special case.
    path.resize(2);
    path[0] = start;
    path[1] = end;
  } else {
    path.resize(count);

    path[0] = start;
    path[count - 1] = end;
    count -= 2;
    it = node->parent;

    while (it->parent) {
      path[count] = it->state;
      it = it->parent;
      --count;
    }
  }

  if (pathCache) {
    costVec.clear();

    PathNode *pn0 = pathNodePool.FetchPathNode(path[0]);
    PathNode *pn1 = 0;
    for (unsigned i = 0; i < path.size() - 1; ++i) {
      pn1 = pathNodePool.FetchPathNode(path[i + 1]);
      nodeCostVec.clear();
      GetNodeNeighbors(pn0, &nodeCostVec);
      for (unsigned j = 0; j < nodeCostVec.size(); ++j) {
        if (nodeCostVec[j].node == pn1) {
          costVec.push_back(nodeCostVec[j].cost);
          break;
        }
      }

      pn0 = pn1;
    }
    pathCache->Add(path, costVec);
  }
}

void MicroPather::GetNodeNeighbors(PathNode *node,
                                   MP_VECTOR<NodeCost> *pNodeCost) {
  if (node->numAdjacent == 0) {
    // it has no neighbors.
    pNodeCost->resize(0);
  } else if (node->cacheIndex < 0) {
    // Not in the cache. Either the first time or just didn't fit. We don't know
    // the number of neighbors and need to call back to the client.
    stateCostVec.resize(0);
    AdjacentCost(node->state, &stateCostVec);

    pNodeCost->resize(stateCostVec.size());
    node->numAdjacent = stateCostVec.size();

    if (node->numAdjacent > 0) {
      // Now convert to pathNodes.
      // Note that the microsoft std library is actually pretty slow.
      // Move things to temp vars to help.
      const unsigned stateCostVecSize = stateCostVec.size();
      const StateCost *stateCostVecPtr = &stateCostVec[0];
      NodeCost *pNodeCostPtr = &(*pNodeCost)[0];

      for (unsigned i = 0; i < stateCostVecSize; ++i) {
        void *state = stateCostVecPtr[i].state;
        pNodeCostPtr[i].cost = stateCostVecPtr[i].cost;
        pNodeCostPtr[i].node =
            pathNodePool.GetPathNode(frame, state, FLT_MAX, FLT_MAX, 0);
      }

      // Can this be cached?
      // NOPE :) This cause a cache problem when getting nears with entities. Do
      // not use it.
      /*int start = 0;
      if (pNodeCost->size() > 0 &&
          pathNodePool.PushCache(pNodeCostPtr, pNodeCost->size(), &start)) {
        node->cacheIndex = start;
      }*/
    }
  } else {
    // In the cache!
    pNodeCost->resize(node->numAdjacent);
    NodeCost *pNodeCostPtr = &(*pNodeCost)[0];
    pathNodePool.GetCache(node->cacheIndex, node->numAdjacent, pNodeCostPtr);

    // A node is uninitialized (even if memory is allocated) if it is from a
    // previous frame. Check for that, and Init() as necessary.
    for (int i = 0; i < node->numAdjacent; ++i) {
      PathNode *pNode = pNodeCostPtr[i].node;
      if (pNode->frame != frame) {
        pNode->Init(frame, pNode->state, FLT_MAX, FLT_MAX, 0);
      }
    }
  }
}

void MicroPather::StatesInPool(MP_VECTOR<void *> *stateVec) {
  stateVec->clear();
  pathNodePool.AllStates(frame, stateVec);
}

void MicroPather::GetCacheData(CacheData *data) {
  memset(data, 0, sizeof(*data));

  if (pathCache) {
    data->nBytesAllocated = pathCache->AllocatedBytes();
    data->nBytesUsed = pathCache->UsedBytes();
    data->memoryFraction =
        (float)((double)data->nBytesUsed / (double)data->nBytesAllocated);

    data->hit = pathCache->hit;
    data->miss = pathCache->miss;
    if (data->hit + data->miss) {
      data->hitFraction =
          (float)((double)(data->hit) / (double)(data->hit + data->miss));
    } else {
      data->hitFraction = 0;
    }
  }
}

int MicroPather::Solve(void *startNode, void *endNode, MP_VECTOR<void *> *path,
                       float *cost) {
  // Important to clear() in case the caller doesn't check the return code.
  // There can easily be a left over path  from a previous call.
  path->clear();

  *cost = 0.0f;

  if (startNode == endNode)
    return START_END_SAME;

  if (pathCache) {
    int cacheResult = pathCache->Solve(startNode, endNode, path, cost);
    if (cacheResult == SOLVED || cacheResult == NO_SOLUTION) {

      return cacheResult;
    }
  }

  ++frame;

  OpenQueue open(graph);
  ClosedSet closed(graph);

  PathNode *newPathNode = pathNodePool.GetPathNode(
      frame, startNode, 0, LeastCostEstimate(startNode, endNode), 0);

  open.Push(newPathNode);
  stateCostVec.resize(0);
  nodeCostVec.resize(0);

  while (!open.Empty()) {
    PathNode *node = open.Pop();

    if (node->state == endNode) {
      GoalReached(node, startNode, endNode, path);
      *cost = node->costFromStart;

      return SOLVED;
    } else {
      closed.Add(node);

      // We have not reached the goal - add the neighbors.
      GetNodeNeighbors(node, &nodeCostVec);

      for (int i = 0; i < node->numAdjacent; ++i) {
        // Not actually a neighbor, but useful. Filter out infinite cost.
        if (nodeCostVec[i].cost == FLT_MAX) {
          continue;
        }
        PathNode *child = nodeCostVec[i].node;
        float newCost = node->costFromStart + nodeCostVec[i].cost;

        PathNode *inOpen = child->inOpen ? child : 0;
        PathNode *inClosed = child->inClosed ? child : 0;
        PathNode *inEither =
            (PathNode *)(((MP_UPTR)inOpen) | ((MP_UPTR)inClosed));

        if (inEither) {
          if (newCost < child->costFromStart) {
            child->parent = node;
            child->costFromStart = newCost;
            child->estToGoal = LeastCostEstimate(child->state, endNode);
            child->CalcTotalCost();
            if (inOpen) {
              open.Update(child);
            }
          }
        } else {
          child->parent = node;
          child->costFromStart = newCost;
          child->estToGoal = LeastCostEstimate(child->state, endNode),
          child->CalcTotalCost();

          open.Push(child);
        }
      }
    }
  }

  if (pathCache) {
    // Could add a bunch more with a little tracking.
    pathCache->AddNoSolution(endNode, &startNode, 1);
  }
  return NO_SOLUTION;
}

int MicroPather::SolveForNearStates(void *startState,
                                    MP_VECTOR<StateCost> *near, float maxCost) {
  /*	 http://en.wikipedia.org/wiki/Dijkstra%27s_algorithm

           1  function Dijkstra(Graph, source):
           2      for each vertex v in Graph:           // Initializations
           3          dist[v] := infinity               // Unknown distance
     function from source to v 4          previous[v] := undefined          //
     Previous node in optimal path from source 5      dist[source] := 0 //
     Distance from source to source 6      Q := the set of all nodes in Graph
                          // All nodes in the graph are unoptimized - thus are
     in Q 7      while Q is not empty:                 // The main loop 8 u :=
     vertex in Q with smallest dist[] 9          if dist[u] = infinity: 10 break
     // all remaining vertices are inaccessible from source 11          remove u
     from Q 12          for each neighbor v of u:         // where v has not yet
     been removed from Q. 13              alt := dist[u] + dist_between(u, v) 14
     if alt < dist[v]:             // Relax (u,v,a) 15                  dist[v]
     := alt 16                  previous[v] := u 17      return dist[]
  */

  ++frame;

  OpenQueue open(graph); // nodes to look at
  ClosedSet closed(graph);

  nodeCostVec.resize(0);
  stateCostVec.resize(0);

  PathNode closedSentinel;
  closedSentinel.Clear();
  closedSentinel.Init(frame, 0, FLT_MAX, FLT_MAX, 0);
  closedSentinel.next = closedSentinel.prev = &closedSentinel;

  PathNode *newPathNode = pathNodePool.GetPathNode(frame, startState, 0, 0, 0);
  open.Push(newPathNode);

  while (!open.Empty()) {
    PathNode *node = open.Pop(); // smallest dist
    closed.Add(node);            // add to the things we've looked at
    closedSentinel.AddBefore(node);

    if (node->totalCost > maxCost)
      continue; // Too far away to ever get here.

    GetNodeNeighbors(node, &nodeCostVec);

    for (int i = 0; i < node->numAdjacent; ++i) {

      float newCost = node->costFromStart + nodeCostVec[i].cost;

      PathNode *inOpen = nodeCostVec[i].node->inOpen ? nodeCostVec[i].node : 0;
      PathNode *inClosed =
          nodeCostVec[i].node->inClosed ? nodeCostVec[i].node : 0;

      PathNode *inEither = inOpen ? inOpen : inClosed;

      if (inEither && inEither->costFromStart <= newCost) {
        continue; // Do nothing. This path is not better than existing.
      }
      // Groovy. We have new information or improved information.
      PathNode *child = nodeCostVec[i].node;

      child->parent = node;
      child->costFromStart = newCost;
      child->estToGoal = 0;
      child->totalCost = child->costFromStart;

      if (inOpen) {
        open.Update(inOpen);
      } else if (!inClosed) {
        open.Push(child);
      }
    }
  }
  near->clear();

  for (PathNode *pNode = closedSentinel.next; pNode != &closedSentinel;
       pNode = pNode->next) {
    if (pNode->totalCost <= maxCost) {
      StateCost sc;
      sc.cost = pNode->totalCost;
      sc.state = pNode->state;

      near->push_back(sc);
    }
  }

  return SOLVED;
}
