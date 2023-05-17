#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/range.hpp"
#include "../system/source.hpp"
#include "arithmetic.hpp"
#include <ranges> // size, data



namespace aa {

	template<template<class> class H = std::hash>
	struct generic_hash {
		template<hashable_by_template<H> T>
		static AA_CONSTEXPR size_t operator()(const T &t) {
			return constant_v<H<T>>(t);
		}

		static AA_CONSTEVAL size_t min() { return numeric_min; }
		static AA_CONSTEVAL size_t max() { return numeric_max; }

		using is_transparent = void;
	};

	generic_hash()->generic_hash<>;



	template<size_t N, template<class> class H = std::hash>
	struct mod_generic_hash {
		template<hashable_by_template<H> T>
		static AA_CONSTEXPR size_t operator()(const T &t) {
			return remainder<N>(constant_v<H<T>>(t));
		}

		static AA_CONSTEVAL size_t min() { return 0; }
		static AA_CONSTEVAL size_t max() { return N - 1; }

		using is_transparent = void;
	};



	// Rekomenduojama naudoti su templates šią klasę kaip tipo parametrą.
	//
	// MAX negali rodyti į kažkurį iš argumentų, nes MAX represents a failure state kai nerandamas nei vienas iš template parametrų.
	// MAX nėra numeric_max, nes tokia reikšmė indikuotų, kad klasė gali gražinti visas reikšmes nuo 0 iki numeric_max.
	template<fixed_string_like auto... A>
		requires (same_as_every<range_char_traits_t<const_t<A>>...>)
	struct string_perfect_hash : pack<A...> {
		using is_transparent = void;
		using traits_type = first_or_void_t<range_char_traits_t<const_t<A>>...>;

	protected:
		template<size_t I, auto V, class T, class F>
		static AA_CONSTEXPR bool trie(const T &t, F &&f, const bool c) {
			if (c) {
				if constexpr (I == std::tuple_size_v<const_t<V>>) {
					invoke<pack_index_v<V, A...>>(std::forward<F>(f));
					return true;
				} else {
					return trie<I + 1, V>(t, std::forward<F>(f),
						traits_type::eq(std::ranges::data(t)[I], const_v<constant_v<getter<I>>(V)>));
				}
			} else return false;
		}

	public:
		template<class T, class F>
		static AA_CONSTEXPR void operator()(const T &t, F &&f) {
			if constexpr (same_range_char_traits_as<T, traits_type>) {
				if ((... || trie<0, A>(t, std::forward<F>(f), std::ranges::size(t) == std::tuple_size_v<const_t<A>>)))
					return;
			}
			invoke<max()>(std::forward<F>(f));
		}

		static AA_CONSTEVAL size_t min() { return 0; }
		static AA_CONSTEVAL size_t max() { return sizeof...(A); }
	};

	template<auto... A>
	using make_string_p_hash
		= string_perfect_hash<conditional_nttp_v<fixed_string_like<const_t<A>>, A, literal_name_v<A>>...>;

	template<size_t N>
	using string_identity_hash = decltype(apply<N>([]<size_t... I>() -> make_string_p_hash<I...> { return {}; }));

}
