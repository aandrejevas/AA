#pragma once

#include "general.hpp"
#include <ostream> // basic_ostream
#include <istream> // basic_istream
#include <type_traits> // remove_reference_t, type_identity
#include <concepts> // same_as, derived_from, constructible_from
#include <utility> // forward
#include <ios> // basic_ios



namespace aa {

	template<class T>
	concept stream_like = std::derived_from<T, apply_traits_t<std::basic_ios, T>>;

	template<class T, class... A>
	concept stream_constructible_from = stream_like<T> && std::constructible_from<T, A...>;



	// Kintamojo tipas T&, o ne T&&, nes standarte naudojamas toks tipas operator<< užklojimuose.
	template<class U, class T>
	concept insertable_into = remove_ref_derived_from<T, apply_traits_t<std::basic_ostream, T>>
		&& requires(apply_traits_t<std::basic_ostream, T> &t, const U & u)
		/**/ { { t << u } -> std::same_as<apply_traits_t<std::basic_ostream, T> &>; };

	template<class U, class T>
	concept extractable_from = remove_ref_derived_from<T, apply_traits_t<std::basic_istream, T>>
		&& requires(apply_traits_t<std::basic_istream, T> &t, U && u)
		/**/ { { t >> std::forward<U>(u) } -> std::same_as<apply_traits_t<std::basic_istream, T> &>; };

	// Nors šitie concepts tikrina kažkokį T tipą, funkcijų parametrai visados turėtų būti basic_ostream ar basic_istream tipo.
	// Taip daryti, nes remiamės polimorfizmu ir C++ standarto funkcijos (pvz. endl) taip pat realizuotos.
	template<class T, class... A>
	concept output_stream = remove_ref_derived_from<T, apply_traits_t<std::basic_ostream, T>> && (... && insertable_into<A, T>);

	template<class T, class... A>
	concept input_stream = remove_ref_derived_from<T, apply_traits_t<std::basic_istream, T>> && (... && extractable_from<A, T>);



	template<class T>
	concept uses_stream_type = (requires { typename std::remove_reference_t<T>::stream_type; })
		&& stream_like<typename std::remove_reference_t<T>::stream_type>;

	template<uses_stream_type T>
	struct stream_type_in_use : std::type_identity<typename std::remove_reference_t<T>::stream_type> {};

	template<uses_stream_type T>
	using stream_type_in_use_t = typename stream_type_in_use<T>::type;

}
