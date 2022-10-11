#pragma once

#include "../metaprogramming/general.hpp"
#include <iterator> // input_or_output_iterator, sentinel_for, next, prev, bidirectional_iterator, sized_sentinel_for, iter_difference_t
#include <ranges> // view_interface
#include <type_traits> // make_unsigned_t



namespace aa {

	template<std::input_or_output_iterator I, std::sentinel_for<I> S = I>
	struct unsafe_subrange : pair<I, S>, std::ranges::view_interface<unsafe_subrange> {
		// Member types
		using difference_type = std::iter_difference_t<I>;
		using size_type = std::make_unsigned_t<difference_type>;



		// Observers
		AA_CONSTEXPR I &begin() { return unit_type<0>::value; }
		AA_CONSTEXPR S &rbegin() { return unit_type<1>::value; }
		AA_CONSTEXPR const I &begin() const { return unit_type<0>::value; }
		AA_CONSTEXPR const S &rbegin() const { return unit_type<1>::value; }
		AA_CONSTEXPR S end() const requires (std::input_or_output_iterator<S>) { return std::ranges::next(rbegin()); }
		AA_CONSTEXPR I rend() const requires (std::bidirectional_iterator<I>) { return std::ranges::prev(begin()); }

		AA_CONSTEXPR difference_type ssize() const requires (std::sized_sentinel_for<S, I>) { return (rbegin() - begin()) + 1; }
		AA_CONSTEXPR size_type size() const requires (std::sized_sentinel_for<S, I>) { return unsign(ssize()); }

		static AA_CONSTEVAL bool empty() { return false; }
	};

	template<std::input_or_output_iterator I, std::sentinel_for<I> S>
	unsafe_subrange(I &&, S &&)->unsafe_subrange<I, S>;

}
