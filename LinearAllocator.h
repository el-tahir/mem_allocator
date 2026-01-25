#ifndef LINEAR_ALLOCATOR_H
#define LINEAR_ALLOCATOR_H

#include <stdint.h>

#include "types.h"

class LinearAllocator {

private:
    uint8_t* memory; // pointer to the start of our memory block

    size_t capacity; // total size
    size_t offset; // current position

public:
    LinearAllocator(uint8_t* ptr, size_t size);

    void* alloc(size_t size, size_t alignment);

    void free(void* ptr);

    void reset();

    size_t get_used() const;

    size_t get_available() const;

};

#endif