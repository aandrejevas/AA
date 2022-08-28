#pragma once

#include "../metaprogramming/general.hpp"
#include <cstddef> // size_t
#include <type_traits> // make_unsigned_t
#include <concepts> // integral, unsigned_integral, signed_integral, floating_point, same_as, convertible_to, equality_comparable
#include <bit> // countl_zero, has_single_bit, bit_cast
#include <limits> // numeric_limits



namespace aa {

	template<std::unsigned_integral T, std::signed_integral X>
	[[gnu::always_inline]] AA_CONSTEXPR T unsign(const X x) {
		return static_cast<T>(std::bit_cast<std::make_unsigned_t<X>>(x));
	}

	template<class T, std::convertible_to<T> X>
	[[gnu::always_inline]] AA_CONSTEXPR T unsign_cast(const X &x) {
		if constexpr (std::unsigned_integral<T> && std::signed_integral<X>) {
			return unsign<T>(x);
		} else {
			return static_cast<T>(x);
		}
	}



	template<std::floating_point T>
	AA_CONSTEXPR T norm(const T value, const T mag) {
		return value / mag;
	}

	template<std::floating_point T>
	AA_CONSTEXPR T norm(const T value, const T start, const T mag) {
		return (value - start) / mag;
	}

	template<std::floating_point T>
	AA_CONSTEXPR T map(const T value, const T mag1, const T mag2) {
		return mag2 * norm(value, mag1);
	}

	template<size_t V, std::floating_point T>
		requires (V == 0)
	AA_CONSTEXPR T map(const T value, const T start1, const T mag1, const T mag2) {
		return mag2 * norm(value, start1, mag1);
	}

	template<size_t V, std::floating_point T>
		requires (V == 1)
	AA_CONSTEXPR T map(const T value, const T mag1, const T start2, const T mag2) {
		return start2 + mag2 * norm(value, mag1);
	}

	template<std::floating_point T>
	AA_CONSTEXPR T map(const T value, const T start1, const T mag1, const T start2, const T mag2) {
		return start2 + mag2 * norm(value, start1, mag1);
	}



	template<std::unsigned_integral T>
	[[gnu::always_inline]] AA_CONSTEXPR T int_log2(const T x) {
		return constant<T, std::numeric_limits<T>::digits - 1>() - unsign<T>(std::countl_zero(x));
	}

	// https://en.wikipedia.org/wiki/Remainder
	template<auto X, std::integral T>
		requires (std::unsigned_integral<decltype(X)>)
	[[gnu::always_inline]] AA_CONSTEXPR auto remainder(const T x) {
		if constexpr (X == 1) return zero_v<T>;
		if constexpr (std::has_single_bit(X)) {
			return x & constant<X - 1>();
		} else {
			return x % X;
		}
	}

	// https://en.wikipedia.org/wiki/Quotient
	template<auto X, std::integral T>
		requires (std::unsigned_integral<decltype(X)>)
	[[gnu::always_inline]] AA_CONSTEXPR auto quotient(const T x) {
		if constexpr (X == 1) return x;
		if constexpr (std::has_single_bit(X)) {
			return x >> constant<int_log2(X)>();
		} else {
			return x / X;
		}
	}

	// https://en.wikipedia.org/wiki/Product_(mathematics)
	template<auto X, std::integral T>
		requires (std::unsigned_integral<decltype(X)>)
	[[gnu::always_inline]] AA_CONSTEXPR auto product(const T x) {
		if constexpr (X == 1) return x;
		if constexpr (std::has_single_bit(X)) {
			return x << constant<int_log2(X)>();
		} else {
			return x * X;
		}
	}



	// Pagal nutylėjimą, one_v yra U tipo, o ne T tipo, nes būtų neteisinga gauti rezultato tipą iš dešinės pusės tipo.
	// x turi būti unsigned, nes undefined behavior jei dešinysis operandas neigiamas << ir >> operatoriuose.
	// Išviso U ir T nepilnai generic, nes reiktų tada tikrinti ar su tais tipais išeitų vykdyti reikiamas operacijas.
	// https://en.wikipedia.org/wiki/Power_of_two
	template<std::integral U = size_t, auto N = 1uz, std::unsigned_integral T>
		requires (std::unsigned_integral<decltype(N)>)
	[[gnu::always_inline]] AA_CONSTEXPR auto int_exp2(const T x) {
		return one_v<U> << product<N>(x);
	}

	// Neturime dar vieno tipo U, nes tada T tipas nebūtų panaudojamas.
	// https://en.wikipedia.org/wiki/Find_first_set
	// https://en.wikipedia.org/wiki/Binary_logarithm
	template<auto N, std::unsigned_integral T>
		requires (std::unsigned_integral<decltype(N)>)
	[[gnu::always_inline]] AA_CONSTEXPR auto int_log2(const T x) {
		return quotient<N>(int_log2(x));
	}



	template<class T>
	[[gnu::always_inline]] AA_CONSTEXPR size_t size_of(const size_t n) {
		return product<sizeof(T)>(n);
	}

	// https://cp-algorithms.com/algebra/binary-exp.html
	// https://en.wikipedia.org/wiki/Modular_exponentiation
	template<std::unsigned_integral T, std::unsigned_integral U>
	AA_CONSTEXPR T power(T a, U b) {
		T res = 1;
		do {
			if (b & 1) res *= a;
			if (b >>= 1) a *= a;
			else return res;
		} while (true);
	}

	// product_result_t tipas gali būti void, bet nieko tokio tai, nes return sakinys gali būti naudojamas
	// su išraišką, jei tos išraiškos tipas yra void, funkcijoje, kurios gražinamas tipas yra void.
	//
	// Galima būtų leisti kompiliatoriui nustatyti gražinamą tipą, bet tada vis tiek reiktų naudoti concept.
	template<class T>
	AA_CONSTEXPR product_result_t<const T &> sq(const T &x) {
		return x * x;
	}



	template<std::equality_comparable T>
	AA_CONSTEXPR bool is_numeric_max(const T &x) {
		return x == std::numeric_limits<T>::max();
	}

	template<std::equality_comparable T>
	AA_CONSTEXPR bool is_numeric_min(const T &x) {
		return x == std::numeric_limits<T>::min();
	}

}
