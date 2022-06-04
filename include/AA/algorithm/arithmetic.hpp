#pragma once

#include "../metaprogramming/general.hpp"
#include <cstddef> // size_t
#include <concepts> // integral, unsigned_integral, floating_point, same_as
#include <bit> // bit_width, has_single_bit



namespace aa {

	template<std::floating_point T>
	inline constexpr T norm(const T value, const T mag) {
		return value / mag;
	}

	template<std::floating_point T>
	inline constexpr T norm(const T value, const T start, const T mag) {
		return (value - start) / mag;
	}

	template<std::floating_point T>
	inline constexpr T map(const T value, const T mag1, const T mag2) {
		return mag2 * norm(value, mag1);
	}

	template<zero<>, std::floating_point T>
	inline constexpr T map(const T value, const T start1, const T mag1, const T mag2) {
		return mag2 * norm(value, start1, mag1);
	}

	template<one<>, std::floating_point T>
	inline constexpr T map(const T value, const T mag1, const T start2, const T mag2) {
		return start2 + mag2 * norm(value, mag1);
	}

	template<std::floating_point T>
	inline constexpr T map(const T value, const T start1, const T mag1, const T start2, const T mag2) {
		return start2 + mag2 * norm(value, start1, mag1);
	}



	// https://en.wikipedia.org/wiki/Find_first_set
	template<std::unsigned_integral T>
	inline constexpr T int_log2(const T x) {
		return std::bit_width(x >> 1);
	}

	// Pagal nutylėjimą, one_v yra U tipo, o ne T tipo, nes būtų neteisinga gauti rezultato tipą iš dešinės pusės tipo.
	// x turi būti unsigned, nes undefined behavior jei dešinysis operandas neigiamas << ir >> operatoriuose.
	// Išviso U ir T nepilnai generic, nes palaikomas toks bendras stilius, kad dirbama tik su sveikaisiais skaičiais.
	// https://en.wikipedia.org/wiki/Power_of_two
	template<std::integral U = size_t, std::unsigned_integral T>
	[[gnu::always_inline]] inline constexpr U int_exp2(const T x) {
		return one_v<U> << x;
	}



	// https://en.wikipedia.org/wiki/Remainder
	template<auto X, unsigned_integral_same_as<decltype(X)> T>
	[[gnu::always_inline]] inline constexpr T remainder(const T x) {
		if constexpr (std::has_single_bit(X)) {
			return x & constant_v<X - 1>;
		} else {
			return x % X;
		}
	}

	// https://en.wikipedia.org/wiki/Quotient
	template<auto X, unsigned_integral_same_as<decltype(X)> T>
	[[gnu::always_inline]] inline constexpr T quotient(const T x) {
		if constexpr (std::has_single_bit(X)) {
			return x >> constant_v<int_log2(X)>;
		} else {
			return x / X;
		}
	}

	// https://en.wikipedia.org/wiki/Product_(mathematics)
	template<auto X, unsigned_integral_same_as<decltype(X)> T>
	[[gnu::always_inline]] inline constexpr T product(const T x) {
		if constexpr (std::has_single_bit(X)) {
			return x << constant_v<int_log2(X)>;
		} else {
			return x * X;
		}
	}



	template<class T>
	[[gnu::always_inline]] inline constexpr size_t size_of(const size_t n) {
		return product<sizeof(T)>(n);
	}



	// https://cp-algorithms.com/algebra/binary-exp.html
	// https://en.wikipedia.org/wiki/Modular_exponentiation
	template<std::unsigned_integral T, std::unsigned_integral U>
	inline constexpr T power(T a, U b) {
		T res = 1;
		do {
			if (b & 1) res *= a;
			if (b >>= 1) a *= a;
			else return res;
		} while (true);
	}

	// product_result_t tipas gali būti void, bet nieko tokio tai, nes return sakinys gali būti naudojamas
	// su išraišką, jei tos išraiškos tipas yra void, funkcijoje, kurios gražinamas tipas yra void.
	template<class T>
	inline constexpr product_result_t<T> sq(const T &x) {
		return x * x;
	}

}
