#include <micropather/pathcache.h>

using namespace micropather;

PathCache::PathCache(int _allocated)
{
    mem = new Item[_allocated];
    memset(mem, 0, sizeof(*mem) * _allocated);
    allocated = _allocated;
    nItems = 0;
    hit = 0;
    miss = 0;
}

PathCache::~PathCache()
{
    delete[] mem;
}

void PathCache::Reset()
{
    if (nItems)
    {
        memset(mem, 0, sizeof(*mem) * allocated);
        nItems = 0;
        hit = 0;
        miss = 0;
    }
}

void PathCache::Add(const MP_VECTOR<void*>& path,
                    const MP_VECTOR<float>& cost)
{
    if (nItems + (int)path.size() > allocated * 3 / 4)
    {
        return;
    }

    for (unsigned i = 0; i < path.size() - 1; ++i)
    {
        // example: a->b->c->d
        // Huge memory saving to only store 3 paths to 'd'
        // Can put more in cache with also adding path to b, c, & d
        // But uses much more memory. Experiment with this commented
        // in and out and how to set.

        void* end = path[path.size() - 1];
        Item  item = { path[i], end, path[i + 1], cost[i] };
        AddItem(item);
    }
}

void PathCache::AddNoSolution(void* end, void* states[], int count)
{
    if (count + nItems > allocated * 3 / 4)
    {
        return;
    }

    for (int i = 0; i < count; ++i)
    {
        Item item = { states[i], end, 0, FLT_MAX };
        AddItem(item);
    }
}

int PathCache::Solve(void* start, void* end, MP_VECTOR<void*>* path, float* totalCost)
{
    const Item* item = Find(start, end);
    if (item)
    {
        if (item->cost == FLT_MAX)
        {
            ++hit;
            return micropather::NO_SOLUTION;
        }

        path->clear();
        path->push_back(start);
        *totalCost = 0;

        for (; start != end; start = item->next, item = Find(start, end))
        {
            *totalCost += item->cost;
            path->push_back(item->next);
        }
        ++hit;
        return micropather::SOLVED;
    }
    ++miss;
    return micropather::NOT_CACHED;
}

void PathCache::AddItem(const Item& item)
{
    unsigned index = item.Hash() % allocated;
    while (true)
    {
        if (mem[index].Empty())
        {
            mem[index] = item;
            ++nItems;

            break;
        }
        else if (mem[index].KeyEqual(item))
        {
            // do nothing; in cache
            break;
        }
        ++index;
        if (index == allocated)
            index = 0;
    }
}

const PathCache::Item* PathCache::Find(void* start, void* end)
{
    Item     fake = { start, end, 0, 0 };
    unsigned index = fake.Hash() % allocated;
    while (true)
    {
        if (mem[index].Empty())
        {
            return 0;
        }
        if (mem[index].KeyEqual(fake))
        {
            return mem + index;
        }
        ++index;
        if (index == allocated)
            index = 0;
    }
}