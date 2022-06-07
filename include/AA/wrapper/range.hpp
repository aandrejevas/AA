#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/range.hpp"
#include <ranges> // size, data



namespace aa {

	struct char_equal_to {
		template<char_range L, same_range_char_traits_as<L> R>
		AA_CONSTEXPR bool operator()(const L &l, const R &r) const {
			return std::ranges::size(l) == std::ranges::size(r) &&
				!range_char_traits_t<L>::compare(std::ranges::data(l), std::ranges::data(r), std::ranges::size(l));
		}

		using is_transparent = void;
	};

}
