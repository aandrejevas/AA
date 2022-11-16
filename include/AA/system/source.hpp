#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/io.hpp"
#include "../container/fixed_string.hpp"
#include "print.hpp"
#include <cstdint> // uint32_t
#include <cstddef> // size_t
#include <cstdlib> // abort
#include <utility> // forward
#include <ostream> // ostream
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

	source_data(allow_ctad_tag)->source_data<0, 0, "", "">;



	template<source_data D, not_ref_same_as<typename decltype(D)::ostream_type> S, class... A>
	[[gnu::always_inline]] AA_CONSTEXPR void log(S &&s, const A&... args) {
		if constexpr (sizeof...(A))		printl(s, D, ": ", args...);
		else							log<D>(s, "Info logged.");
	}

	template<source_data D, class... A>
		requires ((!output_stream<first_or_void_t<A...>>) && std::same_as<typename decltype(D)::ostream_type, std::ostream>)
	[[gnu::always_inline]] AA_CONSTEXPR void log(const A&... args) {
		log<D>(std::clog, args...);
	}

	template<source_data D, not_ref_same_as<typename decltype(D)::ostream_type> S, class... A>
	[[gnu::always_inline, noreturn]] AA_CONSTEXPR void abort(S &&s, const A&... args) {
		if constexpr (sizeof...(A))		log<D>(s, args...);
		else							log<D>(s, "Program aborted.");
		std::abort();
	}

	template<source_data D, class... A>
		requires ((!output_stream<first_or_void_t<A...>>) && std::same_as<typename decltype(D)::ostream_type, std::ostream>)
	[[gnu::always_inline, noreturn]] AA_CONSTEXPR void abort(const A&... args) {
		abort<D>(std::cerr, args...);
	}

	// TODO: Su C++23 čia galima bus naudoti static operator().
	template<source_data D, bool T = true, class F>
	[[gnu::always_inline]] AA_CONSTEXPR void trace(const bool condition, F &&f) {
		if constexpr (T || !AA_ISDEF_NDEBUG) {
			if (!condition) {
				invoke<D>(std::forward<F>(f));
			}
		}
	}

	// Dėl atributo, naudoti šią funkciją turėtų būti tas pats kaip naudoti macro greitaveikos atžvilgiu.
	template<source_data D, bool T = true, not_ref_same_as<typename decltype(D)::ostream_type> S, class... A>
	[[gnu::always_inline]] AA_CONSTEXPR void assert(const bool condition, S &&s, const A&... args) {
		if constexpr (T || !AA_ISDEF_NDEBUG) {
			if (!condition) {
				if constexpr (sizeof...(A))		abort<D>(s, args...);
				else							abort<D>(s, "Assertion failed.");
			}
		}
	}

	template<source_data D, bool T = true, class... A>
		requires ((!output_stream<first_or_void_t<A...>>) && std::same_as<typename decltype(D)::ostream_type, std::ostream>)
	[[gnu::always_inline]] AA_CONSTEXPR void assert(const bool condition, const A&... args) {
		assert<D, T>(condition, std::cerr, args...);
	}

}
