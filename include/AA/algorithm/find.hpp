#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/range.hpp"
#include <ranges> // iterator_t, borrowed_iterator_t, range_size_t, begin, size, bidirectional_range, input_range
#include <iterator> // indirect_strict_weak_order, indirect_binary_predicate, indirectly_unary_invocable
#include <functional> // invoke, less, equal_to
#include <utility> // as_const



namespace aa {

	// Tokia algoritmų filosofija, visi algoritmai unsafe, iškvietėjas būna atsargus jei jam reikia būti atsargiam.
	// Algoritmai priima tik ranges, jei reikia paduoti iteratorius tai galima paduoti iš jų sukurtą range.
	// Išskyrus tada nepaduodami ranges, jei reikia algoritmui tik atskirų iteratorių, reiškia būtų neefektyvu padavinėti iteratorių poras.
	template<sized_random_access_range R, class T, std::indirect_strict_weak_order<const T *, std::ranges::iterator_t<R>> C = std::ranges::less>
	AA_CONSTEXPR std::ranges::borrowed_iterator_t<R> unsafe_upper_bound(R &&r, const T &value, C &&comp = {}) {
		std::ranges::iterator_t<R> first = std::ranges::begin(r);
		std::ranges::range_size_t<R> len = std::ranges::size(r);
		do {
			const std::ranges::range_size_t<R> half = len >> 1;
			const std::ranges::iterator_t<R> middle = first + half;
			if (std::invoke(comp, value, std::as_const(*middle))) {
				len = half;
			} else {
				first = middle + 1;
				len -= half + 1;
			}
		} while (len);
		return first;
	}

	template<sized_random_access_range R, class T, std::indirect_strict_weak_order<const T *, std::ranges::iterator_t<R>> C = std::ranges::less>
	AA_CONSTEXPR std::ranges::borrowed_iterator_t<R> unsafe_lower_bound(R &&r, const T &value, C &&comp = {}) {
		std::ranges::iterator_t<R> first = std::ranges::begin(r);
		std::ranges::range_size_t<R> len = std::ranges::size(r);
		do {
			const std::ranges::range_size_t<R> half = len >> 1;
			const std::ranges::iterator_t<R> middle = first + half;
			if (std::invoke(comp, std::as_const(*middle), value)) {
				first = middle + 1;
				len -= half + 1;
			} else {
				len = half;
			}
		} while (len);
		return first;
	}



	// last kintamasis nėra const&, nes naudojama standarto funkcija, kad inicializuoti jį gražina tiesiog reikšmę.
	template<std::ranges::bidirectional_range R, class T>
		requires (std::indirect_binary_predicate<std::ranges::equal_to, std::ranges::iterator_t<R>, const T *>)
	AA_CONSTEXPR std::ranges::borrowed_iterator_t<R> unsafe_find_last(R &&r, const T &value) {
		std::ranges::iterator_t<R> first = get_rbegin(r);
		if (*first != value) {
			const std::ranges::iterator_t<R> last = std::ranges::begin(r);
			while (first != last && (*--first != value));
		}
		return first;
	}



	template<std::ranges::input_range R, std::indirectly_unary_invocable<std::ranges::iterator_t<R>> F>
	AA_CONSTEXPR void unsafe_for_each(R &&r, F &&f) {
		std::ranges::iterator_t<R> first = std::ranges::begin(r);
		const std::ranges::iterator_t<R> last = get_rbegin(r);
		do {
			std::invoke(f, *first);
			if (first != last) ++first; else return;
		} while (true);
	}

}
