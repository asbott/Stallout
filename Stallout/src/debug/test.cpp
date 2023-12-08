#include "pch.h"

#include "Stallout/debug/tests.h"

#include "Stallout/logger.h"
#include "Stallout/containers.h"
#include "os/io.h"
#include "Stallout/threadpool.h"

#ifndef _ST_DISABLE_ASSERTS 



void log_duration(const char* msg, std::chrono::steady_clock::time_point start, std::chrono::steady_clock::time_point end) {
	log_info("{}: {} ms", msg, (float)(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()) / 1000.f);
}

#ifdef _ST_CONFIG_DEBUG
#define MEM_TEST(...) { ST_ASSERT(start_mem == stallout::Global_Allocator::get_stats().in_use(), "Memory leak. Last was {}, now {}", start_mem, stallout::Global_Allocator::get_stats().in_use()); start_mem = stallout::Global_Allocator::get_stats().in_use(); }
#else
#define MEM_TEST(...)
#endif

#define EXPECT(val, exp) ST_ASSERT(val == exp, "Test Failed: Expected {}, got {}", val, exp)
// TODO: Rewrite tests using EXPECT macro

#ifdef _ST_RUN_TESTS

#pragma warning (disable: 4566)

#ifndef ST_ENABLE_MEMORY_TRACKING
#error Memory tracking needs to be enabled to run tests
#endif

