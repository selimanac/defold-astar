#include <micropather/openqueue.h>

using namespace micropather;

void OpenQueue::Push(PathNode* pNode)
{
    // Add sorted. Lowest to highest cost path. Note that the sentinel has
    // a value of FLT_MAX, so it should always be sorted in.

    PathNode* iter = sentinel->next;
    while (true)
    {
        if (pNode->totalCost < iter->totalCost)
        {
            iter->AddBefore(pNode);
            pNode->inOpen = 1;
            break;
        }
        iter = iter->next;
    }
}

PathNode* OpenQueue::Pop()
{
    PathNode* pNode = sentinel->next;
    pNode->Unlink();

    pNode->inOpen = 0;

    return pNode;
}

void OpenQueue::Update(PathNode* pNode)
{
    // If the node now cost less than the one before it,
    // move it to the front of the list.
    if (pNode->prev != sentinel && pNode->totalCost < pNode->prev->totalCost)
    {
        pNode->Unlink();
        sentinel->next->AddBefore(pNode);
    }

    // If the node is too high, move to the right.
    if (pNode->totalCost > pNode->next->totalCost)
    {
        PathNode* it = pNode->next;
        pNode->Unlink();

        while (pNode->totalCost > it->totalCost)
            it = it->next;

        it->AddBefore(pNode);
    }
}