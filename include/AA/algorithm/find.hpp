#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/range.hpp"
#include <ranges> // iterator_t, borrowed_iterator_t, range_size_t, begin, end, size, rbegin, rend, bidirectional_range, input_range
#include <iterator> // indirect_strict_weak_order, indirect_binary_predicate, indirectly_unary_invocable, input_iterator, sentinel_for
#include <functional> // invoke, less, equal_to
#include <utility> // as_const



namespace aa {

	template<sized_random_access_range R, class T, std::indirect_strict_weak_order<const T *, std::ranges::iterator_t<R>> C = std::ranges::less>
	AA_CONSTEXPR std::ranges::borrowed_iterator_t<R> upper_bound(R &&r, const T &value, C &&comp = {}) {
		std::ranges::iterator_t<R> first = std::ranges::begin(r);
		std::ranges::range_size_t<R> len = std::ranges::size(r);
		while (len) {
			const std::ranges::range_size_t<R> half = len >> 1;
			const std::ranges::iterator_t<R> middle = first + half;
			if (std::invoke(comp, value, std::as_const(*middle))) {
				len = half;
			} else {
				first = middle + 1;
				len -= half + 1;
			}
		}
		return first;
	}

	template<sized_random_access_range R, class T, std::indirect_strict_weak_order<const T *, std::ranges::iterator_t<R>> C = std::ranges::less>
	AA_CONSTEXPR std::ranges::borrowed_iterator_t<R> lower_bound(R &&r, const T &value, C &&comp = {}) {
		std::ranges::iterator_t<R> first = std::ranges::begin(r);
		std::ranges::range_size_t<R> len = std::ranges::size(r);
		while (len) {
			const std::ranges::range_size_t<R> half = len >> 1;
			const std::ranges::iterator_t<R> middle = first + half;
			if (std::invoke(comp, std::as_const(*middle), value)) {
				first = middle + 1;
				len -= half + 1;
			} else {
				len = half;
			}
		}
		return first;
	}

	template<std::ranges::bidirectional_range R, class T>
		requires (std::indirect_binary_predicate<std::ranges::equal_to, std::ranges::iterator_t<R>, const T *>)
	AA_CONSTEXPR std::ranges::borrowed_iterator_t<R> find_last(R &&r, const T &value) {
		std::ranges::iterator_t<R> first = std::ranges::rbegin(r);
		const std::ranges::iterator_t<R> last = std::ranges::rend(r);
		while (first != last && (*first != value)) --first;
		return first;
	}



	template<std::input_iterator I, std::sentinel_for<I> S, std::indirectly_unary_invocable<I> F>
	AA_CONSTEXPR void unsafe_for_each(I first, const S &last, F &&f) {
		do {
			std::invoke(f, *first);
		} while (++first != last);
	}

	template<std::ranges::input_range R, std::indirectly_unary_invocable<std::ranges::iterator_t<R>> F>
	AA_CONSTEXPR void unsafe_for_each(R &&r, F &&f) {
		unsafe_for_each(std::ranges::begin(r), std::ranges::end(r), f);
	}

}
