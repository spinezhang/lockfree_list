#define CATCH_CONFIG_MAIN
#include "test_linkedlist.h"

#include <tbb/concurrent_vector.h>

#include <cstdint>

#define PREPARE_LIST() \
    NodeType node1(1); \
    list.Append(&node1); \
    NodeType node2(2); \
    list.Append(&node2); \
    NodeType node3(3); \
    list.Append(&node3);


#ifdef TEST_MIDDLE_CHANGE
TEST_CASE_HEAD("Concurrently insert one node before the prev_ of nextNode is changed (step1)") {
    INIT_TYPE;

    ListType list;
    PREPARE_LIST();

    NodeType node5(5);
    NodeType node4(4);

    list.Insert(&node4, &node2, true, [&](int step, NodeType *curentNode, NodeType *prevNode, NodeType *nextNode) {
        switch(step) {
            case 1:
                list.InsertBetween(&node5, &node1, &node2);
            break;
            case 2:
            default:
                break;
        }
    });
    
    REQUIRE(true == list.CheckConsistence(5));
}

TEST_CASE_HEAD("Concurrently insert two nodes before the prev_ of nextNode is changed (step1)") {
    INIT_TYPE;

    ListType list;
    PREPARE_LIST();

    NodeType node5(5);
    NodeType node6(6);
    NodeType node4(4);

    list.Insert(&node4, &node2, true, [&](int step, NodeType *curentNode, NodeType *prevNode, NodeType *nextNode) {
        switch(step) {
            case 1:
                list.InsertBetween(&node5, &node1, &node2);
                list.InsertBetween(&node6, &node5, &node2);
                break;
            case 2:
            default:
                break;
        }
    });

    REQUIRE(true == list.CheckConsistence(6));
}

TEST_CASE_HEAD("Concurrently insert one node after the prev_ of nextNode is changed (step2)") {
    INIT_TYPE;

    ListType list;
    PREPARE_LIST();

    NodeType node5(5);
    NodeType node4(4);

    list.Insert(&node4, &node2, true, [&](int step, NodeType *curentNode, NodeType *prevNode, NodeType *nextNode) {
        switch(step) {
            case 2:
                list.InsertBetween(&node5, &node4, &node2);
                break;
            case 1:
            default:
                break;
        }
    });

    REQUIRE(true == list.CheckConsistence(5));
}

#define CHECK_TEST_STEP3() \
    if (!std::is_same<NodeType, LockFreeBiNode<int>>::value) { \
        return; \
    }

TEST_CASE_HEAD("Concurrently after the node successfully chained (step3)") {
    INIT_TYPE;
    CHECK_TEST_STEP3();

    ListType list;
    PREPARE_LIST();

    NodeType node5(5);
    NodeType node4(4);

    list.Insert(&node4, &node2, true, [&](int step, NodeType *curentNode, NodeType *prevNode, NodeType *nextNode) {
        switch(step) {
            case 3:
                list.InsertBetween(&node5, &node4, &node2);
                break;
            case 1:
            case 2:
            default:
                break;
        }
    });

    REQUIRE(true == list.CheckConsistence(5));
}

TEST_CASE_HEAD("Concurrently delete nextNode at (step1)") {
    INIT_TYPE;

    ListType list;
    PREPARE_LIST();
    NodeType node4(4);
    list.Append(&node4);

    NodeType node5(5);

    list.Insert(&node5, &node3, true, [&](int step, NodeType *curentNode, NodeType *prevNode, NodeType *nextNode) {
        switch(step) {
            case 1:
                list.Remove(&node3);
                break;
            case 2:
            default:
                break;
        }
    });
    
    REQUIRE(true == list.CheckConsistence(4));
}

TEST_CASE_HEAD("Concurrently delete nextNode at (step2)") {
    INIT_TYPE;

    ListType list;
    PREPARE_LIST();
    NodeType node4(4);
    list.Append(&node4);

    NodeType node5(5);

    list.Insert(&node5, &node3, true, [&](int step, NodeType *curentNode, NodeType *prevNode, NodeType *nextNode) {
        switch(step) {
            case 2:
                list.Remove(&node3);
                break;
            case 1:
            default:
                break;
        }
    });
    
    REQUIRE(true == list.CheckConsistence(4));
}

