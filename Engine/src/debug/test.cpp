#include "pch.h"

#include "Engine/debug/tests.h"

#include "Engine/logger.h"
#include "Engine/containers.h"
#include "os/io.h"
#include "Engine/threadpool.h"

#ifndef _ST_DISABLE_ASSERTS



void log_duration(const char* msg, std::chrono::steady_clock::time_point start, std::chrono::steady_clock::time_point end) {
    log_info("{}: {} ms", msg, (float)(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()) / 1000.f);
}

#ifdef _ST_CONFIG_DEBUG
#define MEM_TEST(...) { ST_ASSERT(start_mem == engine::Global_Allocator::get_used_mem(), "Memory leak. Last was {}, now {}", start_mem, engine::Global_Allocator::get_used_mem()); start_mem = engine::Global_Allocator::get_used_mem(); }
#else
#define MEM_TEST(...)
#endif
void test_allocators() {

    IN_DEBUG_ONLY(size_t start_mem = engine::Global_Allocator::get_used_mem(););

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
        

        engine::Array<std::thread> threads;
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
    //MEM_TEST();
    //{
    //    struct Data {
    //        char data[128] = "Test";
    //    };

    //    std::vector<Data*> arr;
    //    auto num = 2;
    //    for (size_t i = 0; i < num; i++) {
    //        arr.push_back(ST_NEW(Data));
    //    }

    //    std::vector<void*> prev_pointers;

    //    for (size_t i = 0; i < num; i++) {
    //        prev_pointers.push_back(arr[i]);
    //        ST_DELETE(arr[i]);
    //    }

    //    // Go backwards because allocator allocatoes last free block
    //    for (s64 i = num - 1; i >= 0; i--) {
    //        arr[i] = ST_NEW(Data);

    //        ST_ASSERT(prev_pointers[i] == arr[i], "Mass reusable memory test failed: memory not reused");
    //        ST_ASSERT(strcmp(arr[i]->data, "Test") == 0, "Mass reusable memory test failed: memory corrupt");
    //    }

    //    for (s64 i = num - 1; i >= 0; i--) {
    //        ST_DELETE(arr[i]);
    //    }
    //}
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

    // Testing Hash_Map with STL_Global_Allocator
    MEM_TEST();
    {
        engine::Hash_Map<int, std::string> test_map;
        test_map[1] = "one";
        test_map[2] = "two";
        test_map[4563] = "complex";
        ST_ASSERT(test_map[1] == "one" && test_map[2] == "two" && test_map[4563] == "complex", "Hash_Map test failed");
    }

    // Testing Ordered_Map with STL_Global_Allocator
    MEM_TEST();
    {
        engine::Ordered_Map<int, std::string> test_map;
        test_map[1] = "one";
        test_map[2] = "two";
        ST_ASSERT(test_map[1] == "one" && test_map[2] == "two", "Ordered_Map test failed");
    }

    // Testing Hash_Set with STL_Global_Allocator
    MEM_TEST();
    {
        engine::Hash_Set<int> test_set;
        test_set.insert(1);
        test_set.insert(2);
        ST_ASSERT(test_set.find(1) != test_set.end() && test_set.find(2) != test_set.end(), "Hash_Set test failed");
    }

    // Testing Ordered_Set with STL_Global_Allocator
    MEM_TEST();
    {
        engine::Ordered_Set<int> test_set;
        test_set.insert(1);
        test_set.insert(2);
        ST_ASSERT(test_set.find(1) != test_set.end() && test_set.find(2) != test_set.end(), "Ordered_Set test failed");
    }



    // Testing Queue with STL_Global_Allocator
    MEM_TEST();
    {
        engine::Queue<int> test_queue;
        test_queue.push(1);
        test_queue.push(2);
        ST_ASSERT(test_queue.front() == 1 && test_queue.back() == 2, "Queue test failed");
    }

    // Testing Deque with STL_Global_Allocator
    MEM_TEST();
    {
        engine::Deque<int> test_deque;
        test_deque.push_back(1);
        test_deque.push_front(0);
        ST_ASSERT(test_deque.front() == 0 && test_deque.back() == 1, "Deque test failed");
    }

    // Testing Stack with STL_Global_Allocator
    MEM_TEST();
    {
        engine::Stack<int> test_stack;
        test_stack.push(1);
        test_stack.push(2);
        ST_ASSERT(!test_stack.empty() && test_stack.top() == 2, "Stack test failed");
    }

    // Testing Array with STL_Global_Allocator
    MEM_TEST();
    {
        engine::Array<int> test_array;

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
        s64 num = 300000;
        // For malloc
        auto start_malloc = std::chrono::steady_clock::now();
        engine::Array<int*> malloc_pointers;
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
        engine::Array<int*> st_new_pointers;
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
        s64 num = 3000000;
        auto start_array = std::chrono::steady_clock::now();
        engine::Array<Complex> test_array;
        
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
        s64 num = 3000000;
        auto start_array = std::chrono::steady_clock::now();
        engine::Forbidden_Array<Complex> test_array;
        
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

void test_io () {
    // Test get_exe_path

    using New_String = engine::New_String;

    New_String exe_path = os::io::get_exe_path();
    New_String exe_dir = os::io::get_directory(exe_path.str);
    New_String exe_name = os::io::get_filename(exe_path.str);
    New_String exe_name_no_ext = os::io::get_filename_without_extension(exe_path.str);
    New_String exe_ext = os::io::get_file_extension(exe_path.str);

    log_info(
      "Exe path:        {}"
    "\nExe dir:         {}"
    "\nExe name:        {}"
    "\nExe name no ext: {}"
    "\nExe:             {}", exe_path.str, exe_dir.str, exe_name.str, exe_name_no_ext.str, exe_ext.str);

    // Test write_bytes
    byte_t write_data[] = {0, 1, 2, 3, 4};
    Io_Status status = os::io::write_bytes("test_file.bin", write_data, sizeof(write_data));
    ST_ASSERT(status == IO_STATUS_OK, "Failed to write bytes");

    // Test get_file_info
    os::io::File_Info info;
    status = os::io::get_file_info("test_file.bin", &info);
    ST_ASSERT(status == IO_STATUS_OK && info.file_size == sizeof(write_data), "Failed to get file info");

    // Test read_all_bytes
    byte_t read_data[5];
    status = os::io::read_all_bytes("test_file.bin", read_data, sizeof(read_data));
    ST_ASSERT(status == IO_STATUS_OK && memcmp(write_data, read_data, sizeof(write_data)) == 0, "Failed to read all bytes");

    // Test write_string
    char write_str[] = "Hello, world!";
    status = os::io::write_string("test_file.txt", write_str, strlen(write_str));
    ST_ASSERT(status == IO_STATUS_OK, "Failed to write string");

    // Test read_as_string
    char read_str[50];
    status = os::io::read_as_string("test_file.txt", read_str, sizeof(read_str));
    ST_ASSERT(status == IO_STATUS_OK && strcmp(write_str, read_str) == 0, "Failed to read as string");

    // Test append_bytes
    byte_t append_data[] = {5, 6, 7, 8, 9};
    status = os::io::append_bytes("test_file.bin", append_data, sizeof(append_data));
    ST_ASSERT(status == IO_STATUS_OK, "Failed to append bytes");

    // Test append_string
    char append_str[] = " Appended!";
    status = os::io::append_string("test_file.txt", append_str, strlen(append_str));
    ST_ASSERT(status == IO_STATUS_OK, "Failed to append string");

    // Test copy
    status = os::io::copy("test_file.bin", "test_file_copy.bin");
    ST_ASSERT(status == IO_STATUS_OK, "Failed to copy file");

    // Test remove
    status = os::io::remove("test_file_copy.bin");
    ST_ASSERT(status == IO_STATUS_OK, "Failed to remove file");

    // Test count_directory_entries
    size_t dir_count = 0;
    status = os::io::count_directory_entries(".", &dir_count);
    ST_ASSERT(status == IO_STATUS_OK && dir_count > 0, "Failed to count directory entries");

    // Test scan_directory
    char** dir_entries = (char**)ST_MEM(dir_count * sizeof(uintptr_t));
    for (size_t i = 0; i < dir_count; i++) {
        dir_entries[i] = (char*)ST_MEM(MAX_PATH_LEN);
        memset(dir_entries[i], 0, MAX_PATH_LEN);
    }
    status = os::io::scan_directory(".", dir_entries);
    for (size_t i = 0; i < dir_count; i++) {
        log_info(dir_entries[i]);
        ST_FREE(dir_entries[i], MAX_PATH_LEN);
    }
    ST_FREE(dir_entries, dir_count * sizeof(uintptr_t));
    ST_ASSERT(status == IO_STATUS_OK, "Failed to scan directory");

    // Produce some expected errors
    status = os::io::read_all_bytes("non_existent_file.bin", read_data, sizeof(read_data));
    ST_ASSERT(status == IO_STATUS_INVALID_PATH, "Should return IO_STATUS_INVALID_PATH for nonexistent file");

    status = os::io::get_file_info("non_existent_file.bin", &info);
    ST_ASSERT(status == IO_STATUS_INVALID_PATH, "Should return IO_STATUS_INVALID_PATH for nonexistent file");

    status = os::io::read_all_bytes(nullptr, read_data, sizeof(read_data));
    ST_ASSERT(status == IO_STATUS_INVALID_PATH, "Should return IO_STATUS_INVALID_PATH for null path");

    status = os::io::read_all_bytes("test_file.bin", nullptr, sizeof(read_data));
    ST_ASSERT(status == IO_STATUS_INVALID_BUFFER, "Should return IO_STATUS_INVALID_BUFFER for null buffer");
}

void test_concurrency() {
    constexpr size_t num_threads = 10;
    constexpr size_t num_tasks = 1000;

    // Test basic functionality
    {
        engine::Thread_Pool pool(1);  // Single thread

        std::atomic<int> counter = 0;
        pool.submit([&](){ counter++; });

        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // give some time for the task to complete

        ST_ASSERT(counter == 1);  // Verify the task was executed
    }

    // Concurrency test
    {
        engine::Thread_Pool pool(num_threads);

        std::atomic<int> counter = 0;

        for (size_t i = 0; i < num_threads; ++i) {
            pool.submit([&](){ counter++; });
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // give some time for the tasks to complete

        ST_ASSERT(counter == num_threads);  // Verify all tasks were executed
    }

    // Stress test
    {
        engine::Thread_Pool pool(num_threads);

        std::atomic<int> counter = 0;

        for (size_t i = 0; i < num_tasks; ++i) {
            pool.submit([&](){
                for (int j = 0; j < 1000; ++j) {
                    counter++;
                }
            });
        }

        std::this_thread::sleep_for(std::chrono::seconds(2)); // give some time for all tasks to complete

        ST_ASSERT(counter == num_tasks * 1000); // Verify all tasks were executed and counter incremented correctly
    }
}

void run_tests() {
    test_io();
    test_concurrency();
    test_allocators();
}

#endif