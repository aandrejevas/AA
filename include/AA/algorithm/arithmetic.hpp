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

	template<placeholder V, std::floating_point T>
		requires (is_one(std::is_placeholder_v<V>))
	AA_CONSTEXPR T norm_map(const T value, const T start1, const T mag1, const T mag2) {
		return aa::map(aa::norm(value, start1, mag1), mag2);
	}

	template<placeholder V, std::floating_point T>
		requires (is_two(std::is_placeholder_v<V>))
	AA_CONSTEXPR T norm_map(const T value, const T mag1, const T start2, const T mag2) {
		return aa::map(aa::norm(value, mag1), start2, mag2);
	}

	template<std::floating_point T>
	AA_CONSTEXPR T norm_map(const T value, const T start1, const T mag1, const T start2, const T mag2) {
		return aa::map(aa::norm(value, start1, mag1), start2, mag2);
	}



	// https://en.wikipedia.org/wiki/Product_(mathematics)
	// Pagal X funkcija nustato, kurį x kartotinį gražinti arba X naudoja kaip daukilį.
	// Gražinamas tipas auto, nes expression, kuriame dalyvauja mažesni tipai negu int, gražinamas tipas yra int.
	// Reikalaujama, kad T būtų unsigned, nes per shift operacijas neįmanoma pagreitinti neigiamų skaičių daugybos.
	// Bandžiau rekursijos nenaudoti, gaunasi tas pats tik vietoje X visur atsiranda cast X į T.
	// Turime turėti du tokius pačius return sakinius, nes apjungus sąlygas X gali sąlygoje būti float.
	template<arithmetic T, T X>
	AA_CONSTEXPR T product(const T x) {
		/**/ if constexpr (is_zero(X))					return zero_v<T>;
		else if constexpr (is_one(X))					return x;
		else if constexpr (!std::unsigned_integral<T>)	return x * X;
		else if constexpr (!std::has_single_bit(X))		return x * X;
		else											return x << const_v<int_log2(X)>;
	}

	template<auto X, convertible_from<const_t<X>> T>
	AA_CONSTEXPR T product(const T x) {
		return product<T, cast<T>(X)>(x);
	}

	// https://en.wikipedia.org/wiki/Remainder
	template<arithmetic T, T X>
		requires (!is_zero(X))
	AA_CONSTEXPR T remainder(const T x) {
		/**/ if constexpr (is_one(X))					return zero_v<T>;
		else if constexpr (std::floating_point<T>)		return std::fmod(x, X);
		else if constexpr (std::signed_integral<T>)		return x % X;
		else if constexpr (!std::has_single_bit(X))		return x % X;
		else											return x & const_v<X - 1>;
	}

	template<auto X, convertible_from<const_t<X>> T>
	AA_CONSTEXPR T remainder(const T x) {
		return remainder<T, cast<T>(X)>(x);
	}

	// https://en.wikipedia.org/wiki/Quotient
	template<arithmetic T, T X>
		requires (!is_zero(X))
	AA_CONSTEXPR T quotient(const T x) {
		/**/ if constexpr (is_one(X))					return x;
		else if constexpr (std::floating_point<T>)		return x * const_v<one_v<T> / X>;
		else if constexpr (std::signed_integral<T>)		return x / X;
		else if constexpr (!std::has_single_bit(X))		return x / X;
		else											return x >> const_v<int_log2(X)>;
	}

	template<auto X, convertible_from<const_t<X>> T>
	AA_CONSTEXPR T quotient(const T x) {
		return quotient<T, cast<T>(X)>(x);
	}



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
		return numeric_max_v<U> / (int_exp2<U>(int_exp2(x)) | 1);
	}

	template<arithmetic T>
	AA_CONSTEXPR T halve(const T x) {
		return quotient<two_v<T>>(x);
	}

	template<arithmetic T>
	AA_CONSTEXPR T redouble(const T x) {
		return product<two_v<T>>(x);
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
		return y + (d & unsign(sign(d) >> numeric_digits_v<std::make_signed_t<T>>));
	}

	template<std::unsigned_integral T>
	AA_CONSTEXPR T max(const T x, const T y) {
		const T d = (x - y);
		return x - (d & unsign(sign(d) >> numeric_digits_v<std::make_signed_t<T>>));
	}

	// https://graphics.stanford.edu/~seander/bithacks.html#ReverseParallel
	template<regular_unsigned_integral T>
	AA_CONSTEXPR T bitswap(T v) {
		return apply<int_log2(numeric_digits_v<T>)>([&]<size_t... I> -> T {
			return ((v = ((v >> const_v<int_exp2(I)>) & const_v<magic_binary_number<T>(I)>)
				/*	*/ | ((v & const_v<magic_binary_number<T>(I)>) << const_v<int_exp2(I)>)), ...);
		});
	}

	// https://graphics.stanford.edu/~seander/bithacks.html#SwappingBitsXOR
	template<regular_unsigned_integral T, std::unsigned_integral I, std::unsigned_integral J>
	AA_CONSTEXPR T byteswap(const T b, I i, J j) {
		i = product<numeric_digits_v<byte_t>>(i);
		j = product<numeric_digits_v<byte_t>>(j);
		const T x = ((b >> i) ^ (b >> j)) & constant_v<T, numeric_max_v<byte_t>>;
		return b ^ ((x << i) | (x << j));
	}

}
