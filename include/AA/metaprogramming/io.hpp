#pragma once

#include "general.hpp"
#include <ostream> // basic_ostream
#include <istream> // basic_istream
#include <concepts> // same_as, derived_from
#include <utility> // forward



namespace aa {

	// Kintamojo tipas T&, o ne T&&, nes standarte naudojamas toks tipas operator<< užklojimuose.
	template<class T, class... A>
	concept output_stream = std::derived_from<T, std::basic_ostream<typename T::char_type, typename T::traits_type>>
		&& requires(T & t, const A&... a) { { (t << ... << a) } -> std::same_as<T &>; };

	template<class T, class... A>
	concept input_stream = std::derived_from<T, std::basic_istream<typename T::char_type, typename T::traits_type>>
		&& requires(T & t, A&&... a) { { (t >> ... >> std::forward<A>(a)) } -> std::same_as<T &>; };

}
