#pragma once

#include "../metaprogramming/general.hpp"
#include <cmath> // fmod
#include <numeric> // midpoint



namespace aa {

	template<std::floating_point T>
	constexpr T norm(const T value, const T mag) {
		return value / mag;
	}

	template<std::floating_point T>
	constexpr T norm(const T value, const T start, const T mag) {
		return (value - start) / mag;
	}



	template<std::floating_point T>
	constexpr T map(const T value, const T mag) {
		return value * mag;
	}

	template<std::floating_point T>
	constexpr T map(const T value, const T start, const T mag) {
		return start + value * mag;
	}



	template<std::floating_point T>
	constexpr T norm_map(const T value, const T mag1, const T mag2) {
		return aa::map(aa::norm(value, mag1), mag2);
	}

	template<placeholder<1> auto, std::floating_point T>
	constexpr T norm_map(const T value, const T start1, const T mag1, const T mag2) {
		return aa::map(aa::norm(value, start1, mag1), mag2);
	}

	template<placeholder<2> auto, std::floating_point T>
	constexpr T norm_map(const T value, const T mag1, const T start2, const T mag2) {
		return aa::map(aa::norm(value, mag1), start2, mag2);
	}

	template<std::floating_point T>
	constexpr T norm_map(const T value, const T start1, const T mag1, const T start2, const T mag2) {
		return aa::map(aa::norm(value, start1, mag1), start2, mag2);
	}



	// https://en.wikipedia.org/wiki/Product_(mathematics)
	// Pagal X funkcija nustato, kurį x kartotinį grąžinti arba X naudoja kaip daukilį.
	// Grąžinamas tipas auto, nes expression, kuriame dalyvauja mažesni tipai negu int, grąžinamas tipas yra int.
	// Reikalaujama, kad T būtų unsigned, nes per shift operacijas neįmanoma pagreitinti neigiamų skaičių daugybos.
	// Bandžiau rekursijos nenaudoti, gaunasi tas pats tik vietoje X visur atsiranda cast X į T.
	// Turime turėti du tokius pačius return sakinius, nes apjungus sąlygas X gali sąlygoje būti float.
	template<arithmetic T, T X>
	constexpr T product(const T x) {
		/**/ if constexpr (is_value<0>(X))				return value_v<T, 0>;
		else if constexpr (is_value<1>(X))				return x;
		else if constexpr (!std::unsigned_integral<T>)	return x * X;
		else if constexpr (!std::has_single_bit(X))		return x * X;
		else											return x << const_v<int_log2(X)>;
	}

	template<auto X, arithmetic T>
	constexpr T product(const T x) {
		return product<T, value_v<T, X>>(x);
	}

	// https://en.wikipedia.org/wiki/Remainder
	template<arithmetic T, T X>
		requires (!is_value<0>(X))
	constexpr T remainder(const T x) {
		/**/ if constexpr (is_value<1>(X))				return value_v<T, 0>;
		else if constexpr (std::floating_point<T>)		return std::fmod(x, X);
		else if constexpr (std::signed_integral<T>)		return x % X;
		else if constexpr (!std::has_single_bit(X))		return x % X;
		else											return x & const_v<X - 1>;
	}

	template<auto X, arithmetic T>
	constexpr T remainder(const T x) {
		return remainder<T, value_v<T, X>>(x);
	}

	// https://en.wikipedia.org/wiki/Quotient
	template<arithmetic T, T X>
		requires (!is_value<0>(X))
	constexpr T quotient(const T x) {
		/**/ if constexpr (is_value<1>(X))				return x;
		else if constexpr (std::floating_point<T>)		return x * const_v<value_v<T, 1> / X>;
		else if constexpr (std::signed_integral<T>)		return x / X;
		else if constexpr (!std::has_single_bit(X))		return x / X;
		else											return x >> const_v<int_log2(X)>;
	}

	template<auto X, arithmetic T>
	constexpr T quotient(const T x) {
		return quotient<T, value_v<T, X>>(x);
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
		return numeric_max_v<U> / (int_exp2<U>(int_exp2(x)) | 1);
	}

	template<arithmetic T>
	constexpr T halve(const T x) {
		return quotient<2>(x);
	}

	template<arithmetic T>
	constexpr T redouble(const T x) {
		return product<2>(x);
	}

	template<class T>
	constexpr size_t size_of(const size_t n) {
		return product<sizeof(T)>(n);
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
		/**/ if constexpr (is_value<0>(N))	return value_v<T, 1>;
		else if constexpr (is_value<1>(N))	return x;
		else if constexpr (is_value<2>(N))	return x * x;
		else if constexpr (is_value<3>(N))	return x * x * x;
		else								return pow(pow(x));
	}

	// https://en.wikipedia.org/wiki/Smoothstep
	template<size_t N = 1, std::floating_point T>
		requires (N <= 3)
	constexpr T smoothstep(const T x) {
		/*  */ if constexpr (is_value<0>(N)) {
			return x;
		} else if constexpr (is_value<1>(N)) {
			return pow(x) * (product<-2>(x) + value_v<T, 3>);
		} else if constexpr (is_value<2>(N)) {
			const T xx = pow(x);
			return xx * x * (product<6>(xx) + product<-15>(x) + value_v<T, 10>);
		} else {
			const T x2 = pow(x), x3 = x2 * x;
			return x3 * x * (product<-20>(x3) + product<70>(x2) + product<-84>(x) + value_v<T, 35>);
		}
	}

