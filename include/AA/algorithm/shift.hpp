#pragma once

#include "../metaprogramming/range.hpp"
#include "../algorithm/arithmetic.hpp"
#include <cstring> // memcpy
#include <cstddef> // size_t
#include <ranges> // data, size
#include <iterator> // contiguous_iterator, iter_value_t
#include <memory> // to_address
#include <bit> // bit_cast



namespace aa {

	// Dėl suvaržymų copy konstruktorius turi būti trivial todėl operatorius = veiks gerai.
	// Turime naudoti bit_cast, nes pvz. jei I value būtų int, o O float tai darydami reinterpret_cast susidurtume su u. b.
	template<std::contiguous_iterator I, trivial_output_iterator_for<I> O>
	AA_CONSTEXPR void cshift_right(const I &b, const I &b_An_S1, const O &b2, const O &b2_A1, const size_t n_S1_MS) {
		std::memcpy(std::to_address(b2_A1), std::to_address(b), n_S1_MS);
		*std::to_address(b2) = std::bit_cast<std::iter_value_t<O>>(*std::to_address(b_An_S1));
	}

	template<std::contiguous_iterator I, trivial_output_iterator_for<I> O>
	AA_CONSTEXPR void cshift_right(const I &b, const O &b2, const size_t n_S1) {
		cshift_right(b, b + n_S1, b2, b2 + 1, size_of<std::iter_value_t<I>>(n_S1));
	}

	template<sized_contiguous_range I, trivial_output_range_for<I> O>
	AA_CONSTEXPR void cshift_right(I &&i, O &&o) {
		cshift_right(std::ranges::data(i), std::ranges::data(o), std::ranges::size(i) - 1);
	}



	template<std::contiguous_iterator I, trivial_output_iterator_for<I> O>
	AA_CONSTEXPR void cshift_right(const I &b, const I &b_An_Ss, const O &b2, const O &b2_As, const size_t n_Ss_MS, const size_t s_MS) {
		std::memcpy(std::to_address(b2_As), std::to_address(b), n_Ss_MS);
		std::memcpy(std::to_address(b2), std::to_address(b_An_Ss), s_MS);
	}

	template<std::contiguous_iterator I, trivial_output_iterator_for<I> O>
	AA_CONSTEXPR void cshift_right(const I &b, const O &b2, const size_t n_Ss, const size_t s) {
		cshift_right(b, b + n_Ss, b2, b2 + s, size_of<std::iter_value_t<I>>(n_Ss), size_of<std::iter_value_t<I>>(s));
	}

	template<sized_contiguous_range I, trivial_output_range_for<I> O>
	AA_CONSTEXPR void cshift_right(I &&i, O &&o, const size_t s) {
		cshift_right(std::ranges::data(i), std::ranges::data(o), std::ranges::size(i) - s, s);
	}

}
