#include <iostream>
#include <stdint.h>


class LinearAllocator {

private:
    uint8_t* memory; // pointer to the start of our memory block

    size_t capacity; // total size
    size_t offset; // current position

public:
    LinearAllocator(uint8_t* ptr, size_t size) 
    : memory(ptr), capacity(size), offset(0) {
    }

    void* alloc(size_t size) {

        if (offset + size > capacity) return nullptr;

        void* ptr = memory + offset;

        offset += size;

        return ptr;

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

    int* num1 = static_cast<int*>(allocator.alloc(sizeof(int)));
    if (num1 != nullptr) {
        *num1 = 42;
        std::cout << " allocated at: " << static_cast<void*>(num1) << std::endl;
        std::cout << " value stored: " << *num1 << std::endl;
        std::cout << " memory used: " << allocator.get_used() << " bytes" << std::endl;
    }

    int* num2 = static_cast<int*>(allocator.alloc(sizeof(int)));
    if (num2 != nullptr) {
        *num1 = 69;
        std::cout << " allocated at: " << static_cast<void*>(num2) << std::endl;
        std::cout << " value stored: " << *num2 << std::endl;
        std::cout << " memory used: " << allocator.get_used() << " bytes" << std::endl;
    }

    char* c = static_cast<char*>(allocator.alloc(sizeof(char)));
    if (c != nullptr) {
        *c = 'X';
        std::cout << " allocated at: " << static_cast<void*>(c) << std::endl;
        std::cout << " value stored: " << *c << std::endl;
        std::cout << " memory used: " << allocator.get_used() << " bytes" << std::endl;
    }

    // allocate too much

    std::cout << " available: " << allocator.get_available() << std::endl;
    void* too_much = allocator.alloc(2000);
    if (too_much == nullptr) {
        std::cout << "too much!!" << std::endl;
    } else {
        std::cout << "shouldnt happen" << std::endl;
    }
    
    return 0;
}