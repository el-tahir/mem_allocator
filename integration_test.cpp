#include "FreeListAllocator.h"
#include "STLAllocator.h"
#include "types.h"
#include <cstdint>
#include <cstring>
#include <vector>
#include <ctime>
#include <cassert>
#include <map>
#include <string>

// type defs for stl test
using CharAllocator = STLAllocator<char>;
using StringAllocator = STLAllocator<std::basic_string<char, std::char_traits<char>, CharAllocator>>;

using MyString = std::basic_string<char, std::char_traits<char>, CharAllocator>;

using MyVec = std::vector<MyString, StringAllocator>;

using MapAllocator = STLAllocator<std::pair<const MyString, MyVec>>;
using MyMap = std::map<MyString, MyVec, std::less<MyString>, MapAllocator>;

void stl_test() {
    std::cout << "starting stl test ..." << std::endl;

    const size_t BUFFER_SIZE = 1024 * 1024;
    uint8_t* buffer = new uint8_t[BUFFER_SIZE];

    {
        FreeListAllocator allocator(buffer, BUFFER_SIZE);

        STLAllocator<char> stl_alloc(allocator);

        MyMap map(stl_alloc);

        for (int i = 0; i < 50; i++) {
            std::string tmp = "key_" + std::to_string(i);
            MyString key(tmp.c_str(), stl_alloc);

            MyVec vec(stl_alloc);
            vec.push_back(MyString("value_a", stl_alloc));
            vec.push_back(MyString("value_b", stl_alloc));
            vec.push_back(MyString("value_c", stl_alloc));

            map[key] = vec;
        }

        std::cout << "map populated with " << map.size() << " items " << std::endl;

        MyString lookup_key1(std::string("key_10").c_str(), stl_alloc);
        MyString lookup_key2(std::string("key_49").c_str(), stl_alloc);


        assert(map[lookup_key1][1] == "value_b");
        assert(map[lookup_key2][0] == "value_a");

        std::cout << "data verification successful " << std::endl;

        // stl containters destroyed here,
        // their destructors should call deallocate(), which calls free()
    }

    delete[] buffer;

    std::cout << "stl test passed!" << std::endl;
    std::cout << "-------------------" << std::endl;
}




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
    stl_test();

    return 0;
}
