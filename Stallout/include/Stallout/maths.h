#pragma once //

#define mz_no_force_inline

#include <cmath>

#include <mz_vector.hpp>
#include <mz_matrix.hpp>
#include <mz_algorithms.hpp>

NS_BEGIN(stallout);
NS_BEGIN(math);

constexpr f64 PI64 = 3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679;
constexpr f64 PI = PI64;
constexpr f32 PI32 = (f32)PI;

ST_API u64 rand64();

template <typename type_t, std::enable_if_t<std::is_integral<type_t>::value, bool> = true>
inline type_t rand(type_t min, type_t max) {
    return (type_t)((rand64() % (max - min + 1)) + min);
}

template <typename type_t, std::enable_if_t<std::is_floating_point<type_t>::value, bool> = true>
inline type_t rand(type_t min, type_t max) {
    double scale = (u64)rand64() / static_cast<double>(std::numeric_limits<u64>::max());
    return (type_t)(min + scale * (max - min));
}

template <typename type_t>
inline type_t round(type_t v, type_t base) {
    return std::round(v / base) * base;
}
template <typename type_t>
inline type_t round(type_t v) {
    return std::round(v);
}
template<typename vec_t>
inline mz::vec2<vec_t> round(mz::vec2<vec_t> v) {
    return { round(v.x), round(v.y) };
}
template<typename vec_t>
inline mz::vec3<vec_t> round(mz::vec3<vec_t> v) {
    return { round(v.x), round(v.y), round(v.z) };
}
template<typename vec_t>
inline mz::vec4<vec_t> round(mz::vec4<vec_t> v) {
    return { round(v.x), round(v.y), round(v.z), round(v.w) };
}

template<typename type_t>
inline type_t decimal_of(type_t v) {
    return v - (v > 0 ? floor(v) : ceil(v));
}

template <typename type_t>
inline type_t floor(type_t v) {
    return std::floor(v);
}
template<typename vec_t>
inline mz::vec2<vec_t> floor(mz::vec2<vec_t> v) {
    return { floor(v.x), floor(v.y) };
}
template<typename vec_t>
inline mz::vec3<vec_t> floor(mz::vec3<vec_t> v) {
    return { floor(v.x), floor(v.y), floor(v.z) };
}
template<typename vec_t>
inline mz::vec4<vec_t> floor(mz::vec4<vec_t> v) {
    return { floor(v.x), floor(v.y), floor(v.z), floor(v.w) };
}

template <typename type_t>
inline type_t ceil(type_t v) {
    return std::ceil(v);
}
template<typename vec_t>
inline mz::vec2<vec_t> ceil(mz::vec2<vec_t> v) {
    return { ceil(v.x), ceil(v.y) };
}
template<typename vec_t>
inline mz::vec3<vec_t> ceil(mz::vec3<vec_t> v) {
    return { ceil(v.x), ceil(v.y), ceil(v.z) };
}
template<typename vec_t>
inline mz::vec4<vec_t> ceil(mz::vec4<vec_t> v) {
    return { ceil(v.x), ceil(v.y), ceil(v.z), ceil(v.w) };
}

template <typename type_t>
inline type_t ratio(type_t v, type_t min, type_t max) {
    return (v - min) / (max - min);
}

template <typename type_t>
inline type_t clamp(type_t v, type_t min, type_t max) {
    if (v < min) v = min;
    if (v > max) v = max;
    return v;
}

template <typename type_t, typename interp_t>
inline type_t lerp(type_t a, type_t b, interp_t t) {
    return a + t * (b - a);
}

template <typename first_t, typename ...args_t>
constexpr inline first_t min(first_t first, args_t... args) {
    return std::min(first, args...);
}
template <typename first_t, typename ...args_t>
constexpr inline first_t max(first_t first, args_t... args) {
    return std::max(first, args...);
}

inline f64 tan(f64 v) { return std::tan(v); }
inline f32 tan(f32 v) { return std::tan(v); }
inline f64 atan(f64 v) { return std::atan(v); }
inline f32 atan(f32 v) { return std::atan(v); }
inline f64 atan2(f64 y, f64 x) { return std::atan2(y, x); }
inline f32 atan2(f32 y, f32 x) { return std::atan2(y, x); }

inline f64 sin(f64 v) { return std::sin(v); }
inline f32 sin(f32 v) { return std::sin(v); }
inline f64 asin(f64 v) { return std::asin(v); }
inline f32 asin(f32 v) { return std::asin(v); }

inline f64 cos(f64 v) { return std::cos(v); }
inline f32 cos(f32 v) { return std::cos(v); }
inline f64 acos(f64 v) { return std::acos(v); }
inline f32 acos(f32 v) { return std::acos(v); }

inline f64 sqrt(f64 v) { return std::sqrt(v); }
inline f32 sqrt(f32 v) { return (f32)std::sqrt(v); }

