#include <micropather/pathnodepool.h>

using namespace micropather;

PathNodePool::PathNodePool(unsigned _allocate, unsigned _typicalAdjacent)
    : firstBlock(0)
    , blocks(0)
    , allocate(_allocate)
    , nAllocated(0)
    , nAvailable(0)
{
    freeMemSentinel.InitSentinel();

    cacheCap = allocate * _typicalAdjacent;
    cacheSize = 0;
    cache = (NodeCost*)malloc(cacheCap * sizeof(NodeCost));

    // Want the behavior that if the actual number of states is specified, the
    // cache will be at least that big.
    hashShift = 3; // 8 (only useful for stress testing)

    hashTable = (PathNode**)calloc(HashSize(), sizeof(PathNode*));

    blocks = firstBlock = NewBlock();
    //	printf( "HashSize=%d allocate=%d\n", HashSize(), allocate );
    totalCollide = 0;
}

PathNodePool::~PathNodePool()
{
    Clear();
    free(firstBlock);
    free(cache);
    free(hashTable);
}

bool PathNodePool::PushCache(const NodeCost* nodes, int nNodes, int* start)
{
    *start = -1;
    if (nNodes + cacheSize <= cacheCap)
    {
        for (int i = 0; i < nNodes; ++i)
        {
            cache[i + cacheSize] = nodes[i];
        }
        *start = cacheSize;
        cacheSize += nNodes;
        return true;
    }
    return false;
}

void PathNodePool::GetCache(int start, int nNodes, NodeCost* nodes)
{
    memcpy(nodes, &cache[start], sizeof(NodeCost) * nNodes);
}

void PathNodePool::Clear()
{
    Block* b = blocks;
    while (b)
    {
        Block* temp = b->nextBlock;
        if (b != firstBlock)
        {
            free(b);
        }
        b = temp;
    }
    blocks = firstBlock; // Don't delete the first block (we always need at least
                         // that much memory.)

    // Set up for new allocations (but don't do work we don't need to. Reset/Clear
    // can be called frequently.)
    if (nAllocated > 0)
    {
        freeMemSentinel.next = &freeMemSentinel;
        freeMemSentinel.prev = &freeMemSentinel;

        memset(hashTable, 0, sizeof(PathNode*) * HashSize());
        for (unsigned i = 0; i < allocate; ++i)
        {
            freeMemSentinel.AddBefore(&firstBlock->pathNode[i]);
        }
    }
    nAvailable = allocate;
    nAllocated = 0;
    cacheSize = 0;
}

PathNodePool::Block* PathNodePool::NewBlock()
{
    Block* block =
    (Block*)calloc(1, sizeof(Block) + sizeof(PathNode) * (allocate - 1));
    block->nextBlock = 0;

    nAvailable += allocate;
    for (unsigned i = 0; i < allocate; ++i)
    {
        freeMemSentinel.AddBefore(&block->pathNode[i]);
    }
    return block;
}

unsigned PathNodePool::Hash(void* voidval)
{
    /*
            Spent quite some time on this, and the result isn't quite satifactory.
       The input set is the size of a void*, and is generally (x,y) pairs or
       memory pointers.

            FNV resulting in about 45k collisions in a (large) test and some other
       approaches about the same.

            Simple folding reduces collisions to about 38k - big improvement.
       However, that may be an artifact of the (x,y) pairs being well distributed.
       And for either the x,y case or the pointer case, there are probably very
       poor hash table sizes that cause "overlaps" and grouping. (An x,y encoding
       with a hashShift of 8 is begging for trouble.)

            The best tested results are simple folding, but that seems to beg for
       a pathelogical case. FNV-1a was the next best choice, without obvious
       pathelogical holes.

            Finally settled on h%HashMask(). Simple, but doesn't have the obvious
       collision cases of folding.
    */
    /*
    // Time: 567
    // FNV-1a
    // http://isthe.com/chongo/tech/comp/fnv/
    // public domain.
    MP_UPTR val = (MP_UPTR)(voidval);
    const unsigned char *p = (unsigned char *)(&val);
    unsigned int h = 2166136261;

    for( size_t i=0; i<sizeof(MP_UPTR); ++i, ++p ) {
            h ^= *p;
            h *= 16777619;
    }
    // Fold the high bits to the low bits. Doesn't (generally) use all
    // the bits since the shift is usually < 16, but better than not
    // using the high bits at all.
    return ( h ^ (h>>hashShift) ^ (h>>(hashShift*2)) ^ (h>>(hashShift*3)) ) &
    HashMask();
    */
    /*
    // Time: 526
    MP_UPTR h = (MP_UPTR)(voidval);
    return ( h ^ (h>>hashShift) ^ (h>>(hashShift*2)) ^ (h>>(hashShift*3)) ) &
    HashMask();
    */

    // Time: 512
    // The HashMask() is used as the divisor. h%1024 has lots of common
    // repetitions, but h%1023 will move things out more.
    MP_UPTR h = (MP_UPTR)(voidval);
    return h % HashMask();
}

PathNode* PathNodePool::Alloc()
{
    if (freeMemSentinel.next == &freeMemSentinel)
    {
        Block* b = NewBlock();
        b->nextBlock = blocks;
        blocks = b;
    }
    PathNode* pathNode = freeMemSentinel.next;
    pathNode->Unlink();

    ++nAllocated;

    --nAvailable;
    return pathNode;
}

void PathNodePool::AddPathNode(unsigned key, PathNode* root)
{
    if (hashTable[key])
    {
        PathNode* p = hashTable[key];
        while (true)
        {
            int dir = (root->state < p->state) ? 0 : 1;
            if (p->child[dir])
            {
                p = p->child[dir];
            }
            else
            {
                p->child[dir] = root;
                break;
            }
        }
    }
    else
    {
        hashTable[key] = root;
    }
}

PathNode* PathNodePool::FetchPathNode(void* state)
{
    unsigned  key = Hash(state);

    PathNode* root = hashTable[key];
    while (root)
    {
        if (root->state == state)
        {
            break;
        }
        root = (state < root->state) ? root->child[0] : root->child[1];
    }

    return root;
}

PathNode* PathNodePool::GetPathNode(unsigned frame, void* _state, float _costFromStart, float _estToGoal, PathNode* _parent)
{
    unsigned  key = Hash(_state);

    PathNode* root = hashTable[key];
    while (root)
    {
        if (root->state == _state)
        {
            if (root->frame == frame) // This is the correct state and correct frame.
                break;
            // Correct state, wrong frame.
            root->Init(frame, _state, _costFromStart, _estToGoal, _parent);
            break;
        }
        root = (_state < root->state) ? root->child[0] : root->child[1];
    }
    if (!root)
    {
        // allocate new one
        root = Alloc();
        root->Clear();
        root->Init(frame, _state, _costFromStart, _estToGoal, _parent);
        AddPathNode(key, root);
    }
    return root;
}

void PathNodePool::AllStates(unsigned frame, MP_VECTOR<void*>* stateVec)
{
    for (Block* b = blocks; b; b = b->nextBlock)
    {
        for (unsigned i = 0; i < allocate; ++i)
        {
            if (b->pathNode[i].frame == frame)
                stateVec->push_back(b->pathNode[i].state);
        }
    }
}
