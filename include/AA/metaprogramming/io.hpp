#pragma once

#include "general.hpp"
#include <ostream> // ostream, basic_ostream
#include <istream> // istream, basic_istream
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

	template<class U, class T = std::ostream>
	concept stream_insertable = requires(ostream_t<T> &os, const U &u) {
		{ os << u } -> std::same_as<ostream_t<T> &>;
	};

	// https://en.cppreference.com/w/cpp/ranges/basic_istream_view
	template<class U, class T = std::istream>
	concept stream_extractable = requires(istream_t<T> &is, U &u) {
		{ is >> u } -> std::same_as<istream_t<T> &>;
	};

}
