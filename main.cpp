#include "FreeListAllocator.h"
#include "types.h"
#include <cstdint>
#include <cstring>
#include <vector>
#include <ctime>
#include <cassert>

void stress_test() {
    struct Allocation {
        void* ptr;
        size_t size;
    };

    uint8_t buffer [10 * 1024];
    std::vector<Allocation> allocations;

    FreeListAllocator allocator = FreeListAllocator(buffer, sizeof(buffer));

    std::srand(std::time(0));

    std::cout << "starting stress test..." << std::endl;

    for (int i = 0; i < 1000; i++) {

        int random = std::rand() % 10;

        if (random < 7) { //alloc
            size_t random_size = std::rand() % 100 + 1;
            void* ptr = allocator.alloc(random_size, MIN_ALIGNMENT);
            if (ptr != nullptr) {
                std::memset(ptr, 0xAA, random_size);
                allocations.push_back({ptr, random_size});
            }


        } else { // free
            if (allocations.empty()) continue;
            int random_index = std::rand() % allocations.size();
            Allocation& alloc = allocations[random_index];

            uint8_t* bytes = (uint8_t*)alloc.ptr;

            for (size_t i = 0; i < alloc.size; i++) {
                assert(bytes[i] == 0xAA && "memory corrupted!");
            }

            allocator.free(alloc.ptr);

            allocations.erase(allocations.begin() + random_index);

        }

    }

    // final cleanup
    std::cout << "freeing " << allocations.size() << " reminaing allocations..." << std::endl;
    for (auto& alloc : allocations) {
        allocator.free(alloc.ptr);
    }

    std::cout << "final state should be one block" << std::endl;

    allocator.print_free_list();

}


int main() {

    stress_test();

    return 0;
}
