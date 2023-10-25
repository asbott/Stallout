#pragma once

#define mz_no_force_inline

#include <mz_vector.hpp>
#include <mz_matrix.hpp>
#include <mz_algorithms.hpp>


NS_BEGIN(engine);
NS_BEGIN(math);

ST_API s64 rand64();

template <typename type_t, std::enable_if_t<std::is_integral<type_t>::value, bool> = true>
inline type_t rand64(type_t min, type_t max) {
    return (type_t)((rand64() % (max - min + 1)) + min);
}

template <typename type_t, std::enable_if_t<std::is_floating_point<type_t>::value, bool> = true>
inline type_t rand64(type_t min, type_t max) {
    double scale = rand64() / static_cast<double>(std::numeric_limits<s64>::max());
    return (type_t)(min + scale * (max - min));
}

NS_END(math);
NS_END(engine);