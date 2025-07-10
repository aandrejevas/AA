#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/generator.hpp"
#include "arithmetic.hpp"



namespace aa {

	// In the int distribution generate a random number.
	// Naudotojas turėtų naudoti g(), nebent tikrai nuo 0 kažkodėl jam reiktų reikšmės.
	// [0, MAX - MIN + 1)
	template<bits_generator G>
	constexpr generator_result_t<G> int_generate(G & g) {
		if constexpr (G::min() != numeric_min)	return g() - G::min();
		else									return g();
	}



	// Gali būti paduodamas ir modulus_type tipo kintamasis, tai ne klaida, klaida būtų jei reikšmė paduoto
	// kintamojo būtų didesnė už MODULUS. Jei būtų paduodamas kintamasis su didesne reikšme, generatorius
	// išviso nebegalėtų grąžinti kai kurių reikšmių iš intervalo. Ta pati problema pasireiškia ir su double metodu.
	//
	// mag ir off ne generic tipo, nes taip tik apsisunkiname gyvenimą paduodami į funkcijas konstantas.
	// Naudotojas pats turi sąmoningai atlikti cast, jei jis turi netinkamą tipą.
	// [0, mag)
	template<full_range_generator G>
	constexpr distribution_result_t<G> int_generate(G & g, const distribution_result_t<G> mag) {
		return (mag * cast<distribution_result_t<G>>(g())) >> numeric_digits_v<generator_result_t<G>>;
	}

	// [off, mag + off)
	template<full_range_generator G>
	constexpr distribution_result_t<G> int_generate(G & g, const distribution_result_t<G> off, const distribution_result_t<G> mag) {
		return int_generate(g, mag) + off;
	}

	// {[0, mag1), [0, mag2)}
	template<full_range_generator G>
	constexpr pair<distribution_result_t<G>> int_generate_two(G & g, const distribution_result_t<G> mag1, const distribution_result_t<G> mag2) {
		const distribution_result_t<G> x = int_generate(g, mag1 * mag2);
		return {(x / mag2), (x % mag2)};
	}



	// Čia nėra daromas atsitiktinio skaičiaus cast į kažkokį paduotą tipą, nes T negalime nustatyti iš paduoto argumento tipo.
	// [0, 1)
	template<std::floating_point T = double, generator_modulus_representable_by<T> G>
	constexpr T real_generate(G & g) {
		return quotient<generator_modulus_v<G>>(cast<T>(int_generate(g)));
	}

	// [0, mag)
	template<std::floating_point T, generator_modulus_representable_by<T> G>
	constexpr T real_generate(G & g, const T mag) {
		return real_generate<T>(g) * mag;
	}

	// [off, mag + off)
	template<std::floating_point T, generator_modulus_representable_by<T> G>
	constexpr T real_generate(G & g, const T off, const T mag) {
		return real_generate(g, mag) + off;
	}

}
