#pragma once

#include "../metaprogramming/general.hpp"
#include "../algorithm/arithmetic.hpp"



namespace aa {

	template<std::unsigned_integral T, unsigned_integral_or_same_as<std::monostate> U = std::monostate, bool REVERSED = false>
	struct bitset_view : std::ranges::view_base {
		// Member constants
		static constexpr bool is_offset = !std::same_as<U, std::monostate>;

		// Member types
		using bitset_type = T;
		using offset_type = U;
		using value_type = std::conditional_t<is_offset, offset_type, bitset_type>;
		using size_type = size_t;
		using difference_type = ptrdiff_t;
		using iterator_category = std::forward_iterator_tag;
		using iterator = bitset_view;
		using const_iterator = iterator;
		using conjugate_type = bitset_view<T, U, !REVERSED>;



		// Element access
	protected:
		static constexpr value_type index(const value_type index, const offset_type offset) {
			if constexpr (is_offset) {
				return index + offset;
			} else {
				return index;
			}
		}

	public:
		// Neturime operator[] funkcijos, nes nėra būdo indeksuoti nustatytų bitų. Ir ji negalėtų tiesiog
		// parodyti ar nurodytas bitas yra nustatytas, nes nesutaptų realizacija su front ir back.
		// Modifikuojančių metodų neapibrėžiame, naudotojas gali pats modifikuoti laukus kaip nori.

		constexpr value_type front() const {
			if constexpr (REVERSED) {
				return index((value_v<value_type, max_index()>) - unsign<value_type>(std::countr_zero(bitset)), offset);
			} else {
				return index(unsign<value_type>(std::countr_zero(bitset)), offset);
			}
		}

		constexpr value_type back() const {
			if constexpr (REVERSED) {
				return index(unsign<value_type>(std::countl_zero(bitset)), offset);
			} else {
				return index((value_v<value_type, max_index()>) - unsign<value_type>(std::countl_zero(bitset)), offset);
			}
		}



		// Operations
		constexpr value_type operator*() const {
			return front();
		}

		constexpr iterator &operator++() {
			// (0 & (0 - 1)) == 0
			bitset &= bitset - 1;
			return *this;
		}

		constexpr iterator operator++(const int) {
			const bitset_type b = bitset;
			return {b, operator++().offset};
		}

		friend constexpr bool operator==(const iterator l, const iterator r) {
			return l.bitset == r.bitset && l.offset == r.offset;
		}

		friend constexpr bool operator==(const iterator l, const std::default_sentinel_t) {
			return !l;
		}



		// Iterators
		constexpr const_iterator begin() const { return {bitset, offset}; }
		constexpr std::default_sentinel_t end() const { return std::default_sentinel; }
		constexpr conjugate_type reverse() const { return {bitswap(bitset), offset}; }



		// Capacity
		explicit constexpr operator bool() const { return !empty(); }
		constexpr bool empty() const { return is_numeric_min(bitset); }
		constexpr bool full() const { return is_numeric_max(bitset); }
		constexpr bool single() const { return std::has_single_bit(bitset); }

		constexpr size_type size() const { return unsign<size_type>(std::popcount(bitset)); }

		static consteval size_type max_size() { return numeric_digits_v<bitset_type>; }
		static consteval size_type max_index() { return max_size() - 1; }



		// Special member functions
		consteval bitset_view() = default;
		constexpr bitset_view(const bitset_type b, const offset_type o = default_value) : bitset{b}, offset{o} {}
		constexpr bitset_view(const const_t<std::placeholders::_1>, const bitset_type b, const offset_type o = default_value) : bitset_view{b, o} {}



		// Member objects
		bitset_type bitset;
		[[no_unique_address]] offset_type offset;
	};

	template<class T, class U = std::monostate>
	using reversed_bitset_view = bitset_view<T, U, true>;

	template<class T, class U = std::monostate>
	bitset_view(const T, const U = default_value) -> bitset_view<T, U>;

	template<class T, class U = std::monostate>
	bitset_view(const const_t<std::placeholders::_1>, const T, const U = default_value) -> reversed_bitset_view<T, U>;

}
