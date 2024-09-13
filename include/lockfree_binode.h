//
// Created by Spine Zhang on 2024/9/6.
//

#ifndef LOCKFREE_BINODE_H
#define LOCKFREE_BINODE_H

#include "lockfree_node.h"

template <typename T>
struct LockFreeBiNode : LockFreeNode<T>
{
    shared_ptr<LockFreeBiNode<T>> prev_;

    LockFreeBiNode(T data) : LockFreeNode<T>(data) {}

    virtual ~LockFreeBiNode() {}

    // Override Next to return bidirectional node
    shared_ptr<LockFreeNode<T>> Next() override {
        // Cast the base class's next pointer to the derived class type
        return LockFreeNode<T>::Next();
    }

    // Check if this node is logically deleted
    bool isDeleted() override {
        return LockFreeNode<T>::isDeleted();
    }

    // Retrieve the previous node
    shared_ptr<LockFreeBiNode<T>> Prev() {
        return prev_;
    }
};
#endif //BINODE_H
