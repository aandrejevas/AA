#pragma once

#include "../metaprogramming/general.hpp"
#include <iterator> // input_or_output_iterator, sentinel_for, next, prev, bidirectional_iterator, sized_sentinel_for, iter_difference_t
#include <ranges> // view_interface
#include <type_traits> // make_unsigned_t



namespace aa {

	template<std::input_or_output_iterator I, std::sentinel_for<I> S = I>
	struct unsafe_subrange : pair<I, S>, std::ranges::view_interface<unsafe_subrange<I, S>> {
		// Member types
		using difference_type = std::iter_difference_t<I>;
		using size_type = std::make_unsigned_t<difference_type>;

	protected:
		using b = pair<I, S>;
	public:



		// Observers
		AA_CONSTEXPR I &begin() { return b::template get<0>(); }
		AA_CONSTEXPR S &rbegin() { return b::template get<1>(); }
		AA_CONSTEXPR const I &begin() const { return b::template get<0>(); }
		AA_CONSTEXPR const S &rbegin() const { return b::template get<1>(); }
		AA_CONSTEXPR S end() const requires (std::input_or_output_iterator<S>) { return std::ranges::next(rbegin()); }
		AA_CONSTEXPR I rend() const requires (std::bidirectional_iterator<I>) { return std::ranges::prev(begin()); }

		AA_CONSTEXPR difference_type ssize() const requires (std::sized_sentinel_for<S, I>) { return (rbegin() - begin()) + 1; }
		AA_CONSTEXPR size_type size() const requires (std::sized_sentinel_for<S, I>) { return unsign(ssize()); }

		static AA_CONSTEVAL bool empty() { return false; }

		AA_CONSTEXPR std::iter_reference_t<S> back() const requires (std::input_or_output_iterator<S>) { return *rbegin(); }



		// Special member functions
		AA_CONSTEXPR unsafe_subrange() = default;

		template<class T1 = I, class T2 = S>
		AA_CONSTEXPR unsafe_subrange(T1 &&t1, T2 &&t2) : b{std::forward<T1>(t1), std::forward<T2>(t2)} {}
	};

	template<std::input_or_output_iterator I, std::sentinel_for<I> S>
	unsafe_subrange(I &&, S &&)->unsafe_subrange<I, S>;

}
