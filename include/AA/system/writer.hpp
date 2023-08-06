#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/io.hpp"
#include "print.hpp"
#include <ranges> // input_range, range_value_t
#include <algorithm> // for_each
#include <iomanip> // setw



namespace aa {

	struct tuple_inserter {
		template<ostream_like S, tuple_like U>
		static AA_CONSTEXPR void operator()(S &&s, const U &u) {
			if constexpr (std::tuple_size_v<U>) {
				apply<(std::tuple_size_v<U>) - 1>([&]<size_t... I> -> void {
					print('{');
					(print(s, default_value_v<getter<I>>(u), ", "), ...);
					print(s, default_value_v<getter<sizeof...(I)>>(u), '}');
				});
			} else {
				print("{}");
			}
		}
	};

	// Nors čia ant S tipo galėtų nebūti constraint uždėtas, jis ten padėtas tam, kad pažymėti, kad S tipas nėra bet koks.
	struct identity_inserter {
		template<ostream_like S, class U>
		static AA_CONSTEXPR void operator()(S &&s, const U &u) {
			if constexpr (stream_insertable<U, S>)	print(s, u);
			else									default_value_v<tuple_inserter>(s, u);
		}
	};

	template<auto D = ' '>
	struct delim_r_inserter {
		template<ostream_like S, class U>
		static AA_CONSTEXPR void operator()(S &&s, const U &u) {
			default_value_v<identity_inserter>(s, u);
			print(s, D);
		}
	};

	delim_r_inserter() -> delim_r_inserter<>;

	template<auto D = ' '>
	struct delim_l_inserter {
		template<ostream_like S, class U>
		static AA_CONSTEXPR void operator()(S &&s, const U &u) {
			print(s, D);
			default_value_v<identity_inserter>(s, u);
		}
	};

	delim_l_inserter() -> delim_l_inserter<>;

	template<int N>
	struct width_inserter {
		template<ostream_like S, class U>
		static AA_CONSTEXPR void operator()(S &&s, const U &u) {
			print(s, std::setw(N), u);
		}
	};



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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpadded"
	template<std::ranges::input_range R, class F = delim_r_inserter<>>
	struct range_writer {
#pragma GCC diagnostic pop
		[[no_unique_address]] const R range;
		[[no_unique_address]] const F fun = default_value;

		// Čia writeris galėtų ateiti ir ne const, bet tada reiktų arba kelių užklojimų funkcijos arba pridėti dar vieną
		// template argumentą, abu sprendimai labai nepatogūs. Na iš principo tai yra keista į spausdinimo funkciją
		// paduoti ne const kintamąjį. Išlieka galimybė pvz. turėti mutable lambdas ir keisti range elementus.
		template<char_traits_like T>
			requires (std::invocable<const F &, ostream_t<T> &, const std::ranges::range_value_t<R> &>)
		friend AA_CONSTEXPR ostream_t<T> &operator<<(ostream_t<T> &s, const range_writer &w) {
			std::ranges::for_each(w.range, [&s, &w](const std::ranges::range_value_t<R> &element) -> void {
				std::invoke(w.fun, s, element);
			});
			return s;
		}
	};

	template<class R, class F = delim_r_inserter<>>
	range_writer(R &&, F && = default_value) -> range_writer<R, F>;



#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpadded"
	template<class E, class F = delim_r_inserter<>>
	struct writer {
#pragma GCC diagnostic pop
		[[no_unique_address]] const E element;
		[[no_unique_address]] const F fun = default_value;

		template<char_traits_like T>
			requires (std::invocable<const F &, ostream_t<T> &, const E &>)
		friend AA_CONSTEXPR ostream_t<T> &operator<<(ostream_t<T> &s, const writer &w) {
			std::invoke(w.fun, s, w.element);
			return s;
		}
	};

	// Anksčiau buvo apibrėžti papildomi guides, kad copy elision nesuvalgytų konstruktorių.
	// Bet dabar sakome, kad tokiais atvejais tiesiog reiktų naudoti protingesnius inserters.
	template<class E, class F = delim_r_inserter<>>
	writer(E &&, F && = default_value) -> writer<E, F>;

}
