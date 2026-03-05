#pragma once

#include "../metaprogramming/general.hpp"
#include "permit.hpp"



namespace aa {

	// We define our own std::unique_ptr, bc:
	// 1. we want implicit conversions;
	// 2. we want to unconditionally call the deleter and require the deleter to deal with nullptr;
	// 3. we want to be able to manage not only pointers.
	//
	// Internal object not movable or changeable from outside bc it is a managed object and first it has to be run through the deleter and then it can be initialized to a new state.
	template<wo_cv_movable T,
		cref_nullary_or_unary_invocable<const T &> auto DELETER = default_v<std::default_delete<std::remove_pointer_t<T>>>,
		cref_constructible_to<T> auto EMPTY = default_value,
		cref_nullary_or_unary_predicate<const T &> auto PREDICATE = default_v<equal_to<EMPTY>>>
	struct managed : private unit<T> {
		// Member types
		using typename unit<T>::tuple_type;
		using unit_type = typename tuple_type::unit_type<0>;
		using typename unit_type::value_type,
			typename unit_type::reference, typename unit_type::const_reference,
			typename unit_type::pointer, typename unit_type::const_pointer;

		static constexpr const_t<EMPTY> empty_value = EMPTY;



		// Observers
		constexpr bool has_ownership() const {
			return !greedy_invoke(PREDICATE, unit_type::value);
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
		constexpr auto acquire() & {
			return permit{*this};
		}

		template<class... A>
			requires ((!sizeof...(A)) || (not_const<value_type> && std::constructible_from<value_type, A...>))
		constexpr void reset(A &&... args) & {
			if (!has_ownership()) {
				// We are empty so we do not have to delete or initialize value so that it would be empty
				if constexpr (!!sizeof...(A)) {
					std::ranges::construct_at(std::addressof(unit_type::value), std::forward<A>(args)...);
				}
			} else {
				greedy_invoke(DELETER, std::as_const(unit_type::value));

				if constexpr (!!sizeof...(A)) {
					std::ranges::construct_at(std::addressof(unit_type::value), std::forward<A>(args)...);
				} else if constexpr (not_const<value_type>) {
					std::ranges::construct_at(std::addressof(unit_type::value), empty_value);
				}
			}
		}

		constexpr managed & operator=(managed && o) & {
			std::ranges::destroy_at(this);
			return *std::ranges::construct_at(this, std::move(o));
		}

		// Būtina turėti tokią funkciją, nes nėra kito būdo kaip gauti mutable reikšmę, jei ją reiktų paduoti į funkciją, kuri tikisi tokios reikšmės.
		constexpr value_type release() && {
			return std::exchange(unit_type::value, empty_value);
		}



		// Special member functions
		template<constructible_to<value_type> U = value_type>
		constexpr managed(U && u = empty_value) : tuple_type{std::forward<U>(u)} {}

		constexpr managed(managed && o) : tuple_type{std::move(o).release()} {}

		constexpr ~managed() {
			reset();
		}
	};

	template<class T, auto EMPTY = default_value, auto PREDICATE = default_v<equal_to<EMPTY>>>
	using explicitly_managed = managed<T, [] [[noreturn]] static -> void { std::exit(EXIT_FAILURE); }, EMPTY, PREDICATE>;

	template<class T, auto EMPTY = default_value, auto PREDICATE = default_v<equal_to<EMPTY>>>
	using shallowly_managed = managed<T, default_v<std::identity>, EMPTY, PREDICATE>;

	template<auto DELETER, class T = first_not_t<void, function_argument_t<const_t<DELETER>>, std::monostate>, auto EMPTY = default_value>
	using instrumentally_managed = managed<const T, DELETER, EMPTY, default_v<std::false_type>>;

}
