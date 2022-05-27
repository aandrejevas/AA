#pragma once

#include "../metaprogramming/general.hpp"
#include <cmath> // ceil, sqrt, pow



namespace aa {

	// double tipas gali reprezentuoti sveikuosius skaičius tik iki 2^53 imtinai, todėl paduodamos reikšmės
	// turėtų būti nedidesnės už 2^53. float tipas tik tik 2^24 imtinai.
	//
	// x generic, nes sqrt generic, taip pat sqrt gražinamos reikšmės tipas priklauso nuo paduodamo argumento tipo.
	// https://en.wikipedia.org/wiki/Square_root
	template<convertible_from_floating_point U = void, arithmetic T>
	inline fist_not_void_t<U, T> ceil_sqrt(const T x) {
		return static_cast<fist_not_void_t<U, T>>(std::ceil(std::sqrt(x)));
	}

	// https://en.wikipedia.org/wiki/Exponentiation
	template<convertible_from_floating_point U = void, arithmetic T, arithmetic E>
	inline fist_not_void_t<U, T> ceil_pow(const T x, const E exp) {
		return static_cast<fist_not_void_t<U, T>>(std::ceil(std::pow(x, exp)));
	}

}
