//
// Created by Spine Zhang on 2024/9/6.
//

#ifndef LOCKFREE_NODE_H
#define LOCKFREE_NODE_H

#include "marked_atomic.h"

template<typename T>
struct LockFreeNode {
    MarkedAmotic<LockFreeNode<T>> next_;
    T data_;

    LockFreeNode() {}

    LockFreeNode(T data) : data_(data) {}

    virtual ~LockFreeNode() {}

    virtual LockFreeNode<T> *Next() {
        return next_.getPtr();
    }

    virtual bool isDeleted() {
        return next_.isMarked();
    }
};


#endif //LOCKFREE_NODE_H
