#pragma once

#include "general.hpp"
#include <ostream> // basic_ostream
#include <istream> // basic_istream
#include <concepts> // same_as
#include <utility> // forward
#include <ios> // basic_ios



namespace aa {

	template<class T>
	concept stream_like = remove_ref_derived_from<T, apply_traits_t<std::basic_ios, T>>;



	// Kintamojo tipas T&, o ne T&&, nes standarte naudojamas toks tipas operator<< užklojimuose.
	template<class U, class T>
	concept insertable_into = remove_ref_derived_from<T, apply_traits_t<std::basic_ostream, T>>
		&& requires(apply_traits_t<std::basic_ostream, T> &t, const U & u)
		/**/ { { t << u } -> std::same_as<apply_traits_t<std::basic_ostream, T> &>; };

	template<class U, class T>
	concept extractable_from = remove_ref_derived_from<T, apply_traits_t<std::basic_istream, T>>
		&& requires(apply_traits_t<std::basic_istream, T> &t, U && u)
		/**/ { { t >> std::forward<U>(u) } -> std::same_as<apply_traits_t<std::basic_istream, T> &>; };

	// Fuck dynamic polymorphism. Nors C++ standarto funkcijos (pvz. endl) remiasi juo, yra minusų tokios realizacijos.
	// Pirma reiktų divejų užklojimų funkcijos, kad palaikyti & ir && argumentus. Templates nepadeda išspręsti šios problemos.
	// Antra dinaminis polimorfizmas yra lėtas ir jis neturėtų būti skatinamas.
	template<class T, class... A>
	concept output_stream = remove_ref_derived_from<T, apply_traits_t<std::basic_ostream, T>> && (... && insertable_into<A, T>);

	template<class T, class... A>
	concept input_stream = remove_ref_derived_from<T, apply_traits_t<std::basic_istream, T>> && (... && extractable_from<A, T>);

}
