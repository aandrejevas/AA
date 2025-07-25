#pragma once

#include "../metaprogramming/general.hpp"



namespace aa {

	// Klasė skirta naudoti su klasėmis, kurių move konstruktorius yra ištrintas, nes negalėtume naudoti make(), kad sukonstruoti tokias klases, nes tokios klasės negali dalyvauti NRVO. Pvz.:
	// struct AAA {
	// 	constexpr AAA(AAA &&) = delete;
	// 	constexpr AAA() = default;
	// };
	// https://devblogs.microsoft.com/oldnewthing/20230612-00/?p=108329
	// Laikome konstantą, nes kintamojo nėra prasmės laikyti šioje klasėje, tokiu atveju reikėtų šios klasės nenaudoti.
	template<not_const_not_movable T>
	struct made : unit<const T> {
		// Member types
		using typename unit<const T>::tuple_type;
		using unit_type = typename tuple_type::unit_type<0>;
		using typename unit_type::value_type,
			typename unit_type::reference, typename unit_type::const_reference,
			typename unit_type::pointer, typename unit_type::const_pointer;



		// Observers
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



		// Special member functions
		template<std::invocable<reference> F>
		constexpr made(F && f) {
			std::invoke(std::forward<F>(f), unit_type::value);
		}

		template<std::invocable<reference> F, constructible_to<value_type> U = value_type>
		constexpr made(F && f, U && u) : tuple_type{std::forward<U>(u)} {
			std::invoke(std::forward<F>(f), unit_type::value);
		}
	};

}
