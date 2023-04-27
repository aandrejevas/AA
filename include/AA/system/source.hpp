#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/range.hpp"
#include "../metaprogramming/io.hpp"
#include "../container/fixed_string.hpp"
#include "print.hpp"
#include <cstdlib> // exit, EXIT_FAILURE
#include <iostream> // cerr, clog



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
		if constexpr (fixed_string_like<const_t<A>>)	return A;
		else											return literal_name<A>();
	}

	template<auto A>
	using fixed_string_for = const_t<to_fixed_string<A>()>;



	// Klasės reikia, nes source_location klasės negalima naudoti kaip non type template parameter
	// ir taip pat yra truputį keista man, kad minėtos klasės duomenys pasiekiami tik per metodus.
	template<size_t COL, size_t LINE, basic_fixed_string FILE, basic_fixed_string FUNC>
		requires (std::same_as<typename const_t<FILE>::traits_type, typename const_t<FUNC>::traits_type>)
	struct source_data {
		using ostream_type = typename const_t<FILE>::ostream_type;

		// Naudojamas ostream stream, nes fixed_string galima naudoti tik su tokiu stream, o
		// fixed_string turime naudoti dėl constantų tipo, kuriomis ši klasė inicializuojama.
		// Input/output
		friend AA_CONSTEXPR ostream_type &operator<<(ostream_type &s, const source_data &) {
			print(s, FILE, ':', LINE);
			if constexpr (!is_numeric_max(COL)) {
				print(s, ':', COL);
			}
			return print(s, ": `", FUNC, '`');
		}
	};



	// S constrained, nes kitaip gali būti paduotas ne stream tipas.
	// D tipas ne bet koks, nes funkcija buvo sukurta dirbti su source_data tipu.
	template<instance_of_twnttp<source_data> D, output_stream S, class... A>
	AA_CONSTEXPR void log(S &&s, const A&... args) {
		if constexpr (sizeof...(A))		printl(s, constant_v<D>, ": ", args...);
		else							log<D>(s, "Info logged.");
	}

	template<instance_of_twnttp<source_data> D, class... A>
		requires (!output_stream<first_or_void_t<A...>>)
	AA_CONSTEXPR void log(const A&... args) {
		log<D>(std::clog, args...);
	}

	template<instance_of_twnttp<source_data> D, output_stream S, class... A>
	[[noreturn]] AA_CONSTEXPR void abort(S &&s, const A&... args) {
		if constexpr (sizeof...(A))		log<D>(s, args...);
		else							log<D>(s, "Program aborted.");
		// Netinka abort ar kitos funkcijos, nes gali būti neišspausdintas klaidos pranešimas.
		std::exit(EXIT_FAILURE);
	}

	template<instance_of_twnttp<source_data> D, class... A>
		requires (!output_stream<first_or_void_t<A...>>)
	[[noreturn]] AA_CONSTEXPR void abort(const A&... args) {
		abort<D>(std::cerr, args...);
	}

	// Neturime assert funkcijų, nes nereikia turėti dviejų kelių, kad pasiekti tą patį. Na ir macros
	// sprendimas geresnis greitaveikos atžvilgiu (funkcijos argumentai būtų įvertinami, gal to būtų įmanoma
	// išvengti naudojant atributą) ir kodo trumpumo (naudotojui tektų funkcijai paduoti source_data klasę).

}
