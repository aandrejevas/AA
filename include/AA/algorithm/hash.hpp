#pragma once

#include "../metaprogramming/general.hpp"
#include "../system/source.hpp"
#include "arithmetic.hpp"



namespace aa {

	template<template<class> class H = std::hash>
	struct generic_hash {
		template<hashable_by_template<H> T>
		static constexpr size_t operator()(const T & t) {
			return default_v<H<T>>(t);
		}

		static consteval size_t min() { return numeric_min; }
		static consteval size_t max() { return numeric_max; }

		using is_transparent = void;
	};

	generic_hash() -> generic_hash<>;



	template<size_t N, template<class> class H = std::hash>
	struct mod_generic_hash {
		template<hashable_by_template<H> T>
		static constexpr size_t operator()(const T & t) {
			return remainder<N>(default_v<H<T>>(t));
		}

		static consteval size_t min() { return 0; }
		static consteval size_t max() { return N - 1; }

		using is_transparent = void;
	};



	// Rekomenduojama naudoti su templates šią klasę kaip tipo parametrą.
	//
	// MAX negali rodyti į kažkurį iš argumentų, nes MAX represents a failure state kai nerandamas nei vienas iš template parametrų.
	// MAX nėra numeric_max, nes tokia reikšmė indikuotų, kad klasė gali grąžinti visas reikšmes nuo 0 iki numeric_max.
	template<fixed_string_like auto... A>
		requires (!!sizeof...(A) && same_as_every<range_char_traits_t<const_t<A>>...>)
	struct string_perfect_hash {
		using is_transparent = void;
		using traits_type = range_char_traits_t<const_t<A>>...[0];

		template<same_range_char_traits_as<traits_type> T, class F>
		static constexpr void operator()(const T & t, F && f) {
			// size_t tipas, nes reikės lyginti su size_t tipo kintamuoju.
			const size_t count = std::ranges::size(t);
			if (!(... || ((count == std::tuple_size_v<const_t<A>>) && apply<std::tuple_size_v<const_t<A>>>([&]<size_t... I> -> bool {
				static constexpr const_t<A> V = A;
				return (... && traits_type::eq(std::ranges::data(t)[I], const_v<get_element<I>(V)>)) ?
					(invoke<pack_index_v<V, A...>>(std::forward<F>(f)), true) : false;
			})))) {
				invoke<max()>(std::forward<F>(f));
			}
		}

		static consteval size_t min() { return 0; }
		static consteval size_t max() { return sizeof...(A); }
	};



	// Neturime klasės iš kurios tiesiog galėtume paveldėti is_transparent, nes gcc taip elgiasi. Taip pat, nes
	// tai nėra neįprasta, konteineriai ir iteratoriai turi tokių pačių aliases ir iš sandarto buvo pašalintas tipas
	// std::iterator, kuris buvo naudojamas tokiu pačiu principu, tai reiškia nerekomenduojama tokia realizacija.
	struct string_equal_to {
		template<class L, same_range_char_traits_as<range_char_traits_t<L>> R>
		static constexpr bool operator()(const L & l, const R & r) {
			// size_t tipas, nes compare tikisi tokio tipo parametro.
			const size_t count = std::ranges::size(l);
			return count == std::ranges::size(r) &&
				!range_char_traits_t<L>::compare(std::ranges::data(l), std::ranges::data(r), count);
		}

		using is_transparent = void;
	};

}
