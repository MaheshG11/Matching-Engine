#pragma once

#include <inttypes.h>
#include <cassert>
template <typename T>
class MemoryPool {
    private: 
        union Slot {
            T      data;
            Slot*  next;  // free list pointer when unused
        };
    public:
        MemoryPool(int32_t size);
        ~MemoryPool();

        T* Allocate();
        void Deallocate(T* node);

    private:
        Slot* arena_ = nullptr;
        Slot* free_list_ = nullptr;
};
#include "memory_pool.tpp"