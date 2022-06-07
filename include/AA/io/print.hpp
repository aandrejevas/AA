#pragma once

#include <iostream> // cout



namespace aa {

	// Čia neatliekamas perfect forwarding, nes atrodo spausdinimui užtenka const kintamųjų. Žinoma gali atsirasti
	// situacijų, kai tai nėra tiesa, pavyzdžiui norint sekti kiek kartų kintamasis buvo išspausdintas. Bet tokiems
	// atvejams klasė gali būti taip parašyta, kad ji ignoruotų const kvalifikatorių.
	template<class... A>
	AA_CONSTEXPR void print(const A&... args) {
		(std::cout << ... << args);
	}

	template<class... A>
	AA_CONSTEXPR void println(const A&... args) {
		(std::cout << ... << args) << '\n';
	}

	template<class S, class... A>
	AA_CONSTEXPR void fprint(S &s, const A&... args) {
		(s << ... << args);
	}

	template<class S, class... A>
	AA_CONSTEXPR void fprintln(S &s, const A&... args) {
		(s << ... << args) << '\n';
	}

}
