#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/range.hpp"
#include "arithmetic.hpp"
#include <cstddef> // size_t
#include <functional> // hash
#include <ranges> // size, data



namespace aa {

	template<template<class> class H = std::hash>
	struct generic_hash {
		template<hashable_by_template<H> T>
		AA_CONSTEXPR size_t operator()(const T &t) const {
			return H<T>{}(t);
		}

		static AA_CONSTEVAL size_t min() { return numeric_min; }
		static AA_CONSTEVAL size_t max() { return numeric_max; }

		using is_transparent = void;
	};

	generic_hash()->generic_hash<>;



	template<size_t N, template<class> class H = std::hash>
	struct mod_generic_hash {
		template<hashable_by_template<H> T>
		AA_CONSTEXPR size_t operator()(const T &t) const {
			return remainder<N>(H<T>{}(t));
		}

		static AA_CONSTEVAL size_t min() { return 0; }
		static AA_CONSTEVAL size_t max() { return N - 1; }

		using is_transparent = void;
	};



	// MAX negali rodyti į kažkurį iš argumentų, nes MAX represents a failure state kai nerandamas nei vienas iš template parametrų.
	template<size_t MAX, auto... A>
		requires ((sizeof...(A) <= MAX) && are_same_v<range_char_traits_t<decltype(A)>...>)
	struct string_perfect_hash {
		using is_transparent = void;
		using traits_type = first_or_void_t<range_char_traits_t<decltype(A)>...>;

	protected:
		template<size_t I, auto V, class T>
		static AA_CONSTEXPR void trie(const T &t, size_t &h, const bool c) {
			if (c) {
				if constexpr (I == std::ranges::size(V)) {
					h = pack_index_v<V, A...>;
				} else {
					trie<I + 1, V>(t, h, traits_type::eq(std::ranges::data(t)[I], std::ranges::data(V)[I]));
				}
			}
		}

	public:
		template<class T>
		AA_CONSTEXPR size_t operator()(const T &t) const {
			if constexpr (same_range_char_traits_as<T, traits_type>) {
				size_t h = MAX;
				(trie<0, A>(t, h, std::ranges::size(t) == std::ranges::size(A)), ...);
				return h;
			} else return MAX;
		}

		static AA_CONSTEVAL size_t min() { return 0; }
		static AA_CONSTEVAL size_t max() { return MAX; }
	};

	string_perfect_hash(void)->string_perfect_hash<0>;

	template<auto... A>
	using s_string_perfect_hash = string_perfect_hash<sizeof...(A), A...>;

	template<auto... A>
	using m_string_perfect_hash = string_perfect_hash<numeric_max, A...>;

}
