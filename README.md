# Custom Memory Allocators

Educational C++ implementations of a free-list allocator, a linear (bump) allocator, and an STL allocator adaptor.

## What's Inside

- **FreeListAllocator**: First-fit, split-on-alloc, immediate coalescing on free. Supports `alloc`, `free`, and `realloc`.
- **LinearAllocator**: Bump-pointer allocator for frame/scope-based usage. Use `reset()` to free all at once.
- **STLAllocator**: Adaptor that plugs `FreeListAllocator` into standard containers.
- **Tests**: `main.cpp` includes a randomized stress test and an STL integration test.

## Files

- `FreeListAllocator.h/.cpp`: Free-list allocator implementation.
- `LinearAllocator.h/.cpp`: Linear allocator implementation.
- `STLAllocator.h`: STL allocator adaptor.
- `types.h`: Shared `AllocationHeader`, `Node`, and size/alignment constants.
- `main.cpp`: Stress test + STL test.

## Build & Run

```bash
make
./alloc_test
```

Or run the test target directly:

```bash
make test
```

## Sanitizers (optional)

```bash
make test_asan
make test_ubsan
```
