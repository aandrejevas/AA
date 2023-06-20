#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/generator.hpp"
#include "../metaprogramming/range.hpp"
#include "generate.hpp"
#include <iterator> // iter_swap, sentinel_for
#include <ranges> // iterator_t, sentinel_t, range_value_t, begin, end, rbegin



namespace aa {

	// https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle#The_modern_algorithm
	template<permutable_random_access_range R, differences_generator_for<std::ranges::iterator_t<R>> G>
	AA_CONSTEXPR void shuffle(R &&r, G &&g) {
		if constexpr (!unusual_range<R>) {
			partial_shuffle(r, std::ranges::end(r) - 2, g);
		} else {
			partial_shuffle(r, std::ranges::rbegin(r) - 1, g);
		}
	}

	template<permutable_random_access_range R, std::sentinel_for<std::ranges::iterator_t<R>> S, differences_generator_for<std::ranges::iterator_t<R>> G>
	AA_CONSTEXPR void partial_shuffle(R &&r, const S &m, G &&g) {
		std::ranges::iterator_t<R> b = std::ranges::begin(r);
		const std::ranges::sentinel_t<R> e = std::ranges::end(r);
		do {
			std::ranges::iter_swap(b, b + int_distribution(g, unsign_cast<distribution_result_t<G>>(e - b)));
			if (b != m) ++b; else return;
		} while (true);
	}



	// https://en.wikipedia.org/wiki/Counting_sort
	template<permutable_random_access_range R, key_bool_iterator_for<std::ranges::iterator_t<R>> B>
	AA_CONSTEXPR void counting_sort(R &&r, const B &selected) {
		std::ranges::iterator_t<R> b = std::ranges::begin(r);
		const std::ranges::sentinel_t<R> e_S1 = get_rbegin(r);

		{std::ranges::iterator_t<R> b2 = b; do {
			selected[*b2] = true;
			if (b2 != e_S1) ++b2; else break;
		} while (true);}

		{B s = selected; do {
			if (*s) {
				*s = false;
				*b = unsign_cast<std::ranges::range_value_t<R>>(s - selected);
				if (b == e_S1) return; else ++b;
			}
			++s;
		} while (true);}
	}

	template<permutable_random_access_range R, std::sentinel_for<std::ranges::iterator_t<R>> S, differences_generator_for<std::ranges::iterator_t<R>> G, key_bool_iterator_for<std::ranges::iterator_t<R>> B>
	AA_CONSTEXPR void partial_shuffle_counting_sort(R &&r, const S &m, G &&g, const B &selected) {
		std::ranges::iterator_t<R> b = std::ranges::begin(r);
		const std::ranges::sentinel_t<R> e = std::ranges::end(r);

		{std::ranges::iterator_t<R> b2 = b; do {
			std::ranges::iter_swap(b2, b2 + int_distribution(g, unsign_cast<distribution_result_t<G>>(e - b2)));
			selected[*b2] = true;
			if (b2 != m) ++b2; else break;
		} while (true);}

		{B s = selected; do {
			if (*s) {
				*s = false;
				*b = unsign_cast<std::ranges::range_value_t<R>>(s - selected);
				if (b == m) return; else ++b;
			}
			++s;
		} while (true);}
	}

}
