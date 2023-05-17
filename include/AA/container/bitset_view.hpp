#pragma once

#include "../metaprogramming/general.hpp"
#include <ranges> // view_interface
#include <variant> // monostate



namespace aa {

	template<std::unsigned_integral T, unsigned_integral_or_void U = void>
	struct bitset_view : std::ranges::view_interface<bitset_view<T, U>> {
		// Member constants
		static AA_CONSTEXPR const bool is_offset = !same_as_void<U>;

		// Member types
		using bitset_type = T;
		using value_type = std::conditional_t<is_offset, U, bitset_type>;



		const bitset_type bitset;
		[[no_unique_address]] const std::conditional_t<is_offset, value_type, std::monostate> offset;
	};

}
