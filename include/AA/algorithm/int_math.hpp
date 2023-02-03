#pragma once

#include "../metaprogramming/general.hpp"
#include <cmath> // ceil, sqrt, pow



namespace aa {

	// double tipas gali reprezentuoti sveikuosius skaičius tik iki 2^53 imtinai, todėl paduodamos reikšmės
	// turėtų būti nedidesnės už 2^53. float tipas tik tik 2^24 imtinai.
	//
	// x generic, nes sqrt generic, taip pat sqrt gražinamos reikšmės tipas priklauso nuo paduodamo argumento tipo.
	// https://en.wikipedia.org/wiki/Square_root
	template<convertible_from_floating_point U, arithmetic T>
	AA_CONSTEXPR U ceil_sqrt(const T x) {
		return static_cast<U>(std::ceil(std::sqrt(x)));
	}

	template<same_as_void = void, arithmetic T>
	AA_CONSTEXPR T ceil_sqrt(const T x) {
		return ceil_sqrt<T>(x);
	}

	// https://en.wikipedia.org/wiki/Exponentiation
	template<convertible_from_floating_point U, arithmetic T, arithmetic E>
	AA_CONSTEXPR U ceil_pow(const T x, const E exp) {
		return static_cast<U>(std::ceil(std::pow(x, exp)));
	}

	template<same_as_void = void, arithmetic T, arithmetic E>
	AA_CONSTEXPR T ceil_pow(const T x, const E exp) {
		return ceil_pow<T>(x, exp);
	}

}
