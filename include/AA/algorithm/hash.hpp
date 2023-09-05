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
		static constexpr size_t operator()(const T &t) {
			return default_value_v<H<T>>(t);
		}

		static consteval size_t min() { return numeric_min; }
		static consteval size_t max() { return numeric_max; }

		using is_transparent = void;
	};

	generic_hash() -> generic_hash<>;



	template<size_t N, template<class> class H = std::hash>
	struct mod_generic_hash {
		template<hashable_by_template<H> T>
		static constexpr size_t operator()(const T &t) {
			return remainder<N>(default_value_v<H<T>>(t));
		}

		static consteval size_t min() { return 0; }
		static consteval size_t max() { return N - 1; }

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

		template<same_range_char_traits_as<traits_type> T, class F>
		static constexpr void operator()(const T &t, F &&f) {
			// size_t tipas, nes reikės lyginti su size_t tipo kintamuoju.
			const size_t count = std::ranges::size(t);
			if (!(... || ((count == std::tuple_size_v<const_t<A>>) && apply<const_t<A>>([&]<size_t... I> -> bool {
				static constexpr auto V = A;
				return (... && traits_type::eq(std::ranges::data(t)[I], const_v<getter_v<I>(V)>)) ?
					(invoke<pack_index_v<V, A...>>(std::forward<F>(f)), true) : false;
			}))))
				invoke<max()>(std::forward<F>(f));
		}

		static consteval size_t min() { return 0; }
		static consteval size_t max() { return sizeof...(A); }
	};

	template<auto... A>
	using literal_name_perfect_hash = string_perfect_hash<literal_name_v<A>...>;

	template<size_t N>
	using sequence_perfect_hash = const_t<apply<N>([]<size_t... I> -> literal_name_perfect_hash<I...> { return default_value; })>;

}
