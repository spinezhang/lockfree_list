#include "test_linkedlist.h"

#include <cstdint>

TEST_CASE_HEAD("normal test without concurrent, Push node from head and tail") {
    INIT_TYPE;

    ListType list;
    NodeType node1(1);

    shared_ptr<NodeType> node1Ptr = MakeShared(&node1);
    list.InsertHead(node1Ptr);
    REQUIRE(list.Head() == node1Ptr);
    REQUIRE(list.Tail() == node1Ptr);

    NodeType node2(2);
    shared_ptr<NodeType> node2Ptr = MakeShared(&node2);
    list.Append(node2Ptr);
    REQUIRE(list.Head() == node1Ptr);
    REQUIRE(list.Tail() == node2Ptr);

    NodeType node3(3);
    shared_ptr<NodeType> node3Ptr = MakeShared(&node3);
    list.InsertHead(node3Ptr);
    REQUIRE(list.Head() == node3Ptr);
    REQUIRE(list.Tail() == node2Ptr);

    REQUIRE(list.GetNext(node3Ptr) == node1Ptr);
    REQUIRE(list.GetNext(node1Ptr) == node2Ptr);
    REQUIRE(list.GetNext(node2Ptr) == nullptr);

    REQUIRE(list.GetPrev(node3Ptr) == nullptr);
    REQUIRE(list.GetPrev(node1Ptr) == node3Ptr);
    REQUIRE(list.GetPrev(node2Ptr) == node1Ptr);
}

TEST_CASE_HEAD("normal test without concurrent, Insert node in the middle)") {
    INIT_TYPE;

    ListType list;
    NodeType node1(1);
    shared_ptr<NodeType> node1Ptr = MakeShared(&node1);
    list.InsertHead(node1Ptr);

    NodeType node2(2);
    shared_ptr<NodeType> node2Ptr = MakeShared(&node2);
    list.Append(node2Ptr);
    REQUIRE(list.Head() == node1Ptr);
    REQUIRE(list.Tail() == node2Ptr);

    NodeType node3(3);
    shared_ptr<NodeType> node3Ptr = MakeShared(&node3);
    list.Insert(node3Ptr, node1Ptr);
    REQUIRE(list.Head() == node3Ptr);
    REQUIRE(list.Tail() == node2Ptr);

    REQUIRE(list.GetNext(node3Ptr) == node1Ptr);
    REQUIRE(list.GetNext(node1Ptr) == node2Ptr);
    REQUIRE(list.GetNext(node2Ptr) == nullptr);

    REQUIRE(list.GetPrev(node3Ptr) == nullptr);
    REQUIRE(list.GetPrev(node1Ptr) == node3Ptr);
    REQUIRE(list.GetPrev(node2Ptr) == node1Ptr);
}

TEST_CASE_HEAD("normal test without concurrent, Remove one node") {
    INIT_TYPE;

    ListType list;
    NodeType node1(1);
    shared_ptr<NodeType> node1Ptr = MakeShared(&node1);
    list.InsertHead(node1Ptr);

    list.Remove(node1Ptr);    
    REQUIRE(list.Head() == nullptr);
    REQUIRE(list.Tail() == nullptr);
}

TEST_CASE_HEAD("normal test without concurrent, Remove middle node") {
    INIT_TYPE;

    ListType list;
    NodeType node1(1);
    NodeType node2(2);
    NodeType node3(3);
    shared_ptr<NodeType> node1Ptr = MakeShared(&node1);
    shared_ptr<NodeType> node2Ptr = MakeShared(&node2);
    shared_ptr<NodeType> node3Ptr = MakeShared(&node3);
    list.InsertHead(node1Ptr);
    list.InsertHead(node2Ptr);
    list.InsertHead(node3Ptr);

    list.Remove(node2Ptr);    
    REQUIRE(list.Head() == node3Ptr);
    REQUIRE(list.Tail() == node1Ptr);
    REQUIRE(list.GetPrev(node3Ptr) == nullptr);
    REQUIRE(list.GetPrev(node1Ptr) == node3Ptr);
    REQUIRE(list.GetNext(node1Ptr) == nullptr);
}
