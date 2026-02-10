#include "FreeListAllocator.h"
#include "types.h"
#include <cstddef>
#include <cstdint>
#include <sys/types.h>


FreeListAllocator::FreeListAllocator(uint8_t* ptr, size_t size)
    : memory(ptr), capacity(size) {

        // ensure the initial memory is aligned to allow Node storage
        uintptr_t current_addr = (uintptr_t) ptr;

        // round up the to next alignment of Node
        uintptr_t aligned_addr = (current_addr + alignof(Node) - 1) & ~(alignof(Node) - 1);

        size_t adjustment = aligned_addr - current_addr;

        if (capacity < adjustment + sizeof(Node)) {
            free_list = nullptr;
            capacity = 0;
            return; // memory too small to hold even one node
        }

        free_list = (Node*) aligned_addr;
        free_list->block_size = capacity - adjustment - sizeof(Node);
        free_list->next = nullptr;

}

void* FreeListAllocator::alloc(size_t size, size_t alignment) {

    size = std::max(size, MIN_ALLOC_SIZE); // enforce size
    alignment = std::max(alignment, MIN_ALIGNMENT); // enforce alignment

    Node* prev = nullptr;
    Node* curr = free_list;

    while (curr != nullptr) { // find first block that is large enough

        // calculate alignment padding (front)
        // we want the payload (after header) to be aligned

        uintptr_t current_addr = (uintptr_t) curr;
        uintptr_t raw_payload_addr = current_addr + sizeof(AllocationHeader); // where payload would naturally start

        size_t alignment_padding = 0;
        size_t misalign = raw_payload_addr & (alignment - 1);
        // this is equivalent to % modulo (only when alignment is a power of 2)
        // but better because bitwise AND is faster
        if (misalign != 0) {
            alignment_padding = alignment - misalign;
        }

        // calcuate required size and alignment slack (back)
        // total used memory must be a multiple of alignof(Node)
        // so that the *next* block starts on a valid address
        size_t required_size = sizeof(AllocationHeader) + alignment_padding + size;
        size_t alignment_slack = 0;
        size_t remainder = required_size % alignof(Node);
        if (remainder != 0) {
            alignment_slack = alignof(Node) - remainder;
            required_size += alignment_slack;
        }

        size_t total_available = curr->block_size + sizeof(Node);

        if (total_available >= required_size) {

            Node* next_free_node = curr->next;
            size_t leftover = total_available - required_size;

            if (leftover >= MIN_SPLIT_SIZE) {
                // split
                uintptr_t new_node_addr = current_addr + required_size;
                Node* new_node = (Node*) new_node_addr;
                new_node->block_size = leftover - sizeof(Node);
                new_node->next = next_free_node;
                next_free_node = new_node;

            } else {
                // no split, consume the extra leftover as slack
                alignment_slack += leftover;
                required_size += leftover;
            }

            // setup allocation header
            // header sits just before the aligned payload
            uintptr_t aligned_payload_addr = raw_payload_addr + alignment_padding;
            uintptr_t header_addr = aligned_payload_addr - sizeof(AllocationHeader);

            AllocationHeader* header = (AllocationHeader*) header_addr;
            // std::cout << "alloc_header is " << (uintptr_t) header << std::endl;

            // "padding" stores only the front offset (to rewind ptr in free)
            // "block size" stores requested size + back slack (to bridge gap in free)
            header->padding = alignment_padding;
            header->block_size = size + alignment_slack;

            // update free list
            if (prev == nullptr) {
                free_list = next_free_node;
            } else {
                prev->next = next_free_node;
            }

            return (void*)aligned_payload_addr;

        }

        prev = curr;
        curr = curr->next;
    }

    return nullptr;

}

void FreeListAllocator::free(void* ptr) {
    AllocationHeader* header = (AllocationHeader*)((uint8_t*)ptr - sizeof(AllocationHeader));
    // std::cout << "header inside free() is " << (uintptr_t)header << std::endl;
    Node* node = (Node*) ((uint8_t*)header - header->padding);

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
    } else {
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

void FreeListAllocator::print_free_list() {
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

void* FreeListAllocator::realloc(void* ptr, size_t new_size) {

    if (ptr == nullptr) return alloc(new_size, MIN_ALIGNMENT); // what alignment should i use here?
    if (new_size == 0) {
        free(ptr);
        return nullptr;
    }

    AllocationHeader* header = (AllocationHeader*) ((uint8_t*) ptr - sizeof(AllocationHeader));
    size_t old_size = header->block_size;


    // we need to (ptr + new_size) to be aligned

    size_t required_alignment = alignof(Node);

    uintptr_t split_addr = (uintptr_t) ptr + new_size;

    uintptr_t aligned_split_addr = (split_addr + required_alignment - 1) & ~(required_alignment - 1);

    size_t aligned_new_size = aligned_split_addr - (uintptr_t)ptr;


    // shrink, free the extra space
    if (aligned_new_size < old_size && old_size - aligned_new_size >= MIN_SPLIT_SIZE) {

        // ill try to convert the remaining block
        // as if its an allocated block and then send it to free

        AllocationHeader* extra_memory_header = (AllocationHeader*)((uint8_t*) ptr + aligned_new_size);
        // std::cout << "ptr is " << (uintptr_t)ptr << std::endl;
        // std::cout << "[DEBUG] extra_memory_header is " << (uintptr_t)extra_memory_header << std::endl;
        // std::cout << "is extra_memory header aligned? " << (((uintptr_t)extra_memory_header % 8) == 0) << std::endl;
        // std::cout << "new size is " << new_size << std::endl;
        // std::cout << " is ptr aligned" << ((uintptr_t)ptr % 8 == 0) << std::endl;

        size_t total_extra_memory = old_size - aligned_new_size;
        size_t usable_memory = total_extra_memory - sizeof(AllocationHeader);

        extra_memory_header->block_size = usable_memory;
        extra_memory_header->padding = 0;
        void* extra_memory = (void*)((uint8_t*) extra_memory_header + sizeof(AllocationHeader));

        // std::cout << "is extra_memory aligned " << ((uintptr_t)extra_memory % 8 == 0) << std::endl;
        // std::cout << "extra memory is " << (uintptr_t)extra_memory << std::endl;
        free(extra_memory);

        header->block_size = aligned_new_size;

        return ptr; // return the original memory, now with shrinked size

        // grow, allocate a new block, copy the data, and free the old one
    } else if (new_size > old_size) {

        void* new_ptr = alloc(new_size, MIN_ALIGNMENT);
        if (new_ptr == nullptr) return nullptr; //failed allocation
        std::memcpy(new_ptr, ptr, std::min(aligned_new_size, old_size));
        free(ptr);

        return new_ptr;
    }

    return ptr;

}
