#pragma once

#include "general.hpp"
#include <iterator> // input_iterator, output_iterator, contiguous_iterator, iter_difference_t, permutable, next, prev, distance, iter_reference_t, make_reverse_iterator
#include <ranges> // contiguous_range, random_access_range, bidirectional_range, forward_range, output_range, sized_range, common_range, iterator_t, sentinel_t, range_value_t, range_reference_t, range_difference_t, begin, end, rbegin, rend, size, data, range, input_range, subrange
#include <memory> // to_address



namespace aa {

	template<class I>
	using iter_size_t = std::make_unsigned_t<std::iter_difference_t<I>>;

	template<std::contiguous_iterator I>
	using iter_pointer_t = std::add_pointer_t<std::iter_reference_t<I>>;

	template<std::ranges::contiguous_range R>
	using range_pointer_t = std::add_pointer_t<std::ranges::range_reference_t<R>>;

	template<class T>
	concept reverse_range = requires(T& t) {
		std::ranges::rbegin(t);
		std::ranges::rend(t);
	};

	template<class T>
	using reverse_iterator_t = decltype(std::ranges::rbegin(std::declval<T &>()));

	template<reverse_range R>
	using reverse_sentinel_t = decltype(std::ranges::rend(std::declval<R &>()));

	template<class R>
	concept common_bidirectional_range = std::ranges::common_range<R> && std::ranges::bidirectional_range<R>;

	template<class R>
	concept unusual_range = common_bidirectional_range<R> && std::same_as<reverse_iterator_t<R>, std::ranges::iterator_t<R>>;

	// Nėra atitinkamos funkcijos rend iteratoriui, nes jis nėra svarbus.
	template<std::ranges::range R>
	constexpr std::ranges::iterator_t<R> get_rbegin(R &&r) {
		if constexpr (unusual_range<R>) {
			// std::reverse_iterator nėra constructible į iterator.
			return std::ranges::rbegin(r);
		} else if constexpr (common_bidirectional_range<R>) {
			return std::ranges::prev(std::ranges::end(r));
		} else {
			return std::ranges::next(std::ranges::begin(r), std::ranges::distance(r) - 1);
		}
	}

	template<std::ranges::contiguous_range R>
	constexpr range_pointer_t<R> get_rdata(R &&r) {
		if constexpr (unusual_range<R>) {
			return std::to_address(std::ranges::rbegin(r));
		} else if constexpr (std::ranges::common_range<R>) {
			return std::to_address(std::ranges::end(r)) - 1;
		} else {
			return std::ranges::data(r) + (std::ranges::distance(r) - 1);
		}
	}

	// Negalime naudoti views::reverse, nes funkcija reikalauja, kad range tipas būtų bidirectional.
	template<std::ranges::range R>
	constexpr auto get_reversed(R &&r) {
		if constexpr (unusual_range<R>) {
			return std::ranges::subrange{std::make_reverse_iterator(std::ranges::end(r)), std::make_reverse_iterator(std::ranges::begin(r))};
		} else {
			return std::ranges::subrange{std::ranges::rbegin(r), std::ranges::rend(r)};
		}
	}



	template<class B, class I>
	concept bool_auxiliary_range_for = (std::ranges::random_access_range<B> && std::ranges::output_range<B, bool>
		&& bool_testable<std::ranges::range_reference_t<B>> && constructible_to_and_from<I, std::ranges::range_difference_t<B>>);

	template<class R>
	concept sized_random_access_range = std::ranges::random_access_range<R> && std::ranges::sized_range<R>;

	template<class T>
	concept sized_contiguous_range = std::ranges::contiguous_range<T> && std::ranges::sized_range<T>;

	template<class R>
	concept permutable_range = std::ranges::common_range<R> && std::ranges::forward_range<R> && std::permutable<std::ranges::iterator_t<R>>;

	template<class R>
	concept permutable_random_access_range = std::ranges::random_access_range<R> && permutable_range<R>;

	template<class I>
	concept char_output_iterator = std::output_iterator<I, const char &>;

	template<class I>
	concept int_input_iterator = std::input_iterator<I> && std::assignable_from<int &, std::iter_reference_t<I>>;



	template<class T>
	concept formattable_range = !std::constructible_from<std::string_view, T> && (std::ranges::input_range<T> || tuple_like<T>);

	template<class T>
	concept range_using_traits_type = sized_contiguous_range<T> &&
		std::same_as<char_type_in_use_t<traits_type_in_use_t<T>>, std::ranges::range_value_t<T>>;

	template<sized_contiguous_range T>
	constexpr auto range_char_traits_v = [] static {
		if constexpr (range_using_traits_type<T>)
			return default_value_v<std::type_identity<traits_type_in_use_t<T>>>;
		else
			// Darome prielaidą, kad char_traits yra apibrėžtas su visais tipais.
			return default_value_v<std::type_identity<std::char_traits<std::ranges::range_value_t<T>>>>;
	};

	template<sized_contiguous_range T>
	using range_char_traits_t = type_in_use_t<const_t<range_char_traits_v<T>()>>;

	template<class T, class U>
	concept same_range_char_traits_as = std::same_as<range_char_traits_t<T>, U>;

	template<class T, size_t N = numeric_max>
	concept fixed_string_like = range_using_traits_type<T> && array_like<T, N>;



	// Neturime klasės iš kurios tiesiog galėtume paveldėti is_transparent, nes gcc taip elgiasi. Taip pat, nes
	// tai nėra neįprasta, konteineriai ir iteratoriai turi tokių pačių aliases ir iš sandarto buvo pašalintas tipas
	// std::iterator, kuris buvo naudojamas tokiu pačiu principu, tai reiškia nerekomenduojama tokia realizacija.
	struct string_equal_to {
		template<class L, same_range_char_traits_as<range_char_traits_t<L>> R>
		static constexpr bool operator()(const L &l, const R &r) {
			// size_t tipas, nes compare tikisi tokio tipo parametro.
			const size_t count = std::ranges::size(l);
			return count == std::ranges::size(r) &&
				!range_char_traits_t<L>::compare(std::ranges::data(l), std::ranges::data(r), count);
		}

		using is_transparent = void;
	};

}
