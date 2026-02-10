#pragma once

#include <stdint.h>

#include "types.h"

struct LinearAllocator {
    uint8_t* memory; // pointer to the start of our memory block
    size_t capacity; // total size
    size_t offset; // current position
};

void linear_allocator_init(LinearAllocator* allocator, uint8_t* ptr, size_t size);

void* linear_allocator_alloc(LinearAllocator* allocator, size_t size, size_t alignment);

void linear_allocator_free(LinearAllocator* allocator, void* ptr);

void linear_allocator_reset(LinearAllocator* allocator);

size_t linear_allocator_get_used(const LinearAllocator* allocator);

size_t linear_allocator_get_available(const LinearAllocator* allocator);
