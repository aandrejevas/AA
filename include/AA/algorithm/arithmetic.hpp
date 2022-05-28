#pragma once

#include "../metaprogramming/general.hpp"
#include <cstddef> // size_t
#include <type_traits> // integral_constant
#include <concepts> // unsigned_integral, integral, same_as
#include <limits> // numeric_limits
#include <bit> // countl_zero, has_single_bit



namespace aa {

	// Darome cast'ą į size_t pagal nutylėjimą, nes kitaip negalėtume gražinamos reikšmės paduoti į int_exp2.
	// https://en.wikipedia.org/wiki/Find_first_set
	template<std::integral U = size_t, std::unsigned_integral T>
	inline constexpr U int_log2(const T x) {
		return static_cast<U>(std::numeric_limits<T>::digits - std::countl_zero(x >> 1));
	}

	// x turi būti unsigned, nes undefined behavior jei dešinysis operandas neigiamas << ir >> operatoriuose.
	// https://en.wikipedia.org/wiki/Power_of_two
	template<std::integral U = size_t, std::unsigned_integral T>
	[[gnu::always_inline]] inline constexpr U int_exp2(const T x) {
		return std::integral_constant<U, 1>::value << x;
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
	template<multipliable_with T>
	inline constexpr product_result_t<T> sq(const T &x) {
		return x * x;
	}

}
