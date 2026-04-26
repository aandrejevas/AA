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
	template<
		class DELETER,
		wo_cv_movable T = std::remove_reference_t<function_argument_t<DELETER>>,
		cref_constructible_to<T> auto EMPTY = default_value,
		class PREDICATE = equal_to<EMPTY>
	>
	struct managed : protected unit<T> {
		// Member types
		using typename unit<T>::tuple_type;
		using unit_type = typename tuple_type::unit_type<0>;
		using typename unit_type::value_type,
			typename unit_type::reference, typename unit_type::const_reference,
			typename unit_type::pointer, typename unit_type::const_pointer;

		static consteval t<EMPTY> empty_value() { return EMPTY; }



		// Observers
		constexpr bool has_ownership() const {
			return !greedy_invoke(c<PREDICATE>(), as<unit_type>(*this)());
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

		template<constructible_to<value_type> U = value_type, class... A>
			requires (not_const<value_type>)
		constexpr void reset(U && u, A &&... args) & {
			if (has_ownership()) {
				greedy_invoke(c<DELETER>(), std::as_const(unit_type::value), std::forward<A>(args)...);
			}

			std_r::construct_at(std::addressof(unit_type::value), std::forward<U>(u));
		}

		template<constructible_to<value_type> U = value_type, class... A>
			requires (not_const<value_type>)
		constexpr void reset_unchecked(U && u, A &&... args) & {
			greedy_invoke(c<DELETER>(), std::as_const(unit_type::value), std::forward<A>(args)...);

			std_r::construct_at(std::addressof(unit_type::value), std::forward<U>(u));
		}

		template<class... A>
		constexpr void unset(A &&... args) & {
			if (has_ownership()) {
				unset_unchecked(std::forward<A>(args)...);
			}
		}

		template<class... A>
		constexpr void unset_unchecked(A &&... args) & {
			greedy_invoke(c<DELETER>(), as<unit_type>(*this)(), std::forward<A>(args)...);

			if constexpr (not_const<value_type>) {
				std_r::construct_at(std::addressof(unit_type::value), empty_value());
			}
		}

		constexpr managed & operator=(managed && o) & {
			std_r::destroy_at(this);
			return *std_r::construct_at(this, std::move(o));
		}

		// Būtina turėti tokią funkciją, nes nėra kito būdo kaip gauti mutable reikšmę, jei ją reiktų paduoti į funkciją, kuri tikisi tokios reikšmės.
		constexpr value_type release() && {
			return std::exchange(unit_type::value, empty_value());
		}



		// Special member functions
		template<constructible_to<value_type> U = value_type>
		constexpr managed(U && u = empty_value()) : tuple_type{std::forward<U>(u)} {}

		constexpr managed(managed && o) : tuple_type{std::move(o).release()} {}

		constexpr ~managed() {
			unset();
		}
	};

	template<class T, auto EMPTY = default_value, class PREDICATE = equal_to<EMPTY>>
	using explicitly_managed = managed<t<[] [[noreturn]] static -> void { std::exit(EXIT_FAILURE); }>, T, EMPTY, PREDICATE>;

	template<class T, auto EMPTY = default_value, class PREDICATE = equal_to<EMPTY>>
	using shallowly_managed = managed<noop, T, EMPTY, PREDICATE>;

	template<class DELETER,
		class T = first_not_t<void, std::remove_reference_t<function_argument_t<DELETER>>, std::monostate>,
		auto EMPTY = default_value>
	using instrumentally_managed = managed<DELETER, const T, EMPTY, constant<false>>;

	template<pointer_like T>
	using managed_by_new = managed<std::default_delete<std::remove_pointer_t<T>>>;

	template<class ALLOC>
	using managed_by_allocator = managed<t<[](
		const pointer_in_use_t<std::allocator_traits<ALLOC>> p,
		const size_type_in_use_t<std::allocator_traits<ALLOC>> n = default_value) static -> void
	{
		c<ALLOC>().deallocate(p, n);
	}>>;

}
