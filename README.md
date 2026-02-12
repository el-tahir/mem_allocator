# Custom Memory Allocators

Educational C++ implementations of a free-list allocator, a linear (bump) allocator, and an STL allocator adaptor.

## What's Inside

- **FreeList**: First-fit, split-on-alloc, immediate coalescing on free. API is namespaced as `FreeList::Allocator` + `FreeList::{init, alloc, free, realloc, destroy}`.
- **Linear**: Bump-pointer allocator for frame/scope-based usage. Use `reset()` to free all at once.
- **STLAllocator**: Adaptor that plugs `FreeList::Allocator` into standard containers.

## Build and run all tests

```bash
make test-all
```