TEST_CASE_HEAD("Concurrently delete nextNode at (step3)") {
    INIT_TYPE;
    CHECK_TEST_STEP3();

    ListType list;
    PREPARE_LIST();
    NodeType node4(4);
    list.Append(&node4);

    NodeType node5(5);

    list.Insert(&node5, &node3, true, [&](int step, NodeType *curentNode, NodeType *prevNode, NodeType *nextNode) {
        switch(step) {
            case 3:
                list.Remove(&node3);
                break;
            case 1:
            case 2:
            default:
                break;
        }
    });
    
    REQUIRE(true == list.CheckConsistence(4));
}

TEST_CASE_HEAD("Concurrently delete multiple nextNode at (step1)") {
    INIT_TYPE;

    ListType list;
    PREPARE_LIST();
    NodeType node4(4);
    list.Append(&node4);

    NodeType node5(5);

    list.Insert(&node5, &node3, true, [&](int step, NodeType *curentNode, NodeType *prevNode, NodeType *nextNode) {
        switch(step) {
            case 1:
                list.Remove(&node2);
                list.Remove(&node3);
                break;
            case 2:
            default:
                break;
        }
    });
    
    REQUIRE(true == list.CheckConsistence(3));
}

TEST_CASE_HEAD("Concurrently inert before deleting tail at (step1)") {
    INIT_TYPE;

    ListType list;
    PREPARE_LIST();
    NodeType node4(4);
    list.Append(&node4);

    NodeType node5(5);

    list.Insert(&node5, &node4, true, [&](int step, NodeType *curentNode, NodeType *prevNode, NodeType *nextNode) {
        switch(step) {
            case 1:
                list.Remove(&node4);
                break;
            case 2:
            default:
                break;
        }
    });
    
    REQUIRE(true == list.CheckConsistence(4));
}

TEST_CASE_HEAD("Concurrently inert before deleting tail at (step2)") {
    INIT_TYPE;

    ListType list;
    PREPARE_LIST();
    NodeType node4(4);
    list.Append(&node4);

    NodeType node5(5);

    list.Insert(&node5, &node4, true, [&](int step, NodeType *curentNode, NodeType *prevNode, NodeType *nextNode) {
        switch(step) {
            case 2:
                list.Remove(&node4);
                break;
            case 1:
            default:
                break;
        }
    });
    
    REQUIRE(true == list.CheckConsistence(4));
}

TEST_CASE_HEAD("Concurrently inert before deleting tail at (step3)") {
    INIT_TYPE;
    CHECK_TEST_STEP3();

    ListType list;
    PREPARE_LIST();
    NodeType node4(4);
    list.Append(&node4);

    NodeType node5(5);

    list.Insert(&node5, &node4, true, [&](int step, NodeType *curentNode, NodeType *prevNode, NodeType *nextNode) {
        switch(step) {
            case 3:
                list.Remove(&node4);
                break;
            case 1:
            case 2:
            default:
                break;
        }
    });
    
    REQUIRE(true == list.CheckConsistence(4));
}

TEST_CASE_HEAD("Concurrently delete multiple nextNode with tail at (step1)") {
    INIT_TYPE;

    ListType list;
    PREPARE_LIST();
    NodeType node4(4);
    list.Append(&node4);

    NodeType node5(5);

    list.Insert(&node5, &node4, true, [&](int step, NodeType *curentNode, NodeType *prevNode, NodeType *nextNode) {
        switch(step) {
            case 1:
                list.Remove(&node4);
                list.Remove(&node3);
                break;
            case 2:
            default:
                break;
        }
    });
    
    REQUIRE(true == list.CheckConsistence(3));
}

