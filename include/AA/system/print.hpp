#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/io.hpp"
#include <iostream> // cout, cin
#include <utility> // forward



namespace aa {

	// Dėl atributo neturėtų nukentėti greitaveika, nes taip tai standartiniai srautai yra visiems pasiekiami
	// ir atrodo nereiktų jų padavinėti per parametrus, bet template argumentai irgi ne alternatyva.
	template<class S, class A1, class... A>
		requires (output_stream<S, A1, A...>)
	[[gnu::always_inline]] AA_CONSTEXPR void print(S &s, const A1 &a1, const A&... args) {
		((s << a1) << ... << args);
	}

	// Tikriname ar į output_stream galima insert'inti visus paduotus tipus, nes tarp jų
	// gali būti tipų, su kuriais negali susidoroti klasė, kuri paveldi iš basic_ostream.
	template<auto L = '\n', class S, class... A>
		requires (output_stream<S, A..., decltype(L)>)
	[[gnu::always_inline]] AA_CONSTEXPR void printl(S &s, const A&... args) {
		(s << ... << args) << L;
	}

	template<class S, class A1, class... A>
		requires (input_stream<S, A1, A...>)
	[[gnu::always_inline]] AA_CONSTEXPR void read(S &s, A1 &&a1, A&&... args) {
		((s >> std::forward<A1>(a1)) >> ... >> std::forward<A>(args));
	}



	// Čia neatliekamas perfect forwarding, nes atrodo spausdinimui užtenka const kintamųjų. Žinoma gali atsirasti
	// situacijų, kai tai nėra tiesa, pavyzdžiui norint sekti kiek kartų kintamasis buvo išspausdintas. Bet tokiems
	// atvejams klasė gali būti taip parašyta, kad ji ignoruotų const kvalifikatorių.
	template<class A1, class... A>
		requires (!output_stream<A1>)
	[[gnu::always_inline]] AA_CONSTEXPR void print(const A1 &a1, const A&... args) {
		print(std::cout, a1, args...);
	}

	// L negali būti stream, bet vis tiek mums reikia tipo dėl atvejo kai A sąrašas tuščias.
	template<auto L = '\n', class... A>
		requires (!output_stream<first_t<A..., decltype(L)>>)
	[[gnu::always_inline]] AA_CONSTEXPR void printl(const A&... args) {
		printl<L>(std::cout, args...);
	}

	template<class A1, class... A>
		requires (!input_stream<A1>)
	[[gnu::always_inline]] AA_CONSTEXPR void read(A1 &&a1, A&&... args) {
		read(std::cin, std::forward<A1>(a1), std::forward<A>(args)...);
	}

}
