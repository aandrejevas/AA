#pragma once

#include "../metaprogramming/general.hpp"



namespace aa {

	// Konkretus panaudojimas struktūros yra, sakykime turime globalų kintamąjį, bet jį inicializuojame tik vėliau ir po to jo nekeičiame. Tai ši klasė leidžia jį inicializuoti ir jei bus bandoma pakeisti tą kintamąjį vėliau, iškarto bus išeinama iš programos.
	template<std::movable T, cref_predicate<const T &> auto PREDICATE = default_v<equal_to<default_v<T>>>>
	struct constify : protected unit<T> {
		// Member types
		using typename unit<T>::tuple_type;
		using unit_type = typename tuple_type::unit_type<0>;
		using typename unit_type::value_type,
			typename unit_type::reference, typename unit_type::const_reference,
			typename unit_type::pointer, typename unit_type::const_pointer;

		// Observers
		constexpr const_pointer operator->() const {
			return std::addressof(unit_type::value);
		}

		constexpr const_pointer operator&() const {
			return std::addressof(unit_type::value);
		}

		constexpr operator const_reference() const {
			return unit_type::value;
		}

		constexpr const_reference operator*() const {
			return unit_type::value;
		}

		constexpr const_reference get() const {
			return unit_type::value;
		}

		// Modifiers
		template<invocable_with_one_of<reference, pointer> F>
		constexpr add_cref_t<constify> make(F && f) & {
			if (!std::invoke(PREDICATE, std::as_const(unit_type::value)))
				std::exit(EXIT_FAILURE);

			if constexpr (std::invocable<reference>) {
				std::invoke(std::forward<F>(f), unit_type::value);
			} else {
				std::invoke(std::forward<F>(f), std::addressof(unit_type::value));
			}

			if (std::invoke(PREDICATE, std::as_const(unit_type::value)))
				std::exit(EXIT_FAILURE);

			return *this;
		}

		template<constructible_to<value_type> O = value_type>
		constexpr add_cref_t<constify> operator=(O && other) & {
			return make([&](const pointer ptr) -> void {
				std::ranges::construct_at(ptr, std::forward<O>(other));
			});
		}

		// Special member functions
		// Neturime default konstruktoriaus, nes pradžioje neinicializavus lauko, joks vėliau daromas tikrinimas nebūtų racionalus.
		template<constructible_to<value_type> U = value_type>
		constexpr constify(U && u = default_value) : tuple_type{std::forward<U>(u)} {
			if (!std::invoke(PREDICATE, std::as_const(unit_type::value)))
				std::exit(EXIT_FAILURE);
		}

		constexpr ~constify() {
			if (std::invoke(PREDICATE, std::as_const(unit_type::value)))
				std::exit(EXIT_FAILURE);
		}
	};

}
