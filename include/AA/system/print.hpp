#pragma once

#include "../metaprogramming/general.hpp"
#include <iostream> // cout, cin



namespace aa {

	// Dėl atributo neturėtų nukentėti greitaveika, nes taip tai standartiniai srautai yra visiems pasiekiami
	// ir atrodo nereiktų jų padavinėti per parametrus, bet template argumentai irgi ne alternatyva.
	template<class S, class... A>
	[[gnu::always_inline]] AA_CONSTEXPR void f_print(S &s, const A&... args) {
		(s << ... << args);
	}

	template<class S, class... A>
	[[gnu::always_inline]] AA_CONSTEXPR void f_println(S &s, const A&... args) {
		(s << ... << args) << '\n';
	}

	template<class S, class... A>
	[[gnu::always_inline]] AA_CONSTEXPR void f_read(S &s, A&... args) {
		(s >> ... >> args);
	}



	// Čia neatliekamas perfect forwarding, nes atrodo spausdinimui užtenka const kintamųjų. Žinoma gali atsirasti
	// situacijų, kai tai nėra tiesa, pavyzdžiui norint sekti kiek kartų kintamasis buvo išspausdintas. Bet tokiems
	// atvejams klasė gali būti taip parašyta, kad ji ignoruotų const kvalifikatorių.
	template<class... A>
	[[gnu::always_inline]] AA_CONSTEXPR void print(const A&... args) {
		f_print(std::cout, args...);
	}

	template<class... A>
	[[gnu::always_inline]] AA_CONSTEXPR void println(const A&... args) {
		f_println(std::cout, args...);
	}

	template<class... A>
	[[gnu::always_inline]] AA_CONSTEXPR void read(A&... args) {
		f_read(std::cin, args...);
	}

}
