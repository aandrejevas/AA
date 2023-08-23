#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/io.hpp"
#include "../container/fixed_string.hpp"
#include "print.hpp"
#include <cstdlib> // exit, EXIT_FAILURE
#include <iostream> // cerr, clog
#include <ostream> // ostream



namespace aa {

	// Galima būtų naudoti source_location, bet ta klasė surenka daugiau duomenų negu reikia.
	// clang kompiliatoriui reikia, kad template parametras būtų pavadintas.
	template<class A>
	consteval auto type_name() {
		return fixed_string<(std::extent_v<std::remove_reference_t<decltype(__PRETTY_FUNCTION__)>>) - 43>{__PRETTY_FUNCTION__ + 41};
	}

	template<class A>
	constexpr fixed_string type_name_v = type_name<A>();

	template<auto A>
	consteval auto literal_name() {
		return fixed_string<(std::extent_v<std::remove_reference_t<decltype(__PRETTY_FUNCTION__)>>) - 51>{__PRETTY_FUNCTION__ + 49};
	}

	template<auto A>
	constexpr fixed_string literal_name_v = literal_name<A>();



	// Klasės reikia, nes source_location klasės negalima naudoti kaip non type template parameter
	// ir taip pat yra truputį keista man, kad minėtos klasės duomenys pasiekiami tik per metodus.
	//
	// fixed_string tipas naudojamas, nes source_location file_name, function_name metodai gražina const char*.
	template<size_t COL, size_t LINE, fixed_string FILE, fixed_string FUNC>
	struct source_data {
		// Naudojamas ostream stream, nes fixed_string galima naudoti tik su tokiu stream, o
		// fixed_string turime naudoti dėl constantų tipo, kuriomis ši klasė inicializuojama.
		// Input/output
		friend constexpr std::ostream &operator<<(std::ostream &s, const source_data &) {
			print(s, FILE, ':', LINE);
			if constexpr (!is_numeric_max(COL)) {
				print(s, ':', COL);
			}
			return print(s, ": `", FUNC, '`');
		}
	};



	// source_location neišeitų naudoti, nes turime naudoti parameter pack.
	// ostream naudojame, nes funkcija turi galėti išspausdinti source_data.
	template<stream_insertable auto D, ref_convertible_to<std::ostream &> S, stream_insertable... A>
	constexpr borrowed_t<S, std::ostream &> log(S &&s, const A&... args) {
		if constexpr (sizeof...(A))		return printl(s, D, ": ", args...);
		else							return log<D>(s, "Info logged.");
	}

	template<stream_insertable auto D, stream_insertable... A>
	constexpr std::ostream &log(const A&... args) {
		return log<D>(std::clog, args...);
	}

	template<stream_insertable auto D, ref_convertible_to<std::ostream &> S, stream_insertable... A>
	[[noreturn]] constexpr void abort(S &&s, const A&... args) {
		if constexpr (sizeof...(A))		log<D>(s, args...);
		else							log<D>(s, "Program aborted.");
		// Netinka abort ar kitos funkcijos, nes gali būti neišspausdintas klaidos pranešimas.
		std::exit(EXIT_FAILURE);
	}

	template<stream_insertable auto D, stream_insertable... A>
	[[noreturn]] constexpr void abort(const A&... args) {
		abort<D>(std::cerr, args...);
	}

	// Neturime assert funkcijų, nes nereikia turėti dviejų kelių, kad pasiekti tą patį. Na ir macros
	// sprendimas geresnis greitaveikos atžvilgiu (funkcijos argumentai būtų įvertinami, gal to būtų įmanoma
	// išvengti naudojant atributą) ir kodo trumpumo (naudotojui tektų funkcijai paduoti source_data klasę).

}
