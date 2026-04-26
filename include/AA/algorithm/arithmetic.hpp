#pragma once

#include "../metaprogramming/general.hpp"
#include <cmath>



namespace aa {

	template<std::floating_point T>
	constexpr T map(const T value, const T mag1, const T mag2) {
		return value * (mag2 / mag1);
	}

	template<std::floating_point T>
	constexpr T map(const T value, const pair<T> p1, const T mag2) {
		return (value - get_0(p1)) * (mag2 / get_1(p1));
	}

	template<std::floating_point T>
	constexpr T map(const T value, const T mag1, const pair<T> p2) {
		return std::fma(value, (get_1(p2) / mag1), get_0(p2));
	}

	template<std::floating_point T>
	constexpr T map(const T value, const T start1, const T mag1, const T start2, const T mag2) {
		return std::fma((value - start1), (mag2 / mag1), start2);
	}



	// https://en.wikipedia.org/wiki/Product_(mathematics)
	// Pagal X funkcija nustato, kurį x kartotinį grąžinti arba X naudoja kaip daugiklį.
	// Grąžinamas tipas auto, nes expression, kuriame dalyvauja mažesni tipai negu int, grąžinamas tipas yra int.
	// Reikalaujama, kad T būtų unsigned, nes per shift operacijas neįmanoma pagreitinti neigiamų skaičių daugybos.
	// Bandžiau rekursijos nenaudoti, gaunasi tas pats tik vietoje X visur atsiranda cast X į T.
	// Turime turėti du tokius pačius return sakinius, nes apjungus sąlygas X gali sąlygoje būti float.
	template<arithmetic auto X, arithmetic T>
	constexpr T product(const T x) {
		/**/ if constexpr (different_from<t<X>, T>)		return product<T{X}>(x);
		else if constexpr (X == value<0>)				return default_value;
		else if constexpr (X == value<1>)				return x;
		else if constexpr (std::signed_integral<T>)		return x * X;
		else if constexpr (!std::has_single_bit(X))		return x * X;
		else											return x << c(int_log2(X));
	}

	// https://en.wikipedia.org/wiki/Remainder
	// Nepaduodame pack V, nes arithmetic tipo iš pack neišeitų sukonstruoti.
	// Viduje requires reikia naudoti value_v, nes kitaip pvz. būtų float'ai neteisingai patikrinami kai T int.
	template<arithmetic auto X, arithmetic T>
		requires (T{X} != value<0>)
	constexpr T remainder(const T x) {
		/**/ if constexpr (different_from<t<X>, T>)		return remainder<T{X}>(x);
		else if constexpr (X == value<1>)				return default_value;
		else if constexpr (std::floating_point<T>)		return std::fmod(x, X);
		else if constexpr (std::signed_integral<T>)		return x % X;
		else if constexpr (!std::has_single_bit(X))		return x % X;
		else											return x & c(X - 1);
	}

	// https://en.wikipedia.org/wiki/Quotient
	template<arithmetic auto X, arithmetic T>
		requires (T{X} != value<0>)
	constexpr T quotient(const T x) {
		/**/ if constexpr (different_from<t<X>, T>)		return quotient<T{X}>(x);
		else if constexpr (X == value<1>)				return x;
		else if constexpr (std::floating_point<T>)		return x * c(T{1} / X);
		else if constexpr (std::signed_integral<T>)		return x / X;
		else if constexpr (!std::has_single_bit(X))		return x / X;
		else											return x >> c(int_log2(X));
	}



	// Pagal nutylėjimą, value_v yra U tipo, o ne T tipo, nes būtų neteisinga gauti rezultato tipą iš dešinės pusės tipo.
	// x turi būti unsigned, nes undefined behavior jei dešinysis operandas neigiamas << ir >> operatoriuose.
	// Išviso U ir T nepilnai generic, nes reiktų tada tikrinti ar su tais tipais išeitų vykdyti reikiamas operacijas.
	// https://en.wikipedia.org/wiki/Power_of_two
	template<arithmetic auto N, std::integral U = size_t, std::unsigned_integral T>
	constexpr U int_exp2(const T x) {
		return int_exp2<U>(product<N>(x));
	}

