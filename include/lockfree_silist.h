#ifndef LOCKFREE_SINGLE_LIST_H__
#define LOCKFREE_SINGLE_LIST_H__

#include "lockfree_node.h"
#include "lockfree_list.h"

template<typename T>
class LockFreeSiList : public LockFreeList<LockFreeNode<T>> {
protected:
    void deleteNodeBetween(shared_ptr<LockFreeNode<T>> node, shared_ptr<LockFreeNode<T>> prevNode, shared_ptr<LockFreeNode<T>> nextNode) override {
        shared_ptr<LockFreeNode<T>> actualNextNode = this->getValidNext(node); // maybe not same as origNextNode
        if (actualNextNode == node)
            actualNextNode = nullptr;
        shared_ptr<LockFreeNode<T>> actualPrevNode = getValidPrev(node);
        bool result;
        if (actualPrevNode != nullptr) {
            result = this->updateNext(actualPrevNode, actualNextNode);
        } else {
            result = this->updateHead(this->Head(), actualNextNode);
        }

        if (actualNextNode == nullptr) {
            this->updateTail(this->Tail(), actualPrevNode);
        }
        if (actualPrevNode != nullptr || actualNextNode != nullptr) {
            if (!result || actualPrevNode != prevNode || actualNextNode != nextNode)
                fixDelete(prevNode, nextNode);
        }
    }

    bool isWrongConnection(shared_ptr<LockFreeNode<T>> node, shared_ptr<LockFreeNode<T>> nextNode) override {
        return node->Next() != nextNode;
    }

    shared_ptr<LockFreeNode<T>> getValidPrev(shared_ptr<LockFreeNode<T>> node) override {
        if (nullptr == node)
            return this->Tail();
        auto prevNode = this->Head();
        while (nullptr != prevNode && prevNode->Next() != node && !prevNode->isDeleted()) {
            prevNode = prevNode->Next();
        }
        if (prevNode == node)
            return nullptr;
        return prevNode;
    }

    void fixDelete(shared_ptr<LockFreeNode<T>> prevNode, shared_ptr<LockFreeNode<T>> nextNode) {
        shared_ptr<LockFreeNode<T>> actualPrevNode = prevNode;
        if (actualPrevNode != nullptr && actualPrevNode->isDeleted()) {
            actualPrevNode = getValidPrev(prevNode);
        }
        shared_ptr<LockFreeNode<T>> actualNextNode = nextNode;
        if (actualNextNode != nullptr && actualNextNode->isDeleted()) {
            actualNextNode = this->getValidNext(nextNode);
        }

        if (actualPrevNode == nullptr)
            this->updateHead(actualNextNode);

        if ((actualNextNode == nullptr || actualPrevNode == actualNextNode))
            this->updateTail(actualPrevNode);
        else if (actualPrevNode != nullptr && actualNextNode != nextNode) {
            this->updateNext(actualPrevNode, actualNextNode);
        }
    }
};

#endif /* LOCK_FREE_BILIST_H__ */
