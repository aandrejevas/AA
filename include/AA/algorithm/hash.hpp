#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/range.hpp"
#include "arithmetic.hpp"
#include <cstddef> // size_t
#include <functional> // hash
#include <ranges> // size, data
#include <utility> // forward, tuple_size



namespace aa {

	template<template<class> class H = std::hash>
	struct generic_hash {
		template<hashable_by_template<H> T>
		AA_CONSTEXPR size_t operator()(const T &t) const {
			return constant<H<T>>()(t);
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
			return remainder<N>(constant<H<T>>()(t));
		}

		static AA_CONSTEVAL size_t min() { return 0; }
		static AA_CONSTEVAL size_t max() { return N - 1; }

		using is_transparent = void;
	};



	// Rekomenduojama naudoti su templates šią klasę kaip tipo parametrą.
	//
	// MAX negali rodyti į kažkurį iš argumentų, nes MAX represents a failure state kai nerandamas nei vienas iš template parametrų.
	template<auto... A>
		requires (are_same_v<range_char_traits_t<decltype(A)>...>)
	struct string_perfect_hash {
		using is_transparent = void;
		using traits_type = first_or_void_t<range_char_traits_t<decltype(A)>...>;

	protected:
		template<size_t I, auto V, class T, class F>
		static AA_CONSTEXPR bool trie(const T &t, F &&f, const bool c) {
			if (c) {
				if constexpr (I == std::ranges::size(V)) {
					invoke<pack_index_v<V, A...>>(std::forward<F>(f));
					return true;
				} else {
					return trie<I + 1, V>(t, std::forward<F>(f), traits_type::eq(std::ranges::data(t)[I], std::ranges::data(V)[I]));
				}
			} else return false;
		}

	public:
		template<class T, class F>
		AA_CONSTEXPR void operator()(const T &t, F &&f) const {
			if constexpr (same_range_char_traits_as<T, traits_type>) {
				if ((... || trie<0, A>(t, std::forward<F>(f), std::ranges::size(t) == std::ranges::size(A))))
					return;
			}
			invoke<max()>(std::forward<F>(f));
		}

		static AA_CONSTEVAL size_t min() { return 0; }
		static AA_CONSTEVAL size_t max() { return sizeof...(A); }
	};

}



namespace std {

	template<auto... A>
	struct tuple_size<aa::string_perfect_hash<A...>> : aa::size_t_identity<sizeof...(A)> {};

}
