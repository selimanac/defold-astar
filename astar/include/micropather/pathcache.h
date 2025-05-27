#pragma once

#include <float.h>
#include <memory.h>
#include <micropather/mpvector.h>
#include <micropather/states.h>

namespace micropather
{
    /**
    A pure abstract class used to define a set of callbacks.
    The client application inherits from
    this class, and the methods will be called when MicroPather::Solve() is
    invoked.

    The notion of a "state" is very important. It must have the following
    properties:
    - Unique
    - Unchanging (unless MicroPather::Reset() is called)

    If the client application represents states as objects, then the state
    is usually just the object cast to a void*. If the client application sees
    states as numerical values, (x,y) for example, then state is an encoding of
    these values. MicroPather never interprets or modifies the value of state.
    */

    /* Used to cache results of paths. Much, much faster
       to return an existing solution than to calculate
       a new one. A post on this is here:
       http://grinninglizard.com/altera/programming/a-path-caching-2/
    */
    class PathCache
    {
        public:
        struct Item
        {
            // The key:
            void* start;
            void* end;

            bool  KeyEqual(const Item& item) const
            {
                return start == item.start && end == item.end;
            }
            bool Empty() const
            {
                return start == 0 && end == 0;
            }

            // Data:
            void*    next;
            float    cost; // from 'start' to 'next'. FLT_MAX if unsolveable.

            unsigned Hash() const
            {
                const unsigned char* p = (const unsigned char*)(&start);
                unsigned int         h = 2166136261U;

                for (unsigned i = 0; i < sizeof(void*) * 2; ++i, ++p)
                {
                    h ^= *p;
                    h *= 16777619;
                }
                return h;
            }
        };

        PathCache(int itemsToAllocate);
        ~PathCache();

        void Reset();
        void Add(const MP_VECTOR<void*>& path, const MP_VECTOR<float>& cost);
        void AddNoSolution(void* end, void* states[], int count);
        int  Solve(void* startState, void* endState, MP_VECTOR<void*>* path, float* totalCost);

        int  AllocatedBytes() const
        {
            return allocated * sizeof(Item);
        }
        int UsedBytes() const
        {
            return nItems * sizeof(Item);
        }

        int hit;
        int miss;

        private:
        void        AddItem(const Item& item);
        const Item* Find(void* start, void* end);

        Item*       mem;
        int         allocated;
        int         nItems;
    };
} // namespace micropather