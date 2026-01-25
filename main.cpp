#include <iostream>
#include <stdint.h>
#include <cassert>

struct AllocationHeader {
    size_t block_size; // size requested by user
    uint8_t padding;
};

// padding - header - user memory

class LinearAllocator {

private:
    uint8_t* memory; // pointer to the start of our memory block

    size_t capacity; // total size
    size_t offset; // current position

public:
    LinearAllocator(uint8_t* ptr, size_t size) 
    : memory(ptr), capacity(size), offset(0) {
    }

    void* alloc(size_t size, size_t alignment) {

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

    void free(void* ptr) {
        AllocationHeader* header = (AllocationHeader*)((uint8_t*)ptr - sizeof(AllocationHeader));
        std::cout << "block_size is " << header->block_size << std::endl;
        std::cout << "padding is " << +header->padding << std::endl;
    }

    void reset() {
        offset = 0;
    }

    size_t get_used() const {
        return offset;
    }

    size_t get_available() const {
        return capacity - offset;
    }

};

int main() {

    uint8_t buffer[1024];
    LinearAllocator allocator(buffer, sizeof(buffer));

    std::cout << "avaliable memoery : " << allocator.get_available() << std::endl;

    char* c = static_cast<char*>(allocator.alloc(sizeof(char), 4));
    if (c != nullptr) {
        *c = 'X';
        std::cout << " allocated at: " << static_cast<void*>(c) << std::endl;
        std::cout << " value stored: " << *c << std::endl;
        std::cout << " memory used: " << allocator.get_used() << " bytes" << std::endl;
        allocator.free(c);
    }

    int* num1 = static_cast<int*>(allocator.alloc(sizeof(int), 4));
    if (num1 != nullptr) {
        *num1 = 42;
        std::cout << " allocated at: " << static_cast<void*>(num1) << std::endl;
        std::cout << " value stored: " << *num1 << std::endl;
        std::cout << " memory used: " << allocator.get_used() << " bytes" << std::endl;
        allocator.free(num1);
    }

    return 0;
}