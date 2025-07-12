#pragma once

#include "../metaprogramming/general.hpp"



namespace aa {

	// Konkretus panaudojimas struktūros yra, sakykime turime globalų kintamąjį, bet jį inicializuojame tik vėliau ir po to jo nekeičiame. Tai ši klasė leidžia jį inicializuoti ir jei bus bandoma pakeisti tą kintamąjį vėliau, iškarto bus išeinama iš programos.
	template<std::movable T, cref_predicate<const T &> PREDICATE = equal_to<default_value>>
	struct constify : private unit<T> {
		// Member types
		using typename unit<T>::tuple_type;
		using unit_type = typename tuple_type::unit_type<0>;
		using typename unit_type::value_type,
			typename unit_type::reference, typename unit_type::const_reference,
			typename unit_type::pointer, typename unit_type::const_pointer;

		struct permit {
			// Observers
			constexpr auto operator->() const {
				return to_pointer(constifier.value);
			}

			constexpr pointer operator&() const {
				return std::addressof(constifier.value);
			}

			constexpr operator reference() const {
				return constifier.value;
			}

			constexpr decltype(auto) operator*() const {
				return to_reference(constifier.value);
			}

			constexpr reference get() const {
				return constifier.value;
			}

			// Special member functions
			constexpr permit(constify & c) : constifier{c} {
				constifier.assert_valueless();
			}

			constexpr ~permit() {
				constifier.assert_valueful();
			}

			// Member objects
			constify & constifier;
		};

		// Observers
	private:
		constexpr void assert_valueless() const {
			if (!std::invoke(default_v<PREDICATE>, unit_type::value))
				std::exit(EXIT_FAILURE);
		}

		constexpr void assert_valueful() const {
			if (std::invoke(default_v<PREDICATE>, unit_type::value))
				std::exit(EXIT_FAILURE);
		}

	public:
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
		constexpr permit acquire() & {
			return permit{*this};
		}

		template<constructible_to<value_type> O = value_type>
		constexpr const constify & operator=(O && other) & {
			const permit val = acquire();

			std::ranges::construct_at(&val, std::forward<O>(other));

			return val.constifier;
		}

		// Special member functions
		// Neturime default konstruktoriaus, nes pradžioje neinicializavus lauko, joks vėliau daromas tikrinimas nebūtų racionalus.
		template<constructible_to<value_type> U = value_type>
		constexpr constify(U && u = default_value) : tuple_type{std::forward<U>(u)} {
			assert_valueless();
		}

		constexpr constify() requires (std::constructible_from<value_type, const_t<PREDICATE::value>>)
			: constify{PREDICATE::value} {}

		constexpr ~constify() {
			assert_valueful();
		}
	};

}