	// https://graphics.stanford.edu/~seander/bithacks.html#IntegerMinOrMax
	template<std::unsigned_integral T>
	constexpr T min(const T x, const T y) {
		const T d = (x - y);
		return y + (d & unsign(sign(d) >> numeric_digits_v<std::make_signed_t<T>>));
	}

	template<std::unsigned_integral T>
	constexpr T max(const T x, const T y) {
		const T d = (x - y);
		return x - (d & unsign(sign(d) >> numeric_digits_v<std::make_signed_t<T>>));
	}

	// https://graphics.stanford.edu/~seander/bithacks.html#ReverseParallel
	template<regular_unsigned_integral T>
	constexpr T bitswap(T v) {
		return apply<int_log2(numeric_digits_v<T>)>([&]<size_t... I> -> T {
			return ((v = ((v >> const_v<int_exp2(I)>) & const_v<magic_binary_number<T>(I)>)
				/*	*/ | ((v & const_v<magic_binary_number<T>(I)>) << const_v<int_exp2(I)>)), ...);
		});
	}

	// https://graphics.stanford.edu/~seander/bithacks.html#SwappingBitsXOR
	template<regular_unsigned_integral T, std::unsigned_integral I, std::unsigned_integral J>
	constexpr T byteswap(const T b, I i, J j) {
		i = product<numeric_digits_v<byte_t>>(i);
		j = product<numeric_digits_v<byte_t>>(j);
		const T x = ((b >> i) ^ (b >> j)) & value_v<T, numeric_max_v<byte_t>>;
		return b ^ ((x << i) | (x << j));
	}



	// https://en.wikipedia.org/wiki/Interval_(mathematics)
	template<regular_scalar T, T MIN = numeric_min, T MAX = numeric_max>
	struct interval : pair<T> {
		// Member types
		using typename pair<T>::tuple_type;
		using value_type = T;
		using reference = value_type &;
		using const_reference = const value_type &;
		// Galėtume naudoti type_pack_element_t, bet tada taptų sunkiau suprasti logiką ir nukentėtų kompiliavimo gretis.
		using difference_type = std::conditional_t<pointer<value_type>, ptrdiff_t, value_type>;

		// Member constants
		static constexpr bool subinterval = MIN < MAX;



		// Operations
		constexpr reference min() { return get_0(*this); }
		constexpr reference max() { return get_1(*this); }
		constexpr const_reference min() const { return get_0(*this); }
		constexpr const_reference max() const { return get_1(*this); }

		constexpr bool min_eq(propagate_const_t<value_type> x) const { return min() == x; }
		constexpr bool max_eq(propagate_const_t<value_type> x) const { return max() == x; }
		constexpr bool eq(propagate_const_t<value_type> x1, propagate_const_t<value_type> x2) const {
			return min_eq(x1) && max_eq(x2);
		}

		constexpr bool open_contains(propagate_const_t<value_type> x) const { return min() < x && x < max(); }
		constexpr bool closed_contains(propagate_const_t<value_type> x) const { return min() <= x && x <= max(); }
		constexpr bool left_open_contains(propagate_const_t<value_type> x) const { return min() < x && x <= max(); }
		constexpr bool right_open_contains(propagate_const_t<value_type> x) const { return min() <= x && x < max(); }

		constexpr bool empty() const { return min() > max(); }
		constexpr bool degenerate() const { return min() == max(); }
		constexpr bool left_full() const requires (subinterval) { return min() == MIN; }
		constexpr bool right_full() const requires (subinterval) { return max() == MAX; }
		constexpr bool full() const requires (subinterval) { return left_full() && right_full(); }

		constexpr difference_type diameter() const { return max() - min(); }
		constexpr difference_type radius() const { return halve(diameter()); }
		constexpr value_type centre() const { return std::midpoint(min(), max()); }



		// Modifiers
		constexpr bool left_shrink(const value_type x) { return (min() < x) ? (min() = x, true) : false; }
		constexpr bool right_shrink(const value_type x) { return (x < max()) ? (max() = x, true) : false; }
		constexpr bool shrink(const value_type x1, const value_type x2) {
			return left_shrink(x1) | right_shrink(x2);
		}

		constexpr bool left_expand(const value_type x) { return (x < min()) ? (min() = x, true) : false; }
		constexpr bool right_expand(const value_type x) { return (max() < x) ? (max() = x, true) : false; }
		constexpr bool expand(const value_type x) {
			// Patikrinau, čia greičiausia teisinga realizacija.
			return left_expand(x) | right_expand(x);
		}

		constexpr bool unsafe_expand(const value_type x) {
			return left_expand(x) || right_expand(x);
		}

		// Logiška nenaudoti numeric_*, nes dabar yra galimybė, kad išplėčiant vieno endpoint nereikės pakeisti.
		constexpr void reset() requires (subinterval) {
			min() = MAX;	max() = MIN;
		}

		constexpr void reset(const value_type x) {
			min() = x;		max() = x;
		}

		constexpr void reset(const value_type x1, const value_type x2) {
			min() = x2;		max() = x1;
		}



		// Special member functions
		constexpr interval() requires (subinterval) : tuple_type{MAX, MIN} {}
		constexpr interval(const value_type x) : tuple_type{x, x} {}
		constexpr interval(const value_type x1, const value_type x2) : tuple_type{x2, x1} {}
	};

}
