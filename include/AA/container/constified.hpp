#pragma once

#include "../metaprogramming/general.hpp"
#include "permit.hpp"



namespace aa {

	// Konkretus panaudojimas struktūros yra, sakykime turime globalų kintamąjį, bet jį inicializuojame tik vėliau ir po to jo nekeičiame. Tai ši klasė leidžia jį inicializuoti ir jei bus bandoma pakeisti tą kintamąjį vėliau, iškarto bus išeinama iš programos.
	template<not_const_movable T,
		cref_constructible_to<T> auto EMPTY = default_value,
		cref_predicate<const T &> auto PREDICATE = default_v<equal_to<EMPTY>>>
	struct constified : private unit<T> {
		// Member types
		using typename unit<T>::tuple_type;
		using unit_type = typename tuple_type::unit_type<0>;
		using typename unit_type::value_type,
			typename unit_type::reference, typename unit_type::const_reference,
			typename unit_type::pointer, typename unit_type::const_pointer;

		static constexpr const_t<EMPTY> empty_value = EMPTY;



		// Observers
	private:
		constexpr void assert_valueless() const {
			if (has_ownership())
				std::exit(EXIT_FAILURE);
		}

		constexpr void assert_valueful() const {
			if (!has_ownership())
				std::exit(EXIT_FAILURE);
		}

	public:
		constexpr bool has_ownership() const {
			return !std::invoke(PREDICATE, unit_type::value);
		}

		constexpr auto operator->() const {
			return to_pointer(unit_type::value);
		}

		constexpr const_pointer operator&() const {
			return std::addressof(unit_type::value);
		}

		constexpr operator const_reference() const {
			return unit_type::value;
		}

		constexpr decltype(auto) operator*() const {
			return to_reference(unit_type::value);
		}

		constexpr const_reference get() const {
			return unit_type::value;
		}



		// Modifiers
		// Neturime funkcijos, kuri priimtų funkciją, nes su permit galime tą patį pasiekti ir nereikia kurti lambdos.
		// Tikrinimai valueless ir valueful turi būti atlikti destruktoriuje, nes jei valueless tikrinimas bus atliktas konstruktoriuje, bus leidžiama tokia klaidinga elgsena: sukuriami du permits int objekto, praeina abu tikrinimai, kad objektas yra valueless, tada abu pakeičia reikšmę ir gale praeina abu tikrinimai, kad objektas yra valueful ir taip bus pakeistas du kartus int objektas.
		constexpr permit<constified> acquire() & {
			return {*this};
		}

		template<constructible_to<value_type> O = value_type>
		constexpr const constified & operator=(O && other) & {
			reset(std::forward<O>(other));
			return *this;
		}

		template<class... A>
			requires (std::constructible_from<value_type, A...>)
		constexpr void reset(A &&... args) & {
			assert_valueless();
			std::ranges::construct_at(std::addressof(unit_type::value), std::forward<A>(args)...);
			assert_valueful();
		}



		// Special member functions
		// Neturime default konstruktoriaus, nes pradžioje neinicializavus lauko, joks vėliau daromas tikrinimas nebūtų racionalus.
		constexpr constified() : tuple_type{empty_value} {
			assert_valueless();
		}

		constexpr ~constified() {
			assert_valueful();
		}
	};

}
