#pragma once

#include "../metaprogramming/general.hpp"
#include <cstddef> // size_t
#include <cmath> // fmod
#include <concepts> // integral, unsigned_integral, signed_integral, floating_point, same_as
#include <bit> // countl_zero, has_single_bit, bit_cast
#include <limits> // numeric_limits
#include <type_traits> // make_signed_t



namespace aa {

	template<std::floating_point T>
	[[gnu::always_inline]] AA_CONSTEXPR T norm(const T value, const T mag) {
		return value / mag;
	}

	template<std::floating_point T>
	[[gnu::always_inline]] AA_CONSTEXPR T norm(const T value, const T start, const T mag) {
		return (value - start) / mag;
	}



	template<std::floating_point T>
	[[gnu::always_inline]] AA_CONSTEXPR T map(const T value, const T mag) {
		return value * mag;
	}

	template<std::floating_point T>
	[[gnu::always_inline]] AA_CONSTEXPR T map(const T value, const T start, const T mag) {
		return start + value * mag;
	}



	template<std::floating_point T>
	AA_CONSTEXPR T norm_map(const T value, const T mag1, const T mag2) {
		return aa::map(aa::norm(value, mag1), mag2);
	}

	template<size_t V, std::floating_point T>
		requires (V == 0)
	AA_CONSTEXPR T norm_map(const T value, const T start1, const T mag1, const T mag2) {
		return aa::map(aa::norm(value, start1, mag1), mag2);
	}

	template<size_t V, std::floating_point T>
		requires (V == 1)
	AA_CONSTEXPR T norm_map(const T value, const T mag1, const T start2, const T mag2) {
		return aa::map(aa::norm(value, mag1), start2, mag2);
	}

	template<std::floating_point T>
	AA_CONSTEXPR T norm_map(const T value, const T start1, const T mag1, const T start2, const T mag2) {
		return aa::map(aa::norm(value, start1, mag1), start2, mag2);
	}



	template<std::integral U = size_t, std::unsigned_integral T>
	[[gnu::always_inline]] AA_CONSTEXPR auto int_exp2(const T x) {
		return one_v<U> << x;
	}

	// T yra tipas, kurio bitus skaičiuosime, U nurodo kokiu tipu pateikti rezutatus.
	template<std::integral U = size_t, std::unsigned_integral T>
	[[gnu::always_inline]] AA_CONSTEXPR auto int_log2(const T x) {
		return constant<U, std::numeric_limits<T>::digits - 1>() - unsign<U>(std::countl_zero(x));
	}



#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
	// https://en.wikipedia.org/wiki/Product_(mathematics)
	// X parametras skirtas indikuoti kiek kartų turi būti padaugintas x, išties nebūtinai iš X bus dauginama.
	// Gražinamas tipas auto, nes expression, kuriame dalyvauja mažesni tipai negu int, gražinamas tipas yra int.
	// Reikalaujama, kad T būtų unsigned, nes per shift operacijas neįmanoma pagreitinti neigiamų skaičių daugybos.
	template<arithmetic auto X, arithmetic T>
	[[gnu::always_inline]] AA_CONSTEXPR auto product(const T x) {
		if constexpr (!std::same_as<T, decltype(X)>)	return product<static_cast<T>(X)>(x);
		else if constexpr (X == 0)						return zero_v<T>;
		else if constexpr (X == 1)						return x;
		else if constexpr (!std::unsigned_integral<T>)	return x * X;
		else if constexpr (!std::has_single_bit(X))		return x * X;
		else											return x << constant<int_log2(X)>();
	}

	// https://en.wikipedia.org/wiki/Remainder
	template<arithmetic auto X, arithmetic T>
		requires (X != 0)
	[[gnu::always_inline]] AA_CONSTEXPR auto remainder(const T x) {
		if constexpr (!std::same_as<T, decltype(X)>)	return remainder<static_cast<T>(X)>(x);
		else if constexpr (X == 1)						return zero_v<T>;
		else if constexpr (std::floating_point<T>)		return std::fmod(x, X);
		else if constexpr (std::signed_integral<T>)		return x % X;
		else if constexpr (!std::has_single_bit(X))		return x % X;
		else											return x & constant<X - 1>();
	}

	// https://en.wikipedia.org/wiki/Quotient
	template<arithmetic auto X, arithmetic T>
		requires (X != 0)
	[[gnu::always_inline]] AA_CONSTEXPR auto quotient(const T x) {
		if constexpr (!std::same_as<T, decltype(X)>)	return quotient<static_cast<T>(X)>(x);
		else if constexpr (X == 1)						return x;
		else if constexpr (std::floating_point<T>)		return product<1 / X>(x);
		else if constexpr (std::signed_integral<T>)		return x / X;
		else if constexpr (!std::has_single_bit(X))		return x / X;
		else											return x >> constant<int_log2(X)>();
	}
#pragma GCC diagnostic pop



	// Pagal nutylėjimą, one_v yra U tipo, o ne T tipo, nes būtų neteisinga gauti rezultato tipą iš dešinės pusės tipo.
	// x turi būti unsigned, nes undefined behavior jei dešinysis operandas neigiamas << ir >> operatoriuose.
	// Išviso U ir T nepilnai generic, nes reiktų tada tikrinti ar su tais tipais išeitų vykdyti reikiamas operacijas.
	// https://en.wikipedia.org/wiki/Power_of_two
	template<arithmetic auto N, std::integral U = size_t, std::unsigned_integral T>
	[[gnu::always_inline]] AA_CONSTEXPR auto int_exp2(const T x) {
		return int_exp2<U>(product<N>(x));
	}

	// https://en.wikipedia.org/wiki/Find_first_set
	// https://en.wikipedia.org/wiki/Binary_logarithm
	template<arithmetic auto N, std::integral U = size_t, std::unsigned_integral T>
	[[gnu::always_inline]] AA_CONSTEXPR auto int_log2(const T x) {
		return quotient<N>(int_log2<U>(x));
	}



	template<arithmetic T>
	[[gnu::always_inline]] AA_CONSTEXPR auto halve(const T x) {
		return quotient<2>(x);
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

	template<arithmetic T>
	[[gnu::always_inline]] AA_CONSTEXPR auto sq(const T x) {
		return x * x;
	}

	// https://graphics.stanford.edu/~seander/bithacks.html#IntegerMinOrMax
	template<std::unsigned_integral T>
	[[gnu::always_inline]] AA_CONSTEXPR T min(const T x, const T y) {
		const T d = (x - y);
		return y + (d & std::bit_cast<T>(std::bit_cast<std::make_signed_t<T>>(d) >> constant<std::numeric_limits<T>::digits - 1>()));
	}

	template<std::unsigned_integral T>
	[[gnu::always_inline]] AA_CONSTEXPR T max(const T x, const T y) {
		const T d = (x - y);
		return x - (d & std::bit_cast<T>(std::bit_cast<std::make_signed_t<T>>(d) >> constant<std::numeric_limits<T>::digits - 1>()));
	}

}
