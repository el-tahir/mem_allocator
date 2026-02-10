#include <cstdint>
#include <iostream>
#include <cassert>
#include <sys/types.h>
#include <cstring>
#include "FreeListAllocator.h"

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
TEST(test_basic) {
    const size_t SIZE = 1024;
    uint8_t buffer[SIZE];
    FreeListAllocator allocator;
    free_list_allocator_init(&allocator, buffer, SIZE);

    void* p1 = free_list_allocator_alloc(&allocator, 100, 8);
    assert(p1 != nullptr);

    void* p2 = free_list_allocator_alloc(&allocator, 100, 8);
    assert(p2 != nullptr);
    assert(p1 != p2);

    free_list_allocator_free(&allocator, p1);
    free_list_allocator_free(&allocator, p2);

    // should be able to allocate the full size again (minus overhead)

    void* p3 = free_list_allocator_alloc(&allocator, SIZE - 100, 8);
    assert(p3 != nullptr);
}

TEST(test_alignment) {
    const size_t SIZE = 1024;
    uint8_t buffer[SIZE];
    FreeListAllocator allocator;
    free_list_allocator_init(&allocator, buffer, SIZE);

    //standarad alignment
    void* p1 = free_list_allocator_alloc(&allocator, 10, 8);
    assert((uintptr_t)p1 % 8 == 0);

    // large power-of-2 alignment
    void* p2 = free_list_allocator_alloc(&allocator, 10, 64);
    assert((uintptr_t)p2 % 64 == 0);

    // check that padding doesnt overlap
    assert((uintptr_t)p2 >= (uintptr_t)p1 + 10);

    free_list_allocator_free(&allocator, p1);
    free_list_allocator_free(&allocator, p2);
}

TEST(test_coalescence) {
    // tests that freeing A, B, C in various orders merges them back into one block
    //
    const size_t SIZE = 2048;
    uint8_t buffer[SIZE];
    FreeListAllocator allocator;
    free_list_allocator_init(&allocator, buffer, SIZE);

    void* p1 = free_list_allocator_alloc(&allocator, 100, 8); // uses around 120 bytes (for header)
    void* p2 = free_list_allocator_alloc(&allocator, 100, 8);
    void* p3 = free_list_allocator_alloc(&allocator, 100, 8);

    void* filler = free_list_allocator_alloc(&allocator, SIZE - 400, 8);

    free_list_allocator_free(&allocator, p2);
    free_list_allocator_free(&allocator, p1);
    free_list_allocator_free(&allocator, p3);

    void* huge = free_list_allocator_alloc(&allocator, 300, 8);
    // this will only succeed if coalesceing worked

    assert(huge != nullptr);

    free_list_allocator_free(&allocator, huge);
    free_list_allocator_free(&allocator, filler);
}

TEST(test_realloc_growth) {
    const size_t SIZE = 1024;
    uint8_t buffer[SIZE];

    FreeListAllocator allocator;
    free_list_allocator_init(&allocator, buffer, SIZE);

    // alloc small
    int* p = (int*) free_list_allocator_alloc(&allocator, sizeof(int) * 10, 8);
    for (int i = 0; i < 10; i++) p[i] = i;

    // grow
    int* new_p = (int*)free_list_allocator_realloc(&allocator, p, sizeof(int) * 20);

    for (int i = 0; i < 10; i++) assert(new_p[i] == i);

    free_list_allocator_free(&allocator, new_p);

}

TEST(test_realloc_shrink_alignment_safety) {
    const size_t SIZE = 1024;
    uint8_t buffer[SIZE];
    FreeListAllocator allocator;
    free_list_allocator_init(&allocator, buffer, SIZE);

    // allocate block aligned to 8
    // total size should be approx 128 + overhead
    void* ptr = free_list_allocator_alloc(&allocator, 128, 8);
    assert((uintptr_t)ptr % 8 == 0);

    // shrink to a size that is not a multiple of 8
    // if allocator splits at 'ptr + 33', the new free block
    // will start at a misaligned address
    // the allocator will try to write a 'Node' struct to a misaligned address
    ptr = free_list_allocator_realloc(&allocator, ptr, 33);

    //try to allocate from that split area
    // the allocation should come from that hole
    void* split_area = free_list_allocator_alloc(&allocator, 8, 8);

    // should be aligned
    assert((uintptr_t) split_area % 8 == 0);

    free_list_allocator_free(&allocator, ptr);
    free_list_allocator_free(&allocator, split_area);

}

int main() {

    std::cout << "------unit tests-------" << std::endl;

    RUN_TEST(test_basic);
    RUN_TEST(test_alignment);
    RUN_TEST(test_coalescence);
    RUN_TEST(test_realloc_growth);
    RUN_TEST(test_realloc_shrink_alignment_safety);

    return 0;
}