	// https://en.wikipedia.org/wiki/Find_first_set
	// https://en.wikipedia.org/wiki/Binary_logarithm
	template<arithmetic auto N, std::integral U = size_t, std::unsigned_integral T>
	constexpr U int_log2(const T x) {
		return quotient<N>(int_log2<U>(x));
	}



	template<std::unsigned_integral U = size_t, std::unsigned_integral T>
	constexpr U magic_binary_number(const T x) {
		// exp expression tipai galėtų būti: uint << (size_t << uint). Bet greitaveikos neprarandame.
		return c<U>(numeric_max) / (int_exp2<U>(int_exp2(x)) | c<U>(1));
	}

	template<arithmetic T>
	constexpr T half(const T x) {
		return quotient<2>(x);
	}

	template<arithmetic T>
	constexpr T twice(const T x) {
		return product<2>(x);
	}

	template<std::integral T>
	constexpr bool is_even(const T x) {
		return !remainder<2>(unsign(x));
	}

	template<std::integral T>
	constexpr bool is_odd(const T x) {
		return !!remainder<2>(unsign(x));
	}

	// https://cp-algorithms.com/algebra/binary-exp.html
	// https://en.wikipedia.org/wiki/Modular_exponentiation
	template<std::unsigned_integral T, std::unsigned_integral U>
	constexpr T pow(T a, U b) {
		T res = 1;
		do {
			if (b & 1) res *= a;
			if (b >>= 1) a *= a;
			else return res;
		} while (true);
	}

	template<size_t N = 2, arithmetic T>
		requires (N <= 4)
	constexpr T pow(const T x) {
		/**/ if constexpr (N == value<0>)	return value<1>;
		else if constexpr (N == value<1>)	return x;
		else if constexpr (N == value<2>)	return x * x;
		else if constexpr (N == value<3>)	return x * x * x;
		else								return pow(pow(x));
	}

	// https://en.wikipedia.org/wiki/Smoothstep
	template<size_t N = 1, std::floating_point T>
		requires (N <= 3)
	constexpr T smoothstep(const T x) {
		/*  */ if constexpr (N == value<0>) {
			return x;
		} else if constexpr (N == value<1>) {
			return pow(x) * (product<-2>(x) + c(T{3}));
		} else if constexpr (N == value<2>) {
			const T xx = pow(x);
			return xx * x * (product<6>(xx) + product<-15>(x) + c(T{10}));
		} else {
			const T x2 = pow(x), x3 = x2 * x;
			return x3 * x * (product<-20>(x3) + product<70>(x2) + product<-84>(x) + c(T{35}));
		}
	}

	// https://graphics.stanford.edu/~seander/bithacks.html#IntegerMinOrMax
	template<std::unsigned_integral T>
	constexpr T min(const T x, const T y) {
		const T d = (x - y);
		return y + (d & unsign(sign(d) >> numeric_digits<std::make_signed_t<T>>()));
	}

	template<std::unsigned_integral T>
	constexpr T max(const T x, const T y) {
		const T d = (x - y);
		return x - (d & unsign(sign(d) >> numeric_digits<std::make_signed_t<T>>()));
	}

	// https://graphics.stanford.edu/~seander/bithacks.html#ReverseParallel
	template<regular_unsigned_integral T>
	constexpr T bitswap(T v) {
		return apply<int_log2(numeric_digits<T>())>(
			[&]<size_t... I> -> T {
				return ((v = ((v >> c(int_exp2(I))) & c(magic_binary_number<T>(I)))
					/*	*/ | ((v & c(magic_binary_number<T>(I))) << c(int_exp2(I)))), ...);
			});
	}

	// https://graphics.stanford.edu/~seander/bithacks.html#SwappingBitsXOR
	template<regular_unsigned_integral T, std::unsigned_integral I, std::unsigned_integral J>
	constexpr T byteswap(const T b, I i, J j) {
		i = product<numeric_digits<std::byte>()>(i);
		j = product<numeric_digits<std::byte>()>(j);
		const T x = ((b >> i) ^ (b >> j)) & c(T{std::byte{numeric_max}});
		return b ^ ((x << i) | (x << j));
	}

}
