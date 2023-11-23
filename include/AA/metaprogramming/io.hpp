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

	template<class T>
	concept streambuf_like = ref_convertible_to<T, streambuf_t<T> &>;



	template<class U, class T>
	concept evaluable_by = std::default_initializable<std::remove_reference_t<T>> && invocable_r<T, bool, int, U &>;

	template<class T, class... U>
	concept evaluator_for = !!sizeof...(U) && (... && evaluable_by<U, T &>);



	using char_traits_t = std::char_traits<char>;

}
