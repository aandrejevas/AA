#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/generator.hpp"
#include "../container/unsafe_subrange.hpp"
#include "../container/invoke_iterator.hpp"
#include "generate.hpp"
#include "arithmetic.hpp"



namespace aa {

	enum struct shuffle_config : size_t {
		NONE = 0uz,
		ODD = int_exp2(0uz),
		MIXED = int_exp2(1uz) | ODD,
		SINGLE = int_exp2(2uz) | ODD,
		MIXED_SINGLE = MIXED | SINGLE,
		EMPTY = int_exp2(3uz),
		ODD_EMPTY = ODD | EMPTY,
		MIXED_EMPTY = MIXED | EMPTY,
		SINGLE_EMPTY = SINGLE | EMPTY,
		MIXED_SINGLE_EMPTY = MIXED_SINGLE | EMPTY,
	};

	// https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle#The_modern_algorithm
	template<shuffle_config C = shuffle_config::MIXED_SINGLE_EMPTY, permutable_range R,
		distribution_result_ctible_to_and_from<std::ranges::range_difference_t<R>> G>
	constexpr void shuffle(R &&r, G &&g) {
		using namespace operators; using enum shuffle_config;
		if constexpr (C & EMPTY) { if (std::ranges::size(r) < 2) return; }
		// Iš end negalime atimti 2, nes nežinome ar end yra random access.
		partial_shuffle<to_enum<const_t<C>>(C & ~EMPTY)>(r, get_rbegin(r) - 1, g);
	}

	template<shuffle_config C = shuffle_config::MIXED_SINGLE_EMPTY, permutable_range R, std::sized_sentinel_for<std::ranges::iterator_t<R>> S,
		distribution_result_ctible_to_and_from<std::ranges::range_difference_t<R>> G>
	constexpr void partial_shuffle(R &&r, const S &m, G &&g) {
		using namespace operators; using enum shuffle_config;
		distribution_result_t<G> swap_range = unsign<distribution_result_t<G>>(std::ranges::size(r));
		if constexpr (C & EMPTY) { if (swap_range < 2) return; }
		std::ranges::iterator_t<R> b = std::ranges::begin(r);

		if constexpr (C & ODD) {
			if constexpr (C & MIXED) { if (is_odd(m - b)) goto REST_EVEN; }
			std::ranges::iter_swap(b, b + sign<std::ranges::range_difference_t<R>>(int_generate(g, swap_range)));
			if constexpr (C & SINGLE) { if (b == m) return; } ++b; --swap_range;
		}
	REST_EVEN:
		do {
			const pair<distribution_result_t<G>> p = int_generate_two(g, swap_range, swap_range - 1);
			std::ranges::iter_swap(b, b + sign<std::ranges::range_difference_t<R>>(get_0(p)));
			++b;
			std::ranges::iter_swap(b, b + sign<std::ranges::range_difference_t<R>>(get_1(p)));
			if (b != m) { ++b; swap_range -= 2; } else return;
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
				*b = un_sign_or_cast<std::ranges::range_value_t<R>>(s - std::ranges::begin(a));
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

	template<shuffle_config C = shuffle_config::MIXED_SINGLE_EMPTY, permutable_range R,
		distribution_result_ctible_to_and_from<std::ranges::range_difference_t<R>> G, bool_auxiliary_range_for<std::ranges::range_value_t<R>> B>
	constexpr void partial_shuffle_counting_sort(R &&r, const std::ranges::iterator_t<R> m, G &&g, B &&a) {
		using namespace operators; using enum shuffle_config;
		const std::ranges::range_size_t<R> swap_range = std::ranges::size(r);
		if constexpr (C & EMPTY) { if (swap_range < 2) return; }

		partial_shuffle<to_enum<const_t<C>>(C & ~EMPTY)>(std::ranges::subrange{invoke_iterator<invoke_iter_config::PRE_INCREMENT,
			([](const std::ranges::iterator_t<R> b, const std::ranges::iterator_t<B> s) -> void { s[*b] = true; })
		>{std::ranges::begin(r), std::ranges::begin(a)}, std::unreachable_sentinel, swap_range}, m, g);

		if constexpr (C & SINGLE) { if (std::ranges::begin(r) == m) return; }
		std::ranges::begin(a)[*m] = true;
		counting_fill(unsafe_subrange{std::ranges::begin(r), m}, a);
	}

}
