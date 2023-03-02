#pragma once

#include "general.hpp"
#include <random> // uniform_random_bit_generator
#include <type_traits> // invoke_result, remove_reference_t
#include <iterator> // iter_difference_t
#include <concepts> // convertible_to
#include <limits> // numeric_limits
#include <bit> // bit_width



namespace aa {

	template<class G>
	concept bits_generator = std::uniform_random_bit_generator<std::remove_reference_t<G>>;

	template<class G>
	concept full_range_generator = bits_generator<G>
		&& is_numeric_min(std::remove_reference_t<G>::min()) && is_numeric_max(std::remove_reference_t<G>::max());



	// Viduje invoke_result_t nėra naudojamas remove_reference_t<G>, nes tiesiog pasinaudojama reference collapse taisyklėmis.
	template<bits_generator G>
	struct generator_result : std::invoke_result<G &> {};

	template<class G>
	using generator_result_t = typename generator_result<G>::type;

	template<full_range_generator G>
	struct distribution_result : next_unsigned<generator_result_t<G>> {};

	template<class G>
	using distribution_result_t = typename distribution_result<G>::type;

	template<bits_generator G>
	struct generator_modulus : constant<std::remove_reference_t<G>::max() - std::remove_reference_t<G>::min() + 1> {};

	template<full_range_generator G>
	struct generator_modulus<G> : constant<cast<distribution_result_t<G>>(std::remove_reference_t<G>::max()) + 1> {};

	template<class G>
	using generator_modulus_t = typename generator_modulus<G>::value_type;

	template<class G>
	AA_CONSTEXPR const generator_modulus_t<G> generator_modulus_v = generator_modulus<G>::value;



	template<class G, class I>
	concept differences_generator_for = (full_range_generator<G>
		&& std::convertible_to<std::iter_difference_t<I>, distribution_result_t<G>>
		&& std::convertible_to<distribution_result_t<G>, std::iter_difference_t<I>>);

	template<class G, class T>
	concept generator_result_convertible_to = std::convertible_to<generator_result_t<G>, T>;

	template<class G, class T>
	concept distribution_result_convertible_to = std::convertible_to<distribution_result_t<G>, T>;

	template<class G, class T>
	concept generator_modulus_representable_by = (std::numeric_limits<T>::digits >= std::bit_width(generator_modulus_v<G>));

}
