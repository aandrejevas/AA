#pragma once

#include "../metaprogramming/general.hpp"
#include <cstddef> // size_t
#include <iterator> // input_or_output_iterator, sentinel_for, next, prev, bidirectional_iterator, random_access_iterator, contiguous_iterator, sized_sentinel_for, iter_difference_t, iter_reference_t
#include <ranges> // view_interface
#include <type_traits> // make_unsigned_t, add_pointer_t, type_identity
#include <utility> // forward, tuple_size, tuple_element
#include <memory> // to_address



namespace aa {

	template<std::input_or_output_iterator I, std::sentinel_for<I> S = I>
	struct unsafe_subrange : pair<I, S>, std::ranges::view_interface<unsafe_subrange<I, S>> {
		// Member types
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

		AA_CONSTEXPR std::iter_difference_t<I> ssize() const requires (std::sized_sentinel_for<S, I>) { return (rbegin() - begin()) + 1; }
		AA_CONSTEXPR std::make_unsigned_t<std::iter_difference_t<I>> size() const requires (std::sized_sentinel_for<S, I>) { return unsign(ssize()); }

		static AA_CONSTEVAL bool empty() { return false; }

		AA_CONSTEXPR std::iter_reference_t<S> back() const requires (std::input_or_output_iterator<S>) { return *rbegin(); }
		AA_CONSTEXPR std::add_pointer_t<std::iter_reference_t<S>> rdata() const requires (std::contiguous_iterator<S>) { return std::to_address(rbegin()); }

		AA_CONSTEXPR I begin(const std::iter_difference_t<I> n) const requires (std::random_access_iterator<I>) { return begin() + n; }
		AA_CONSTEXPR S rbegin(const std::iter_difference_t<S> n) const requires (std::random_access_iterator<S>) { return rbegin() - n; }

		AA_CONSTEXPR std::iter_reference_t<I> elem(const std::iter_difference_t<I> n) const requires (std::random_access_iterator<I>) { return *begin(n); }
		AA_CONSTEXPR std::iter_reference_t<S> relem(const std::iter_difference_t<S> n) const requires (std::random_access_iterator<S>) { return *rbegin(n); }



		// Special member functions
		// Reikia konstruktorių, nes kitaip metami warnings -Wmissing-field-initializers.
		AA_CONSTEXPR unsafe_subrange() = default;

		template<class T1 = I, class T2 = S>
		AA_CONSTEXPR unsafe_subrange(T1 &&t1, T2 &&t2) : b{std::forward<T1>(t1), std::forward<T2>(t2)} {}
	};

	template<class I, class S>
	unsafe_subrange(I &&, S &&)->unsafe_subrange<I, S>;

}



namespace std {

	template<class I, class S>
	struct tuple_size<aa::unsafe_subrange<I, S>> : aa::constant_identity<size_t, 2> {};

	template<class I, class S>
	struct tuple_element<0, aa::unsafe_subrange<I, S>> : std::type_identity<I> {};

	template<class I, class S>
	struct tuple_element<1, aa::unsafe_subrange<I, S>> : std::type_identity<S> {};

}
