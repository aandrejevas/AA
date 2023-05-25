#pragma once

#include "../metaprogramming/general.hpp"
#include "../algorithm/arithmetic.hpp"
#include <ranges> // view_base
#include <variant> // monostate
#include <iterator> // default_sentinel_t, default_sentinel, forward_iterator_tag



namespace aa {

	template<std::unsigned_integral T, unsigned_integral_or_same_as<std::monostate> U = std::monostate, bool REVERSED = false>
	struct bitset_view : std::ranges::view_base {
		// Member constants
		static AA_CONSTEXPR const bool is_offset = !std::same_as<U, std::monostate>;

		// Member types
		using bitset_type = T;
		using offset_type = U;
		using value_type = type_pack_element_t<!is_offset, offset_type, bitset_type>;
		using size_type = size_t;
		using difference_type = ptrdiff_t;
		using iterator_category = std::forward_iterator_tag;
		using iterator = bitset_view<T, U, REVERSED>;
		using const_iterator = iterator;
		using reverse_iterator = bitset_view<T, U, !REVERSED>;
		using const_reverse_iterator = reverse_iterator;



		// Element access
	protected:
		static AA_CONSTEXPR value_type index(const value_type index, const offset_type offset) {
			if constexpr (is_offset) {
				return index + offset;
			} else {
				return index;
			}
		}

	public:
		// Neturime operator[] funckijos, nes nėra būdo indeksuoti nustatytų bitų. Ir ji negalėtų tiesiog
		// parodyti ar nurodytas bitas yra nustatytas, nes nesutaptų realizacija su front ir back.
		// Modifikuojančių metodų neapibrėžiame, naudotojas gali pats modifikuoti laukus kaip nori.

		AA_CONSTEXPR value_type front() const {
			if constexpr (REVERSED) {
				return index((constant_v<value_type, max_index()>) - unsign<value_type>(std::countr_zero(bitset)), offset);
			} else {
				return index(unsign<value_type>(std::countr_zero(bitset)), offset);
			}
		}

		AA_CONSTEXPR value_type back() const {
			if constexpr (REVERSED) {
				return index(unsign<value_type>(std::countl_zero(bitset)), offset);
			} else {
				return index((constant_v<value_type, max_index()>) - unsign<value_type>(std::countl_zero(bitset)), offset);
			}
		}



		// Operations
		AA_CONSTEXPR value_type operator*() const {
			return front();
		}

		AA_CONSTEXPR iterator &operator++() {
			// (0 & (0 - 1)) == 0
			bitset &= bitset - 1;
			return *this;
		}

		AA_CONSTEXPR iterator operator++(const int) {
			const bitset_type b = bitset;
			return {b, operator++().offset};
		}

		friend AA_CONSTEXPR bool operator==(const iterator &l, const iterator &r) {
			return l.bitset == r.bitset && l.offset == r.offset;
		}

		friend AA_CONSTEXPR bool operator==(const iterator &l, const std::default_sentinel_t) {
			return !l;
		}



		// Iterators
		AA_CONSTEXPR const_iterator begin() const { return {bitset, offset}; }
		AA_CONSTEXPR std::default_sentinel_t end() const { return std::default_sentinel; }
		AA_CONSTEXPR const_reverse_iterator rbegin() const { return {bitswap(bitset), offset}; }
		AA_CONSTEXPR std::default_sentinel_t rend() const { return std::default_sentinel; }



		// Capacity
		explicit AA_CONSTEXPR operator bool() const { return !empty(); }
		AA_CONSTEXPR bool empty() const { return is_numeric_min(bitset); }
		AA_CONSTEXPR bool full() const { return is_numeric_max(bitset); }
		AA_CONSTEXPR bool single() const { return std::has_single_bit(bitset); }

		AA_CONSTEXPR size_type size() const { return unsign<size_type>(std::popcount(bitset)); }
		AA_CONSTEXPR difference_type ssize() const { return std::bit_cast<difference_type>(size()); }

		static AA_CONSTEVAL size_type max_size() { return std::numeric_limits<bitset_type>::digits; }
		static AA_CONSTEVAL size_type max_index() { return max_size() - 1; }



		// Special member functions
		AA_CONSTEVAL bitset_view() = default;
		AA_CONSTEXPR bitset_view(const bitset_type b, const offset_type o = default_value) : bitset{b}, offset{o} {}



		// Member objects
		bitset_type bitset;
		[[no_unique_address]] offset_type offset;
	};

	template<class T, class U = std::monostate>
	bitset_view(const T, const U = default_value) -> bitset_view<T, U>;

	template<bool REVERSED = false, class T, class U = std::monostate>
	AA_CONSTEXPR bitset_view<T, U, REVERSED> make_bitset_view(const T t, const U u = default_value) {
		return {t, u};
	}

}