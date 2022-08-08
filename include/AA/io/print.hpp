#pragma once

#include "../metaprogramming/general.hpp"
#include <istream> // istream
#include <ostream> // ostream
#include <iostream> // cout, cerr, clog, cin



namespace aa {

	// Negalime kode tiesiog naudoti pavyzdžiui cout, nes gali prireikti pakeisti srautą. Negalime naudoti to
	// pačio srauto ir tiesiog pastoviai keisti streambuf objektus, nes tai lėta alternatyva. Yra neleidžiama
	// naudoti cout kaip template argumento. Negalime padavinėti srautų kaip funkcijų argumentų, nes standartiniai
	// srautai yra visiems pasiekiami tiesiog todėl tai būtų lėta alternatyva. Tai reiškia, kad reikia turėti
	// kažką kas suteiktų prieigą prie srautų ir naujai sukurti srautai turės būti globalūs objektai.
	AA_CONSTEXPR std::ostream &get_cout() {
		return std::cout;
	}

	AA_CONSTEXPR std::ostream &get_cerr() {
		return std::cerr;
	}

	AA_CONSTEXPR std::ostream &get_clog() {
		return std::clog;
	}

	AA_CONSTEXPR std::istream &get_cin() {
		return std::cin;
	}



	// Čia neatliekamas perfect forwarding, nes atrodo spausdinimui užtenka const kintamųjų. Žinoma gali atsirasti
	// situacijų, kai tai nėra tiesa, pavyzdžiui norint sekti kiek kartų kintamasis buvo išspausdintas. Bet tokiems
	// atvejams klasė gali būti taip parašyta, kad ji ignoruotų const kvalifikatorių.
	template<auto S = &get_cout, class... A>
		requires (function_pointer<decltype(S)>)
	AA_CONSTEXPR void print(const A&... args) {
		(S() << ... << args);
	}

	template<auto S = &get_cout, class... A>
		requires (function_pointer<decltype(S)>)
	AA_CONSTEXPR void println(const A&... args) {
		(S() << ... << args) << '\n';
	}

	template<auto S = &get_cin, class... A>
		requires (function_pointer<decltype(S)>)
	AA_CONSTEXPR void read(A&... args) {
		(S() >> ... >> args);
	}

}
