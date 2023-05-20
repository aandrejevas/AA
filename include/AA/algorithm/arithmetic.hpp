#pragma once

#include "../metaprogramming/general.hpp"
#include <cmath> // fmod



namespace aa {

	template<std::floating_point T>
	AA_CONSTEXPR T norm(const T value, const T mag) {
		return value / mag;
	}

	template<std::floating_point T>
	AA_CONSTEXPR T norm(const T value, const T start, const T mag) {
		return (value - start) / mag;
	}



	template<std::floating_point T>
	AA_CONSTEXPR T map(const T value, const T mag) {
		return value * mag;
	}

	template<std::floating_point T>
	AA_CONSTEXPR T map(const T value, const T start, const T mag) {
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



#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
	// https://en.wikipedia.org/wiki/Product_(mathematics)
	// X parametras skirtas indikuoti kiek kartų turi būti padaugintas x, išties nebūtinai iš X bus dauginama.
	// Gražinamas tipas auto, nes expression, kuriame dalyvauja mažesni tipai negu int, gražinamas tipas yra int.
	// Reikalaujama, kad T būtų unsigned, nes per shift operacijas neįmanoma pagreitinti neigiamų skaičių daugybos.
	template<arithmetic auto X, arithmetic T>
	AA_CONSTEXPR T product(const T x) {
		if constexpr (!std::same_as<T, const_t<X>>)		return product<static_cast<T>(X)>(x);
		else if constexpr (X == 0)						return zero_v<T>;
		else if constexpr (X == 1)						return x;
		else if constexpr (!std::unsigned_integral<T>)	return x * X;
		else if constexpr (!std::has_single_bit(X))		return x * X;
		else											return x << const_v<int_log2(X)>;
	}

	// https://en.wikipedia.org/wiki/Remainder
	template<arithmetic auto X, arithmetic T>
		requires (X != 0)
	AA_CONSTEXPR T remainder(const T x) {
		if constexpr (!std::same_as<T, const_t<X>>)		return remainder<static_cast<T>(X)>(x);
		else if constexpr (X == 1)						return zero_v<T>;
		else if constexpr (std::floating_point<T>)		return std::fmod(x, X);
		else if constexpr (std::signed_integral<T>)		return x % X;
		else if constexpr (!std::has_single_bit(X))		return x % X;
		else											return x & const_v<X - 1>;
	}

	// https://en.wikipedia.org/wiki/Quotient
	template<arithmetic auto X, arithmetic T>
		requires (X != 0)
	AA_CONSTEXPR T quotient(const T x) {
		if constexpr (!std::same_as<T, const_t<X>>)		return quotient<static_cast<T>(X)>(x);
		else if constexpr (X == 1)						return x;
		else if constexpr (std::floating_point<T>)		return product<1 / X>(x);
		else if constexpr (std::signed_integral<T>)		return x / X;
		else if constexpr (!std::has_single_bit(X))		return x / X;
		else											return x >> const_v<int_log2(X)>;
	}
#pragma GCC diagnostic pop



	// Pagal nutylėjimą, one_v yra U tipo, o ne T tipo, nes būtų neteisinga gauti rezultato tipą iš dešinės pusės tipo.
	// x turi būti unsigned, nes undefined behavior jei dešinysis operandas neigiamas << ir >> operatoriuose.
	// Išviso U ir T nepilnai generic, nes reiktų tada tikrinti ar su tais tipais išeitų vykdyti reikiamas operacijas.
	// https://en.wikipedia.org/wiki/Power_of_two
	template<arithmetic auto N, std::integral U = size_t, std::unsigned_integral T>
	AA_CONSTEXPR U int_exp2(const T x) {
		return int_exp2<U>(product<N>(x));
	}

	// https://en.wikipedia.org/wiki/Find_first_set
	// https://en.wikipedia.org/wiki/Binary_logarithm
	template<arithmetic auto N, std::integral U = size_t, std::unsigned_integral T>
	AA_CONSTEXPR U int_log2(const T x) {
		return quotient<N>(int_log2<U>(x));
	}



	template<std::unsigned_integral U = size_t, std::unsigned_integral T>
	AA_CONSTEXPR U magic_binary_number(const T x) {
		return constant_v<U, numeric_max> / (int_exp2<U>(int_exp2(x)) | 1);
	}

	template<arithmetic T>
	AA_CONSTEXPR T halve(const T x) {
		return quotient<2>(x);
	}

	template<class T>
	AA_CONSTEXPR size_t size_of(const size_t n) {
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
	AA_CONSTEXPR T sq(const T x) {
		return x * x;
	}

	// https://graphics.stanford.edu/~seander/bithacks.html#IntegerMinOrMax
	template<std::unsigned_integral T>
	AA_CONSTEXPR T min(const T x, const T y) {
		const T d = (x - y);
		return y + (d & std::bit_cast<T>(std::bit_cast<std::make_signed_t<T>>(d) >> std::numeric_limits<std::make_signed_t<T>>::digits));
	}

	template<std::unsigned_integral T>
	AA_CONSTEXPR T max(const T x, const T y) {
		const T d = (x - y);
		return x - (d & std::bit_cast<T>(std::bit_cast<std::make_signed_t<T>>(d) >> std::numeric_limits<std::make_signed_t<T>>::digits));
	}

	// https://graphics.stanford.edu/~seander/bithacks.html#ReverseParallel
	template<regular_unsigned_integral T>
	AA_CONSTEXPR T bitswap(T v) {
		return apply<int_log2(unsign(std::numeric_limits<T>::digits))>([&]<size_t... I>() -> T {
			return ((v = ((v >> const_v<int_exp2(I)>) & const_v<magic_binary_number<T>(I)>)
					   | ((v & const_v<magic_binary_number<T>(I)>) << const_v<int_exp2(I)>)), ...);
		});
	}

}
