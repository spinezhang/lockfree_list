#ifndef LOCKFREE_LIST_H__
#define LOCKFREE_LIST_H__

#include <memory>
#include <functional>
#include <iostream>

template<typename NODE>
class LockFreeList {
public:
    LockFreeList() {
        // head_.store(nullptr);
        // tail_.store(nullptr);
        size_.store(0);
    }

    virtual ~LockFreeList() = default;

    /**
     * Inserts a new node at the head of the list.
     * Continuously attempts to insert the new node until successful.
     * NOTE: Set forceSuccess as true does not guarantee original order for sorted list,
     * if you need to grant original order, set forceSuccess as false and keep the order before call this function.
     *
     * @param node The new node to be inserted.
     * @param forceSuccess To grant the insertion successfully.
     * @return True if the insertion is successful, false otherwise.
     */
    bool InsertHead(shared_ptr<NODE> node, bool forceSuccess=true) {
        return Insert(node, Head(), forceSuccess);
    }

    /**
     * Appends a new node at the tail of the list.
     * Continuously attempts to insert the new node until successful.
     * NOTE: Set forceSuccess as true does not guarantee original order for sorted list,
     * if you need to grant original order, set forceSuccess as false and keep the order before call this function.
     *
     * @param node The new node to be appended.
     * @param forceSuccess To grant the insertion successfully.
     * @return True if the insertion is successful, false otherwise.
     */
    bool Append(shared_ptr<NODE> node, bool forceSuccess=true) {
        while(true) {
            bool result = InsertBetween(node, Tail(), nullptr);
            if (!forceSuccess || result)
                return result;
        }
    }

/**
 * Inserts a new node before a target node in the lock-free list.
 * Continuously attempts to insert the new node before the target node until successful.
 * NOTE: Set forceSuccess as true does not guarantee original order for sorted list,
 * if you need to grant original order, set forceSuccess as false and keep the order before call this function.
 *
 * @param newNode The new node to be inserted.
 * @param targetNode The target node before which the new node should be inserted.
 * @param forceSuccess To grant the insertion successfully.
 * @param interFunc The test function to be called during the insertion process to modify the list.
 * @return True if the insertion is successful, false otherwise.
 */
#ifdef TEST_MIDDLE_CHANGE
    using InterferenceFunc = function<void(int step, shared_ptr<NODE> curentNode, shared_ptr<NODE> prevNode, shared_ptr<NODE> nextNode)>;

