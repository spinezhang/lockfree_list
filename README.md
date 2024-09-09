# Lock-Free Single/bidirectional List

## Overview

This project implements a **lock-free single and bidirectional list** for concurrent applications using atomic operations. It is designed to allow safe insertion, deletion, and traversal of nodes in a multi-threads environment without the need for explicit locking. It leverages atomic primitives to ensure non-blocking synchronization, making it ideal for high-performance applications where latency due to locks is unacceptable.

## Features

- **Lock-Free Design**: Provides thread-safe insertion and deletion of nodes without the need for traditional locking.
- **Atomic Operations**: The use of `std::atomic` ensures that concurrent operations on the list are synchronized without locks.
- **Head and Tail Access**: Supports efficient operations at both ends of the list (head and tail).
- **Safe Deletion**: Nodes are marked as deleted and removed safely in concurrent environments.
- **Custom Test Hooks**: Optionally provides hooks for testing (`TestFunc`), allowing controlled observation during node insertion.


## How to Use

   ```cpp
   #include "lockfree_bilist.h"
   
   LockFreeBiList<int> list;
   
   // Append node from the tail
   LockFreeBiNode<int> node1(1);
   list.Append(&node1);
   
   // Insert node from the head
   LockFreeBiNode<int> node2(2);
   list.InsertHead(&node2);
   
   // Insert node in the middle
   LockFreeBiNode<int> node3(3);
   list.Insert(&node3, &node2);
   
   // Get the head node
   assert(&node1 == list.Head());
   
   // Get the tail node
   assert(&node2 == list.Tail());
   
   // Get the size
   assert(3 == list.Size());
   
   
   // Remove a node
   list.Remove(&node3);
   
   ```