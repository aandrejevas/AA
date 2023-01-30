#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/range.hpp"
#include "../metaprogramming/io.hpp"
#include "../container/fixed_string.hpp"
#include "print.hpp"
#include <cstdint> // uint32_t
#include <cstddef> // size_t
#include <cstdlib> // exit, EXIT_FAILURE
#include <utility> // forward
#include <iostream> // cerr, clog
#include <concepts> // same_as
#include <type_traits> // remove_reference_t, extent_v
#undef assert



namespace aa {

	// Galima būtų naudoti source_location, bet ta klasė surenka daugiau duomenų negu reikia.
	// clang kompiliatoriui reikia, kad template parametras būtų pavadintas.
	template<class A>
	AA_CONSTEVAL auto type_name() {
		return fixed_string<(std::extent_v<std::remove_reference_t<decltype(__PRETTY_FUNCTION__)>>) - 43>{__PRETTY_FUNCTION__ + 41};
	}

	template<auto A>
	AA_CONSTEVAL auto literal_name() {
		return fixed_string<(std::extent_v<std::remove_reference_t<decltype(__PRETTY_FUNCTION__)>>) - 51>{__PRETTY_FUNCTION__ + 49};
	}

	template<auto A>
	AA_CONSTEVAL auto to_fixed_string() {
		if constexpr (fixed_string_like<decltype(A)>)	return A;
		else											return literal_name<A>();
	}

	template<auto A>
	using fixed_string_for = decltype(to_fixed_string<A>());



	// Klasės reikia, nes source_location klasės negalima naudoti kaip non type template parameter
	// ir taip pat yra truputį keista man, kad minėtos klasės duomenys pasiekiami tik per metodus.
	template<size_t COL, size_t LINE, basic_fixed_string FILE, basic_fixed_string FUNC>
		requires (std::same_as<typename decltype(FILE)::traits_type, typename decltype(FUNC)::traits_type>)
	struct source_data {
		using ostream_type = typename decltype(FILE)::ostream_type;

		// Naudojamas ostream stream, nes fixed_string galima naudoti tik su tokiu stream, o
		// fixed_string turime naudoti dėl constantų tipo, kuriomis ši klasė inicializuojama.
		// Input/output
		friend AA_CONSTEXPR ostream_type &operator<<(ostream_type &s, const source_data &) {
			print(s, FILE, '(', LINE);
			if constexpr (!is_numeric_max(COL)) {
				print(s, ':', COL);
			}
			return print(s, ") `", FUNC, '`');
		}
	};



	// Nereikia constraints S tipui, nes printl viskuo pasirūpina.
	// D tipas ne bet koks, nes funkcija buvo sukurta dirbti su source_data tipu.
	template<instance_of_twntp<source_data> D, class S, class... A>
	AA_CONSTEXPR void log(S &&s, const A&... args) {
		if constexpr (sizeof...(A))		printl(s, constant<D>(), ": ", args...);
		else							log<D>(s, "Info logged.");
	}

	template<instance_of_twntp<source_data> D, class... A>
		requires (!output_stream<first_or_void_t<A...>>)
	AA_CONSTEXPR void log(const A&... args) {
		log<D>(std::clog, args...);
	}

	template<instance_of_twntp<source_data> D, class S, class... A>
	[[noreturn]] AA_CONSTEXPR void abort(S &&s, const A&... args) {
		if constexpr (sizeof...(A))		log<D>(s, args...);
		else							log<D>(s, "Program aborted.");
		// Netinka abort ar kitos funkcijos, nes gali būti neišspausdintas klaidos pranešimas.
		std::exit(EXIT_FAILURE);
	}

	template<instance_of_twntp<source_data> D, class... A>
		requires (!output_stream<first_or_void_t<A...>>)
	[[noreturn]] AA_CONSTEXPR void abort(const A&... args) {
		abort<D>(std::cerr, args...);
	}

	template<instance_of_twntp<source_data> D, bool T = true, class F>
	AA_CONSTEXPR void trace(const bool condition, F &&f) {
		if constexpr (T || !AA_ISDEF_NDEBUG) {
			if (!condition) {
				invoke<D>(std::forward<F>(f));
			}
		}
	}

	// Dėl atributo, naudoti šią funkciją turėtų būti tas pats kaip naudoti macro greitaveikos atžvilgiu.
	template<instance_of_twntp<source_data> D, bool T = true, class S, class... A>
	AA_CONSTEXPR void assert(const bool condition, S &&s, const A&... args) {
		if constexpr (T || !AA_ISDEF_NDEBUG) {
			if (!condition) {
				if constexpr (sizeof...(A))		abort<D>(s, args...);
				else							abort<D>(s, "Assertion failed.");
			}
		}
	}

	template<instance_of_twntp<source_data> D, bool T = true, class... A>
		requires (!output_stream<first_or_void_t<A...>>)
	AA_CONSTEXPR void assert(const bool condition, const A&... args) {
		assert<D, T>(condition, std::cerr, args...);
	}

}
