#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <stdint.h>
#include <sys/types.h>

#include "types.h"

struct FreeListAllocator {
    void* memory;
    size_t capacity;
    Node* free_list;
};

void free_list_allocator_init(FreeListAllocator* allocator, void* ptr, size_t size);

void* free_list_allocator_alloc(FreeListAllocator* allocator, size_t size, size_t alignment);

void free_list_allocator_free(FreeListAllocator* allocator, void* ptr);

void free_list_allocator_print_free_list(FreeListAllocator* allocator);

void* free_list_allocator_realloc(FreeListAllocator* allocator, void* ptr, size_t new_size);
