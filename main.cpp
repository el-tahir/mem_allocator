#include <iostream>
#include <stdint.h>
#include "types.h"

const size_t MIN_SPLIT_SIZE = sizeof(Node) + 1;

class FreeListAllocator {
private:
    uint8_t* memory;
    size_t capacity;
    Node* free_list;

public:

    FreeListAllocator(uint8_t* ptr, size_t size)
        : memory(ptr), capacity(size) {
            free_list = (Node*)memory;
            free_list->block_size = capacity - sizeof(Node);
            free_list->next = nullptr;
    }

    void* alloc(size_t size, size_t alignment) {
        
        Node* prev = nullptr;
        Node* curr = free_list;

        while (curr != nullptr) {
            if(curr->block_size >= size + sizeof(AllocationHeader)) {
                break;
            }
            prev = curr;
            curr = curr->next;
        }
        
        if (curr == nullptr) return nullptr;

        size_t total_available = sizeof(Node) + curr->block_size;
        size_t required_size = sizeof(AllocationHeader) + size;
        size_t leftover = total_available - required_size;

        // can we create a new node with the leftover?
        if (leftover >= MIN_SPLIT_SIZE) {

            // split the block

            Node* curr_next = curr->next; // save curr->next before overriding. //segfault happened here because it gets overriden!!!

            uint8_t* allocation_start = (uint8_t*) curr;

            AllocationHeader* header = (AllocationHeader*) allocation_start;
            header->block_size = size;

            uint8_t* new_node_addr = allocation_start + required_size;

            Node* new_node = (Node*) new_node_addr;
            new_node->block_size = leftover - sizeof(Node);
            new_node->next = curr_next;

            // remove current node from list and insert new_node
            if (prev == nullptr) {
                free_list = new_node;
            } else {
                prev->next = new_node;
            }

            return (void*) (allocation_start + sizeof(AllocationHeader));

        } else { // not enough space, use entire block

            AllocationHeader* header = (AllocationHeader*) curr;
            header->block_size = size;

            // remove this node from free_list
            if (prev == nullptr) {
                free_list = curr->next;
            } else {
                prev->next = curr->next;
            }

            return (void*)((uint8_t*)curr + sizeof(AllocationHeader));

        }

    }

    void free(void* ptr) {
        AllocationHeader* header = (AllocationHeader*)((uint8_t*)ptr - sizeof(AllocationHeader));
        size_t total_size = header->block_size;

        Node* node = (Node*) header;
        

        node->next = free_list;
        free_list = node;

        node->block_size = total_size + sizeof(AllocationHeader) - sizeof(Node);

    }

    void print_free_list() {
        std::cout << "free list: " << std::endl;
        
        Node* curr = free_list;
        int count = 0;
        while (curr != nullptr) {
            std::cout << "block: " << count++ << " , size = " << curr->block_size << " at " << (void*)curr << std::endl;
            curr = curr->next;
        }
        if (count == 0) std::cout << "empty!" << std::endl;

    }
};


int main() {

    uint8_t buffer[1024];
    FreeListAllocator allocator(buffer, sizeof(buffer));

    std::cout << "initial state " << std::endl;

    allocator.print_free_list();

    std::cout << "allocating 100 bytes" << std::endl;
    void* ptr1 = allocator.alloc(100, 8);
    std::cout << "allocated at: " << ptr1 << std::endl;
    allocator.print_free_list();

    std::cout << "allocating 200 bytes" << std::endl;
    void* ptr2 = allocator.alloc(200, 8);
    std::cout << "allocated at: " << ptr2 << std::endl;
    allocator.print_free_list();

    std::cout << "freeing first alloc" << std::endl;
    allocator.free(ptr1);
    allocator.print_free_list();

    std::cout << "allocating 50 bytes" << std::endl;
    void* ptr3 = allocator.alloc(50, 8);
    std::cout << "allocated at: " << ptr3 << std::endl;
    allocator.print_free_list();

    return 0;
}