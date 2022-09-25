#pragma once

#include "general.hpp"
#include <ostream> // basic_ostream
#include <istream> // basic_istream
#include <type_traits> // remove_reference_t, type_identity
#include <concepts> // convertible_to, derived_from, constructible_from
#include <utility> // forward
#include <ios> // basic_ios



namespace aa {

	template<class T>
	concept stream_like = std::derived_from<T, apply_traits_t<std::basic_ios, T>>;

	template<class T, class... A>
	concept stream_constructible_from = stream_like<T> && std::constructible_from<T, A...>;

	// Kintamojo tipas T&, o ne T&&, nes standarte naudojamas toks tipas operator<< užklojimuose.
	template<class T, class... A>
	concept output_stream = std::derived_from<T, apply_traits_t<std::basic_ostream, T>>
		&& requires(T & t, const A&... a) { { (t << ... << a) } -> std::convertible_to<apply_traits_t<std::basic_ostream, T> &>; };

	template<class T, class... A>
	concept input_stream = std::derived_from<T, apply_traits_t<std::basic_istream, T>>
		&& requires(T & t, A&&... a) { { (t >> ... >> std::forward<A>(a)) } -> std::convertible_to<apply_traits_t<std::basic_istream, T> &>; };



	template<class T>
	concept uses_stream_type = (requires { typename std::remove_reference_t<T>::stream_type; })
		&& stream_like<typename std::remove_reference_t<T>::stream_type>;

	template<uses_stream_type T>
	struct stream_type_in_use : std::type_identity<typename std::remove_reference_t<T>::stream_type> {};

	template<uses_stream_type T>
	using stream_type_in_use_t = stream_type_in_use<T>::type;

}
