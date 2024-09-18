//
// Created by Spine Zhang on 2024/9/6.
//

#ifndef LOCKFREE_NODE_H
#define LOCKFREE_NODE_H

#include <atomic>
#include <memory>

using namespace std;

template<typename T>
struct LockFreeNode {
    shared_ptr<LockFreeNode<T>> next_;
    T data_;

    LockFreeNode() {}

    LockFreeNode(T data) : data_(data) {}

    virtual ~LockFreeNode() {}

    virtual shared_ptr<LockFreeNode<T>> Next() {
        shared_ptr<LockFreeNode<T>> nextNode = atomic_load(&next_);
        if (nextNode == dummyNode)
            return nullptr;
        return nextNode;
    }

    virtual void SetNext(shared_ptr<LockFreeNode<T>> node) {
        atomic_store(&next_, node);
    }

    virtual bool isDeleted() {
        return atomic_load(&next_) == dummyNode;
    }

    virtual bool Delete(shared_ptr<LockFreeNode<T>> oldNext) {
        return atomic_compare_exchange_strong(&next_, &oldNext, dummyNode);
    }

    virtual bool CompareAndSetNext(shared_ptr<LockFreeNode<T>> oldNext, shared_ptr<LockFreeNode<T>> newNext) {
        return atomic_compare_exchange_strong(&next_, &oldNext, newNext);
    }

protected:
    static const shared_ptr<LockFreeNode<T>> dummyNode;
};

template <typename T>
const shared_ptr<LockFreeNode<T>> LockFreeNode<T>::dummyNode = shared_ptr<LockFreeNode<T>>(
    new LockFreeNode<T>, [](LockFreeNode<T> *) {} // Custom deleter (no-op)
);
#endif //LOCKFREE_NODE_H