template <typename type_t>
inline constexpr type_t pow(type_t base, type_t exp) {
    return (exp == 0) ? 1 : base * pow(base, exp - 1);
}

template <typename type_t>
inline type_t abs(type_t v) {
    return v < (type_t)0 ? v * (type_t)-1 : v;
}

inline bool point_in_triangle(mz::fvec2 p, mz::fvec2 a, mz::fvec2 b, mz::fvec2 c) {
    float area = 0.5f *(-b.y * c.x + a.y * (-b.x + c.x) + a.x * (b.y - c.y) + b.x * c.y);
    float s = 1 / (2 * area) * (a.y * c.x - a.x * c.y + (c.y - a.y) * p.x + (a.x - c.x) * p.y);
    float t = 1 / (2 * area) * (a.x * b.y - a.y * b.x + (a.y - b.y) * p.x + (b.x - a.x) * p.y);

    return s > 0 && t > 0 && (1 - s - t) > 0;
}

inline mz::fvec2 get_rect_overlap(const mz::frect& a, const mz::frect& b) {
    float left = std::max(a.left, b.left);
    float right = std::min(a.right, b.right);

    float bottom = std::max(a.bottom, b.bottom);
    float top = std::min(a.top, b.top);

    if (left > right || top < bottom) {
        return 0;
    }

    // Return the overlapping rectangle
    return mz::fvec2{right - left, top - bottom};
}

inline bool point_in_rect(mz::fvec2 p, const mz::frect& rect) {
    return (p.x >= rect.left && p.x <= rect.right)
        && (p.y >= rect.bottom && p.y <= rect.top);
}

inline bool ray_intersects_ray(mz::fray2d a, mz::fray2d b, mz::fvec2* intersection = NULL) {

    mz::fvec2 p0 = a.xy;
    mz::fvec2 p1 = a.zw;
    mz::fvec2 p2 = b.xy;
    mz::fvec2 p3 = b.zw;


    f32 s1_x, s1_y, s2_x, s2_y;
    s1_x = p1.x - p0.x;     
    s1_y = p1.y - p0.y;
    s2_x = p3.x - p2.x;     
    s2_y = p3.y - p2.y;

    f32 denominator = (-s2_x * s1_y + s1_x * s2_y);
    if (denominator == 0) {
        return false;
    }

    f32 s, t;
    s = (-s1_y * (p0.x - p2.x) + s1_x * (p0.y - p2.y)) / (-s2_x * s1_y + s1_x * s2_y);
    t = ( s2_x * (p0.y - p2.y) - s2_y * (p0.x - p2.x)) / (-s2_x * s1_y + s1_x * s2_y);

    if (s >= 0 && s <= 1 && t >= 0 && t <= 1) {
        
        if (intersection != NULL) {
            intersection->x = p0.x + (t * s1_x);
            intersection->y = p0.y + (t * s1_y);
        }
        return true;
    }

    return false;
}

inline bool ray_intersects_rect(mz::fray2d ray, const mz::frect& rect, mz::fvec2* intersect_a = NULL, mz::fvec2* intersect_b = NULL) {
    mz::fray2d r0 = { rect.left, rect.top, rect.right, rect.top };
    mz::fray2d r1 = { rect.right, rect.top, rect.right, rect.bottom };
    mz::fray2d r2 = { rect.right, rect.bottom, rect.left, rect.bottom };
    mz::fray2d r3 = { rect.left, rect.bottom, rect.left, rect.top };

    bool first_intersection_found = false;

    mz::fvec2 intersect;

    if (ray_intersects_ray(ray, r0, &intersect)) {
        if (intersect_a) *intersect_a = intersect;
        first_intersection_found = true;
    }
    if (ray_intersects_ray(ray, r1, &intersect)) {
        if (!first_intersection_found && intersect_a)     *intersect_a = intersect;
        else if (first_intersection_found && intersect_b) *intersect_b = intersect;
        first_intersection_found = true;
    }
    if (ray_intersects_ray(ray, r2, &intersect)) {
        if (!first_intersection_found && intersect_a)     *intersect_a = intersect;
        else if (first_intersection_found && intersect_b) *intersect_b = intersect;
        first_intersection_found = true;
    }
    if (ray_intersects_ray(ray, r3, &intersect)) {
        if (!first_intersection_found && intersect_a)     *intersect_a = intersect;
        else if (first_intersection_found && intersect_b) *intersect_b = intersect;
        first_intersection_found = true;
    }

    return first_intersection_found;
}

template <typename type_t>
inline type_t align(type_t v, type_t align) {
    return round(v / align) * align;
}


inline u64 ceil_pow2(u64 x) {
    if (x < 0)
        return 0;
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x+1;
}


NS_END(math);
NS_END(stallout);