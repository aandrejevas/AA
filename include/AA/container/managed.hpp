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
	template<not_const_movable T,
		cref_invocable<const T &> auto DELETER = default_v<std::default_delete<std::remove_pointer_t<T>>>,
		cref_constructible_to<T> auto EMPTY = default_value,
		cref_predicate<const T &> auto PREDICATE = default_v<equal_to<EMPTY>>>
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
		constexpr permit<managed> acquire() & {
			return {*this};
		}

		template<constructible_to<value_type> O = value_type>
		constexpr managed & operator=(O && other) & {
			reset(std::forward<O>(other));
			return *this;
		}

		template<class... A>
			requires ((!sizeof...(A)) || std::constructible_from<value_type, A...>)
		constexpr void reset(A &&... args) & {
			if (!has_ownership()) {
				// We are empty so we do not have to delete or initialize value so that it would be empty
				if constexpr (!!sizeof...(A)) {
					std::ranges::construct_at(std::addressof(unit_type::value), std::forward<A>(args)...);
				}
			} else {
				std::invoke(DELETER, std::as_const(unit_type::value));

				if constexpr (!!sizeof...(A)) {
					std::ranges::construct_at(std::addressof(unit_type::value), std::forward<A>(args)...);
				} else {
					std::ranges::construct_at(std::addressof(unit_type::value), empty_value);
				}
			}
		}

		constexpr managed & operator=(managed && o) & {
			unit_type::value = std::move(o).release();
			return *this;
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
	using explicitly_managed = managed<T, default_v<lift_exit_t<EXIT_FAILURE>>, EMPTY, PREDICATE>;

	template<class T, auto EMPTY = default_value, auto PREDICATE = default_v<equal_to<EMPTY>>>
	using shallowly_managed = managed<T, default_v<std::identity>, EMPTY, PREDICATE>;

	template<auto DELETER>
	using instrumentally_managed = managed<std::monostate, DELETER, default_value, default_v<not_equal_to<default_v<std::monostate>>>>;

}
