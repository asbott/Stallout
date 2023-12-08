#include "pch.h"

#include "maths.h"

#include <random>

NS_BEGIN(stallout);
NS_BEGIN(math);

// Create a random device
std::random_device rd;

// Use the random device to seed a 64-bit Mersenne Twister engine
std::mt19937_64 rand_engine64(rd());

// Define a distribution that will produce 64-bit integers
std::uniform_int_distribution<int64_t> s64_min_max_dist(INT64_MIN, INT64_MAX);


u64 rand64() {
    return s64_min_max_dist(rand_engine64);
}

NS_END(math);
NS_END(stallout);