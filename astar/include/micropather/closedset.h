#pragma once
#include <micropather/graph.h>
#include <micropather/pathnode.h>

namespace micropather
{
    class ClosedSet
    {
        public:
        ClosedSet(Graph* _graph)
        {
            this->graph = _graph;
        }
        ~ClosedSet() {}

        void Add(PathNode* pNode)
        {
            pNode->inClosed = 1;
        }

        void Remove(PathNode* pNode)
        {
            pNode->inClosed = 0;
        }

        private:
        ClosedSet(const ClosedSet&);
        void   operator=(const ClosedSet&);
        Graph* graph;
    };

}; // namespace micropather