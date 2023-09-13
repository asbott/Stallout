#include "pch.h"

#include "Engine/debug/tests.h"

#include "Engine/logger.h"
#include "Engine/memory.h"
#include "Engine/containers.h"

#ifndef _ST_DISABLE_ASSERTS

void log_duration(const char* msg, std::chrono::steady_clock::time_point start, std::chrono::steady_clock::time_point end) {
    log_info("{}: {} ms", msg, (float)(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()) / 1000.f);
}

#ifdef _ST_CONFIG_DEBUG
#define MEM_TEST(...) { ST_ASSERT(start_mem == Global_Allocator::get_used_mem(), "Memory leak. Last was {}, now {}", start_mem, Global_Allocator::get_used_mem()); start_mem = Global_Allocator::get_used_mem(); }
#else
#define MEM_TEST(...)
#endif
void run_tests() {
    IN_DEBUG_ONLY(size_t start_mem = Global_Allocator::get_used_mem(););

    // Testing single allocation and deallocation
    MEM_TEST();
    {
        int* single = ST_NEW(int, 42);
        ST_ASSERT(single != nullptr && *single == 42, "Single allocation failed");
        ST_DELETE(single);
    }

    // Testing multiple allocations and deallocations
    MEM_TEST();
    {
        std::vector<int*> multiples;
        for(int i = 0; i < 100; ++i) {
            multiples.push_back(ST_NEW(int, i));
        }
        for(int i = 0; i < multiples.size(); ++i) {
            ST_ASSERT(multiples[i] != nullptr && *multiples[i] == i, "Multiple allocation failed");
            ST_DELETE(multiples[i]);
        }
    }

    // Testing multithreading
    MEM_TEST();
    {
        
        
        struct Complex {
            int x;
            std::string s;
            Complex(int x, const std::string& s) : x(x), s(s) {}
        };
        

        Array<std::thread> threads;
        for(int i = 0; i < 100; ++i) {
            threads.push_back(std::thread([](){
                Complex* complex = ST_NEW(Complex, 42, "hello");
                ST_ASSERT(complex != nullptr && complex->x == 42 && complex->s == "hello", "Complex object allocation failed");
                ST_DELETE(complex);
            }));
        }
        for(auto& t : threads) {
            t.join();
        }
    }

    #ifndef DISABLE_ALLOCATORS
    // Testing mass reusable memory
    MEM_TEST();
    {
        struct Data {
            char data[128] = "Test";
        };

        std::vector<Data*> arr;
        auto num = 2;
        for (size_t i = 0; i < num; i++) {
            arr.push_back(ST_NEW(Data));
        }

        std::vector<void*> prev_pointers;

        for (size_t i = 0; i < num; i++) {
            prev_pointers.push_back(arr[i]);
            ST_DELETE(arr[i]);
        }

        // Go backwards because allocator allocatoes last free block
        for (s64 i = num - 1; i >= 0; i--) {
            arr[i] = ST_NEW(Data);

            ST_ASSERT(prev_pointers[i] == arr[i], "Mass reusable memory test failed: memory not reused");
            ST_ASSERT(strcmp(arr[i]->data, "Test") == 0, "Mass reusable memory test failed: memory corrupt");
        }

        for (s64 i = num - 1; i >= 0; i--) {
            ST_DELETE(arr[i]);
        }
    }
    #endif

    // Testing lifecycle of complex objects
    MEM_TEST();
    {
        struct Complex {
            int x;
            std::string s;
            Complex(int x, const std::string& s) : x(x), s(s) {}
        };
        Complex* complex = ST_NEW(Complex, 42, "hello");
        ST_ASSERT(complex != nullptr && complex->x == 42 && complex->s == "hello", "Complex object allocation failed");
        ST_DELETE(complex);
    }

    // Testing large allocations
    MEM_TEST();
    {
        void* p = ST_MEM(1024 * 1000 * 20);
        ST_ASSERT(p, "Big allocation failed");
        ST_FREE(p, 1024 * 1000 * 20);
    }

    // Testing memory overwrite
    MEM_TEST();
    {
        int* overwrite_test = ST_NEW(int, 42);
        int* adjacent_memory = ST_NEW(int, 43);
        *adjacent_memory = 44;  // Simulating memory overwrite
        ST_ASSERT(*overwrite_test != 44, "Memory overwrite test failed");
        ST_DELETE(overwrite_test);
        ST_DELETE(adjacent_memory);
    }

    // Testing boundary conditions
    MEM_TEST();
    {
        void* boundary = ST_MEM(SIZE_MAX); // Trying to allocate the maximum possible size.
        ST_ASSERT(boundary == nullptr, "Boundary condition test failed");
        
    }

    // Stress testing for concurrency
    MEM_TEST();
    {
        std::vector<std::thread> threads;
        for(int i = 0; i < 100; ++i) {
            threads.push_back(std::thread([](){
                for (int j = 0; j < 1000; ++j) {
                    int* multi = ST_NEW(int, j);
                    ST_ASSERT(multi != nullptr && *multi == j, "Concurrency stress test failed");
                    ST_DELETE(multi);
                }
            }));
        }
        for(auto& t : threads) {
            t.join();
        }
    }

    #ifndef DISABLE_ALLOCATORS
    // Testing memory initialization
    MEM_TEST();
    {
        char* mem = static_cast<char*>(ST_MEM(100));
        bool isMemset = true;
        for (int i = 0; i < 100; ++i) {
            if (mem[i] != 0) {
                isMemset = false;
                break;
            }
        }
        ST_ASSERT(isMemset, "Memory Initialization test failed");
        ST_FREE(mem, 100);
    }
    

    // Testing memory initialization (small)
    MEM_TEST();
    {
        char* mem = static_cast<char*>(ST_MEM(7));
        bool isMemset = true;
        for (int i = 0; i < 7; ++i) {
            if (mem[i] != 0) {
                isMemset = false;
                break;
            }
        }
        ST_ASSERT(isMemset, "Memory Initialization test failed");
        ST_FREE(mem, 7);
    }

    #endif

    // Testing Hash_Map with STL_Global_Allocator
    MEM_TEST();
    {
        Hash_Map<int, std::string> test_map;
        test_map[1] = "one";
        test_map[2] = "two";
        test_map[4563] = "complex";
        ST_ASSERT(test_map[1] == "one" && test_map[2] == "two" && test_map[4563] == "complex", "Hash_Map test failed");
    }

    // Testing Ordered_Map with STL_Global_Allocator
    MEM_TEST();
    {
        Ordered_Map<int, std::string> test_map;
        test_map[1] = "one";
        test_map[2] = "two";
        ST_ASSERT(test_map[1] == "one" && test_map[2] == "two", "Ordered_Map test failed");
    }

    // Testing Hash_Set with STL_Global_Allocator
    MEM_TEST();
    {
        Hash_Set<int> test_set;
        test_set.insert(1);
        test_set.insert(2);
        ST_ASSERT(test_set.find(1) != test_set.end() && test_set.find(2) != test_set.end(), "Hash_Set test failed");
    }

    // Testing Ordered_Set with STL_Global_Allocator
    MEM_TEST();
    {
        Ordered_Set<int> test_set;
        test_set.insert(1);
        test_set.insert(2);
        ST_ASSERT(test_set.find(1) != test_set.end() && test_set.find(2) != test_set.end(), "Ordered_Set test failed");
    }

    // Testing Dynamic_String with STL_Global_Allocator
    MEM_TEST();
    {
        Dynamic_String test_str = "hello";
        test_str += " world";
        ST_ASSERT(test_str == "hello world", "Dynamic_String test failed");
    }

    // Testing Queue with STL_Global_Allocator
    MEM_TEST();
    {
        Queue<int> test_queue;
        test_queue.push(1);
        test_queue.push(2);
        ST_ASSERT(test_queue.front() == 1 && test_queue.back() == 2, "Queue test failed");
    }

    // Testing Deque with STL_Global_Allocator
    MEM_TEST();
    {
        Deque<int> test_deque;
        test_deque.push_back(1);
        test_deque.push_front(0);
        ST_ASSERT(test_deque.front() == 0 && test_deque.back() == 1, "Deque test failed");
    }

    // Testing Stack with STL_Global_Allocator
    MEM_TEST();
    {
        Stack<int> test_stack;
        test_stack.push(1);
        test_stack.push(2);
        ST_ASSERT(!test_stack.empty() && test_stack.top() == 2, "Stack test failed");
    }

    // Testing Array with STL_Global_Allocator
    MEM_TEST();
    {
        Array<int> test_array;

        ST_ASSERT(test_array.empty(), "Array test failed");

        ST_ASSERT(test_array.push_back(1) == 1, "Array test failed");
        test_array.push_back(3);
        test_array.push_back(4);

        ST_ASSERT(!test_array.empty(), "Array test failed");

        for (auto item : test_array)
        {
            ST_ASSERT(item >= 1 && item <= 4, "Array test failed");
        }

        test_array.insert(1, 2);
        
        ST_ASSERT(test_array.size() == 4, "Array test failed");
        ST_ASSERT(test_array.capacity() >= 4, "Array test failed");
        ST_ASSERT(!test_array.empty() && test_array.back() == 4, "Array test failed");

        test_array.pop_back();

        ST_ASSERT(test_array.size() == 3, "Array test failed");
        ST_ASSERT(test_array.capacity() > 3, "Array test failed");
        ST_ASSERT(!test_array.empty() && test_array.back() == 3, "Array test failed");

        test_array.erase(1, 2);

        ST_ASSERT(test_array.size() == 1, "Array test failed");
        ST_ASSERT(test_array.capacity() <= 8, "Array test failed");
        ST_ASSERT(!test_array.empty() && test_array.back() == 1, "Array test failed");

        test_array.shrink_to_fit();

        ST_ASSERT(test_array.capacity() == 1, "Array test failed");
    }

    

    // Timing comparison between malloc and ST_NEW
    MEM_TEST();
    {
        s64 num = 10000000;
        // For malloc
        auto start_malloc = std::chrono::steady_clock::now();
        Array<int*> malloc_pointers;
        for (int i = 0; i < num; ++i) {
            int* ptr = (int*)malloc(sizeof(int));
            *ptr = i;
            malloc_pointers.push_back(ptr);
        }
        for (int* ptr : malloc_pointers) {
            free(ptr);
        }
        auto end_malloc = std::chrono::steady_clock::now();
        log_duration("malloc duration", start_malloc, end_malloc);

        // For ST_NEW
        auto start_st_new = std::chrono::steady_clock::now();
        Array<int*> st_new_pointers;
        for (int i = 0; i < num; ++i) {
            int* ptr = ST_NEW(int, i);
            st_new_pointers.push_back(ptr);
        }
        for (int* ptr : st_new_pointers) {
            ST_DELETE(ptr);
        }
        auto end_st_new = std::chrono::steady_clock::now();
        log_duration("ST_NEW duration", start_st_new, end_st_new);
    }

    // Test array speeds (for Array)
    MEM_TEST();
    {
        class Complex {
        public:
            char hej[65] = "test";

            int bling = 3;
        };
        s64 num = 10000000; // 10 million
        auto start_array = std::chrono::steady_clock::now();
        Array<Complex> test_array;
        
        // Insertion
        for (int i = 0; i < num; ++i) {
            test_array.emplace_back();
        }

        // Random access
        int sum = 0;
        for (int i = 0; i < num; ++i) {
            sum += test_array[i].bling;
        }
        
        // Iteration
        sum = 0;
        for (const auto& val : test_array) {
            sum += val.bling;
        }

        // Erasure
        while (!test_array.empty()) {
            test_array.pop_back();
        }

        // Reversal
        for (int i = 0; i < num; ++i) {
            test_array.emplace_back();
        }
        std::reverse(test_array.begin(), test_array.end());

        auto end_array = std::chrono::steady_clock::now();
        log_duration("Array duration", start_array, end_array);
    }

    // Test array speeds (for Forbidden_Array)
    MEM_TEST();
    {
        class Complex {
        public:
            char hej[65] = "test";

            int bling = 3;
        };
        s64 num = 10000000; // 10 million
        auto start_array = std::chrono::steady_clock::now();
        Forbidden_Array<Complex> test_array;
        
        // Insertion
        for (int i = 0; i < num; ++i) {
            test_array.emplace_back();
        }

        // Random access
        int sum = 0;
        for (int i = 0; i < num; ++i) {
            sum += test_array[i].bling;
        }
        
        // Iteration
        sum = 0;
        for (const auto& val : test_array) {
            sum += val.bling;
        }

        // Erasure
        while (!test_array.empty()) {
            test_array.pop_back();
        }

        // Reversal
        for (int i = 0; i < num; ++i) {
            test_array.emplace_back();
        }
        std::reverse(test_array.begin(), test_array.end());

        auto end_array = std::chrono::steady_clock::now();
        log_duration("Forbidden_Array duration", start_array, end_array);
    }

}

#endif