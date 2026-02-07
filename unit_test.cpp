#include <cstdint>
#include <iostream>
#include <cassert>
#include <sys/types.h>
#include <vector>
#include <algorithm>
#include <cstring>
#include "FreeListAllocator.h"
#include "LinearAllocator.h"

// helpers
#define TEST(name) void name()
#define RUN_TEST(name) \
    std::cout << "[RUN] " << #name << "..." << std::endl; \
    name(); \
    std::cout << "[PASS] " << #name << std::endl << std::endl;


// helper to align a pointer
uintptr_t align_forward(uintptr_t ptr, size_t alignment) {
    return (ptr + alignment - 1) & ~(alignment - 1);
}

// --- tests ---
TEST(test_freelist_basic) {
    const size_t SIZE = 1024;
    uint8_t buffer[SIZE];
    FreeListAllocator allocator(buffer, SIZE);

    void* p1 = allocator.alloc(100, 8);
    assert(p1 != nullptr);

    void* p2 = allocator.alloc(100, 8);
    assert(p2 != nullptr);
    assert(p1 != p2);

    allocator.free(p1);
    allocator.free(p2);

    // should be able to allocate the full size again (minus overhead)

    void* p3 = allocator.alloc(SIZE - 100, 8);
    assert(p3 != nullptr);
}

TEST(test_freelist_alignment) {
    const size_t SIZE = 1024;
    uint8_t buffer[SIZE];
    FreeListAllocator allocator(buffer, SIZE);

    //standarad alignment
    void* p1 = allocator.alloc(10, 8);
    assert((uintptr_t)p1 % 8 == 0);

    // large power-of-2 alignment
    void* p2 = allocator.alloc(10, 64);
    assert((uintptr_t)p2 % 64 == 0);

    // check that padding doesnt overlap
    assert((uintptr_t)p2 >= (uintptr_t)p1 + 10);

    allocator.free(p1);
    allocator.free(p2);
}

TEST(test_freelist_coalescence) {
    // tests that freeing A, B, C in various orders merges them back into one block
    //
    const size_t SIZE = 2048;
    uint8_t buffer[SIZE];
    FreeListAllocator allocator(buffer, SIZE);

    void* p1 = allocator.alloc(100, 8); // uses around 120 bytes (for header)
    void* p2 = allocator.alloc(100, 8);
    void* p3 = allocator.alloc(100, 8);

    void* filler = allocator.alloc(SIZE - 400, 8);

    allocator.free(p2);
    allocator.free(p1);
    allocator.free(p3);

    void* huge = allocator.alloc(300, 8);
    // this will only succeed if coalesceing worked

    assert(huge != nullptr);

    allocator.free(huge);
    allocator.free(filler);
}

TEST(test_realloc_growth) {
    const size_t SIZE = 1024;
    uint8_t buffer[SIZE];

    FreeListAllocator allocator(buffer, SIZE);

    // alloc small
    int* p = (int*) allocator.alloc(sizeof(int) * 10, 8);
    for (int i = 0; i < 10; i++) p[i] = i;

    // grow
    int* new_p = (int*)allocator.realloc(p, sizeof(int) * 20);

    for (int i = 0; i < 10; i++) assert(new_p[i] == i);

    allocator.free(new_p);

}

int main() {

    std::cout << "------unit tests-------" << std::endl;

    RUN_TEST(test_freelist_basic);
    RUN_TEST(test_freelist_alignment);
    RUN_TEST(test_freelist_coalescence);
    RUN_TEST(test_realloc_growth);

    return 0;
}
