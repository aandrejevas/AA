#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/io.hpp"
#include "../algorithm/init.hpp"
#include <iostream> // cout, cin
#include <utility> // forward
#include <ostream> // basic_ostream, ostream, wostream, endl, flush
#include <istream> // basic_istream, istream
#include <type_traits> // remove_const_t



namespace aa {

	// Dėl atributo neturėtų nukentėti greitaveika, nes taip tai standartiniai srautai yra visiems pasiekiami
	// ir atrodo nereiktų jų padavinėti per parametrus, bet template argumentai irgi ne alternatyva.
	template<class T, class A1, class... A>
		requires (output_stream<T, A1, A...>)
	AA_CONSTEXPR apply_traits_t<std::basic_ostream, T> &print(T &&s, const A1 &a1, const A&... args) {
		return ((s << a1) << ... << args);
	}

	// Tikriname ar į output_stream galima insert'inti visus paduotus tipus, nes tarp jų
	// gali būti tipų, su kuriais negali susidoroti klasė, kuri paveldi iš basic_ostream.
	template<auto L = '\n', class T, class... A>
		requires (output_stream<T, A..., decltype(L)>)
	AA_CONSTEXPR apply_traits_t<std::basic_ostream, T> &printl(T &&s, const A&... args) {
		return (s << ... << args) << L;
	}

	template<class T, class A1, class... A>
		requires (input_stream<T, A1, A...>)
	AA_CONSTEXPR apply_traits_t<std::basic_istream, T> &read(T &&s, A1 &&a1, A&&... args) {
		return ((s >> std::forward<A1>(a1)) >> ... >> std::forward<A>(args));
	}

	template<class T, class S>
	AA_CONSTEXPR std::remove_const_t<T> read(S &&s) {
		return make_with_invocable<T>([&](std::remove_const_t<T> &t) -> void { read(s, t); });
	}



	// Čia neatliekamas perfect forwarding, nes atrodo spausdinimui užtenka const kintamųjų. Žinoma gali atsirasti
	// situacijų, kai tai nėra tiesa, pavyzdžiui norint sekti kiek kartų kintamasis buvo išspausdintas. Bet tokiems
	// atvejams klasė gali būti taip parašyta, kad ji ignoruotų const kvalifikatorių.
	template<class A1, class... A>
		requires (!output_stream<A1>)
	AA_CONSTEXPR std::ostream &print(const A1 &a1, const A&... args) {
		return print(std::cout, a1, args...);
	}

	// L negali būti stream, bet vis tiek mums reikia tipo dėl atvejo kai A sąrašas tuščias.
	template<auto L = '\n', class... A>
		requires (!output_stream<first_t<A..., decltype(L)>>)
	AA_CONSTEXPR std::ostream &printl(const A&... args) {
		return printl<L>(std::cout, args...);
	}

	template<class A1, class... A>
		requires (!input_stream<A1>)
	AA_CONSTEXPR std::istream &read(A1 &&a1, A&&... args) {
		return read(std::cin, std::forward<A1>(a1), std::forward<A>(args)...);
	}

	template<class T>
	AA_CONSTEXPR std::remove_const_t<T> read() {
		return read<T>(std::cin);
	}



	AA_CONSTEXPR std::ostream &endl(std::ostream &os) { return std::endl(os); }
	AA_CONSTEXPR std::wostream &wendl(std::wostream &os) { return std::endl(os); }

	AA_CONSTEXPR std::ostream &flush(std::ostream &os) { return std::flush(os); }
	AA_CONSTEXPR std::wostream &wflush(std::wostream &os) { return std::flush(os); }

}
