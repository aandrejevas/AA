#pragma once

#include "../metaprogramming/general.hpp"
#include <cstddef> // size_t
#include <utility> // tuple_size, tuple_size_v, tuple_element
#include <type_traits> // integral_constant, type_identity, index_sequence, index_sequence_for



namespace aa {

	template<size_t I, class T>
	struct tuple_unit {
		// Member types
		using value_type = T;



		// Member objects
		value_type value;
	};



	template<class, class...>
	struct tuple_base;

	template<size_t... I, class... T>
	struct tuple_base<std::index_sequence<I...>, T...> : tuple_unit<I, T>... {};



	// https://ldionne.com/2015/11/29/efficient-parameter-pack-indexing/
	template<size_t I, class... T>
	struct pack_element : decltype(([]<class U>(const tuple_unit<I, U>) consteval ->
		std::type_identity<U> { return {}; })(tuple_base<std::index_sequence_for<T...>, T...>{})) {};

	template<size_t I, class... T>
	using pack_element_t = typename pack_element<I, T...>::type;



	// https://danlark.org/2020/04/13/why-is-stdpair-broken/
	template<class... T>
	struct tuple : tuple_base<std::index_sequence_for<T...>, T...> {
		// Member types
		template<size_t I>
		using value_type = pack_element_t<I, T...>;

		template<size_t I>
		using unit_type = tuple_unit<I, value_type<I>>;



		// Element access
		template<size_t I>
			requires (I < sizeof...(T))
		AA_CONSTEXPR value_type<I> &get() { return unit_type<I>::value; }

		template<size_t I>
			requires (I < sizeof...(T))
		AA_CONSTEXPR const value_type<I> &get() const { return unit_type<I>::value; }
	};

	template<class... T>
	tuple(T&&...)->tuple<T...>;



	template<class T1, class T2 = T1>
	using pair = tuple<T1, T2>;

}



namespace std {

	template<class... T>
	struct tuple_size<aa::tuple<T...>> : std::integral_constant<size_t, sizeof...(T)> {};

	template<size_t I, class... T>
		requires (I < std::tuple_size_v<aa::tuple<T...>>)
	struct tuple_element<I, aa::tuple<T...>> : aa::pack_element<I, T...> {};

}
