#ifndef LOCKFREE_SINGLE_LIST_H__
#define LOCKFREE_SINGLE_LIST_H__

#include "lockfree_node.h"
#include "lockfree_list.h"

template<typename T>
class LockFreeSiList : public LockFreeList<LockFreeNode<T>> {
protected:
    void deleteNodeBetween(LockFreeNode<T> *node, LockFreeNode<T> *prevNode, LockFreeNode<T> *nextNode) override {
        if (nextNode == node)
            nextNode = nullptr;
        if (nextNode == nullptr) // Mark as delete first, then do the deletion
            LockFreeList<LockFreeNode<T>>::updateTail(prevNode);
        if (prevNode != nullptr) {
            LockFreeList<LockFreeNode<T>>::updateNext(prevNode, nextNode);
        } else
            LockFreeList<LockFreeNode<T>>::updateHead(nextNode);
    }

    bool isWrongConnection(LockFreeNode<T> *node, LockFreeNode<T> *nextNode) override {
        return node->Next() != nextNode;
    }

    LockFreeNode<T>* getValidPrev(LockFreeNode<T>* node) override {
        if (nullptr == node)
            return this->tail_.load();
        LockFreeNode<T>* prevNode = this->head_.load();
        while (true) {
            if (nullptr == prevNode)
                return nullptr;
            LockFreeNode<T>* nextNode = prevNode->Next();
            bool prevDeleted = prevNode->isDeleted();
            if (nextNode == node && !prevDeleted) {
                return prevNode;
            }
            if (prevDeleted && nextNode != nullptr && !nextNode->isDeleted() && this->head_.load() == prevNode) {
                this->head_.store(nextNode);
            }
            prevNode = nextNode;
        }
    }
};

#endif /* LOCK_FREE_BILIST_H__ */
