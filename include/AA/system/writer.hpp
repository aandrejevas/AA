#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/io.hpp"
#include "print.hpp"
#include <ranges> // input_range, range_value_t
#include <functional> // invoke
#include <concepts> // invocable
#include <ostream> // basic_ostream
#include <algorithm> // for_each
#include <utility> // as_const
#include <iomanip> // setw



namespace aa {

	struct identity_inserter {
		template<class C, char_traits_for<C> T, class U>
		AA_CONSTEXPR void operator()(std::basic_ostream<C, T> &s, const U &u) const { print(s, u); }
	};

	template<auto D = ' '>
	struct delim_inserter {
		template<class C, char_traits_for<C> T, class U>
		AA_CONSTEXPR void operator()(std::basic_ostream<C, T> &s, const U &u) const { print(s, u, D); }
	};

	delim_inserter()->delim_inserter<>;

	template<int N>
	struct width_inserter {
		template<class C, char_traits_for<C> T, class U>
		AA_CONSTEXPR void operator()(std::basic_ostream<C, T> &s, const U &u) const { print(s, std::setw(N), u); }
	};

	template<auto D1 = ':', auto D2 = ' '>
	struct pair_inserter {
		template<class C, char_traits_for<C> T, tuple2_like U>
		AA_CONSTEXPR void operator()(std::basic_ostream<C, T> &s, const U &u) const {
			print(s, get_0(u), D1, get_1(u), D2);
		}
	};

	pair_inserter()->pair_inserter<>;



	// Anksčiau turėjau klasę, kurią padavinėdavau naudotojui, ji savyje turėjo stream reference ir rodyklę į esamą elementą,
	// naudotojas galėjo kviesti tos klasės operator() su norimais spausdinti argumentais. Problema, kad visiškai vieną dieną
	// nebemačiau prasmės tokios konstrukcijos, kiek prisimenu esmė buvo turėti galimybę spausdinti visus argumentus padavus
	// juos į metodą, bet tai labai beprasmiška. Taip pat lėta, nes reikėjo kiekvieną iteraciją gauti adresą elemento.
	//
	// Mes galėtume tikrinti ar F tipas išviso turi galimybes būti iškviečiamas (t. y galėtume tikrinti ar jis turi
	// operator(), arba yra function pointer ir t. t.), bet nusprendžiau, kad neapsimoka to daryti, nes net patirktinus
	// tai, vis tiek galėtų būti paduotas toks tipas, su kuriuo negalėtume naudoti operator<<.
	//
	// Nors į for_each galime paduoti dar projection, nelaikome papildomo kintamojo struktūroje ir jo nepaduodame į for_each, nes
	// tą patį efektą galima pasiekti padavus tinkamą range, tą galima padaryti pavyzdžiui pasinaudojus std::views::transform.
	template<std::ranges::input_range R, class F = delim_inserter<>>
	struct range_writer {
		[[no_unique_address]] R range;
		[[no_unique_address]] F fun = {};

		// Čia writeris galėtų ateiti ir ne const, bet tada reiktų arba kelių užklojimų funkcijos arba pridėti dar vieną
		// template argumentą, abu sprendimai labai nepatogūs. Na iš principo tai yra keista į spausdinimo funkciją
		// paduoti ne const kintamąjį. Išlieka galimybė pvz. turėti mutable lambdas ir keisti range elementus.
		template<class C, char_traits_for<C> T>
			requires (std::invocable<F &, std::basic_ostream<C, T> &, const std::ranges::range_value_t<R> &>)
		friend AA_CONSTEXPR std::basic_ostream<C, T> &operator<<(std::basic_ostream<C, T> &s, const range_writer<R, F> &w) {
			std::ranges::for_each(w.range, [&s, &w](const std::ranges::range_value_t<R> &element) -> void {
				std::invoke(w.fun, s, element);
			});
			return s;
		}
	};

	template<class R, class F = delim_inserter<>>
	range_writer(R &&, F && = {})->range_writer<R, F>;



	template<class E, class F = delim_inserter<>>
	struct writer {
		[[no_unique_address]] E element;
		[[no_unique_address]] F fun = {};

		template<class C, char_traits_for<C> T>
			requires (std::invocable<F &, std::basic_ostream<C, T> &, const E &>)
		friend AA_CONSTEXPR std::basic_ostream<C, T> &operator<<(std::basic_ostream<C, T> &s, const writer<E, F> &w) {
			std::invoke(w.fun, s, std::as_const(w.element));
			return s;
		}
	};

	template<class E, class F = delim_inserter<>>
	writer(E &&, F && = {})->writer<E, F>;

}
