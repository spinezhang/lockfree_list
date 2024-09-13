#ifndef LINKED_LIST_TEST_H__
#define LINKED_LIST_TEST_H__

#include <catch.hpp>
#include <memory>

#define TEST_MIDDLE_CHANGE
#include "lockfree_silist.h"
#include "lockfree_bilist.h"

#define TEST_TYPE_LIST 1
#define TEST_TYPE_BILIST 2
#define TEST_TYPE_TEMPLATE 3

#define TEST_TYPE TEST_TYPE_LIST

#if (TEST_TYPE == TEST_TYPE_TEMPLATE)
#define TEST_CASE_HEAD(description) \
TEMPLATE_TEST_CASE(description, "[lockfree][templated]", \
            (std::tuple<LockFreeList<int>, LockFreeNode<int>>), \
            (std::tuple<LockFreeBiList<int>, LockFreeBiNode<int>>))
#define INIT_TYPE \
    using ListType = typename std::tuple_element<0, TestType>::type; \
    using NodeType = typename std::tuple_element<1, TestType>::type;

#else
#define TEST_CASE_HEAD(description) TEST_CASE(description)

#if (TEST_TYPE == TEST_TYPE_LIST)
#define INIT_TYPE \
    using ListType = LockFreeSiList<int>; \
    using NodeType = LockFreeNode<int>;

#else
#define INIT_TYPE \
    using ListType = LockFreeBiList<int>; \
    using NodeType = LockFreeBiNode<int>;
#endif
#endif

template <typename T>
std::shared_ptr<T> MakeShared(T *node) {
    return shared_ptr<T>(node, [](T *){});
}

#endif