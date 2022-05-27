#pragma once

#include "general.hpp"
#include <random> // uniform_random_bit_generator
#include <limits> // numeric_limits
#include <type_traits> // invoke_result_t, type_identity, remove_reference_t
#include <iterator> // iter_difference_t
#include <concepts> // convertible_to



namespace aa {

	template<std::uniform_random_bit_generator G>
	struct generator_result : std::type_identity<std::invoke_result_t<G &>> {};

	template<class G>
	using generator_result_t = generator_result<G>::type;



	template<class G>
	concept full_range_generator = std::uniform_random_bit_generator<G>
		&& G::min() == std::numeric_limits<generator_result_t<G>>::min()
		&& G::max() == std::numeric_limits<generator_result_t<G>>::max();



	template<full_range_generator G>
	struct generator_modulus : std::type_identity<next_unsigned_t<generator_result_t<G>>> {};

	template<class G>
	using generator_modulus_t = generator_modulus<G>::type;



	template<class G, class I>
	concept differences_generator_for = full_range_generator<std::remove_reference_t<G>>
		&& std::convertible_to<std::iter_difference_t<I>, generator_modulus_t<std::remove_reference_t<G>>>
		&& std::convertible_to<generator_modulus_t<std::remove_reference_t<G>>, std::iter_difference_t<I>>;

}
