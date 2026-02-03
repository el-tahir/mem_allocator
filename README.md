# Custom Memory Allocators

A C++ implementation of a free list and linear allocators, for educational purposes.

## Overview

- **FreeListAllocator**: Implements immediate coalescing on free and block splitting on allocation.
- **LinearAllocator**: Standard bump-pointer implementation for frame/scope-based allocations.
- **Validation**: `main.cpp` contains a randomized stress test that verifies data integrity and proper block merging.

## Build & Run

```bash
g++ main.cpp FreeListAllocator.cpp -o alloc
./alloc


I would appreciate any feedback!
