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
#include <utility>

// type defs for stl test
using CharAllocator = STLAllocator<char>;
using MyString = std::basic_string<char, std::char_traits<char>, CharAllocator>;
using StringAllocator = STLAllocator<MyString>;
using MyVec = std::vector<MyString, StringAllocator>;
using MapAllocator = STLAllocator<std::pair<const MyString, MyVec>>;
using MyMap = std::map<MyString, MyVec, std::less<MyString>, MapAllocator>;

static MyString make_my_string(const std::string& s, const CharAllocator& alloc) {
    return MyString(s.c_str(), alloc);
}

static void assert_map_matches_expected(
    const MyMap& map,
    const std::map<std::string, std::vector<std::string>>& expected,
    const CharAllocator& alloc
) {
    assert(map.size() == expected.size());

    for (const auto& [key_std, expected_values] : expected) {
        MyString key = make_my_string(key_std, alloc);
        auto it = map.find(key);
        assert(it != map.end());

        const MyVec& values = it->second;
        assert(values.size() == expected_values.size());

        for (size_t i = 0; i < expected_values.size(); ++i) {
            assert(values[i] == expected_values[i].c_str());
        }
    }
}

void stl_test() {
    std::cout << "starting stl test ..." << std::endl;

    const size_t BUFFER_SIZE = 1024 * 1024;

    FreeList::Allocator allocator;
    assert(FreeList::init(allocator, BUFFER_SIZE));

    {
        STLAllocator<char> stl_alloc(allocator);
        MyMap map(stl_alloc);
        std::map<std::string, std::vector<std::string>> expected;

        // build a map with long keys/values so string buffers are allocated from the custom allocator
        for (int i = 0; i < 200; ++i) {
            std::string key_std = "key_" + std::to_string(i) + "_" + std::string(48, char('a' + (i % 26)));
            MyString key = make_my_string(key_std, stl_alloc);
            MyVec vec(stl_alloc);
            std::vector<std::string> expected_vec;

            int value_count = 1 + (i % 5);
            for (int j = 0; j < value_count; ++j) {
                std::string value_std = "value_" + std::to_string(i) + "_" + std::to_string(j) +
                                        "_" + std::string(80, char('A' + (j % 26)));
                vec.push_back(make_my_string(value_std, stl_alloc));
                expected_vec.push_back(value_std);
            }

            auto inserted = map.emplace(std::move(key), std::move(vec));
            assert(inserted.second);
            expected.emplace(std::move(key_std), std::move(expected_vec));
        }

        assert_map_matches_expected(map, expected, stl_alloc);

        // mutate vectors for a subset of keys
        for (int i = 0; i < 200; i += 3) {
            std::string key_std = "key_" + std::to_string(i) + "_" + std::string(48, char('a' + (i % 26)));
            MyString key = make_my_string(key_std, stl_alloc);
            auto it = map.find(key);
            assert(it != map.end());

            std::string extra_std = "extra_" + std::to_string(i) + "_" + std::string(96, 'Z');
            it->second.push_back(make_my_string(extra_std, stl_alloc));
            expected[key_std].push_back(extra_std);
        }

        // erase a deterministic subset
        for (int i = 0; i < 200; i += 5) {
            std::string key_std = "key_" + std::to_string(i) + "_" + std::string(48, char('a' + (i % 26)));
            MyString key = make_my_string(key_std, stl_alloc);
            size_t erased = map.erase(key);
            assert(erased == 1);
            expected.erase(key_std);
        }

        // replace vectors for another subset
        for (int i = 1; i < 200; i += 7) {
            std::string key_std = "key_" + std::to_string(i) + "_" + std::string(48, char('a' + (i % 26)));
            if (expected.find(key_std) == expected.end()) continue;

            MyString key = make_my_string(key_std, stl_alloc);
            auto it = map.find(key);
            assert(it != map.end());

            MyVec replacement(stl_alloc);
            std::vector<std::string> replacement_expected;
            for (int j = 0; j < 4; ++j) {
                std::string value_std = "replacement_" + std::to_string(i) + "_" + std::to_string(j) +
                                        "_" + std::string(72, 'R');
                replacement.push_back(make_my_string(value_std, stl_alloc));
                replacement_expected.push_back(value_std);
            }
            it->second = std::move(replacement);
            expected[key_std] = std::move(replacement_expected);
        }

        assert_map_matches_expected(map, expected, stl_alloc);

        // copy and move semantics under the adaptor
        MyMap copied(stl_alloc);
        copied = map;
        assert_map_matches_expected(copied, expected, stl_alloc);

        MyMap moved(stl_alloc);
        moved = std::move(copied);
        assert_map_matches_expected(moved, expected, stl_alloc);

        MyMap reassigned(stl_alloc);
        reassigned = moved;
        assert_map_matches_expected(reassigned, expected, stl_alloc);

        std::cout << "stl adaptor data integrity and container semantics verified" << std::endl;
    }

    // after all STL containers are destroyed, allocator should recover a large contiguous block
    assert(allocator.free_list != nullptr);
    assert(allocator.free_list->next == nullptr);
    void* recovery = FreeList::alloc(allocator, BUFFER_SIZE / 2, MIN_ALIGNMENT);
    assert(recovery != nullptr);
    FreeList::free(allocator, recovery);

    FreeList::destroy(allocator);

    std::cout << "stl test passed!" << std::endl;
    std::cout << "-------------------" << std::endl;
}




void stress_test() {
    struct Allocation {
        void* ptr;
        size_t size;
    };

    std::vector<Allocation> allocations;

    FreeList::Allocator allocator;
    assert(FreeList::init(allocator, 10 * 1024));

    std::srand(std::time(0));

    std::cout << "starting stress test..." << std::endl;

    for (int i = 0; i < 1000; i++) {

        int random = std::rand() % 10;

        if (random < 7) { //alloc
            size_t random_size = std::rand() % 100 + 1;
            void* ptr = FreeList::alloc(allocator, random_size, MIN_ALIGNMENT);
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

            FreeList::free(allocator, alloc.ptr);

            allocations.erase(allocations.begin() + random_index);

        }

    }

    // final cleanup
    std::cout << "freeing " << allocations.size() << " reminaing allocations..." << std::endl;
    for (auto& alloc : allocations) {
        FreeList::free(allocator, alloc.ptr);
    }

    std::cout << "final state should be one block" << std::endl;

    FreeList::printFreeList(allocator);

    FreeList::destroy(allocator);

}


int main() {

    stress_test();
    stl_test();

    return 0;
}
