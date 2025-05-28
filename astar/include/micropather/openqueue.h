#pragma once

#include <micropather/graph.h>
#include <micropather/pathnode.h>

namespace micropather
{
    class OpenQueue
    {
        public:
        OpenQueue(Graph* _graph)
        {
            graph = _graph;
            sentinel = (PathNode*)sentinelMem;
            sentinel->InitSentinel();
        }
        ~OpenQueue() {}

        void      Push(PathNode* pNode);
        PathNode* Pop();
        void      Update(PathNode* pNode);

        bool      Empty()
        {
            return sentinel->next == sentinel;
        }

        private:
        OpenQueue(const OpenQueue&); // undefined and unsupported
        void      operator=(const OpenQueue&);

        PathNode* sentinel;
        int       sentinelMem[(sizeof(PathNode) + sizeof(int)) / sizeof(int)];
        Graph*    graph; // for debugging
    };

} // namespace micropather