#pragma once
#include <iostream>

struct AllocationHeader {
    size_t block_size; // size requested by user
    size_t padding;
};

struct Node {
    Node* next;
    size_t block_size; // total size, including header
};

const size_t MIN_ALLOC_SIZE = sizeof(Node);
const size_t MIN_SPLIT_SIZE = sizeof(AllocationHeader) + MIN_ALLOC_SIZE; // make sure it fits everything
const size_t MIN_ALIGNMENT = alignof(Node);
