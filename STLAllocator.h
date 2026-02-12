#pragma once

#include "FreeListAllocator.h"
#include <cstddef>

#include <limits>
#include <new>
#include <type_traits>

template <typename T>
class STLAllocator {
    public:
        using value_type = T;

        using propagate_on_container_copy_assignment = std::true_type;
        using propagate_on_container_move_assignment = std::true_type;
        using propagate_on_container_swap = std::true_type;
        using is_always_equal = std::false_type;

        // a pointer to the custom allocator
        FreeList::Allocator* allocator;

        STLAllocator() : allocator(nullptr) {}

        // constructor
        STLAllocator(FreeList::Allocator& allocator_ref) : allocator(&allocator_ref) {}

        // copy constructor
        template <typename U>
        STLAllocator(const STLAllocator<U>& other) : allocator (other.allocator) {}

        // 1. allocate: translates "n" elements to bytes
        T* allocate (size_t n) {
            if (n > std::numeric_limits<size_t>::max() / sizeof(T))
                throw std::bad_alloc();

            if (allocator == nullptr)
                throw std::bad_alloc();

            void* ptr = FreeList::alloc(*allocator, n * sizeof(T), alignof(T));

            if (ptr == nullptr)
                throw std::bad_alloc();

            return static_cast<T*>(ptr);
        }

        // deallocate
        void deallocate(T* p, size_t) noexcept {
            if (allocator)
                FreeList::free(*allocator, p);
        }

        // equallity comparators (stateless allocators are always equal, but ours is stateful)
        // we say they are equal if they point to the same underlying allocator instance
        template <typename U>
        bool operator==(const STLAllocator<U>& other) const {
            return allocator == other.allocator;
        }

        template <typename U>
        bool operator!=(const STLAllocator<U>& other) const {
            return !(*this == other);
        }


};
