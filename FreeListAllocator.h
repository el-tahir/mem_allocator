#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <stdint.h>
#include <sys/types.h>

#include "types.h"

namespace FreeList {
struct Allocator {
    void* memory;
    size_t capacity;
    Node* free_list;
};

bool init(Allocator& allocator, size_t total_size);

void* alloc(Allocator& allocator, size_t size, size_t alignment);

void free(Allocator& allocator, void* ptr);

void printFreeList(Allocator& allocator);

void* realloc(Allocator& allocator, void* ptr, size_t new_size);

void destroy(Allocator& allocator);
}
