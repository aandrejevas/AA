#pragma once

#include "../metaprogramming/general.hpp"
#include <cmath> // ceil, round, sqrt, pow, lerp



namespace aa {

	// double tipas gali reprezentuoti sveikuosius skaičius tik iki 2^53 imtinai, todėl paduodamos reikšmės
	// turėtų būti nedidesnės už 2^53. float tipas tik tik 2^24 imtinai.
	//
	// x generic, nes sqrt generic, taip pat sqrt grąžinamos reikšmės tipas priklauso nuo paduodamo argumento tipo.
	// https://en.wikipedia.org/wiki/Square_root
	template<constructible_from_floating_point U, arithmetic T>
	constexpr U ceil_sqrt(const T x) {
		return cast<U>(std::ceil(std::sqrt(x)));
	}

	template<same_as_void = void, arithmetic T>
	constexpr T ceil_sqrt(const T x) {
		return ceil_sqrt<T>(x);
	}

	// https://en.wikipedia.org/wiki/Exponentiation
	template<constructible_from_floating_point U, arithmetic T, arithmetic E>
	constexpr U ceil_pow(const T x, const E exp) {
		return cast<U>(std::ceil(std::pow(x, exp)));
	}

	template<same_as_void = void, arithmetic T, arithmetic E>
	constexpr T ceil_pow(const T x, const E exp) {
		return ceil_pow<T>(x, exp);
	}

	// https://en.wikipedia.org/wiki/Linear_interpolation
	template<constructible_from_floating_point U, arithmetic A, arithmetic B, arithmetic E>
	constexpr U round_lerp(const A a, const B b, const E t) {
		return cast<U>(std::round(std::lerp(a, b, t)));
	}

	template<same_as_void = void, arithmetic T, arithmetic E>
	constexpr T round_lerp(const T a, const T b, const E t) {
		return round_lerp<T>(a, b, t);
	}

}
