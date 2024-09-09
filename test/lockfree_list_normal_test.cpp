#include "test_linkedlist.h"

#include <cstdint>

TEST_CASE_HEAD("normal test without concurrent, Push node from head and tail") {
    INIT_TYPE;

    ListType list;
    NodeType node1(1);

    list.InsertHead(&node1);
    REQUIRE(list.Head() == &node1);
    REQUIRE(list.Tail() == &node1);

    NodeType node2(2);
    list.Append(&node2);
    REQUIRE(list.Head() == &node1);
    REQUIRE(list.Tail() == &node2);

    NodeType node3(3);
    list.InsertHead(&node3);
    REQUIRE(list.Head() == &node3);
    REQUIRE(list.Tail() == &node2);

    REQUIRE(list.GetNext(&node3) == &node1);
    REQUIRE(list.GetNext(&node1) == &node2);
    REQUIRE(list.GetNext(&node2) == nullptr);

    REQUIRE(list.GetPrev(&node3) == nullptr);
    REQUIRE(list.GetPrev(&node1) == &node3);
    REQUIRE(list.GetPrev(&node2) == &node1);
}

TEST_CASE_HEAD("normal test without concurrent, Insert node in the middle)") {
    INIT_TYPE;

    ListType list;
    NodeType node1(1);
    list.InsertHead(&node1);

    NodeType node2(2);
    list.Append(&node2);
    REQUIRE(list.Head() == &node1);
    REQUIRE(list.Tail() == &node2);

    NodeType node3(3);
    list.Insert(&node3, &node1);
    REQUIRE(list.Head() == &node3);
    REQUIRE(list.Tail() == &node2);

    REQUIRE(list.GetNext(&node3) == &node1);
    REQUIRE(list.GetNext(&node1) == &node2);
    REQUIRE(list.GetNext(&node2) == nullptr);

    REQUIRE(list.GetPrev(&node3) == nullptr);
    REQUIRE(list.GetPrev(&node1) == &node3);
    REQUIRE(list.GetPrev(&node2) == &node1);
}

TEST_CASE_HEAD("normal test without concurrent, Remove one node") {
    INIT_TYPE;

    ListType list;
    NodeType node1(1);
    list.InsertHead(&node1);

    list.Remove(&node1);    
    REQUIRE(list.Head() == nullptr);
    REQUIRE(list.Tail() == nullptr);
}

TEST_CASE_HEAD("normal test without concurrent, Remove middle node") {
    INIT_TYPE;

    ListType list;
    NodeType node1(1);
    NodeType node2(2);
    NodeType node3(3);
    list.InsertHead(&node1);
    list.InsertHead(&node2);
    list.InsertHead(&node3);

    list.Remove(&node2);    
    REQUIRE(list.Head() == &node3);
    REQUIRE(list.Tail() == &node1);
    REQUIRE(list.GetPrev(&node3) == nullptr);
    REQUIRE(list.GetPrev(&node1) == &node3);
    REQUIRE(list.GetNext(&node1) == nullptr);
}
