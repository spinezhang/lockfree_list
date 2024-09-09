//
// Created by Spine Zhang on 2024/9/6.
//

#ifndef LOCKFREE_BINODE_H
#define LOCKFREE_BINODE_H

#include "lockfree_node.h"

template<typename T>
struct LockFreeBiNode : LockFreeNode<T> {
    std::atomic<LockFreeBiNode<T> *> prev_;

    LockFreeBiNode(T data) : LockFreeNode<T>(data) {}

    virtual ~LockFreeBiNode() {}

    LockFreeBiNode<T> *Next() override {
        return dynamic_cast<LockFreeBiNode<int>*>(LockFreeNode<T>::Next());
    }

    bool isDeleted() override {
        return LockFreeNode<T>::isDeleted();
    }

    LockFreeBiNode<T> *Prev() {
        return prev_.load();
    }
};

#endif //BINODE_H
