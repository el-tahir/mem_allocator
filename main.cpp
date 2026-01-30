#include <alloca.h>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdint.h>
#include <sys/types.h>
#include "types.h"


const size_t MIN_ALLOC_SIZE = sizeof(Node);
const size_t MIN_SPLIT_SIZE = sizeof(Node) + 1;
const size_t MIN_ALIGNMENT = alignof(Node);

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

        size = std::max(size, MIN_ALLOC_SIZE); // enforce size
        alignment = std::max(alignment, MIN_ALIGNMENT); // enforce alignment

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
            header->padding = 0; // we assume perfect padding (for now)

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

        Node* node = (Node*) header;
        size_t physical_size = header->block_size + header->padding + sizeof(AllocationHeader);
        node->block_size = physical_size - sizeof(Node);

        Node* prev = nullptr;
        Node* curr = free_list;

        // find position such that prev < node < curr
        while ( curr != nullptr && (uint8_t*) curr < (uint8_t*) node) { // need to convert to uint8_t* for pointer arithmetic
            prev = curr;
            curr = curr->next;
        }

        if (prev == nullptr) {
            node->next = free_list;
            free_list = node;
        }
        else {
            prev->next = node;
            node->next = curr;
        }


        // join next
        if (node->next != nullptr && ((uint8_t*) node + sizeof(Node) + node->block_size == (uint8_t*) node->next)) {
            node->block_size += sizeof(Node) + node->next->block_size;
            node->next = node->next->next;
        }

        // join prev
        if (prev != nullptr && (uint8_t*)prev + sizeof(Node) + prev->block_size == (uint8_t*) node) {
            prev->block_size += sizeof(Node) + node->block_size;
            prev->next = node->next;
        }

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

        std::cout << "-------------------------------------" << std::endl;

    }

    void* realloc(void* ptr, size_t new_size) {

        if (ptr == nullptr) return alloc(new_size, MIN_ALIGNMENT); // what alignment should i use here?
        if (new_size == 0) {
            free(ptr);
            return nullptr;
        }

        AllocationHeader* header = (AllocationHeader*) ((uint8_t*) ptr - sizeof(AllocationHeader));
        size_t old_size = header->block_size;

        // shrink, free the extra space
        if (new_size < old_size && old_size - new_size >= MIN_SPLIT_SIZE) {

            // ill try to convert the remaining block
            // as if its an allocated block and then send it to free

            AllocationHeader* extra_memory_header = (AllocationHeader*)((uint8_t*) ptr + new_size);
            size_t total_extra_memory = old_size - new_size;
            size_t usable_memory = total_extra_memory - sizeof(AllocationHeader);

            extra_memory_header->block_size = usable_memory;
            extra_memory_header->padding = 0;
            void* extra_memory = (void*)((uint8_t*) extra_memory_header + sizeof(AllocationHeader));

            free(extra_memory);

            header->block_size = new_size;

            return ptr; // return the original memory, now with shrinked size

            // grow, allocate a new block, copy the data, and free the old one
        } else if (new_size > old_size) {

            void* new_ptr = alloc(new_size, MIN_ALIGNMENT);
            if (new_ptr == nullptr) return nullptr; //failed allocation
            std::memcpy(new_ptr, ptr, std::min(new_size, old_size));
            free(ptr);

            return new_ptr;
        }

        return ptr;

    }

};


int main() {

    uint8_t buffer[1024];

    FreeListAllocator allocator = FreeListAllocator(buffer, sizeof(buffer));
    allocator.print_free_list();

    std::cout << "allocate 100 bytes and fill with 'A'" << std::endl;
    char* ptr = (char*)allocator.alloc(100, 1);
    std::memset(ptr, 'A', 100);
    std::cout << "realloc to size 200" << std::endl;
    ptr = (char*)allocator.realloc(ptr, 200);
    std::cout << "first 10 bytes after resizing to 200" << std::endl;
    for (int i = 0; i < 10; i++) {
        std::cout << ptr[i] << ", ";
    }
    std::cout << std::endl;
    allocator.print_free_list();

    std::cout << "realloc to size 50" << std::endl;
    ptr = (char*)allocator.realloc(ptr, 50);
    std::cout << "first 10 bytes after resizig to 50" << std::endl;
    for (int i = 0; i < 10; i++) {
        std::cout << ptr[i] << ", ";
    }
    std::cout << std::endl;
    allocator.print_free_list();

    return 0;
}
