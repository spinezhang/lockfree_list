#ifndef LOCKFREE_SINGLE_LIST_H__
#define LOCKFREE_SINGLE_LIST_H__

#include "lockfree_node.h"
#include "lockfree_list.h"

template<typename T>
class LockFreeSiList : public LockFreeList<LockFreeNode<T>> {
protected:
    void deleteNodeBetween(shared_ptr<LockFreeNode<T>> node, shared_ptr<LockFreeNode<T>> prevNode, shared_ptr<LockFreeNode<T>> nextNode) override {
        if (nextNode == node)
            nextNode = nullptr;
        if (nextNode == nullptr) // Mark as delete first, then do the deletion
            this->updateTail(prevNode);
        if (prevNode != nullptr) {
            this->updateNext(prevNode, nextNode);
        } else
            this->updateHead(nextNode);
    }

    bool isWrongConnection(shared_ptr<LockFreeNode<T>> node, shared_ptr<LockFreeNode<T>> nextNode) override {
        return node->Next() != nextNode;
    }

    shared_ptr<LockFreeNode<T>> getValidPrev(shared_ptr<LockFreeNode<T>> node) override {
        if (nullptr == node)
            return this->Tail();
        auto prevNode = this->Head();
        while (true) {
            if (nullptr == prevNode)
                return nullptr;
            shared_ptr<LockFreeNode<T>> nextNode = prevNode->Next();
            if (nextNode == node && !prevNode->isDeleted()) {
                return prevNode;
            }
            prevNode = nextNode;
        }
    }
};

#endif /* LOCK_FREE_BILIST_H__ */
