#include "FreeListAllocator.h"
#include "types.h"
#include <cstring>
#include <iostream>

int main() {
    constexpr size_t BUFFER_SIZE = 1024;

    FreeList::Allocator allocator;
    if (!FreeList::init(allocator,BUFFER_SIZE)) {
        return 1;
    }

    void* ptr = FreeList::alloc(allocator, 128, MIN_ALIGNMENT);
    if (ptr == nullptr) {
        std::cerr << "allocation failed" << std::endl;
        return 1;
    }

    std::memset(ptr, 0xAB, 128);
    FreeList::free(allocator, ptr);

    FreeList::destroy(allocator);

    std::cout << "allocator demo completed" << std::endl;

    return 0;
}
