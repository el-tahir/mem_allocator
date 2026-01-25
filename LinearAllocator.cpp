#include "LinearAllocator.h"
#include <cassert>


LinearAllocator::LinearAllocator(uint8_t* ptr, size_t size) 
: memory(ptr), capacity(size), offset(0) {
}

void* LinearAllocator::alloc(size_t size, size_t alignment) {

    assert((alignment != 0) && ((alignment & (alignment - 1)) == 0) && "alignment must be a power of 2");

    void* current_address = memory + offset + sizeof(AllocationHeader);

    uintptr_t addr = (uintptr_t)current_address;
    uintptr_t aligned_addr = (addr + alignment - 1) & ~(alignment - 1); // gets the aligned address

    uintptr_t padding = aligned_addr - addr; // padding

    if (offset + sizeof(AllocationHeader) + padding + size > capacity) return nullptr;

    offset += sizeof(AllocationHeader) + padding + size;

    AllocationHeader* header = (AllocationHeader*) (aligned_addr - sizeof(AllocationHeader));
    header->block_size = size;
    header->padding = padding; //adjustment

    return (void*)aligned_addr;

}

void LinearAllocator::free(void* ptr) {
    AllocationHeader* header = (AllocationHeader*)((uint8_t*)ptr - sizeof(AllocationHeader));
    std::cout << "block_size is " << header->block_size << std::endl;
    std::cout << "padding is " << +header->padding << std::endl;
}

void LinearAllocator::reset() {
    offset = 0;
}

size_t LinearAllocator::get_used() const {
    return offset;
}

size_t LinearAllocator::get_available() const {
    return capacity - offset;
}
