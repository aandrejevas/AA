#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/io.hpp"
#include <iostream> // cout, cin
#include <utility> // forward
#include <ostream> // basic_ostream, ostream, wostream, endl, flush
#include <istream> // basic_istream, istream



namespace aa {

	// Dėl atributo neturėtų nukentėti greitaveika, nes taip tai standartiniai srautai yra visiems pasiekiami
	// ir atrodo nereiktų jų padavinėti per parametrus, bet template argumentai irgi ne alternatyva.
	template<class C, char_traits_for<C> T, class A1, class... A>
		requires (output_stream<std::basic_ostream<C, T>, A1, A...>)
	[[gnu::always_inline]] AA_CONSTEXPR std::basic_ostream<C, T> &print(std::basic_ostream<C, T> &s, const A1 &a1, const A&... args) {
		return ((s << a1) << ... << args);
	}

	// Tikriname ar į output_stream galima insert'inti visus paduotus tipus, nes tarp jų
	// gali būti tipų, su kuriais negali susidoroti klasė, kuri paveldi iš basic_ostream.
	template<auto L = '\n', class C, char_traits_for<C> T, class... A>
		requires (output_stream<std::basic_ostream<C, T>, A..., decltype(L)>)
	[[gnu::always_inline]] AA_CONSTEXPR std::basic_ostream<C, T> &printl(std::basic_ostream<C, T> &s, const A&... args) {
		return (s << ... << args) << L;
	}

	template<class C, char_traits_for<C> T, class A1, class... A>
		requires (input_stream<std::basic_istream<C, T>, A1, A...>)
	[[gnu::always_inline]] AA_CONSTEXPR std::basic_istream<C, T> &read(std::basic_istream<C, T> &s, A1 &&a1, A&&... args) {
		return ((s >> std::forward<A1>(a1)) >> ... >> std::forward<A>(args));
	}



	// Čia neatliekamas perfect forwarding, nes atrodo spausdinimui užtenka const kintamųjų. Žinoma gali atsirasti
	// situacijų, kai tai nėra tiesa, pavyzdžiui norint sekti kiek kartų kintamasis buvo išspausdintas. Bet tokiems
	// atvejams klasė gali būti taip parašyta, kad ji ignoruotų const kvalifikatorių.
	template<class A1, class... A>
		requires (!output_stream<A1>)
	[[gnu::always_inline]] AA_CONSTEXPR std::ostream &print(const A1 &a1, const A&... args) {
		return print(std::cout, a1, args...);
	}

	// L negali būti stream, bet vis tiek mums reikia tipo dėl atvejo kai A sąrašas tuščias.
	template<auto L = '\n', class... A>
		requires (!output_stream<first_t<A..., decltype(L)>>)
	[[gnu::always_inline]] AA_CONSTEXPR std::ostream &printl(const A&... args) {
		return printl<L>(std::cout, args...);
	}

	template<class A1, class... A>
		requires (!input_stream<A1>)
	[[gnu::always_inline]] AA_CONSTEXPR std::istream &read(A1 &&a1, A&&... args) {
		return read(std::cin, std::forward<A1>(a1), std::forward<A>(args)...);
	}



	AA_CONSTEXPR std::ostream &endl(std::ostream &os) { return std::endl(os); }
	AA_CONSTEXPR std::wostream &wendl(std::wostream &os) { return std::endl(os); }

	AA_CONSTEXPR std::ostream &flush(std::ostream &os) { return std::flush(os); }
	AA_CONSTEXPR std::wostream &wflush(std::wostream &os) { return std::flush(os); }

}
