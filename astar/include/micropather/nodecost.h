#pragma once

#include <micropather/pathnode.h>

namespace micropather
{
    class PathNode;

    struct NodeCost
    {
        PathNode* node;
        float     cost;
    };

} // namespace micropather