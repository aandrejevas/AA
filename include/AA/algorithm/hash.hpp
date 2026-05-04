#pragma once

#include "../metaprogramming/general.hpp"
#include "arithmetic.hpp"



namespace aa {

	template<template<class> class H = std::hash>
	struct generic_hash {
		template<hashable_by_template<H> T>
		static constexpr size_t operator()(const T & t) {
			return c<H<T>>()(t);
		}

		static consteval size_t max() { return numeric_max; }
		static consteval size_t min() { return numeric_min; }
	};



	template<size_t N, template<class> class H = std::hash>
	struct mod_generic_hash {
		template<hashable_by_template<H> T>
		static constexpr size_t operator()(const T & t) {
			return remainder<N>(c<H<T>>()(t));
		}

		static consteval size_t max() { return N - 1; }
		static consteval size_t min() { return 0; }
	};



	// Rekomenduojama naudoti su templates šią klasę kaip tipo parametrą.
	//
	// MAX negali rodyti į kažkurį iš argumentų, nes MAX represents a failure state kai nerandamas nei vienas iš template parametrų.
	// MAX nėra numeric_max, nes tokia reikšmė indikuotų, kad klasė gali grąžinti visas reikšmes nuo 0 iki numeric_max.
	template<fixed_string_like auto... A>
		requires (!!sizeof...(A) && same_as_every<range_char_traits_t<t<A>>...>)
	struct string_perfect_hash {
		using traits_type = range_char_traits_t<t<A>>...[0];

		// GCC 16.1.0 BUG ICE: If we pass index as nttp to a lambda.
		template<same_range_char_traits_as<traits_type> T>
		static constexpr size_t operator()(const T & str) {
			// size_t tipas, nes reikės lyginti su size_t tipo kintamuoju.
			const size_t count = std_r::size(str);
			const range_pointer_t<const T> str_data = std_r::data(str);

			constexpr auto [...PARAMETER_INDEXES] = c<std::make_index_sequence<sizeof...(A)>>();
			template for (constexpr size_t I : {PARAMETER_INDEXES...}) {
				if (count != tuple_size<t<A...[I]>>())
					continue;

				constexpr auto [...CHAR_INDEXES] = c<std::make_index_sequence<tuple_size<t<A...[I]>>()>>();
				template for (constexpr size_t J : {CHAR_INDEXES...}) {
					if (!traits_type::eq(str_data[J], c(get_element<J>(AA_EXPAND(A)[I]))))
						break;

					if constexpr (J == (tuple_size<t<A...[I]>>() - 1)) {
						// invoke<get_call<I>()>(std::forward<F>(f));
						return I;
					}
				}
			}
			// invoke<get_call<max()>()>(std::forward<F>(f));
			return max();
		}

		// template<same_range_char_traits_as<traits_type> T>
		// static constexpr size_t operator()(const T & t) {
		// 	return make(
		// 		[&](size_t & hash) {
		// 			operator()(t, [&]<size_t I> { hash = I; });
		// 		});
		// }

		static consteval size_t max() { return sizeof...(A); }
		static consteval size_t min() { return 0; }
	};



	// Neturime klasės iš kurios tiesiog galėtume paveldėti is_transparent, nes gcc taip elgiasi. Taip pat, nes
	// tai nėra neįprasta, konteineriai ir iteratoriai turi tokių pačių aliases ir iš standarto buvo pašalintas tipas
	// std::iterator, kuris buvo naudojamas tokiu pačiu principu, tai reiškia nerekomenduojama tokia realizacija.
	//
	// Norint, kad klasė turėtų is_transparent, tai reikia turėti klasę, kuri turėtų tą esybę ir paveldėtų iš šios klasės.
	struct string_equal_to {
		template<class L, same_range_char_traits_as<range_char_traits_t<L>> R>
		static constexpr bool operator()(const L & l, const R & r) {
			// size_t tipas, nes compare tikisi tokio tipo parametro.
			const size_t count = std_r::size(l);
			return count == std_r::size(r) &&
				!range_char_traits_t<L>::compare(std_r::data(l), std_r::data(r), count);
		}
	};

}
