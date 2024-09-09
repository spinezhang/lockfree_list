#ifndef CombineAtomic_H__
#define CombineAtomic_H__

#include <atomic>
#include <iostream>
#include <memory>
#include <tuple>

template<typename T>
class MarkedAmotic {
public:
    MarkedAmotic() {
        set(nullptr, false);
    }
    MarkedAmotic(T* initialPtr, bool initialMark) {
        set(initialPtr, initialMark);
    }

    virtual ~MarkedAmotic() {}

    bool compareAndSet(T* oldPtr, T* newPtr, bool oldMark, bool newMark) {
        T* oldCombined = combine(oldPtr, oldMark);
        T* newCombined = combine(newPtr, newMark);
        return combined.compare_exchange_strong(oldCombined, newCombined);
    }

    std::pair<T*, bool> get() const {
        uintptr_t combinedVal = reinterpret_cast<uintptr_t>(combined.load());
        T* ptr = reinterpret_cast<T*>(combinedVal & ~uintptr_t(1));
        bool mark = combinedVal & uintptr_t(1);
        return {ptr, mark};
    }

    T* getPtr() const {
        return get().first;
    }

    bool isMarked() const {
        return get().second;
    }

    void set(T* newPtr, bool newMark) {
        combined.store(combine(newPtr, newMark));
    }

    bool isNull() const {
        return getPtr() == nullptr;
    }

private:
    std::atomic<T *> combined;

    static T* combine(T* ptr, bool mark) {
        return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(ptr) | mark);
    }
};

#endif