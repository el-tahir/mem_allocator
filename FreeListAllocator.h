#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <stdint.h>
#include <sys/types.h>

#include "types.h"

class FreeListAllocator {
private:
    uint8_t* memory;
    size_t capacity;


public:
Node* free_list;

    FreeListAllocator(uint8_t* ptr, size_t size);

    void* alloc(size_t size, size_t alignment);

    void free(void* ptr);

    void print_free_list();

    void* realloc(void* ptr, size_t new_size);

};
