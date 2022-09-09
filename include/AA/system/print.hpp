#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/io.hpp"
#include <ostream> // ostream
#include <istream> // istream
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

	// Netikriname ar į output_stream galima insertinti char, nes žinoma su paprastais
	// tipais gali susidoroti klasė, kuri paveldi iš basic_ostream.
	template<class S, class... A>
		requires (output_stream<S, A...>)
	[[gnu::always_inline]] AA_CONSTEXPR void println(S &s, const A&... args) {
		(s << ... << args) << '\n';
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
		requires (output_stream<std::ostream, A1, A...> && !output_stream<A1>)
	[[gnu::always_inline]] AA_CONSTEXPR void print(const A1 &a1, const A&... args) {
		print(std::cout, a1, args...);
	}

	template<class... A>
		requires (output_stream<std::ostream, A...> && !output_stream<first_or_void_t<A...>>)
	[[gnu::always_inline]] AA_CONSTEXPR void println(const A&... args) {
		println(std::cout, args...);
	}

	template<class A1, class... A>
		requires (input_stream<std::istream, A1, A...> && !input_stream<A1>)
	[[gnu::always_inline]] AA_CONSTEXPR void read(A1 &&a1, A&&... args) {
		read(std::cin, std::forward<A1>(a1), std::forward<A>(args)...);
	}

}
