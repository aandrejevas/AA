#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/generator.hpp"
#include "../metaprogramming/range.hpp"
#include "generate.hpp"
#include <iterator> // iter_swap, iter_value_t, sized_sentinel_for, sentinel_for
#include <utility> // forward
#include <ranges> // iterator_t, begin, end



namespace aa {

	// https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle#The_modern_algorithm
	// Ne forward'iname g, nes tiesiog visais atvejais paduosime reference ir tiek, čia nėra reikalo palaikyti move semantics.
	template<permutable_random_access_range R, differences_generator_for<std::ranges::iterator_t<R>> G>
	inline constexpr void shuffle(R &&r, G &&g) {
		shuffle(std::ranges::begin(r), std::ranges::end(r), g);
	}

	// Ne move'iname b, nes jei b būtų lvalue ref, tai b būtų sugadintas, negali b būti const&, nes tada neišeitų palaikyti move semantics.
	template<permutable_random_access_iterator I, std::sized_sentinel_for<I> SS, differences_generator_for<I> G>
	inline constexpr void shuffle(I &&b, const SS &e, G &&g) {
		partial_shuffle(std::forward<I>(b), e, e - 2, g);
	}

	template<permutable_random_access_iterator I, std::sized_sentinel_for<I> SS, std::sentinel_for<I> S, differences_generator_for<I> G>
	inline constexpr void partial_shuffle(I b, const SS &e, const S &m, G &&g) {
		do {
			std::ranges::iter_swap(b, b + int_distribution(g, static_cast<generator_modulus_t<G>>(e - b)));
			if (b != m) ++b; else return;
		} while (true);
	}



	// https://en.wikipedia.org/wiki/Counting_sort
	template<permutable_random_access_iterator I, std::sentinel_for<I> S, key_bool_iterator_for<I> R>
	inline constexpr void counting_sort(I b, const S &e_S1, R selected) {
		{I b2 = b; do {
			selected[*b2] = true;
			if (b2 != e_S1) ++b2; else break;
		} while (true);}

		const R s = selected; do {
			if (*selected) {
				*selected = false;
				*b = static_cast<std::iter_value_t<I>>(selected - s);
				if (b == e_S1) return; else ++b;
			}
			++selected;
		} while (true);
	}

	template<permutable_random_access_range R, key_bool_iterator_for<std::ranges::iterator_t<R>> S>
	inline constexpr void counting_sort(R &&r, S &&s) {
		counting_sort(std::ranges::begin(r), std::ranges::end(r) - 1, std::forward<S>(s));
	}

	template<permutable_random_access_iterator I, std::sized_sentinel_for<I> SS, std::sentinel_for<I> S, differences_generator_for<I> G, key_bool_iterator_for<I> R>
	inline constexpr void partial_shuffle_counting_sort(I b, const SS &e, const S &m, G &&g, R selected) {
		{I b2 = b; do {
			std::ranges::iter_swap(b2, b2 + int_distribution(g, static_cast<generator_modulus_t<G>>(e - b2)));
			selected[*b2] = true;
			if (b2 != m) ++b2; else break;
		} while (true);}

		const R s = selected; do {
			if (*selected) {
				*selected = false;
				*b = static_cast<std::iter_value_t<I>>(selected - s);
				if (b == m) return; else ++b;
			}
			++selected;
		} while (true);
	}

}
