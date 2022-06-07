#pragma once

#include "../metaprogramming/general.hpp"
#include <cstddef> // size_t
#include <concepts> // invocable
#include <functional> // invoke



namespace aa {

	// e galėtų būti bet kokio integral tipo kintamasis, bet tada funkcijos naudojimas taptų nepatogus.
	// Taip pat, kad ir koks e tipas būtų, vis tiek kažkokią reikšmę reiktų įkopijuoti į kintamąjį e todėl
	// geriau jau naudoti size_t tipą, nes jį yra priimta naudoti kaip ciklo indekso kintamajį.
	//
	// Yra įmanoma naudojantis parameter pack ir fold expression kalbos sąvybėmis padaryti iteravimą, bet
	// parameter pack negali savyje turėti daugiau nei 1024 elementus tai ir iteracijų daugiau nei tiek negalėtų
	// būti, tai nurodo, kad šios kalbos sąvybės nėra skirtos tokioms reikmėms. Išties toks iteravimas greitesnis,
	// nes nereiktų tikrinti kiekvieną iteraciją ar pasibaigė ciklas, bet tas pagreitėjimas nebūtų reikšmingas.
	//
	// Jei reikia indekso, tada jau reikia naudoti nebe šią funkciją, o pavyzdžiui iota_view su for_each.
	template<class F, class... A>
		requires (std::invocable<F &, A&...>)
	AA_CONSTEXPR void repeat(size_t e, F &&f, A&&... args) {
		do {
			std::invoke(f, args...);
			if (e != 1) --e; else return;
		} while (true);
	}

}
