#pragma once

#include "general.hpp"
#include <ostream> // basic_ostream
#include <istream> // basic_istream
#include <ios> // basic_ios
#include <string_view> // basic_string_view
#include <streambuf> // basic_streambuf
#include <format> // basic_format_string



namespace aa {

	template<class T, class... A>
	using format_string_t = std::basic_format_string<char_type_in_use_t<traits_type_in_use_t<T>>, std::type_identity_t<A>...>;

	template<class T>
	using string_view_t = std::basic_string_view<char_type_in_use_t<traits_type_in_use_t<T>>, traits_type_in_use_t<T>>;

	template<class T>
	using streambuf_t = std::basic_streambuf<char_type_in_use_t<traits_type_in_use_t<T>>, traits_type_in_use_t<T>>;

	template<class T>
	using stream_t = std::basic_ios<char_type_in_use_t<traits_type_in_use_t<T>>, traits_type_in_use_t<T>>;

	template<class T>
	using ostream_t = std::basic_ostream<char_type_in_use_t<traits_type_in_use_t<T>>, traits_type_in_use_t<T>>;

	template<class T>
	using istream_t = std::basic_istream<char_type_in_use_t<traits_type_in_use_t<T>>, traits_type_in_use_t<T>>;

	template<class T>
	concept stream_like = ref_convertible_to<T, stream_t<T> &>;

	template<class T>
	concept ostream_like = ref_convertible_to<T, ostream_t<T> &>;

	template<class T>
	concept istream_like = ref_convertible_to<T, istream_t<T> &>;

}
