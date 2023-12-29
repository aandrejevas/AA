#pragma once

#include "../metaprogramming/general.hpp"
#include <cmath> // ceil, round, sqrt, pow, lerp



namespace aa {

	// double tipas gali reprezentuoti sveikuosius skaičius tik iki 2^53 imtinai, todėl paduodamos reikšmės
	// turėtų būti nedidesnės už 2^53. float tipas tik tik 2^24 imtinai.
	//
	// Kadangi grąžinamo expression tipas (be cast) nėra nuspėjamas iš parametrų, todėl turime template parametrą, kuris nurodo
	// į kokį tipą reiktų cast'inti. Pvz. int_exp2 neturi tokio parametro, nes ten template parametras dalyvauja expression'e.
	// https://en.wikipedia.org/wiki/Square_root
	template<constructible_from_floating O = std::ranges::dangling, arithmetic T>
	constexpr auto ceil_sqrt(const T x) {
		using U = std::conditional_t<std::same_as<std::ranges::dangling, O>, T, O>;
		return cast<U>(std::ceil(std::sqrt(x)));
	}

	// https://en.wikipedia.org/wiki/Exponentiation
	template<constructible_from_floating O = std::ranges::dangling, arithmetic T, arithmetic E>
	constexpr auto ceil_pow(const T x, const E exp) {
		using U = std::conditional_t<std::same_as<std::ranges::dangling, O>, T, O>;
		return cast<U>(std::ceil(std::pow(x, exp)));
	}

	// https://en.wikipedia.org/wiki/Linear_interpolation
	template<constructible_from_floating O = std::ranges::dangling, arithmetic T, std::floating_point E>
	constexpr auto round_lerp(const T a, const T b, const E t) {
		using U = std::conditional_t<std::same_as<std::ranges::dangling, O>, T, O>;
		return cast<U>(std::round(std::lerp(a, b, t)));
	}

}
