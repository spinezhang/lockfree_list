//
// Created by Spine Zhang on 2024/9/6.
//

#ifndef LOCKFREE_BILIST_H
#define LOCKFREE_BILIST_H

#include "lockfree_binode.h"
#include "lockfree_list.h"

template<typename T>
class LockFreeBiList : public LockFreeList<LockFreeBiNode<T>> {
protected:
    inline void setPrev(shared_ptr<LockFreeBiNode<T>> node, shared_ptr<LockFreeBiNode<T>> prevNode) override {
        node->SetPrev(prevNode);
    }

    inline shared_ptr<LockFreeBiNode<T>> getPrev(shared_ptr<LockFreeBiNode<T>> node) override {
        return node->Prev();
    }

    inline bool hasPrev() override {
        return true;
    }

    void deleteNodeBetween(shared_ptr<LockFreeBiNode<T>> node, shared_ptr<LockFreeBiNode<T>> prevNode, shared_ptr<LockFreeBiNode<T>> nextNode) override {
        shared_ptr<LockFreeBiNode<T>> actualNextNode = this->getValidNext(node); // maybe not same as origNextNode
        if (actualNextNode == node)
            actualNextNode = nullptr;
        shared_ptr<LockFreeBiNode<T>> actualPrevNode = getValidPrev(node);
        bool result;
        if (actualPrevNode != nullptr) {
            result = this->updateNext(actualPrevNode, actualNextNode);
        } else {
            result = updateHead(this->Head(), actualNextNode);
        }

        if (actualNextNode != nullptr) {
            updatePrev(actualNextNode, actualPrevNode);
        } else {
            this->updateTail(this->Tail(), actualPrevNode);
        }
        if (actualPrevNode != nullptr || actualNextNode != nullptr) {
            if (!result || actualPrevNode != prevNode || actualNextNode != nextNode)
                fixDelete(prevNode, nextNode);
        }
    }

    bool isWrongConnection(shared_ptr<LockFreeBiNode<T>> node, shared_ptr<LockFreeBiNode<T>> nextNode) override {
        return nextNode->Prev() != node || node->Next() != nextNode;
    }

    shared_ptr<LockFreeBiNode<T>> getValidPrev(shared_ptr<LockFreeBiNode<T>> node) override {
        if (nullptr == node)
            return nullptr;
        shared_ptr<LockFreeBiNode<T>> prevNode = node->Prev();
        while (prevNode != nullptr && prevNode->isDeleted()) {
            prevNode = prevNode->Prev();
        }
        return prevNode;
    }

private:
    void fixPrev(shared_ptr<LockFreeBiNode<T>> nextNode, shared_ptr<LockFreeBiNode<T>> actualPrevNode, shared_ptr<LockFreeBiNode<T>> actualNextNode) override {
        if (nextNode->isDeleted()) // may be changed
            actualNextNode = this->getValidNext(nextNode);
        if (actualNextNode == nullptr)
            return;
        if (actualPrevNode != nullptr) {
            shared_ptr<LockFreeBiNode<T>> tempNode = actualPrevNode;
            while (tempNode != nullptr && tempNode != actualNextNode->Next()) {
                shared_ptr<LockFreeBiNode<T>> tempNext = static_pointer_cast<LockFreeBiNode<T>>(tempNode->Next());
                shared_ptr<LockFreeBiNode<T>> temPrevOfNext = nullptr;
                if (tempNext != nullptr)
                    temPrevOfNext = getValidPrev(tempNext);
                if (tempNext != nullptr && !tempNext->isDeleted() && temPrevOfNext != tempNext && tempNext->Prev() != tempNode) {
                    if (temPrevOfNext == nullptr ||
                        this->isNodeIn(temPrevOfNext, actualPrevNode, static_pointer_cast<LockFreeBiNode<T>>(actualNextNode->Next()), false)) {
                        updatePrev(tempNext, tempNode);
                    }
                }
                tempNode = tempNext;
            }
        }
    }

    void fixDelete(shared_ptr<LockFreeBiNode<T>> prevNode, shared_ptr<LockFreeBiNode<T>> nextNode) {
        shared_ptr<LockFreeBiNode<T>> actualPrevNode = prevNode;
        if (actualPrevNode != nullptr && actualPrevNode->isDeleted()) {
            actualPrevNode = getValidPrev(prevNode);
        }
        shared_ptr<LockFreeBiNode<T>> actualNextNode = nextNode;
        if (actualNextNode != nullptr && actualNextNode->isDeleted()) {
            actualNextNode = this->getValidNext(nextNode);
        }

        if (actualPrevNode == nullptr)
            updateHead(actualNextNode);

        if ((actualNextNode == nullptr || actualPrevNode == actualNextNode))
            this->updateTail(actualPrevNode);
        else if (actualPrevNode != nullptr && actualNextNode != nextNode) {
            this->updateNext(actualPrevNode, actualNextNode);
        }

        // Fix backward chain
        if (actualPrevNode != nullptr && actualNextNode != nullptr)
            fixPrev(nextNode, actualPrevNode, actualNextNode);
    }

    bool updateHead(shared_ptr<LockFreeBiNode<T>> node, shared_ptr<LockFreeBiNode<T>>prevHead=nullptr) override {
        bool result = LockFreeList<LockFreeBiNode<T>>::updateHead(node, prevHead);
        if (result && node != nullptr) {
            node->SetPrev(nullptr);
        }
        return result;
    }

    void updatePrev(shared_ptr<LockFreeBiNode<T>> node, shared_ptr<LockFreeBiNode<T>> newNode, shared_ptr<LockFreeBiNode<T>> prevNode=nullptr) override {
        if (prevNode == nullptr)
            prevNode = node->Prev();
        // avoid ABA
        shared_ptr<LockFreeBiNode<T>> prevOfNewNode(nullptr);
        if(newNode != nullptr)
            prevOfNewNode = static_pointer_cast<LockFreeBiNode<T>>(newNode->Prev());
        if (node != newNode && prevOfNewNode != node)
            node->SetPrev(newNode);
    }
};

#endif //LOCKFREE_BILIST_H
