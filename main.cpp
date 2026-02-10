#include "FreeListAllocator.h"
#include "types.h"

#include <cstdint>
#include <cstring>
#include <iostream>
#include <new>

int main() {
    constexpr size_t BUFFER_SIZE = 1024;

    void* buffer = std::malloc(BUFFER_SIZE);

    if (buffer == nullptr) {
        throw std::bad_alloc();
    }

    FreeListAllocator allocator;
    free_list_allocator_init(&allocator, buffer, BUFFER_SIZE);

    void* ptr = free_list_allocator_alloc(&allocator, 128, MIN_ALIGNMENT);
    if (ptr == nullptr) {
        std::cerr << "allocation failed" << std::endl;
        return 1;
    }

    std::memset(ptr, 0xAB, 128);
    free_list_allocator_free(&allocator, ptr);

    std::cout << "allocator demo completed" << std::endl;

    std::free(buffer);
    return 0;
}
