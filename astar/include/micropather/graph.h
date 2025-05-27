#pragma once
#include <micropather/mpvector.h>
namespace micropather
{
    /**
            Used to pass the cost of states from the cliet application to
       MicroPather. This structure is copied in a vector.

            @sa AdjacentCost
    */
    struct StateCost
    {
        void* state; ///< The state as a void*
        float cost;  ///< The cost to the state. Use FLT_MAX for infinite cost.
    };

    class Graph
    {
        public:
        virtual ~Graph() {}

        /**
                Return the least possible cost between 2 states. For example, if your
           pathfinding is based on distance, this is simply the straight distance
           between 2 points on the map. If you pathfinding is based on minimum time,
           it is the minimal travel time between 2 points given the best possible
           terrain.
        */
        virtual float LeastCostEstimate(void* stateStart, void* stateEnd) = 0;

        /**
                Return the exact cost from the given state to all its neighboring
           states. This may be called multiple times, or cached by the solver. It
           *must* return the same exact values for every call to MicroPather::Solve().
           It should generally be a simple, fast function with no callbacks into the
           pather.
        */
        virtual void AdjacentCost(void*                              state,
                                  MP_VECTOR<micropather::StateCost>* adjacent) = 0;

        /**
                This function is only used in DEBUG mode - it dumps output to stdout.
           Since void* aren't really human readable, normally you print out some
           concise info (like "(1,2)") without an ending newline.
        */
        virtual void PrintStateInfo(void* state) = 0;
    };
}; // namespace micropather