#ifndef LOCKFREE_LIST_H__
#define LOCKFREE_LIST_H__

#include <atomic>
#include <functional>

template<class NODE>
class LockFreeList {
public:
    LockFreeList() {
        head_.store(nullptr);
        tail_.store(nullptr);
        size_.store(0);
    }

    virtual ~LockFreeList() = default;

    bool InsertHead(NODE *node, bool forceSuccess=true) {
        return Insert(node, head_.load(), forceSuccess);
    }

    bool Append(NODE *node, bool forceSuccess=true) {
        while(true) {
            bool result = InsertBetween(node, tail_.load(), nullptr);
            if (!forceSuccess || result)
                return result;
        }
    }

/**
 * Inserts a new node before a target node in the lock-free bidirectional list.
 * Continuously attempts to insert the new node before the target node until successful.
 * 
 * @param newNode The new node to be inserted.
 * @param targetNode The target node before which the new node should be inserted.
 * 
 * @return True if the insertion is successful, false otherwise.
 */
#ifdef TEST_MIDDLE_CHANGE
    using TestFunc = std::function<void(int step, NODE *curentNode, NODE *prevNode, NODE *nextNode)>;

    bool Insert(NODE *newNode, NODE *targetNode, bool forceSuccess=true, TestFunc testFunc=nullptr) {
        int count = 0;
#else
    bool Insert(NODE *newNode, NODE *targetNode, bool forceSuccess=true) {
#endif
        while(true) {
            NODE *prevOfTarget = nullptr;
            if (nullptr != targetNode) {
                if (targetNode->isDeleted())
                    targetNode = GetNext(targetNode);
                prevOfTarget = getValidPrev(targetNode);
            }

            bool result;
#ifdef TEST_MIDDLE_CHANGE
            if (0 == count++)
                result = InsertBetween(newNode, prevOfTarget, targetNode, testFunc);
            else
                result = InsertBetween(newNode, prevOfTarget, targetNode);
#else
            result = InsertBetween(newNode, prevOfTarget, targetNode);
#endif
            if (!forceSuccess || result)
                return result;
        }
    }

    NODE *GetHead() {
        return head_.load();
    }

    NODE *GetTail() {
        return tail_.load();
    }

    NODE *GetNext(NODE *node) {
        if (nullptr == node)
            return nullptr;
        // Find the next node not marked as deleted
        NODE *nextNode = node->Next();
        while (nextNode != nullptr && nextNode->isDeleted()) {
            nextNode = nextNode->Next();
        }
        return nextNode;
    }

    NODE *GetPrev(NODE *node) {
        return getValidPrev(node);
    }

    NODE * PopHead(void) {
        NODE* head = GetHead();
        if (nullptr == head || !deleteNode(head))
            return nullptr;
        return head;
    }

    NODE * PopTail(void) {
        NODE* tail = GetTail();
        if (nullptr == tail || !deleteNode(tail))
            return nullptr;
        return tail;
    }

    bool Remove(NODE *node, bool forceSuccess=true) {
        if (node == nullptr || node->isDeleted()) {
            return false;
        }

        while(true) {
            NODE* nextNode = node->Next();

            // mark as delete first
            if (node->next_.compareAndSet(nextNode, nullptr, false, true)) {
                NODE* prevNode = getValidPrev(node);
                bool headOrTail = false;
                if (node == this->tail_.load() || nextNode == nullptr) {
                    headOrTail = updateTail(prevNode, node);
                }
                if (node == this->head_.load() || prevNode == nullptr) {
                    headOrTail = updateHead(nextNode, node);
                }
                if (!headOrTail) {
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

    bool CheckConsistence(int count = -1) {
        // This help function doest not work in concurrency mode, it is only used in thread-safe mode to check the consistency of the list
        NODE *tempNode = this->head_.load();

        while(tempNode != nullptr && tempNode != nullptr) {
            if (tempNode->isDeleted())
                return false;
            NODE* nextNode = tempNode->Next();
            if (nextNode != nullptr && !nextNode->isDeleted()) {
                if (nextNode == nullptr)
                    return true;
                if (isWrongConnection(tempNode, nextNode)) {
                    return false;
                }
            }

            tempNode = nextNode;
        }

        if (count != -1 && this->size_.load() != count) {
            std::cout << "count:" << count << "actualCount: " << this->size_.load() << std::endl;
            return false;
        }
        return true;
    }

#ifdef TEST_MIDDLE_CHANGE
    bool InsertBetween(NODE* newNode, NODE* prevNode, NODE* nextNode, TestFunc testFunc=nullptr) {
#else
protected:
    bool InsertBetween(NODE* newNode, NODE* prevNode, NODE* nextNode) {
#endif
//          printf("insert:0x%x (0x%x, 0x%x)\n", newNode, prevNode, nextNode);
        if (prevNode == nullptr && nextNode == nullptr) {
            newNode->next_.set(nullptr, false);
            setPrev(newNode, nullptr);
            this->head_.store(newNode);
            this->tail_.store(newNode);
            this->size_.fetch_add(1);
            return true;
        }
        // insert in the middle
        newNode->next_.set(nextNode, false);
        setPrev(newNode, prevNode);
#ifdef TEST_MIDDLE_CHANGE
        if (testFunc)
            testFunc(1, newNode, prevNode, nextNode);
#endif
        // Step2: Update the next pointer of previous node first, if failed, means concurrently changed by another thread,
        // return failed and let the caller retry with updated prevNode or nextNode;
        if (prevNode != nullptr && (!prevNode->isDeleted()) && !isNodeIn(newNode, prevNode, nextNode, false)) {
            if (!updateNext(prevNode, newNode, nextNode))
                return false;
        } else if (prevNode == nullptr || (prevNode->isDeleted() && getPrev(prevNode) == nullptr))  {
            // prevNode == nullptr, means insert from head
            if (updateHead(newNode, nextNode))
                return false;
        }
        this->size_.fetch_add(1);

#ifdef TEST_MIDDLE_CHANGE
        if (testFunc)
            testFunc(2, newNode, prevNode, nextNode);
#endif

        // Step3: Update the prev pointer of next node
        // Because the next pointer is already chained successfully, so here need to chain the pointer and fix consistance.
        if (hasPrev() && nextNode != nullptr && (!nextNode->isDeleted()) && !isNodeIn(newNode, nextNode, prevNode, true)) {
            updatePrev(nextNode, newNode, prevNode);
        } else if (nextNode == nullptr || nextNode->Next() == nullptr) {
            updateTail(newNode, prevNode);
        }

#ifdef TEST_MIDDLE_CHANGE
        if (testFunc)
            testFunc(3, newNode, prevNode, nextNode);
#endif

        bool breaked;
        if (hasPrev())
            breaked = ((prevNode != nullptr && prevNode->isDeleted()) || getPrev(newNode) != prevNode
            || (nextNode != nullptr && nextNode->isDeleted()) || newNode->Next() != nextNode);
        else
            breaked = ((prevNode != nullptr && prevNode->isDeleted()) || (nextNode != nullptr && nextNode->isDeleted()) ||     newNode->Next() != nextNode);

        if (!breaked)
            return true;
        if (nullptr != nextNode)
            fixInsert(newNode, prevNode, nextNode);
        return true;
    }

protected:
    std::atomic<NODE *> head_;
    std::atomic<NODE *> tail_;
    std::atomic<int> size_;

    virtual void setPrev(NODE *node, NODE *prevNode) {}
    virtual NODE *getPrev(NODE *node) {return nullptr;}
    virtual bool hasPrev() {return false;}
    virtual void updatePrev(NODE*node, NODE*newNode, NODE*prevNode) {}
    virtual void fixPrev(NODE *nextNode, NODE *actualPrevNode, NODE *actualNextNode) {}

    virtual void deleteNodeBetween(NODE *node, NODE *prevNode, NODE *nextNode) = 0;
    virtual NODE *getValidPrev(NODE *node) = 0;
    virtual bool isWrongConnection(NODE *node, NODE *nextNode) = 0;
    // virtual void fixDelete(NODE *prevNode, NODE *nextNode) = 0;

    /**
 * Returns the back node of the given node in the lock-free bidirectional list.
 *
 * @param node The node for which to find the back node.
 * @return The back node of the given node.
 *         Returns nullptr if the back node is not found.
 */
    virtual bool updateHead(NODE *node, NODE *prevHead=nullptr) {
        return this->head_.compare_exchange_strong(prevHead, node);
    }

    virtual bool updateTail(NODE *node, NODE *prevTail = nullptr) {
        if (this->tail_.compare_exchange_strong(prevTail, node)) {
            if (nullptr != node)
                node->next_.set(nullptr, false);
            return true;
        }
        return false;
    }

    bool updateNext(NODE *node, NODE *newNode, NODE *nextNode = nullptr) {
        if (nextNode == nullptr)
            nextNode = node->Next();
        // avoid ABA
        NODE *nexOfNewNode = nullptr;
        if (newNode != nullptr)
            nexOfNewNode = newNode->Next();
        if (node != newNode && nexOfNewNode != node)
            return node->next_.compareAndSet(nextNode, newNode, false, false);
        return false;
    }

    bool isNodeIn(NODE *node, NODE *startNode, NODE *endNode, bool reverse) {
        NODE *checkNode = startNode;
        bool nodeInChain = false;
        while (checkNode != nullptr && checkNode != endNode) {
            if (checkNode == node)
                nodeInChain = true;
            if (reverse && hasPrev())
                checkNode = getPrev(checkNode);
            else
                checkNode = checkNode->Next();
        }
        return nodeInChain && checkNode == endNode;
    }

    void fixInsert(NODE *node, NODE *prevNode, NODE *nextNode) {
        NODE *actualNextNode = nextNode;
        if (nextNode->isDeleted()) {
            actualNextNode = LockFreeList<NODE>::GetNext(nextNode);
        }
        NODE *actualPrevNode = prevNode;
        if (actualPrevNode == nullptr)
            updateHead(node);
        else if (prevNode->isDeleted()) {
            actualPrevNode = getValidPrev(actualNextNode);
            if (actualPrevNode == nullptr)
                updateHead(node);
        }
        if ((actualNextNode == nullptr || actualPrevNode == actualNextNode) && node != this->tail_.load())
            LockFreeList<NODE>::updateTail(node);
        else if (actualPrevNode == node)
            actualPrevNode = this->head_.load();

        // Fix forward chain
        if (actualPrevNode != nullptr && !isNodeIn(node, actualPrevNode, actualNextNode, false)) {
            actualNextNode = LockFreeList<NODE>::GetNext(actualPrevNode);
            if (actualNextNode != nextNode)
                LockFreeList<NODE>::updateNext(node, actualNextNode, nextNode);
            LockFreeList<NODE>::updateNext(actualPrevNode, node);
        }
        else if (node->Next() == nextNode)
            LockFreeList<NODE>::updateNext(node, actualNextNode, nextNode);

        // Fix backward chain
        if (hasPrev())
            fixPrev(nextNode, actualPrevNode, actualNextNode);
    }
};

#endif /* LOCK_FREE_BILIST_H__ */
