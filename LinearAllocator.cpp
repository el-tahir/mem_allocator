#include "LinearAllocator.h"
#include <cassert>
#include <cstdint>

namespace Linear {
    bool init(Allocator& allocator, size_t total_size) {
        allocator.memory = std::malloc(total_size);
        if (allocator.memory == nullptr) {
            return false;
        }
        allocator.capacity = total_size;
        allocator.offset = 0;
        return true;
    }

    void* alloc(Allocator& allocator, size_t size, size_t alignment) {

        assert((alignment != 0) && ((alignment & (alignment - 1)) == 0) && "alignment must be a power of 2");

        uintptr_t addr = (uintptr_t)allocator.memory + allocator.offset + sizeof(AllocationHeader);

        uintptr_t aligned_addr = (addr + alignment - 1) & ~(alignment - 1); // gets the aligned address

        uintptr_t padding = aligned_addr - addr; // padding

        if (allocator.offset + sizeof(AllocationHeader) + padding + size > allocator.capacity) return nullptr;

        allocator.offset += sizeof(AllocationHeader) + padding + size;

        AllocationHeader* header = (AllocationHeader*) (aligned_addr - sizeof(AllocationHeader));
        header->block_size = size;
        header->padding = padding; //adjustment

        return (void*)aligned_addr;

    }

    void free(Allocator& allocator, void* ptr) {
        (void)allocator;
        AllocationHeader* header = (AllocationHeader*)((uint8_t*)ptr - sizeof(AllocationHeader));
        std::cout << "block_size is " << header->block_size << std::endl;
        std::cout << "padding is " << +header->padding << std::endl;
    }

    void destroy(Allocator& allocator) {
        if (allocator.memory) {
            std::free(allocator.memory);
            allocator.memory = nullptr;
        }
    }

    void reset(Allocator& allocator) {
        allocator.offset = 0;
    }

    size_t getUsed(const Allocator& allocator) {
        return allocator.offset;
    }

    size_t getAvailable(const Allocator& allocator) {
        return allocator.capacity - allocator.offset;
    }
}
