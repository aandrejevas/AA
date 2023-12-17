#pragma once

#include "general.hpp"
#include <random> // uniform_random_bit_generator



namespace aa {

	template<class G>
	concept bits_generator = std::uniform_random_bit_generator<std::remove_reference_t<G>>;

	template<class G>
	concept full_range_generator = bits_generator<G>
		&& is_numeric_min(std::remove_reference_t<G>::min()) && is_numeric_max(std::remove_reference_t<G>::max());



	// Viduje invoke_result_t nėra naudojamas remove_reference_t<G>, nes tiesiog pasinaudojama reference collapse taisyklėmis.
	template<bits_generator G>
	using generator_result_t = std::invoke_result_t<G &>;

	template<full_range_generator G>
	using distribution_result_t = next_unsigned_t<generator_result_t<G>>;

	template<bits_generator G>
	constexpr generator_result_t<G> generator_modulus_v = (std::remove_reference_t<G>::max() - std::remove_reference_t<G>::min() + 1);

	template<full_range_generator G>
	constexpr distribution_result_t<G> generator_modulus_v<G> = (cast<distribution_result_t<G>>(std::remove_reference_t<G>::max()) + 1);

	template<bits_generator G>
	using generator_modulus_t = const_t<generator_modulus_v<G>>;



	template<class G, class T>
	concept distribution_result_ctible_to_and_from = constructible_to_and_from<T, distribution_result_t<G>>;

	template<class G, class T>
	concept generator_result_constructible_to = std::constructible_from<T, generator_result_t<G>>;

	template<class G, class T>
	concept distribution_result_constructible_to = std::constructible_from<T, distribution_result_t<G>>;

	template<class G, class T>
	concept generator_modulus_representable_by = (numeric_digits_v<T> >= std::bit_width(generator_modulus_v<G>));

}
