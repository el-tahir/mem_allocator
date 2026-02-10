#include "FreeListAllocator.h"
#include "types.h"

#include <cstdint>
#include <cstring>
#include <iostream>

int main() {
    constexpr size_t BUFFER_SIZE = 1024;
    uint8_t buffer[BUFFER_SIZE];

    FreeListAllocator allocator(buffer, BUFFER_SIZE);

    void* ptr = allocator.alloc(128, MIN_ALIGNMENT);
    if (!ptr) {
        std::cerr << "allocation failed" << std::endl;
        return 1;
    }

    std::memset(ptr, 0xAB, 128);
    allocator.free(ptr);

    std::cout << "allocator demo completed" << std::endl;
    return 0;
}
