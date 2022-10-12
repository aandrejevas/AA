#pragma once

#include "general.hpp"
#include <random> // uniform_random_bit_generator
#include <type_traits> // invoke_result, remove_reference_t
#include <iterator> // iter_difference_t
#include <concepts> // convertible_to



namespace aa {

	template<class G>
	concept random_bit_generator = std::uniform_random_bit_generator<std::remove_reference_t<G>>;



	// Viduje invoke_result_t nėra naudojamas remove_reference_t<G>, nes tiesiog pasinaudojama reference collapse taisyklėmis.
	template<random_bit_generator G>
	struct generator_result : std::invoke_result<G &> {};

	template<class G>
	using generator_result_t = typename generator_result<G>::type;

	template<random_bit_generator G>
	struct generator_modulus : apply_if<next_unsigned_t,
		is_numeric_max(std::remove_reference_t<G>::max()), generator_result_t<G>> {};

	template<class G>
	using generator_modulus_t = typename generator_modulus<G>::type;



	template<class G>
	concept full_range_generator = random_bit_generator<G>
		&& is_numeric_min(std::remove_reference_t<G>::min()) && is_numeric_max(std::remove_reference_t<G>::max());

	template<class G, class I>
	concept differences_generator_for = full_range_generator<G>
		&& std::convertible_to<std::iter_difference_t<I>, generator_modulus_t<G>>
		&& std::convertible_to<generator_modulus_t<G>, std::iter_difference_t<I>>;

}