    bool Insert(shared_ptr<NODE> node, shared_ptr<NODE> targetNode, bool forceSuccess=true, InterferenceFunc interFunc=nullptr) {
        int count = 0;
#else
    bool Insert(shared_ptr<NODE> node, shared_ptr<NODE> targetNode, bool forceSuccess=true) {
#endif
        while(true) {
            shared_ptr<NODE> nextNode = targetNode;
            shared_ptr<NODE> prevNode;
            if (nullptr != nextNode) {
                if (nextNode->isDeleted())
                    nextNode = getValidNext(nextNode);
                prevNode = getValidPrev(nextNode);
            }

            bool result;
#ifdef TEST_MIDDLE_CHANGE
            if (0 == count++)
                result = InsertBetween(node, prevNode, nextNode, interFunc);
            else
                result = InsertBetween(node, prevNode, nextNode);
#else
            result = InsertBetween(node, prevNode, nextNode);
#endif
            if (!forceSuccess || result)
                return result;
        }
    }

    shared_ptr<NODE> Head() const {
        return atomic_load(&head_);
    }

    shared_ptr<NODE> Tail() const {
        return atomic_load(&tail_);
    }

    int Size() const {
        return size_.load();
    }

    shared_ptr<NODE> GetNext(shared_ptr<NODE> node) {
        if (nullptr == node)
            return nullptr;
        return getValidNext(node);
    }

    shared_ptr<NODE> GetPrev(shared_ptr<NODE> node) {
        if (nullptr == node)
            return nullptr;
        return getValidPrev(node);
    }

    shared_ptr<NODE> PopHead(void) {
        shared_ptr<NODE> head = Head();
        if (nullptr == head || !deleteNode(head))
            return nullptr;
        return head;
    }

    shared_ptr<NODE> PopTail(void) {
        shared_ptr<NODE> tail = Tail();
        if (nullptr == tail || !deleteNode(tail))
            return nullptr;
        return tail;
    }

    /**
     * Removes a node from the lock-free list.
     * Continuously attempts to remove the node until successful.
     * NOTE: Set forceSuccess as true does not guarantee original order for sorted list,
     * if you need to grant original order, set forceSuccess as false and keep the order before call this function.
     *
     * @param node The node to be removed.
     * @param forceSuccess To grant the removal successfully.
     * @return True if the removal is successful, false otherwise.
     */
    bool Remove(shared_ptr<NODE> node, bool forceSuccess=true) {
        if (node == nullptr || node->isDeleted()) {
            return false;
        }
        while(true) {
            shared_ptr<NODE> nextNode = static_pointer_cast<NODE>(node->Next());

            // mark as delete first
            if (node->Delete(nextNode)) {
                shared_ptr<NODE> prevNode = getValidPrev(node);
                bool headOrTail = false;
                if (node == Tail() || nextNode == nullptr) {
                    headOrTail = updateTail(prevNode, node);
                }

                nextNode = getValidNext(node);
                if (node == Head() || prevNode == nullptr) {
                    headOrTail = updateHead(nextNode, node);
                }
                if (!headOrTail && nullptr != prevNode || nullptr != nextNode) {
                    deleteNodeBetween(node, prevNode, nextNode);
                }

                this->size_.fetch_add(-1);
                return true;
            }
            if (nextNode == nullptr && node->isDeleted())
                return false;
            if (!forceSuccess)
                return false;
        }
    }

    /**
     * This function is used for testing only. It checks the consistency of the lock-free list.
     * This function only works in thread-safe mode and is used to check the consistency of the list.
     * It checks the following conditions:
     * 1. All nodes are not marked as deleted.
     * 2. The next pointer of each node points to a valid node or nullptr.
     * 3. If it is a Bidirectional list, the prev pointer of each node points to a valid node or nullptr.
     * 4. The size of the list is correct.
     * If any of the above conditions are not met, this function returns false, otherwise returns true.
     * @param count The expected size of the list, if not provided, it is ignored.
     * @return True if the list is consistent, false otherwise.
     */
    bool CheckConsistence(int count = -1) {
        shared_ptr<NODE> tempNode = Head();
        int i = 0;
        while(tempNode != nullptr && tempNode != nullptr) {
            if (tempNode->isDeleted()) {
                std::cout << "fatal: " << tempNode.get() << " is deleted" << endl;
                return false;
            }
            shared_ptr<NODE> nextNode = static_pointer_cast<NODE>(tempNode->Next());
            if (nextNode != nullptr && !nextNode->isDeleted()) {
                if (nextNode == nullptr)
                    return true;
                if (isWrongConnection(tempNode, nextNode)) {
                    return false;
                }
            }

            tempNode = nextNode;
            i++;
        }

        if (count != -1 && this->size_.load() != count) {
            cout << "count:" << count << "actualCount: " << this->size_.load() << endl;
            return false;
        }
        return true;
    }

#ifdef TEST_MIDDLE_CHANGE
/**
 * Inserts a new node between prevNode and nextNode in the lock-free list.
 * If prevNode is nullptr, it is inserted at the head of the list.
 * If nextNode is nullptr, it is inserted at the tail of the list.
 * The function returns true if the insertion is successful, false otherwise.
 * @param newNode The new node to be inserted.
 * @param prevNode The prev node of the new node.
 * @param nextNode The next node of the new node.
 * @param interFunc The test function to be called during the insertion process to modify the list.
 * @return True if the insertion is successful, false otherwise.
 */
    bool InsertBetween(shared_ptr<NODE> node, shared_ptr<NODE> prevNode, shared_ptr<NODE> nextNode, InterferenceFunc interFunc=nullptr) {
#else
protected:
    bool InsertBetween(shared_ptr<NODE> node, shared_ptr<NODE> prevNode, shared_ptr<NODE> nextNode) {
#endif
        if (prevNode == nullptr && nextNode == nullptr) {
            node->SetNext(nullptr);
            setPrev(node, nullptr);
            atomic_store(&this->head_, node);
            atomic_store(&this->tail_, node);
            this->size_.fetch_add(1);
            return true;
        }
        // insert in the middle
        node->SetNext(nextNode);
        setPrev(node, prevNode);
#ifdef TEST_MIDDLE_CHANGE
        if (interFunc)
            interFunc(1, node, prevNode, nextNode);
#endif
        // Step2: Update the next pointer of previous node first, if failed, means concurrently changed by another thread,
        // return failed and let the caller retry with updated prevNode or nextNode;
        if (prevNode != nullptr && (!prevNode->isDeleted()) && !isNodeIn(node, prevNode, nextNode, false)) {
            if (!updateNext(prevNode, node, nextNode))
                return false;
        } else if (prevNode == nullptr || (prevNode->isDeleted() && getPrev(prevNode) == nullptr))  {
            // prevNode == nullptr, means insert from head
            if (updateHead(node, nextNode))
                return false;
        }
        this->size_.fetch_add(1);

#ifdef TEST_MIDDLE_CHANGE
        if (interFunc)
            interFunc(2, node, prevNode, nextNode);
#endif

        // Step3: Update the prev pointer of next node
        // Because the next pointer is already chained successfully, so here need to chain the pointer and fix consistance.
        if (hasPrev() && nextNode != nullptr && (!nextNode->isDeleted()) && !isNodeIn(node, nextNode, prevNode, true)) {
            updatePrev(nextNode, node, prevNode);
        } else if (nextNode == nullptr || nextNode->Next() == nullptr) {
            updateTail(node, prevNode);
        }

#ifdef TEST_MIDDLE_CHANGE
        if (interFunc)
            interFunc(3, node, prevNode, nextNode);
#endif

        bool breaked;
        if (hasPrev())
            breaked = ((prevNode != nullptr && prevNode->isDeleted()) || getPrev(node) != prevNode
            || (nextNode != nullptr && nextNode->isDeleted()) || node->Next() != nextNode);
        else
            breaked = ((prevNode != nullptr && prevNode->isDeleted()) || (nextNode != nullptr && nextNode->isDeleted()) ||     node->Next() != nextNode);

        if (!breaked)
            return true;
        if (nullptr != nextNode)
            fixInsert(node, prevNode, nextNode);
        return true;
    }

protected:
    shared_ptr<NODE> head_;
    shared_ptr<NODE> tail_;
    atomic<int> size_;

    virtual void setPrev(shared_ptr<NODE> node, shared_ptr<NODE> prevNode) {}
    virtual shared_ptr<NODE> getPrev(shared_ptr<NODE> node) {return nullptr;}
    virtual bool hasPrev() {return false;}
    virtual void updatePrev(shared_ptr<NODE> node, shared_ptr<NODE> newNode, shared_ptr<NODE> prevNode) {}
    virtual void fixPrev(shared_ptr<NODE> nextNode, shared_ptr<NODE> actualPrevNode, shared_ptr<NODE> actualNextNode) {}

    virtual void deleteNodeBetween(shared_ptr<NODE> node, shared_ptr<NODE> prevNode, shared_ptr<NODE> nextNode) = 0;
    virtual shared_ptr<NODE> getValidPrev(shared_ptr<NODE> node) = 0;
    virtual bool isWrongConnection(shared_ptr<NODE> node, shared_ptr<NODE> nextNode) = 0;

    shared_ptr<NODE> getValidNext(shared_ptr<NODE> node) {
        shared_ptr<NODE>nextNode = static_pointer_cast<NODE>(node->Next());
        while (nextNode != nullptr && nextNode->isDeleted()) {
            nextNode = static_pointer_cast<NODE>(nextNode->Next());
        }
        return nextNode;
    }

    virtual bool updateHead(shared_ptr<NODE> node, shared_ptr<NODE> prevHead=nullptr) {
        if (prevHead == nullptr)
            prevHead = this->head_;
        return atomic_compare_exchange_weak(&this->head_, &prevHead, node);
    }

    virtual bool updateTail(shared_ptr<NODE> node, shared_ptr<NODE> prevTail = nullptr) {
        if (atomic_compare_exchange_weak(&tail_, &prevTail, node)) {
            if (nullptr != node)
                node->SetNext(nullptr);
            return true;
        }
        return false;
    }

    bool updateNext(shared_ptr<NODE> node, shared_ptr<NODE> newNode, shared_ptr<NODE> nextNode = nullptr) {
        if (nextNode == nullptr)
            nextNode = static_pointer_cast<NODE>(node->Next());
        // avoid ABA
        shared_ptr<NODE> nexOfNewNode = nullptr;
        if (newNode != nullptr)
            nexOfNewNode = static_pointer_cast<NODE>(newNode->Next());
        if (node != newNode && nexOfNewNode != node)
            return node->CompareAndSetNext(nextNode, newNode);
        return true;
    }

    bool isNodeIn(shared_ptr<NODE> node, shared_ptr<NODE> startNode, shared_ptr<NODE> endNode, bool reverse) {
        shared_ptr<NODE> checkNode = startNode;
        bool nodeInChain = false;
        while (checkNode != nullptr && checkNode != endNode) {
            if (checkNode == node)
                nodeInChain = true;
            if (reverse && hasPrev())
                checkNode = getPrev(checkNode);
            else
                checkNode = static_pointer_cast<NODE>(checkNode->Next());
        }
        return nodeInChain && checkNode == endNode;
    }

    void fixInsert(shared_ptr<NODE> node, shared_ptr<NODE> prevNode, shared_ptr<NODE> nextNode) {
        shared_ptr<NODE> actualNextNode = nextNode;
        if (nextNode->isDeleted()) {
            actualNextNode = getValidNext(nextNode);
        }
        shared_ptr<NODE> actualPrevNode = prevNode;
        if (actualPrevNode == nullptr)
            updateHead(node);
        else if (prevNode->isDeleted()) {
            actualPrevNode = getValidPrev(actualNextNode);
            if (actualPrevNode == nullptr || actualPrevNode == node)
                updateHead(node);
        }
        if ((actualNextNode == nullptr || actualPrevNode == actualNextNode) && node != Tail())
            this->updateTail(node);
        else if (actualPrevNode == node || actualPrevNode == nullptr)
            actualPrevNode = Head();

        // Fix forward chain
        if (actualPrevNode != nullptr && !isNodeIn(node, actualPrevNode, actualNextNode, false)) {
            actualNextNode = getValidNext(actualPrevNode);
            if (actualNextNode != nextNode && (actualNextNode == nullptr || !actualNextNode->isDeleted()))
                this->updateNext(node, actualNextNode, nextNode);
            this->updateNext(actualPrevNode, node);
        } else if (node->Next() == nextNode && nextNode != actualNextNode && (actualNextNode == nullptr || !actualNextNode->isDeleted()))
            this->updateNext(node, actualNextNode, nextNode);

        // Fix backward chain
        if (hasPrev())
            fixPrev(nextNode, actualPrevNode, actualNextNode);
    }
};

#endif /* LOCK_FREE_BILIST_H__ */
