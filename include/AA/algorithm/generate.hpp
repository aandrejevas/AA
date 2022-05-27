#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/generator.hpp"
#include <random> // uniform_random_bit_generator, uniform_real_distribution, uniform_int_distribution
#include <limits> // numeric_limits
#include <concepts> // floating_point
#include <type_traits> // conditional_t



namespace aa {

	template<arithmetic T>
	inline std::conditional_t<std::floating_point<T>, std::uniform_real_distribution<T>, std::uniform_int_distribution<T>>
		make_uniform_distribution(const T a, const T b)
	{
		if constexpr (std::floating_point<T>) {
			return std::uniform_real_distribution<T>{a, b};
		} else {
			return std::uniform_int_distribution<T>{a, b};
		}
	}



	// [0, MODULUS)
	template<class T = void, std::uniform_random_bit_generator G>
		requires (convertible_from<T, generator_result_t<G>>)
	inline constexpr fist_not_void_t<T, generator_result_t<G>> int_distribution(G &g) {
		return static_cast<fist_not_void_t<T, generator_result_t<G>>>(g());
	}

	// Gali būti paduodamas ir modulus_type tipo kintamasis, tai ne klaida, klaida būtų jei reikšmė paduoto
	// kintamojo būtų didesnė už MODULUS. Jei būtų paduodamas kintamasis su didesne reikšme, generatorius
	// išviso nebegalėtų gražinti kai kurių reikšmių iš intervalo. Ta pati problema pasireiškia ir su double metodu.
	//
	// mag ir off ne generic tipo, nes taip tik apsisunkiname gyvenimą paduodami į funkcijas konstantas.
	// Naudotojas pats turi sąmoningai atlikti static_cast, jei jis turi netinkamą tipą.
	// [0, mag)
	template<class T = void, full_range_generator G>
		requires (convertible_from<T, generator_modulus_t<G>>)
	inline constexpr fist_not_void_t<T, generator_modulus_t<G>> int_distribution(G &g, const generator_modulus_t<G> mag) {
		return static_cast<fist_not_void_t<T, generator_modulus_t<G>>>(
			(mag * static_cast<generator_modulus_t<G>>(g())) >> std::numeric_limits<generator_result_t<G>>::digits
		);
	}

	// [off, mag + off)
	template<class T = void, full_range_generator G>
		requires (convertible_from<T, generator_modulus_t<G>>)
	inline constexpr fist_not_void_t<T, generator_modulus_t<G>> int_distribution(G &g,
		const generator_modulus_t<G> off, const generator_modulus_t<G> mag) {

		return static_cast<fist_not_void_t<T, generator_modulus_t<G>>>(
			int_distribution(g, mag) + off
		);
	}



	// [0, 1)
	template<std::floating_point T = double, std::uniform_random_bit_generator G>
		requires (std::numeric_limits<T>::digits > std::numeric_limits<generator_result_t<G>>::digits)
	inline constexpr T real_distribution(G &g) {
		return static_cast<T>(g()) * constant_v<static_cast<T>(1) / representable_values_v<generator_result_t<G>>>;
	}

	// [0, mag)
	template<std::floating_point T = double, std::uniform_random_bit_generator G>
		requires (std::numeric_limits<T>::digits > std::numeric_limits<generator_result_t<G>>::digits)
	inline constexpr T real_distribution(G &g, const T mag) {
		return real_distribution<T>(g) * mag;
	}

	// [off, mag + off)
	template<std::floating_point T = double, std::uniform_random_bit_generator G>
		requires (std::numeric_limits<T>::digits > std::numeric_limits<generator_result_t<G>>::digits)
	inline constexpr T real_distribution(G &g, const T off, const T mag) {
		return real_distribution<T>(g, mag) + off;
	}



	// [0, mag)
	template<class U, floating_point_and_convertible_to<U> T = double, std::uniform_random_bit_generator G>
		requires (std::numeric_limits<T>::digits > std::numeric_limits<generator_result_t<G>>::digits)
	inline constexpr U real_to_int_distribution(G &g, const T mag) {
		return static_cast<U>(real_distribution<T>(g, mag));
	}

	// [off, mag + off)
	template<class U, floating_point_and_convertible_to<U> T = double, std::uniform_random_bit_generator G>
		requires (std::numeric_limits<T>::digits > std::numeric_limits<generator_result_t<G>>::digits)
	inline constexpr U real_to_int_distribution(G &g, const T off, const T mag) {
		return static_cast<U>(real_distribution<T>(g, off, mag));
	}

}
