#include <cassert>
#include <iostream>
#include <map>
#include <mutex>
#include <random>
#include <set>
#include <thread>
#include <vector>

#include "include/lockfree_bilist.h"
#include "include/lockfree_silist.h"

unsigned seed;

struct NodesItem {
  std::set< shared_ptr<LockFreeNode<uint64_t>>> nodes;
  std::mutex mtx;

  NodesItem() : nodes(), mtx() {}

  void add_node( shared_ptr<LockFreeNode<uint64_t>> node) {
    std::lock_guard<std::mutex> lock(mtx);
    nodes.insert(node);
  }

   shared_ptr<LockFreeNode<uint64_t>> remove_node() {
    std::lock_guard<std::mutex> lock(mtx);

    if (nodes.empty()) {
      return nullptr;
    }

    std::random_device rd;  // 获取一个随机数种子
    std::mt19937 gen(rd());  // 使用Mersenne Twister算法创建一个随机数生成器
    std::uniform_int_distribution<> dis(0, nodes.size() - 1);

    auto it = nodes.begin();
    std::advance(it, dis(gen));  // 移动迭代器到随机位置
     shared_ptr<LockFreeNode<uint64_t>> node = *it;
    nodes.erase(it);

    return node;
  }
};

uint64_t generateRandomUint64() {
  std::mt19937_64 generator(seed);
  return generator();
}

int main() {
  std::cout << "hello cxx" << std::endl;

  seed = std::chrono::system_clock::now().time_since_epoch().count();
  LockFreeSiList<uint64_t> list;

  std::vector<std::thread> threads01;
  std::vector<std::thread> threads02;
  std::vector<std::thread> threads03;
  std::vector<std::thread> threads04;

  std::vector<NodesItem> nodes01(100);
  std::vector<NodesItem> nodes02(100);

  for (int i = 0; i < 99; ++i) {
    threads01.push_back(std::thread([&]() {
      while (true) {
        uint64_t random = generateRandomUint64();
        shared_ptr<LockFreeNode<uint64_t>> node = make_shared<LockFreeNode<uint64_t>>(random);
        list.Append(node);
        nodes01[i].add_node(node);
      }
    }));
  }

  for (int i = 0; i < 99; ++i) {
    threads02.push_back(std::thread([&]() {
      while (true) {
        uint64_t random = generateRandomUint64();
        shared_ptr<LockFreeNode<uint64_t>> node = make_shared<LockFreeNode<uint64_t>>(random);
        list.InsertHead(node);
        nodes02[i].add_node(node);
      }
    }));
  }

  for (int i = 0; i < 99; ++i) {
    threads03.push_back(std::thread([&]() {
      while (true) {
         shared_ptr<LockFreeNode<uint64_t>> node = nodes01[i].remove_node();
        if (node == nullptr) {
          continue;
        }
        list.Remove(shared_ptr<LockFreeNode<uint64_t>>(node));
          // delete node;
      }
    }));
  }

  for (int i = 0; i < 99; ++i) {
    threads04.push_back(std::thread([&]() {
      while (true) {
         shared_ptr<LockFreeNode<uint64_t>> node = nodes02[i].remove_node();
        if (node == nullptr) {
          continue;
        }

        list.Remove(shared_ptr<LockFreeNode<uint64_t>>(node));
          // delete node;
      }
    }));
  }

  auto thread = std::thread([&]() {
    while (true) {
      std::cout << "size: " << list.Size() << std::endl;
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  });
  thread.join();

  for (auto& thread : threads01) {
    if (thread.joinable()) {
      thread.join();
    }
  }

  for (auto& thread : threads02) {
    if (thread.joinable()) {
      thread.join();
    }
  }

  for (auto& thread : threads03) {
    if (thread.joinable()) {
      thread.join();
    }
  }

  for (auto& thread : threads04) {
    if (thread.joinable()) {
      thread.join();
    }
  }

  return 0;
}
