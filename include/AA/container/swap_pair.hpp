#pragma once

#include "../metaprogramming/general.hpp"
#include <cstddef> // size_t
#include <concepts> // swap
#include <utility> // forward, tuple_size, tuple_element
#include <type_traits> // common_type_t, type_identity



namespace aa {

	template<class T>
	struct swap_pair : pair<T>, protected pair<T *> {
		// Member types
		using value_type = T;

	protected:
		using b1 = pair<value_type>;
		using b2 = pair<value_type *>;
	public:



		// Element access
		template<size_t I>
		AA_CONSTEXPR value_type &get() { return *b2::template get<I>(); }

		template<size_t I>
		AA_CONSTEXPR const value_type &get() const { return *b2::template get<I>(); }



		// Modifiers
		AA_CONSTEXPR void swap() { std::ranges::swap(b2::template get<0>(), b2::template get<1>()); }



		// Special member functions
		AA_CONSTEXPR swap_pair()
			: b2{&(b1::template get<0>()), &(b1::template get<1>())} {}

		template<class T1 = value_type, class T2 = value_type>
		AA_CONSTEXPR swap_pair(T1 &&t1, T2 &&t2)
			: b1{std::forward<T1>(t1), std::forward<T2>(t2)},
			b2{&(b1::template get<0>()), &(b1::template get<1>())} {}
	};

	template<class T1, class T2>
	swap_pair(T1 &&, T2 &&) -> swap_pair<std::common_type_t<T1, T2>>;

}



namespace std {

	template<class T>
	struct tuple_size<aa::swap_pair<T>> : aa::uz2_identity {};

	template<class T>
	struct tuple_element<0, aa::swap_pair<T>> : std::type_identity<T> {};

	template<class T>
	struct tuple_element<1, aa::swap_pair<T>> : std::type_identity<T> {};

}
