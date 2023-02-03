#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/generator.hpp"
#include "arithmetic.hpp"
#include <random> // uniform_real_distribution, uniform_int_distribution
#include <limits> // numeric_limits
#include <concepts> // floating_point
#include <type_traits> // conditional_t



namespace aa {

	template<arithmetic T>
	AA_CONSTEXPR std::conditional_t<std::floating_point<T>, std::uniform_real_distribution<T>, std::uniform_int_distribution<T>>
		make_uniform_distribution(const T a, const T b)
	{
		if constexpr (std::floating_point<T>)		return std::uniform_real_distribution<T>{a, b};
		else										return std::uniform_int_distribution<T>{a, b};
	}



	// Naudotojas turėtų naudoti g(), nebent tikrai nuo 0 kažkodėl jam reiktų reikšmės.
	// [0, MAX - MIN + 1)
	template<class T, generator_result_convertible_to<T> G>
	AA_CONSTEXPR T int_distribution(G &g) {
		if constexpr (!is_numeric_min(G::min()))
			return static_cast<T>(g() - G::min());
		else
			return static_cast<T>(g());
	}

	template<same_as_void = void, bits_generator G>
	AA_CONSTEXPR generator_result_t<G> int_distribution(G &g) {
		return int_distribution<generator_result_t<G>>(g);
	}



	// Gali būti paduodamas ir modulus_type tipo kintamasis, tai ne klaida, klaida būtų jei reikšmė paduoto
	// kintamojo būtų didesnė už MODULUS. Jei būtų paduodamas kintamasis su didesne reikšme, generatorius
	// išviso nebegalėtų gražinti kai kurių reikšmių iš intervalo. Ta pati problema pasireiškia ir su double metodu.
	//
	// mag ir off ne generic tipo, nes taip tik apsisunkiname gyvenimą paduodami į funkcijas konstantas.
	// Naudotojas pats turi sąmoningai atlikti static_cast, jei jis turi netinkamą tipą.
	// [0, mag)
	template<class T, distribution_result_convertible_to<T> G>
	AA_CONSTEXPR T int_distribution(G &g, const distribution_result_t<G> mag) {
		return static_cast<T>((mag * static_cast<distribution_result_t<G>>(g())) >> std::numeric_limits<generator_result_t<G>>::digits);
	}

	template<same_as_void = void, full_range_generator G>
	AA_CONSTEXPR distribution_result_t<G> int_distribution(G &g, const distribution_result_t<G> mag) {
		return int_distribution<distribution_result_t<G>>(g, mag);
	}

	// [off, mag + off)
	template<class T, distribution_result_convertible_to<T> G>
	AA_CONSTEXPR T int_distribution(G &g, const distribution_result_t<G> off, const distribution_result_t<G> mag) {
		return static_cast<T>(int_distribution(g, mag) + off);
	}

	template<same_as_void = void, full_range_generator G>
	AA_CONSTEXPR distribution_result_t<G> int_distribution(G &g, const distribution_result_t<G> off, const distribution_result_t<G> mag) {
		return int_distribution<distribution_result_t<G>>(g, off, mag);
	}



	// Čia nėra daromas atsitiktinio skaičiaus cast į kažkokį paduotą tipą, nes T negalime nustatyti iš paduoto argumento tipo.
	// [0, 1)
	template<std::floating_point T = double, generator_result_representable_by<T> G>
	AA_CONSTEXPR T real_distribution(G &g) {
		return quotient<generator_modulus_v<G>>(int_distribution<T>(g));
	}

	// [0, mag)
	template<convertible_from_floating_point U, std::floating_point T, generator_result_representable_by<T> G>
	AA_CONSTEXPR U real_distribution(G &g, const T mag) {
		return static_cast<U>(real_distribution<T>(g) * mag);
	}

	template<same_as_void = void, std::floating_point T, generator_result_representable_by<T> G>
	AA_CONSTEXPR T real_distribution(G &g, const T mag) {
		return real_distribution<T>(g, mag);
	}

	// [off, mag + off)
	template<convertible_from_floating_point U, std::floating_point T, generator_result_representable_by<T> G>
	AA_CONSTEXPR U real_distribution(G &g, const T off, const T mag) {
		return static_cast<U>(real_distribution(g, mag) + off);
	}

	template<same_as_void = void, std::floating_point T, generator_result_representable_by<T> G>
	AA_CONSTEXPR T real_distribution(G &g, const T off, const T mag) {
		return real_distribution<T>(g, off, mag);
	}

}
