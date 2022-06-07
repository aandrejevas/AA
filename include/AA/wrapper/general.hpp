#pragma once

#include "../metaprogramming/general.hpp"
#include <concepts> // totally_ordered_with, equality_comparable_with



namespace aa {

	template<auto R>
	struct less {
		template<std::totally_ordered_with<decltype(R)> L>
		AA_CONSTEXPR bool operator()(const L &l) const { return l < R; }
	};

	template<auto R>
	struct less_equal {
		template<std::totally_ordered_with<decltype(R)> L>
		AA_CONSTEXPR bool operator()(const L &l) const { return l <= R; }
	};

	template<auto R>
	struct greater {
		template<std::totally_ordered_with<decltype(R)> L>
		AA_CONSTEXPR bool operator()(const L &l) const { return l > R; }
	};

	template<auto R>
	struct greater_equal {
		template<std::totally_ordered_with<decltype(R)> L>
		AA_CONSTEXPR bool operator()(const L &l) const { return l >= R; }
	};

	template<auto R>
	struct equal_to {
		template<std::equality_comparable_with<decltype(R)> L>
		AA_CONSTEXPR bool operator()(const L &l) const { return l == R; }
	};

	template<auto R>
	struct not_equal_to {
		template<std::equality_comparable_with<decltype(R)> L>
		AA_CONSTEXPR bool operator()(const L &l) const { return l != R; }
	};



	// Atrodo galima būtų turėti tipą, kuris generalizuotu šią klasę, tai yra tiesiog jis susidėtų iš
	// paduotų tipų. Bet toks tipas nebūtų naudingas, nes užklojimai neveiktų gerai, todėl būtent ir
	// reikia šio overload tipo, kad bent jau veiktų operatoriaus () užklojimai gerai. Tačiau realizacija
	// reikalauja, kad visi paduodami tipai turėtų būtinai tik vieną operatorių (), gal būtų galima realizuoti
	// taip tipą, kad tokio reikalavimo neliktų, bet tokios realizacijos savybės dabar nereikalingos.
	template<functor... T>
	struct overload : T... {
		using T::operator()...;
		using is_transparent = void;
	};

}
