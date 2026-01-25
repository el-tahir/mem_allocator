#include <iostream>

struct AllocationHeader {
    size_t block_size; // size requested by user
    size_t padding;
};

struct Node {
    Node* next;
    size_t block_size; // total size, including header
};