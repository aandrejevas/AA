#pragma once

#include "general.hpp"
#include <ostream> // ostream
#include <istream> // istream
#include <ios> // ios



namespace aa {

	template<class T>
	using stream_t = traits_type_filled_t<T, std::ios>;

	template<class T>
	using ostream_t = traits_type_filled_t<T, std::ostream>;

	template<class T>
	using istream_t = traits_type_filled_t<T, std::istream>;

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
