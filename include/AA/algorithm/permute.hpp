#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/generator.hpp"
#include "../metaprogramming/range.hpp"
#include "../container/unsafe_subrange.hpp"
#include "generate.hpp"
#include <iterator> // iter_swap
#include <ranges> // forward_range, iterator_t, range_value_t, range_difference_t, begin, end, rbegin



namespace aa {

	// https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle#The_modern_algorithm
	template<permutable_random_access_range R, distribution_result_ctible_to_and_from<std::ranges::range_difference_t<R>> G>
	constexpr void shuffle(R &&r, G &&g) {
		if constexpr (!unusual_range<R>) {
			partial_shuffle(r, std::ranges::end(r) - 2, g);
		} else {
			partial_shuffle(r, std::ranges::rbegin(r) - 1, g);
		}
	}

	template<permutable_random_access_range R, distribution_result_ctible_to_and_from<std::ranges::range_difference_t<R>> G>
	constexpr void partial_shuffle(R &&r, const std::ranges::iterator_t<R> m, G &&g) {
		std::ranges::iterator_t<R> b = std::ranges::begin(r);
		const std::ranges::iterator_t<R> e = std::ranges::end(r);
		do {
			std::ranges::iter_swap(b, b + int_distribution(g, unsign_cast<distribution_result_t<G>>(e - b)));
			if (b != m) ++b; else return;
		} while (true);
	}



	template<std::ranges::forward_range R, bool_auxiliary_range_for<std::ranges::range_value_t<R>> B>
	constexpr void counting_fill(R &&r, B &&a) {
		std::ranges::iterator_t<R> b = std::ranges::begin(r);
		const std::ranges::iterator_t<R> m = get_rbegin(r);
		std::ranges::iterator_t<B> s = std::ranges::begin(a);
		do {
			if (*s) {
				*s = false;
				*b = unsign_cast<std::ranges::range_value_t<R>>(s - std::ranges::begin(a));
				if (b == m) return; else ++b;
			}
		} while ((++s, true));
	}

	// https://en.wikipedia.org/wiki/Counting_sort
	// B yra range, nors galėtų būti iterator, nes range parametrus yra teisingiau turėti kur įmanoma.
	// std::ranges::begin naudoti yra tas pats kaip naudoti iterator kintamąjį.
	template<std::ranges::forward_range R, bool_auxiliary_range_for<std::ranges::range_value_t<R>> B>
	constexpr void counting_sort(R &&r, B &&a) {
		const std::ranges::iterator_t<R> m = get_rbegin(r);
		{
			std::ranges::iterator_t<R> b = std::ranges::begin(r);
			do {
				std::ranges::begin(a)[*b] = true;
				if (b != m) ++b; else break;
			} while (true);
		}
		counting_fill(unsafe_subrange{std::ranges::begin(r), m}, a);
	}

	template<permutable_random_access_range R, distribution_result_ctible_to_and_from<std::ranges::range_difference_t<R>> G, bool_auxiliary_range_for<std::ranges::range_value_t<R>> B>
	constexpr void partial_shuffle_counting_sort(R &&r, const std::ranges::iterator_t<R> m, G &&g, B &&a) {
		{
			std::ranges::iterator_t<R> b = std::ranges::begin(r);
			const std::ranges::iterator_t<R> e = std::ranges::end(r);
			do {
				std::ranges::iter_swap(b, b + int_distribution(g, unsign_cast<distribution_result_t<G>>(e - b)));
				std::ranges::begin(a)[*b] = true;
				if (b != m) ++b; else break;
			} while (true);
		}
		counting_fill(unsafe_subrange{std::ranges::begin(r), m}, a);
	}

}
