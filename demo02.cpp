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
  std::set<shared_ptr<LockFreeBiNode<uint64_t>>> nodes;
  std::mutex mtx;

  NodesItem() : nodes(), mtx() {}

  void add_node(shared_ptr<LockFreeBiNode<uint64_t>> node) {
    std::lock_guard<std::mutex> lock(mtx);
    nodes.insert(node);
  }

  shared_ptr<LockFreeBiNode<uint64_t>> remove_node() {
    std::lock_guard<std::mutex> lock(mtx);

    if (nodes.empty()) {
      return nullptr;
    }

    std::random_device rd;  // 获取一个随机数种子
    std::mt19937 gen(rd());  // 使用Mersenne Twister算法创建一个随机数生成器
    std::uniform_int_distribution<> dis(0, nodes.size() - 1);

    auto it = nodes.begin();
    std::advance(it, dis(gen));  // 移动迭代器到随机位置
    shared_ptr<LockFreeBiNode<uint64_t>> node = *it;
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
  std::vector<std::thread> threads01;
  std::vector<std::thread> threads02;
  std::vector<std::thread> threads03;
  std::vector<std::thread> threads04;

  std::vector<NodesItem> nodes01(100);
  std::vector<NodesItem> nodes02(100);

  seed = std::chrono::system_clock::now().time_since_epoch().count();
  LockFreeBiList<uint64_t> list;

  for (int i = 0; i < 99; ++i) {
    threads01.push_back(std::thread(
        [&](int n) {
          int count = 0;
          while (++count < 1000) {
            uint64_t random = generateRandomUint64();
            shared_ptr<LockFreeBiNode<uint64_t>> node =
                make_shared<LockFreeBiNode<uint64_t>>(random);
            if (!list.Append(node)) {
              std::cout << "-------------- append failed" << std::endl;
              std::this_thread::sleep_for(std::chrono::seconds(10));
            }
            nodes01[n].add_node(node);
          }
          printf("thread[%d] append done\n", n);
        },
        i));
  }

  for (int i = 0; i < 99; ++i) {
    threads02.push_back(std::thread(
        [&](int n) {
          int count = 0;
          while (++count < 1000) {
            uint64_t random = generateRandomUint64();
            shared_ptr<LockFreeBiNode<uint64_t>> node =
                make_shared<LockFreeBiNode<uint64_t>>(random);
            if (!list.InsertHead(node)) {
              std::cout << "-------------- insert failed" << std::endl;
              std::this_thread::sleep_for(std::chrono::seconds(10));
            }
            nodes02[n].add_node(node);
          }
          printf("thread[%d] insert head done\n", n);
        },
        i));
  }

  for (int i = 0; i < 99; ++i) {
    threads03.push_back(std::thread(
        [&](int n) {
          while (true) {
            shared_ptr<LockFreeBiNode<uint64_t>> node =
                nodes01[n].remove_node();
            if (node == nullptr) {
              printf("no node to remove\n");
              std::this_thread::sleep_for(std::chrono::seconds(10));
              continue;
            }

            if (!list.Remove(shared_ptr<LockFreeBiNode<uint64_t>>(node))) {
              std::cout << "-------------- remove failed" << std::endl;
              std::this_thread::sleep_for(std::chrono::seconds(1));
            } else {
              std::cout << "-------------- remove " << node << std::endl;
            }
          }
        },
        i));
  }

  for (int i = 0; i < 99; ++i) {
    threads04.push_back(std::thread(
        [&](int n) {
          while (true) {
            shared_ptr<LockFreeBiNode<uint64_t>> node =
                nodes02[n].remove_node();
            if (node == nullptr) {
              printf("no node to remove\n");
              std::this_thread::sleep_for(std::chrono::seconds(10));
              continue;
            }

            if (!list.Remove(shared_ptr<LockFreeBiNode<uint64_t>>(node))) {
              std::cout << "-------------- remove failed" << std::endl;
              std::this_thread::sleep_for(std::chrono::seconds(1));
            } else {
              std::cout << "-------------- remove2 " << node << std::endl;
            }
          }
        },
        i));
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
