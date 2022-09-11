#pragma once

#include "general.hpp"
#include <ostream> // basic_ostream
#include <istream> // basic_istream
#include <concepts> // convertible_to, derived_from
#include <utility> // forward



namespace aa {

	// Kintamojo tipas T&, o ne T&&, nes standarte naudojamas toks tipas operator<< užklojimuose.
	template<class T, class... A>
	concept output_stream = std::derived_from<T, apply_traits_t<std::basic_ostream, T>>
		&& requires(T & t, const A&... a) { { (t << ... << a) } -> std::convertible_to<apply_traits_t<std::basic_ostream, T> &>; };

	template<class T, class... A>
	concept input_stream = std::derived_from<T, apply_traits_t<std::basic_istream, T>>
		&& requires(T & t, A&&... a) { { (t >> ... >> std::forward<A>(a)) } -> std::convertible_to<apply_traits_t<std::basic_istream, T> &>; };

}
