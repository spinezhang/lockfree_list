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
    void setPrev(LockFreeBiNode<T> *node, LockFreeBiNode<T> *prevNode) override {
        node->prev_.store(prevNode);
    }

    LockFreeBiNode<T> *getPrev(LockFreeBiNode<T> *node) override {
        return node->Prev();
    }

    bool hasPrev() override {
        return true;
    }

    /**
     * Deletes a node in a lock-free bidirectional list.
     *
     * This function is used to delete a node in the list. It first checks if the node is the head node, if so, it
     * updates the head_ to the new head node. If the node is not the head node, it updates the prev_ of the next node
     * to the prev node. If the prev node is not valid, it updates the head_ to the next node. If the next node is not
     * valid, it updates the tail_ to the prev node. If the prev node is not valid and the next node is valid, it
     * updates the tail_ to the next node. If the prev node is valid and the next node is not valid, it updates the
     * head_ to the prev node. Then it checks if the prev node and the next node is valid, if not, it calls fixDelete
     * to fix the deletion.
     *
     * @param node The node to be deleted.
     * @param prevNode The prev node of the node to be deleted.
     * @param nextNode The next node of the node to be deleted.
     */
    void deleteNodeBetween(LockFreeBiNode<T> *node, LockFreeBiNode<T> *prevNode, LockFreeBiNode<T> *nextNode) override {
        LockFreeBiNode<T>* actualNextNode = LockFreeList<LockFreeBiNode<T>>::GetNext(node); // maybe not same as origNextNode
        if (actualNextNode == node)
            actualNextNode = nullptr;
        LockFreeBiNode<T>* actualPrevNode = getValidPrev(node);
        bool result;
        if (actualPrevNode != nullptr) {
            result = LockFreeList<LockFreeBiNode<T>>::updateNext(actualPrevNode, actualNextNode);
        } else {
            result = updateHead(this->head_.load(), actualNextNode);
        }

        if (actualNextNode != nullptr) {
            updatePrev(actualNextNode, actualPrevNode);
        } else {
            LockFreeList<LockFreeBiNode<T>>::updateTail(this->tail_.load(), actualPrevNode);
        }
        if (actualPrevNode != nullptr || actualNextNode != nullptr) {
            if (!result || actualPrevNode != prevNode || actualNextNode != nextNode)
                fixDelete(prevNode, nextNode);
        }
    }

    bool isWrongConnection(LockFreeBiNode<T> *node, LockFreeBiNode<T> *nextNode) override {
        return nextNode->Prev() != node || node->Next() != nextNode;
    }

    LockFreeBiNode<T>* getValidPrev(LockFreeBiNode<T>* node) override {
        if (nullptr == node)
            return nullptr;
        LockFreeBiNode<T>* prevNode = node->Prev();
        while (prevNode != nullptr && prevNode->isDeleted()) {
            prevNode = prevNode->Prev();
        }
        return prevNode;
    }

private:
    /**
     * Fixes the prev_ pointer of all nodes between actualPrevNode and actualNextNode.
     *
     * The function iterates from actualPrevNode to actualNextNode (exclusive) and checks if the prev_ pointer of each
     * node is valid. If the prev_ pointer is invalid, the function updates the prev_ pointer to the correct node.
     *
     * The function is used to fix the prev_ pointers after a node is deleted.
     *
     * @param nextNode The next node of the node to be fixed.
     * @param actualPrevNode The actual prev node of the node to be fixed.
     * @param actualNextNode The actual next node of the node to be fixed.
     */
    void fixPrev(LockFreeBiNode<T> *nextNode, LockFreeBiNode<T> *actualPrevNode, LockFreeBiNode<T> *actualNextNode) override {
        if (nextNode->isDeleted()) // may be changed
            actualNextNode = LockFreeList<LockFreeBiNode<T>>::GetNext(nextNode);
        if (actualNextNode == nullptr)
            return;
        if (actualPrevNode != nullptr) {
            LockFreeBiNode<T>* tempNode = actualPrevNode;
            while (tempNode != nullptr && tempNode != actualNextNode->Next()) {
                LockFreeBiNode<T>* tempNext = tempNode->Next();
                LockFreeBiNode<T>* temPrevOfNext = nullptr;
                if (tempNext != nullptr)
                    temPrevOfNext = getValidPrev(tempNext);
                if (tempNext != nullptr && !tempNext->isDeleted() && temPrevOfNext != tempNext) {
                    if (temPrevOfNext == nullptr ||
                        LockFreeList<LockFreeBiNode<T>>::isNodeIn(temPrevOfNext, actualPrevNode, actualNextNode->Next(), false)) {
                        updatePrev(tempNext, tempNode);
                    }
                }
                tempNode = tempNext;
            }
        }
    }

    /**
     * Fixes the deletion of a node.
     *
     * This function is called after a node is deleted to fix the prev_ and next_ pointers of the adjacent nodes.
     *
     * @param prevNode The prev node of the node to be fixed.
     * @param nextNode The next node of the node to be fixed.
     */
    void fixDelete(LockFreeBiNode<T>* prevNode, LockFreeBiNode<T>* nextNode) {
        LockFreeBiNode<T>* actualPrevNode = prevNode;
        if (actualPrevNode != nullptr && actualPrevNode->isDeleted())
        {
            actualPrevNode = getValidPrev(prevNode);
        }
        LockFreeBiNode<T>* actualNextNode = nextNode;
        if (actualNextNode != nullptr && actualNextNode->isDeleted())
        {
            actualNextNode = LockFreeList<LockFreeBiNode<T>>::GetNext(nextNode);
        }

        if (actualPrevNode == nullptr)
            updateHead(actualNextNode);

        if ((actualNextNode == nullptr || actualPrevNode == actualNextNode))
            LockFreeList<LockFreeBiNode<T>>::updateTail(actualPrevNode);
        else if (actualPrevNode != nullptr && actualNextNode != nextNode) {
            LockFreeList<LockFreeBiNode<T>>::updateNext(actualPrevNode, actualNextNode);
        }

        // Fix backward chain
        if (actualPrevNode != nullptr && actualNextNode != nullptr)
            fixPrev(nextNode, actualPrevNode, actualNextNode);
    }

    bool updateHead(LockFreeBiNode<T> *node, LockFreeBiNode<T> *prevHead=nullptr) override {
        bool result = LockFreeList<LockFreeBiNode<T>>::updateHead(node, prevHead);
        if (result && node != nullptr) {
            node->prev_.store(nullptr);
        }
        return result;
    }

    void updatePrev(LockFreeBiNode<T> *node, LockFreeBiNode<T> *newNode, LockFreeBiNode<T> *prevNode=nullptr) override {
        if (prevNode == nullptr)
            prevNode = node->Prev();
        // avoid ABA
        LockFreeBiNode<T>* prevOfNewNode = nullptr;
        if(newNode != nullptr)
            prevOfNewNode = newNode->Prev();
        if (node != newNode && prevOfNewNode != node)
            node->prev_.compare_exchange_strong(prevNode, newNode);
    }
};

#endif //LOCKFREE_BILIST_H
