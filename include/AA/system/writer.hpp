#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/io.hpp"
#include <ranges> // input_range, range_value_t
#include <functional> // invoke
#include <concepts> // invocable
#include <ostream> // basic_ostream
#include <algorithm> // for_each
#include <utility> // forward, as_const, tuple_element_t
#include <iomanip> // setw



namespace aa {

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
	template<std::ranges::input_range R, class F>
	struct range_writer {
		[[no_unique_address]] R range;
		[[no_unique_address]] F fun;

		// Čia writeris galėtų ateiti ir ne const, bet tada reiktų arba kelių užklojimų funkcijos arba pridėti dar vieną
		// template argumentą, abu sprendimai labai nepatogūs. Na iš principo tai yra keista į spausdinimo funkciją
		// paduoti ne const kintamąjį. Išlieka galimybė pvz. turėti mutable lambdas ir keisti range elementus.
		template<class C, class T>
			requires (std::invocable<F &, std::basic_ostream<C, T> &, const std::ranges::range_value_t<R> &>)
		friend AA_CONSTEXPR std::basic_ostream<C, T> &operator<<(std::basic_ostream<C, T> &s, const range_writer<R, F> &w) {
			std::ranges::for_each(w.range, [&s, &w](const std::ranges::range_value_t<R> &element) -> void {
				std::invoke(w.fun, s, element);
			});
			return s;
		}
	};

	template<class E, class F>
	struct writer {
		[[no_unique_address]] E element;
		[[no_unique_address]] F fun;

		template<class C, class T>
			requires (std::invocable<F &, std::basic_ostream<C, T> &, const E &>)
		friend AA_CONSTEXPR std::basic_ostream<C, T> &operator<<(std::basic_ostream<C, T> &s, const writer<E, F> &w) {
			std::invoke(w.fun, s, std::as_const(w.element));
			return s;
		}
	};



	struct identity_inserter {
		template<class S, class T>
			requires (output_stream<S, T>)
		AA_CONSTEXPR void operator()(S &s, const T &t) const { s << t; }
	};

	template<char D = ' '>
	struct delim_inserter {
		template<class S, class T>
			requires (output_stream<S, T>)
		AA_CONSTEXPR void operator()(S &s, const T &t) const { s << t << D; }
	};

	delim_inserter()->delim_inserter<>;

	template<int N>
	struct width_inserter {
		template<class S, class T>
			requires (output_stream<S, T>)
		AA_CONSTEXPR void operator()(S &s, const T &t) const { s << std::setw(N) << t; }
	};

	template<char D1 = ':', char D2 = ' '>
	struct pair_inserter {
		template<class S, pair_like T>
			requires (output_stream<S, std::tuple_element_t<0, T>, std::tuple_element_t<1, T>>)
		AA_CONSTEXPR void operator()(S &s, const T &t) const {
			const auto &[m1, m2] = t;
			s << m1 << D1 << m2 << D2;
		}
	};

	pair_inserter()->pair_inserter<>;



	template<class R, class F = delim_inserter<>>
	AA_CONSTEXPR range_writer<R, F> make_range_writer(R &&r, F &&f = {}) {
		return {std::forward<R>(r), std::forward<F>(f)};
	}

	template<class E, class F = delim_inserter<>>
	AA_CONSTEXPR writer<E, F> make_writer(E &&e, F &&f = {}) {
		return {std::forward<E>(e), std::forward<F>(f)};
	}

}