void test_allocators() {

	log_info("Testing Memory...");

	size_t start_mem = stallout::Global_Allocator::get_stats().in_use();
	(void)start_mem;

	// Testing single allocation and deallocation
	MEM_TEST();
	{
		int* single = ST_NEW(int) (42);
		ST_ASSERT(single != nullptr && *single == 42, "Single allocation failed");
		ST_DELETE(single);
	}

	// Testing multiple allocations and deallocations
	MEM_TEST();
	{
		std::vector<int*> multiples;
		for(int i = 0; i < 100; ++i) {
			multiples.push_back(stnew (int) (i));
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


		stallout::Array<std::thread> threads;
		for(int i = 0; i < 1000; ++i) {
			threads.push_back(std::thread([](){
				Complex* complex = stnew (Complex) (42, "hello");
				ST_ASSERT(complex != nullptr && complex->x == 42 && complex->s == "hello", "Complex object allocation failed");
				ST_DELETE(complex);
			}));
		}
		for(auto& t : threads) {
			t.join();
		}
	}

	// Testing lifecycle of complex objects
	MEM_TEST();
	{
		struct Complex {
			int x;
			std::string s;
			Complex(int x, const std::string& s) : x(x), s(s) {}
		};
		Complex* complex = stnew (Complex) (42, "hello");
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
		int* overwrite_test = stnew (int)(42);
		int* adjacent_memory = stnew (int)(43);
		*adjacent_memory = 44;  // Simulating memory overwrite
		ST_ASSERT(*overwrite_test != 44, "Memory overwrite test failed");
		ST_DELETE(overwrite_test);
		ST_DELETE(adjacent_memory);
	}

	// Stress testing for concurrency
	MEM_TEST();
	{
		size_t nthreads = std::thread::hardware_concurrency();
		std::vector<std::thread> threads;
		for(int i = 0; i < nthreads; ++i) {
			threads.push_back(std::thread([](){
				for (int j = 0; j < 1000; ++j) {
					int* multi = stnew (int)(j);
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
		stallout::Hash_Map<int, std::string> test_map;
		test_map[1] = "one";
		test_map[2] = "two";
		test_map[4563] = "complex";
		ST_ASSERT(test_map[1] == "one" && test_map[2] == "two" && test_map[4563] == "complex", "Hash_Map test failed");
	}

	// Testing Ordered_Map with STL_Global_Allocator
	MEM_TEST();
	{
		stallout::Ordered_Map<int, std::string> test_map;
		test_map[1] = "one";
		test_map[2] = "two";
		ST_ASSERT(test_map[1] == "one" && test_map[2] == "two", "Ordered_Map test failed");
	}

	// Testing Hash_Set with STL_Global_Allocator
	MEM_TEST();
	{
		stallout::Hash_Set<int> test_set;
		test_set.insert(1);
		test_set.insert(2);
		ST_ASSERT(test_set.find(1) != test_set.end() && test_set.find(2) != test_set.end(), "Hash_Set test failed");
	}

	// Testing Ordered_Set with STL_Global_Allocator
	MEM_TEST();
	{
		stallout::Ordered_Set<int> test_set;
		test_set.insert(1);
		test_set.insert(2);
		ST_ASSERT(test_set.find(1) != test_set.end() && test_set.find(2) != test_set.end(), "Ordered_Set test failed");
	}



	// Testing Queue with STL_Global_Allocator
	MEM_TEST();
	{
		stallout::Queue<int> test_queue;
		test_queue.push(1);
		test_queue.push(2);
		ST_ASSERT(test_queue.front() == 1 && test_queue.back() == 2, "Queue test failed");
	}

	// Testing Deque with STL_Global_Allocator
	MEM_TEST();
	{
		stallout::Deque<int> test_deque;
		test_deque.push_back(1);
		test_deque.push_front(0);
		ST_ASSERT(test_deque.front() == 0 && test_deque.back() == 1, "Deque test failed");
	}

	// Testing Stack with STL_Global_Allocator
	MEM_TEST();
	{
		stallout::Stack<int> test_stack;
		test_stack.push(1);
		test_stack.push(2);
		ST_ASSERT(!test_stack.empty() && test_stack.top() == 2, "Stack test failed");
	}

	// Testing Array with STL_Global_Allocator
	MEM_TEST();
	{
		stallout::Array<int> test_array;

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
		stallout::Array<int*> malloc_pointers;
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
		stallout::Array<int*> st_new_pointers;
		for (int i = 0; i < num; ++i) {
			int* ptr = stnew (int)(i);
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
		stallout::Array<Complex> test_array;

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
		stallout::Forbidden_Array<Complex> test_array;

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

	MEM_TEST();
	{
		stallout::String str = "Hi";

		str.concat("Hey %s", "yo");
		str.replace_all("H", 'e');
		str.insert(1, "Shmonk");
	}

	MEM_TEST();
	{
		stallout::LString str = "Hi";
		str.concat(" chmo");

		str.replace_all(L"H", 'A');

		str.insert(1, "Shmonk");
	}

	MEM_TEST();

	log_info("OK!");
}

void test_io () {
	log_info("Testing IO...");

	using String = stallout::String;

	for (size_t i = 0; i < 10; i++) {
		String test = "hej"; (void)test;
		String test2 = test; (void)test;


		//test = test2;


		String test3 = test;
		test3.concat(test2.str);
	}

	String exe_path = stallout::os::io::get_exe_path();
	String exe_dir = stallout::os::io::get_directory(exe_path.str);
	String exe_name = stallout::os::io::get_filename(exe_path.str);
	String exe_name_no_ext = stallout::os::io::get_filename_without_extension(exe_path.str);
	String exe_ext = stallout::os::io::get_file_extension(exe_path.str);

	/*log_info(
	"Exe path:        {}"
	"\nExe dir:         {}"
	"\nExe name:        {}"
	"\nExe name no ext: {}"
	"\nExe:             {}", exe_path.str, exe_dir.str, exe_name.str, exe_name_no_ext.str, exe_ext.str);
	*/
	// Test write_bytes
	byte_t write_data[] = {0, 1, 2, 3, 4};
	Io_Status status = stallout::os::io::write_bytes("test_file.bin", write_data, sizeof(write_data));
	ST_ASSERT(status == IO_STATUS_OK, "Failed to write bytes");

	// Test get_file_info
	stallout::os::io::File_Info info;
	status = stallout::os::io::get_file_info("test_file.bin", &info);
	ST_ASSERT(status == IO_STATUS_OK && info.file_size == sizeof(write_data), "Failed to get file info");

	// Test read_all_bytes
	byte_t read_data[5];
	status = stallout::os::io::read_all_bytes("test_file.bin", read_data, sizeof(read_data));
	ST_ASSERT(status == IO_STATUS_OK && memcmp(write_data, read_data, sizeof(write_data)) == 0, "Failed to read all bytes");

	// Test write_string
	char write_str[] = "Hello, world!";
	status = stallout::os::io::write_string("test_file.txt", write_str, strlen(write_str));
	ST_ASSERT(status == IO_STATUS_OK, "Failed to write string");

	// Test read_as_string
	char read_str[sizeof(write_str)];
	status = stallout::os::io::read_as_string("test_file.txt", read_str, sizeof(read_str));
	ST_ASSERT(status == IO_STATUS_OK && strcmp(write_str, read_str) == 0, "Failed to read as string");

	// Test append_bytes
	byte_t append_data[] = {5, 6, 7, 8, 9};
	status = stallout::os::io::append_bytes("test_file.bin", append_data, sizeof(append_data));
	ST_ASSERT(status == IO_STATUS_OK, "Failed to append bytes");

	// Test append_string
	char append_str[] = " Appended!";
	status = stallout::os::io::append_string("test_file.txt", append_str, strlen(append_str));
	ST_ASSERT(status == IO_STATUS_OK, "Failed to append string");

	// Test copy
	status = stallout::os::io::copy("test_file.bin", "test_file_copy.bin");
	ST_ASSERT(status == IO_STATUS_OK, "Failed to copy file");

	// Test remove
	status = stallout::os::io::remove("test_file_copy.bin");
	ST_ASSERT(status == IO_STATUS_OK, "Failed to remove file");

	// Test count_directory_entries
	size_t dir_count = 0;
	status = stallout::os::io::count_directory_entries(".", &dir_count);
	ST_ASSERT(status == IO_STATUS_OK && dir_count > 0, "Failed to count directory entries");


	// Produce some expected errors
	status = stallout::os::io::read_all_bytes("non_existent_file.bin", read_data, sizeof(read_data));
	ST_ASSERT(status == IO_STATUS_INVALID_PATH, "Should return IO_STATUS_INVALID_PATH for nonexistent file");

	status = stallout::os::io::get_file_info("non_existent_file.bin", &info);
	ST_ASSERT(status == IO_STATUS_INVALID_PATH, "Should return IO_STATUS_INVALID_PATH for nonexistent file");

	status = stallout::os::io::read_all_bytes(nullptr, read_data, sizeof(read_data));
	ST_ASSERT(status == IO_STATUS_INVALID_PATH, "Should return IO_STATUS_INVALID_PATH for null path");

	status = stallout::os::io::read_all_bytes("test_file.bin", nullptr, sizeof(read_data));
	ST_ASSERT(status == IO_STATUS_INVALID_BUFFER, "Should return IO_STATUS_INVALID_BUFFER for null buffer");

	log_info("OK!");
}

void test_concurrency() {
	log_info("Testing concurrency...");
	constexpr size_t num_threads = 1000;
	constexpr size_t num_tasks = 1000;

	// Test basic functionality
	{
		stallout::Thread_Pool pool(1);  // Single thread

		std::atomic<int> counter = 0;
		pool.submit([&](){ counter++; });

		std::this_thread::sleep_for(std::chrono::milliseconds(100)); // give some time for the task to complete

		ST_ASSERT(counter == 1);  // Verify the task was executed
	}

	// Concurrency test
	{
		stallout::Thread_Pool pool(num_threads);

		std::atomic<int> counter = 0;

		for (size_t i = 0; i < num_threads; ++i) {
			pool.submit([&](){ counter++; });
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(100)); // give some time for the tasks to complete

		ST_ASSERT(counter == num_threads);  // Verify all tasks were executed
	}

	// Stress test
	{
		stallout::Thread_Pool pool(num_threads);

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

	log_info("OK!");
}

void test_strings() {
	log_info("Testing strings...");
	// Constructor Tests
	{
		stallout::String str1("Hello, world!"); 
		ST_ASSERT(str1 == "Hello, world!");

		stallout::String str2(str1);
		ST_ASSERT(str2 == str1);

		stallout::String str3(std::move(str2));
		ST_ASSERT(str3 == "Hello, world!");
		ST_ASSERT(!str2.str);
	}

	// Assignment Tests
	{
		stallout::String str;
		str = "Assignment Test";
		ST_ASSERT(str == "Assignment Test");

		stallout::String str2;
		str2 = str;
		ST_ASSERT(str2 == "Assignment Test");

		stallout::String str3;
		str3 = std::move(str2);
		ST_ASSERT(str3 == "Assignment Test");
	}

	// Concatenation Tests
	{
		stallout::String str = "Concat";
		str.concat("enation");
		ST_ASSERT(str == "Concatenation");

		str.concat('!');
		ST_ASSERT(str == "Concatenation!");
	}

	// Concatenation Tests
	{
		stallout::String str = "Iertination";
		str.insert(1, "ns");
		ST_ASSERT(str == "Insertination");
	}
	{
		stallout::String str(sizeof("Insertination"));
		str.concat("Iertination");
		str.insert(1, "ns");
		ST_ASSERT(str == "Insertination");
	}

	// Utility and Manipulation Tests
	{
		stallout::String str = "A quick brown fox.";
		ST_ASSERT(str.contains("fox"));
		ST_ASSERT(!str.contains("dog"));
		ST_ASSERT(str.count('o') == 2);
		ST_ASSERT(str.first_index('o') == 10);
		ST_ASSERT(str.last_index('o') == 15);

		str.replace_all("fox", 'c');
		ST_ASSERT(str == "A quick brown c.");

		stallout::String sub = str.sub_string(2, 5);
		ST_ASSERT(sub == "quick");
	}

	// Other Operation Tests
	{
		stallout::String str = "Comparison";
		ST_ASSERT(str == "Comparison");
		ST_ASSERT(str != "comparison");
		ST_ASSERT(str == str);
	}

	{
		stallout::String str1 = "HashString";
		stallout::String str2 = "HashString";
		stallout::String str3 = "HashString2";

		ST_ASSERT(std::hash<stallout::String>()(str1) == std::hash<stallout::String>()(str2));
		ST_ASSERT(std::hash<stallout::String>()(str1) != std::hash<stallout::String>()(str3));
	}




	// Long Strings

	{
		stallout::LString str1(L"Ἐν ἀρχῇ ἦν ὁ λόγος");
		ST_ASSERT(str1 == L"Ἐν ἀρχῇ ἦν ὁ λόγος");

		stallout::LString str2(str1);
		ST_ASSERT(str2 == str1);

		stallout::LString str3(std::move(str2));
		ST_ASSERT(str3 == L"Ἐν ἀρχῇ ἦν ὁ λόγος");
		ST_ASSERT(!str2.str);
	}

	// Assignment Tests
	{
		stallout::LString str;
		str = "Assignment Test";
		ST_ASSERT(str == "Assignment Test");

		stallout::LString str2;
		str2 = str;
		ST_ASSERT(str2 == "Assignment Test");

		stallout::LString str3;
		str3 = std::move(str2);
		ST_ASSERT(str3 == "Assignment Test");
	}

	// Concatenation Tests
	{
		stallout::LString str = L"Ἐν ἀρχῇ ἦν ὁ ";
		str.concat(L"λόγος");
		ST_ASSERT(str == L"Ἐν ἀρχῇ ἦν ὁ λόγος");

		str.concat('!');
		ST_ASSERT(str == L"Ἐν ἀρχῇ ἦν ὁ λόγος!");
	}

	// Insertion Tests
	{
		stallout::LString str = L"Ἐν ἦν ὁ λόγος";
		str.insert(3, L"ἀρχῇ ");
		ST_ASSERT(str == L"Ἐν ἀρχῇ ἦν ὁ λόγος");
	}

	// Utility and Manipulation Tests
	{
		stallout::LString str = L"Ἐν ἀρχῇ ἦν ὁ λόγος λόγος";
		ST_ASSERT(str.contains(L"λόγος"));
		ST_ASSERT(!str.contains(L"Logos"));
		ST_ASSERT(str.count(L'λ') == 2, "Expected 2, got {}", str.count('λ'));
		ST_ASSERT(str.first_index(L'λ') == 13);
		ST_ASSERT(str.last_index(L'λ') == 19);

		str.replace_all(L"λόγος", 'X');
		ST_ASSERT(str == L"Ἐν ἀρχῇ ἦν ὁ X X");

		stallout::LString sub = str.sub_string(3, 4);

		ST_ASSERT(sub == L"ἀρχῇ");
	}

	// Other Operation Tests
	{
		stallout::LString str = L"Ἐν ἀρχῇ ἦν ὁ λόγος";
		ST_ASSERT(str == L"Ἐν ἀρχῇ ἦν ὁ λόγος");
		ST_ASSERT(str != "comparison");
		ST_ASSERT(str == str);
	}

	{
		stallout::LString str1 = L"λόγος";
		stallout::LString str2 = L"λόγος";
		stallout::LString str3 = L"ἀρχῇ";

		ST_ASSERT(std::hash<stallout::LString>()(str1) == std::hash<stallout::LString>()(str2));
		ST_ASSERT(std::hash<stallout::LString>()(str1) != std::hash<stallout::LString>()(str3));
	}



	{
		stallout::String str8 = "Ascii";
		stallout::LString str32 = L"Ἐν ἀρχῇ ἦν ὁ λόγος";

		ST_ASSERT(str8 != str32.to_ascii());
		ST_ASSERT(str32 != str8.to_utf32());

		str32 = str8;

		ST_ASSERT(str8 == str32.to_ascii());
		ST_ASSERT(str32 == str8.to_utf32());

		str32 = L"Ἐν ἀρχῇ";
		str8 = str32;

		log_debug("{}\n{}", str8, str32);

		ST_ASSERT(str8 == str32.to_ascii());
		ST_ASSERT(str32.to_ascii() == str8);
		ST_ASSERT(str32 != str8.to_utf32());
	}

	log_info("OK!");
}

void run_tests() {
	test_allocators(); 
	test_io();
	test_concurrency();
	test_strings();
}

#endif
#endif