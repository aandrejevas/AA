#pragma once

#include "../metaprogramming/general.hpp"
#include <concepts> // swap
#include <utility> // forward, tuple_size, tuple_size_v, tuple_element
#include <type_traits> // common_type_t, integral_constant, type_identity



namespace aa {

	template<class T>
	struct swap_pair : pair<T>, protected pair<T *> {
		// Member types
		using value_type = T;

	protected:
		template<size_t I>
		using p_unit_type = typename pair<value_type *>::unit_type<I>;

	public:
		template<size_t I>
		using unit_type = typename pair<value_type>::unit_type<I>;



		// Element access
		template<size_t I>
		AA_CONSTEXPR value_type &get() { return *p_unit_type<I>::value; }

		template<size_t I>
		AA_CONSTEXPR const value_type &get() const { return *p_unit_type<I>::value; }



		// Modifiers
		AA_CONSTEXPR void swap() { std::ranges::swap(p_unit_type<0>::value, p_unit_type<1>::value); }



		// Special member functions
		AA_CONSTEXPR swap_pair()
			: pair<value_type *>{&(unit_type<0>::value), &(unit_type<1>::value)} {}

		template<class T1 = value_type, class T2 = value_type>
		AA_CONSTEXPR swap_pair(T1 &&t1, T2 &&t2)
			: pair<value_type>{std::forward<T1>(t1), std::forward<T2>(t2)},
			pair<value_type *>{&(unit_type<0>::value), &(unit_type<1>::value)} {}
	};

	template<class T1, class T2>
	swap_pair(T1 &&, T2 &&)->swap_pair<std::common_type_t<T1, T2>>;

}



namespace std {

	template<class T>
	struct tuple_size<aa::swap_pair<T>> : std::integral_constant<size_t, 2> {};

	template<size_t I, class T>
		requires (I < std::tuple_size_v<aa::swap_pair<T>>)
	struct tuple_element<I, aa::swap_pair<T>> : std::type_identity<T> {};

}
