#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/io.hpp"
#include "../container/fixed_string.hpp"
#include "print.hpp"
#include <cstdlib> // exit, EXIT_FAILURE
#include <iostream> // cerr, clog
#include <ostream> // ostream
#include <format> // format_string, formatter, format_parse_context, format_context, format_to



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
	template<size_t LINE, fixed_string FILE, fixed_string FUNC>
	struct source_data {
		// Naudojamas ostream stream, nes fixed_string galima naudoti tik su tokiu stream, o
		// fixed_string turime naudoti dėl constantų tipo, kuriomis ši klasė inicializuojama.
		// Input/output
	};



	// source_location neišeitų naudoti, nes turime naudoti parameter pack.
	// ostream naudojame, nes funkcija turi galėti išspausdinti source_data.
	template<auto D, ref_convertible_to<std::ostream &> S, class... A>
	constexpr void log(S &&s, const std::format_string<const A&...> &fmt = "Info logged.", const A&... args) {
		print(s, "{}: ", D);
		printl(s, fmt, args...);
	}

	template<auto D, class... A>
	constexpr void log(const std::format_string<const A&...> &fmt = "Info logged.", const A&... args) {
		log<D>(std::clog, fmt, args...);
	}

	template<auto D, ref_convertible_to<std::ostream &> S, class... A>
	[[noreturn]] constexpr void abort(S &&s, const std::format_string<const A&...> &fmt = "Program aborted.", const A&... args) {
		log<D>(s, fmt, args...);
		// Netinka abort ar kitos funkcijos, nes gali būti neišspausdintas klaidos pranešimas.
		std::exit(EXIT_FAILURE);
	}

	template<auto D, class... A>
	[[noreturn]] constexpr void abort(const std::format_string<const A&...> &fmt = "Program aborted.", const A&... args) {
		abort<D>(std::cerr, fmt, args...);
	}

	// Neturime assert funkcijų, nes nereikia turėti dviejų kelių, kad pasiekti tą patį. Na ir macros
	// sprendimas geresnis greitaveikos atžvilgiu (funkcijos argumentai būtų įvertinami, gal to būtų įmanoma
	// išvengti naudojant atributą) ir kodo trumpumo (naudotojui tektų funkcijai paduoti source_data klasę).

}



// https://www.cppstories.com/2022/custom-stdformat-cpp20/
template<size_t LINE, aa::fixed_string FILE, aa::fixed_string FUNC>
struct std::formatter<aa::source_data<LINE, FILE, FUNC>> {
	constexpr aa::iterator_in_use_t<std::format_parse_context> parse(const std::format_parse_context &ctx) const {
		return ctx.begin();
	}

	constexpr aa::iterator_in_use_t<std::format_context> format(const aa::source_data<LINE, FILE, FUNC> &, std::format_context &ctx) const {
		return std::format_to(ctx.out(), "{}:{}: `{}`", FILE, LINE, FUNC);
	}
};
