#pragma once

#include "../metaprogramming/general.hpp"
#include "arithmetic.hpp"



namespace aa {

	// Tokia algoritmų filosofija, visi algoritmai unsafe, iškvietėjas būna atsargus jei jam reikia būti atsargiam.
	// Algoritmai priima tik ranges, jei reikia paduoti iteratorius tai galima paduoti iš jų sukurtą range.
	// Išskyrus tada nepaduodami ranges, jei reikia algoritmui tik atskirų iteratorių, reiškia būtų neefektyvu padavinėti iteratorių poras.
	template<sized_random_access_range R, class T, std::indirect_strict_weak_order<const T *, std::ranges::iterator_t<R>> C = const std::ranges::less>
	constexpr std::ranges::borrowed_iterator_t<R> unsafe_upper_bound(R &&r, const T &value, C comp = default_value) {
		std::ranges::iterator_t<R> first = std::ranges::begin(r);
		std::ranges::range_size_t<R> len = std::ranges::size(r);
		do {
			const std::ranges::range_size_t<R> half = halve(len);
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

	template<sized_random_access_range R, class T, std::indirect_strict_weak_order<const T *, std::ranges::iterator_t<R>> C = const std::ranges::less>
	constexpr std::ranges::borrowed_iterator_t<R> unsafe_lower_bound(R &&r, const T &value, C comp = default_value) {
		std::ranges::iterator_t<R> first = std::ranges::begin(r);
		std::ranges::range_size_t<R> len = std::ranges::size(r);
		do {
			const std::ranges::range_size_t<R> half = halve(len);
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



	// last kintamasis nėra const&, nes naudojama standarto funkcija, kad inicializuoti jį grąžina tiesiog reikšmę.
	template<std::ranges::forward_range R, ptr_in_indirect_predicate_with<std::ranges::iterator_t<R>, std::ranges::equal_to> T>
	constexpr std::ranges::borrowed_iterator_t<R> unsafe_find_last(R &&r, const T &value) {
		if constexpr (std::ranges::bidirectional_range<R>) {
			std::ranges::iterator_t<R> first = get_rbegin(r);
			if (*first != value) {
				const std::ranges::iterator_t<R> last = std::ranges::begin(r);
				while (first != last && (*--first != value));
			}
			return first;
		} else {
			std::ranges::iterator_t<R> first = std::ranges::begin(r), res;
			const std::ranges::iterator_t<R> last = get_rbegin(r);
			do {
				if (*first == value) res = first;
				if (first != last) ++first; else return res;
			} while (true);
		}
	}

	template<std::ranges::forward_range R, ptr_in_indirect_predicate_with<std::ranges::iterator_t<R>, std::ranges::equal_to> T>
	constexpr std::ranges::borrowed_iterator_t<R> unsafe_find(R &&r, const T &value) {
		std::ranges::iterator_t<R> first = std::ranges::begin(r);
		if (*first != value) {
			const std::ranges::iterator_t<R> last = get_rbegin(r);
			while (first != last && (*++first != value));
		}
		return first;
	}



	template<std::ranges::forward_range R, std::indirect_unary_predicate<std::ranges::iterator_t<R>> P>
	constexpr bool unsafe_all_of(R &&r, P pred) {
		std::ranges::iterator_t<R> first = std::ranges::begin(r);
		if (std::invoke(pred, *first)) {
			const std::ranges::iterator_t<R> last = get_rbegin(r);
			while (first != last)
				if (!std::invoke(pred, *++first))
					return false;
			return true;
		} else return false;
	}



	template<std::ranges::forward_range R, std::indirectly_unary_invocable<std::ranges::iterator_t<R>> F>
	constexpr void unsafe_for_each(R &&r, F f) {
		std::ranges::iterator_t<R> first = std::ranges::begin(r);
		const std::ranges::iterator_t<R> last = get_rbegin(r);
		do {
			std::invoke(f, *first);
			if (first != last) ++first; else return;
		} while (true);
	}

	template<std::ranges::forward_range R,
		std::indirectly_unary_invocable<std::ranges::iterator_t<R>> F1, std::indirectly_unary_invocable<std::ranges::iterator_t<R>> F2>
	constexpr std::invoke_result_t<F2, std::ranges::range_reference_t<R>> unsafe_for_each_peel_last(R &&r, F1 f1, F2 &&f2) {
		std::ranges::iterator_t<R> first = std::ranges::begin(r);
		const std::ranges::iterator_t<R> last = get_rbegin(r);
		while (first != last)
			std::invoke(f1, *first++);
		return std::invoke(std::forward<F2>(f2), *first);
	}

	template<std::ranges::forward_range R,
		std::indirectly_unary_invocable<std::ranges::iterator_t<R>> F1, std::indirectly_unary_invocable<std::ranges::iterator_t<R>> F2>
	constexpr void unsafe_for_each_peel_first(R &&r, F1 &&f1, F2 f2) {
		std::ranges::iterator_t<R> first = std::ranges::begin(r);
		const std::ranges::iterator_t<R> last = get_rbegin(r);
		std::invoke(std::forward<F1>(f1), *first);
		while (first != last)
			std::invoke(f2, *++first);
	}

}
