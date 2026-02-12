#pragma once

#include <stdint.h>

#include "types.h"

struct LinearAllocator {
    void* memory; // pointer to the start of our memory block
    size_t capacity; // total size
    size_t offset; // current position
};

bool linear_allocator_init(LinearAllocator* allocator, size_t total_size);

void* linear_allocator_alloc(LinearAllocator* allocator, size_t size, size_t alignment);

void linear_allocator_free(LinearAllocator* allocator, void* ptr);

void linear_allocator_reset(LinearAllocator* allocator);

size_t linear_allocator_get_used(const LinearAllocator* allocator);

size_t linear_allocator_get_available(const LinearAllocator* allocator);

void linear_allocator_destroy(LinearAllocator* allocator);
