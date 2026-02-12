#pragma once

#include <stdint.h>

#include "types.h"

namespace Linear {
    struct Allocator {
        void* memory; // pointer to the start of our memory block
        size_t capacity; // total size
        size_t offset; // current position
    };
    bool init(Allocator& allocator, size_t total_size);

    void* alloc(Allocator& allocator, size_t size, size_t alignment);

    void free(Allocator& allocator, void* ptr);

    void destroy(Allocator& allocator);

    void reset(Allocator& allocator);

    size_t getUsed(const Allocator& allocator);

    size_t getAvailable(const Allocator& allocator);
}
