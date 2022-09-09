#pragma once

#include "general.hpp"
#include <ios> // basic_ios
#include <concepts> // same_as, derived_from
#include <utility> // forward



namespace aa {

	template<class T>
	concept stream = std::derived_from<T, std::basic_ios<typename T::char_type, typename T::traits_type>>;

	// Kintamojo tipas T&, o ne T&&, nes standarte naudojamas toks tipas operator<< užklojimuose.
	template<class T, class... A>
	concept output_stream = stream<T>
		&& requires(T & t, const A&... a) { { (t << ... << a) } -> std::same_as<T &>; };

	template<class T, class... A>
	concept input_stream = stream<T>
		&& requires(T & t, A&&... a) { { (t >> ... >> std::forward<A>(a)) } -> std::same_as<T &>; };

}
