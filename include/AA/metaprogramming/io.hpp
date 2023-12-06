#pragma once

#include "general.hpp"
#include <string_view> // basic_string_view
#include <streambuf> // basic_streambuf
#include <string> // char_traits



namespace aa {

	template<class T>
	using string_view_t = std::basic_string_view<char_type_in_use_t<traits_type_in_use_t<T>>, traits_type_in_use_t<T>>;

	template<class T>
	using streambuf_t = std::basic_streambuf<char_type_in_use_t<traits_type_in_use_t<T>>, traits_type_in_use_t<T>>;



	template<class U, class T>
	concept evaluable_by = invocable_r<T, bool, int, U &>;

	template<class T, class U>
	concept evaluator_for = evaluable_by<U, T &>;

	template<class U, class T, size_t N = numeric_max>
	concept tuple_evaluable_by = apply<U, N>([]<size_t... I> -> bool { return (... && evaluable_by<std::tuple_element_t<I, U>, T>); });

	template<class T, class U, size_t N = numeric_max>
	concept evaluator_for_tuple = tuple_evaluable_by<U, T &, N>;



	using char_traits_t = std::char_traits<char>;

}
