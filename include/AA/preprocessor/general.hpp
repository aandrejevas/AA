#pragma once
#
#include <type_traits> // integral_constant
#
#
#
#define AA_STRINGIFY_I(...) #__VA_ARGS__
#define AA_STRINGIFY(...) AA_STRINGIFY_I(__VA_ARGS__)
#
#
#
#define AA_CONCATENATE_I(a, b) a ## b
#define AA_CONCATENATE(a, b) AA_CONCATENATE_I(a, b)
#
#
#
#define AA_EXPAND(...) __VA_ARGS__
#define AA_EXPAND_FIRST(a, ...) AA_EXPAND a
#
#
#
#define AA_VARIADIC_SIZE_I(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, size, ...) size
#define AA_VARIADIC_SIZE(...) AA_VARIADIC_SIZE_I(__VA_ARGS__ __VA_OPT__(,) 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#
#
#
#define AA_OVERLOAD(prefix, ...) AA_CONCATENATE(prefix, AA_VARIADIC_SIZE(__VA_ARGS__))
#
#
#
#define AA_NAMEOF(name) ((void)name, AA_STRINGIFY(name))
#
#
#
#ifdef NDEBUG
#define AA_IF_DEBUG(...) ((void)0)
#else
#define AA_IF_DEBUG(...) AA_EXPAND(__VA_ARGS__)
#endif
#
#
#
#define AA_EXP2(...) AA_OVERLOAD(AA_EXP2_, __VA_ARGS__)(__VA_ARGS__)
#define AA_EXP2_1(n) (1uz << (n))
#define AA_EXP2_2(T, n) (std::integral_constant<T, 1>::value << (n))
