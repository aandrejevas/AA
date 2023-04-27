#pragma once

#include "general.hpp"
#include <iterator> // contiguous_iterator, random_access_iterator, iter_value_t, iter_difference_t, permutable, output_iterator, next, prev, distance, iter_reference_t
#include <ranges> // contiguous_range, random_access_range, bidirectional_range, sized_range, iterator_t, sentinel_t, range_value_t, range_reference_t, begin, end, rbegin, size, data, range
#include <string> // char_traits
#include <memory> // to_address



namespace aa {

	template<class I>
	using iter_size_t = std::make_unsigned_t<std::iter_difference_t<I>>;

	template<std::contiguous_iterator I>
	using iter_pointer_t = std::add_pointer_t<std::iter_reference_t<I>>;

	template<std::ranges::contiguous_range R>
	using range_pointer_t = std::add_pointer_t<std::ranges::range_reference_t<R>>;

	template<class R>
	concept unusual_range = std::ranges::range<R>
		&& requires(R & r) { { std::ranges::rbegin(r) } -> std::same_as<std::ranges::sentinel_t<R>>; };

	// Darome daug prielaidų čia, nes atrodo, kad C++ standartas jas daro taip pat.
	//
	// Nėra atitinkamos funkcijos rend iteratoriui, nes jis nėra svarbus.
	template<std::ranges::range R>
	AA_CONSTEXPR std::ranges::sentinel_t<R> get_rbegin(R &&r) {
		if constexpr (unusual_range<R>) {
			return std::ranges::rbegin(r);
		} else if constexpr (std::ranges::bidirectional_range<R>) {
			return std::ranges::prev(std::ranges::end(r));
		} else {
			return std::ranges::next(std::ranges::begin(r), std::ranges::distance(r) - 1);
		}
	}

	template<std::ranges::contiguous_range R>
	AA_CONSTEXPR range_pointer_t<R> get_data_end(R &&r) {
		if constexpr (std::same_as<range_pointer_t<R>, iter_pointer_t<std::ranges::sentinel_t<R>>>) {
			return std::to_address(std::ranges::end(r));
		} else {
			return std::ranges::data(r) + std::ranges::distance(r);
		}
	}



	// Turėtų ne convertible_to<bool> būti naudojamas, o boolean-testable, bet toks concept kol kas yra tik exposition only.
	// https://en.cppreference.com/w/cpp/concepts/boolean-testable.
	template<class B, class I>
	concept key_bool_iterator_for = (std::random_access_iterator<B> && std::convertible_to<std::iter_value_t<B>, bool>
		&& std::output_iterator<B, bool> && std::convertible_to<std::iter_value_t<I>, std::iter_difference_t<B>>
		&& std::convertible_to<std::iter_difference_t<B>, std::iter_value_t<I>>);

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
	concept char_range = sized_contiguous_range<T> && std::same_as<std::ranges::range_value_t<T>, char>;

	template<class T>
	concept range_using_traits_type = sized_contiguous_range<T> && uses_traits_type<T>
		&& char_traits_for<traits_type_in_use_t<T>, std::ranges::range_value_t<T>>;

	// Darome prielaidą, kad char_traits yra apibrėžtas su visais tipais.
	template<sized_contiguous_range T>
	struct range_char_traits : std::type_identity<std::char_traits<std::ranges::range_value_t<T>>> {};

	template<range_using_traits_type T>
	struct range_char_traits<T> : std::type_identity<traits_type_in_use_t<T>> {};

	template<sized_contiguous_range T>
	using range_char_traits_t = typename range_char_traits<T>::type;

	template<class T, class U>
	concept same_range_char_traits_as = sized_contiguous_range<T> && std::same_as<range_char_traits_t<T>, U>;

	template<class T, size_t N = numeric_max>
	concept fixed_string_like = range_using_traits_type<T> && array_like<T, N>;



	// Neturime klasės iš kurios tiesiog galėtume paveldėti is_transparent, nes gcc taip elgiasi. Taip pat, nes
	// tai nėra neįprasta, konteineriai ir iteratoriai turi tokių pačių aliases ir iš sandarto buvo pašalintas tipas
	// std::iterator, kuris buvo naudojamas tokiu pačiu principu, tai reiškia nerekomenduojama tokia realizacija.
	struct string_equal_to {
		template<sized_contiguous_range L, same_range_char_traits_as<range_char_traits_t<L>> R>
		AA_CONSTEXPR bool operator()(const L &l, const R &r) const {
			const size_t count = std::ranges::size(l);
			return count == std::ranges::size(r) &&
				!range_char_traits_t<L>::compare(std::ranges::data(l), std::ranges::data(r), count);
		}

		using is_transparent = void;
	};

}
