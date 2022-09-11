#pragma once

#include "general.hpp"
#include <type_traits> // type_identity
#include <concepts> // convertible_to, same_as
#include <iterator> // contiguous_iterator, random_access_iterator, iter_value_t, iter_difference_t, permutable, output_iterator, next, prev, distance
#include <ranges> // contiguous_range, random_access_range, bidirectional_range, sized_range, iterator_t, sentinel_t, range_value_t, begin, end, rbegin, size, data, range
#include <string> // char_traits



namespace aa {

	// Darome daug prielaidų čia, nes atrodo, kad c++ standartas jas daro taip pat.
	//
	// Nėra atitinkamos funkcijos rend iteratoriui, nes jis nėra svarbus.
	template<std::ranges::range R>
	AA_CONSTEXPR std::ranges::sentinel_t<R> get_rbegin(R &&r) {
		if constexpr (requires { { std::ranges::rbegin(r) } -> std::same_as<std::ranges::sentinel_t<R>>; }) {
			return std::ranges::rbegin(r);
		} else if constexpr (std::ranges::bidirectional_range<R>) {
			return std::ranges::prev(std::ranges::end(r));
		} else {
			return std::ranges::next(std::ranges::begin(r), std::ranges::distance(r) - 1);
		}
	}



	// Turėtų ne convertible_to<bool> būti naudojamas, o boolean-testable, bet toks concept kol kas yra tik exposition only.
	// https://en.cppreference.com/w/cpp/concepts/boolean-testable.
	template<class B, class I>
	concept key_bool_iterator_for = std::random_access_iterator<B> && std::convertible_to<std::iter_value_t<B>, bool>
		&& std::output_iterator<B, bool> && std::convertible_to<std::iter_value_t<I>, std::iter_difference_t<B>>
		&& std::convertible_to<std::iter_difference_t<B>, std::iter_value_t<I>>;

	template<class O, class I>
	concept trivial_output_iterator_for = std::contiguous_iterator<O> && std::contiguous_iterator<I>
		&& trivially_copyable<std::iter_value_t<O>> && trivially_copyable<std::iter_value_t<I>>
		&& sizeof(std::iter_value_t<O>) == sizeof(std::iter_value_t<I>);



	template<class R>
	concept sized_random_access_range = std::ranges::random_access_range<R> && std::ranges::sized_range<R>;

	template<class T>
	concept sized_contiguous_range = std::ranges::contiguous_range<T> && std::ranges::sized_range<T>;

	template<class R>
	concept permutable_random_access_range = std::ranges::random_access_range<R> && std::permutable<std::ranges::iterator_t<R>>;

	template<class O, class I>
	concept trivial_output_range_for = sized_contiguous_range<O> && sized_contiguous_range<I>
		&& trivial_output_iterator_for<std::ranges::iterator_t<O>, std::ranges::iterator_t<I>>;



	template<class T>
	concept range_using_char_traits = sized_contiguous_range<T> && using_char_traits<T>
		&& std::same_as<std::ranges::range_value_t<T>, typename T::traits_type::char_type>;

	template<class T>
	concept char_range = sized_contiguous_range<T>
		&& ((requires { typename std::char_traits<std::ranges::range_value_t<T>>; }) || range_using_char_traits<T>);

	namespace detail {
		template<class T, bool = range_using_char_traits<T>>
		struct range_char_traits_selector : std::type_identity<std::char_traits<std::ranges::range_value_t<T>>> {};

		template<class T>
		struct range_char_traits_selector<T, true> : std::type_identity<typename T::traits_type> {};
	}

	template<char_range T>
	struct range_char_traits : detail::range_char_traits_selector<T> {};

	template<class T>
	using range_char_traits_t = range_char_traits<T>::type;

	template<class T, class U>
	concept same_range_char_traits_as = char_range<T> && std::same_as<range_char_traits_t<T>, U>;



	struct char_equal_to {
		template<char_range L, same_range_char_traits_as<range_char_traits_t<L>> R>
		AA_CONSTEXPR bool operator()(const L &l, const R &r) const {
			return std::ranges::size(l) == std::ranges::size(r) &&
				!range_char_traits_t<L>::compare(std::ranges::data(l), std::ranges::data(r), std::ranges::size(l));
		}

		using is_transparent = void;
	};

}