TEST_CASE_HEAD("Concurrently delete head at (step1)") {
    INIT_TYPE;

    ListType list;
    PREPARE_LIST();
    NodeType node4(4);
    list.Append(&node4);

    NodeType node5(5);

    list.Insert(&node5, &node2, true, [&](int step, NodeType *curentNode, NodeType *prevNode, NodeType *nextNode) {
        switch(step) {
            case 1:
                list.Remove(&node1);
                break;
            case 2:
            default:
                break;
        }
    });
    
    REQUIRE(true == list.CheckConsistence(4));
}

TEST_CASE_HEAD("Concurrently delete head at (step2)") {
    INIT_TYPE;

    ListType list;
    PREPARE_LIST();
    NodeType node4(4);
    list.Append(&node4);

    NodeType node5(5);

    list.Insert(&node5, &node2, true, [&](int step, NodeType *curentNode, NodeType *prevNode, NodeType *nextNode) {
        switch(step) {
            case 2:
                list.Remove(&node1);
                break;
            case 1:
            default:
                break;
        }
    });
    
    REQUIRE(true == list.CheckConsistence(4));
}

TEST_CASE_HEAD("Concurrently delete head at (step3)") {
    INIT_TYPE;
    CHECK_TEST_STEP3();

    ListType list;
    PREPARE_LIST();
    NodeType node4(4);
    list.Append(&node4);

    NodeType node5(5);

    list.Insert(&node5, &node2, true, [&](int step, NodeType *curentNode, NodeType *prevNode, NodeType *nextNode) {
        switch(step) {
            case 3:
                list.Remove(&node1);
                break;
            case 1:
            case 2:
            default:
                break;
        }
    });
    
    REQUIRE(true == list.CheckConsistence(4));
}

TEST_CASE_HEAD("Concurrently delete multiple nextNode with head at (step1)") {
    INIT_TYPE;

    ListType list;
    PREPARE_LIST();
    NodeType node4(4);
    list.Append(&node4);

    NodeType node5(5);

    list.Insert(&node5, &node1, true, [&](int step, NodeType *curentNode, NodeType *prevNode, NodeType *nextNode) {
        switch(step) {
            case 1:
                list.Remove(&node1);
                list.Remove(&node2);
                break;
            case 2:
            default:
                break;
        }
    });
    
    REQUIRE(true == list.CheckConsistence(3));
}
#endif

const int initListSize = 1000;
const int threadCount = 4000;
std::vector<std::thread> threads;
#if (TEST_TYPE == TEST_TYPE_LIST)
tbb::concurrent_vector<LockFreeNode<int>*> nodes;
tbb::concurrent_vector<LockFreeNode<int> *> deletedNodes;
#else
tbb::concurrent_vector<LockFreeBiNode<int> *> nodes;
tbb::concurrent_vector<LockFreeBiNode<int> *> deletedNodes;
#endif

TEST_CASE_HEAD("Multi-threads concurrency random operations") {
    INIT_TYPE;

    ListType list;

    for (int i = 0; i < initListSize; i++) {
        NodeType *node = new NodeType(i);
        list.Append(node);
        nodes.push_back(node);
    }

    // initialize random
    std::random_device randomDevice;
    std::minstd_rand linearRand(randomDevice());
    std::uniform_int_distribution<int>distrib(0, 2 * initListSize);

    for (int i = 0; i < threadCount; i++) {
        threads.push_back(std::thread([&]() {
            for (int i = 0; i < initListSize; i++) {
                // std::this_thread::yield();
                int index = distrib(linearRand);
                if (index >= initListSize) {
                    // NodeType *node = new NodeType(index);
                    // list.Insert(node, nodes[index]);
                    // nodes.push_back(node);
                    NodeType *node = nodes[index - initListSize];
                    if (!node->isDeleted()) {
                        if (list.Remove(node))
                            deletedNodes.push_back(node);
                    }
                } else {
                    NodeType *node = new NodeType(index);
                    list.Insert(node, nodes[index]);
                    nodes.push_back(node);
                }
            }
        }));
    }

    for (auto& th : threads) {
        th.join();
    }
    printf("\n");
    REQUIRE(true == list.CheckConsistence(nodes.size() - deletedNodes.size()));
}