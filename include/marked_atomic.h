#ifndef CombineAtomic_H__
#define CombineAtomic_H__

#include <atomic>
#include <iostream>
#include <memory>
#include <tuple>

#include <memory>
#include <atomic>
#include <utility>
#include <functional>

using namespace std;

template<typename T>
class MarkedAtomic {
public:
    MarkedAtomic() {
        set(nullptr, false);
    }

    MarkedAtomic(shared_ptr<T> ptr, bool mark) {
        set(ptr, mark);
    }

    virtual ~MarkedAtomic() {}

    // Compare and set the value atomically
    inline bool compareAndSet(shared_ptr<T> oldPtr, shared_ptr<T> newPtr, bool oldMark, bool newMark) {
        T* oldCombined = reinterpret_cast<T*>(uintptr_t(oldPtr.get()) | oldMark);
        T* newCombined = reinterpret_cast<T*>(uintptr_t(newPtr.get()) | newMark);
        return combined.compare_exchange_strong(oldCombined, newCombined);
    }

    // Get the current pointer and mark value
    inline pair<shared_ptr<T>, bool> get() const {
        T* combinedVal = combined.load();
        shared_ptr<T> ptr(reinterpret_cast<T*>(uintptr_t(combinedVal) & ~uintptr_t(1)), [](T *){});
        bool mark = uintptr_t(combinedVal) & uintptr_t(1);
        return {ptr, mark};
    }

    // Get the raw pointer
    inline shared_ptr<T> getPtr() const {
        return get().first;
    }

    // Check if the node is logically deleted
    inline bool isMarked() const {
        return get().second;
    }

    // Set the pointer and mark value
    inline void set(shared_ptr<T> ptr, bool mark) {
        uintptr_t value = reinterpret_cast<uintptr_t>(ptr.get()) | mark;
        combined.store(reinterpret_cast<T*>(value));
    }

private:
    atomic<T*> combined;  // Use uintptr_t to store both pointer and mark flag
};
#endif