#pragma once

#include "../metaprogramming/general.hpp"



namespace aa {

	template<class ACQUIRER>
	struct permit : unit<value_type_in_use_t<ACQUIRER>> {
		// Member types
		using typename unit<value_type_in_use_t<ACQUIRER>>::tuple_type;
		using unit_type = typename tuple_type::unit_type<0>;
		using typename unit_type::value_type,
			typename unit_type::reference, typename unit_type::const_reference,
			typename unit_type::pointer, typename unit_type::const_pointer;



		// Observers
		constexpr auto operator->() {
			return to_pointer(unit_type::value);
		}

		constexpr pointer operator&() {
			return std::addressof(unit_type::value);
		}

		constexpr operator reference() {
			return unit_type::value;
		}

		constexpr decltype(auto) operator*() {
			return to_reference(unit_type::value);
		}

		constexpr reference get() {
			return unit_type::value;
		}



		// Special member functions
		constexpr permit(ACQUIRER & a) : tuple_type{ACQUIRER::empty_value}, acquirer{a} {}

		constexpr ~permit() {
			acquirer.reset(std::move(unit_type::value));
		}



		// Member objects
		ACQUIRER & acquirer;
	};

}
