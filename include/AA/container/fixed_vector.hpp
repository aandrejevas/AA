#pragma once

#include "../metaprogramming/general.hpp"
#include <memory> // construct_at
#include <algorithm> // copy, copy_n, copy_backward
#include <ranges> // input_range
#include <iterator> // default_sentinel_t



namespace aa {

	// https://en.wikipedia.org/wiki/Dynamic_array
	// Negali būti tuple like, nes kinta kiek masyvo elementų yra "naudojama".
	template<trivially_copyable T, size_t N>
	struct fixed_vector {
		// Member types
		using value_type = T;
		using size_type = size_t;
		using difference_type = ptrdiff_t;
		using reference = value_type &;
		using const_reference = const value_type &;
		using pointer = value_type *;
		using const_pointer = const value_type *;
		using iterator = pointer;
		using const_iterator = const_pointer;



		// Element access
		constexpr reference operator[](const size_type pos) { return get(pos); }
		constexpr const_reference operator[](const size_type pos) const { return get(pos); }

		constexpr reference get(const size_type pos) { return *data(pos); }
		constexpr const_reference get(const size_type pos) const { return *data(pos); }

		constexpr reference rget(const size_type pos) { return *rdata(pos); }
		constexpr const_reference rget(const size_type pos) const { return *rdata(pos); }

		constexpr pointer data(const size_type pos) { return data() + pos; }
		constexpr const_pointer data(const size_type pos) const { return data() + pos; }

		constexpr pointer rdata(const size_type pos) { return rdata() - pos; }
		constexpr const_pointer rdata(const size_type pos) const { return rdata() - pos; }

		constexpr pointer data() { return elements.data(); }
		constexpr const_pointer data() const { return elements.data(); }

		constexpr pointer rdata() { return r_begin; }
		constexpr const_pointer rdata() const { return r_begin; }

		constexpr reference front() { return *data(); }
		constexpr const_reference front() const { return *data(); }

		constexpr reference back() { return *rdata(); }
		constexpr const_reference back() const { return *rdata(); }



		// Iterators
		constexpr iterator begin() { return data(); }
		constexpr const_iterator begin() const { return data(); }

		constexpr iterator end() { return rdata() + 1; }
		constexpr const_iterator end() const { return rdata() + 1; }

		constexpr iterator rbegin() { return rdata(); }
		constexpr const_iterator rbegin() const { return rdata(); }

		constexpr iterator rend() { return r_end; }
		constexpr const_iterator rend() const { return r_end; }



		// Capacity
		constexpr bool empty() const { return r_begin == r_end; }
		constexpr bool single() const { return r_begin == data(); }
		constexpr bool full() const { return size() == max_size(); }

		constexpr difference_type ssize() const { return r_begin - r_end; }
		constexpr size_type size() const { return unsign(ssize()); }

		static consteval size_type max_size() { return N; }
		static consteval size_type max_index() { return N - 1; }

		constexpr difference_type sindexl() const { return r_begin - data(); }
		constexpr size_type indexl() const { return unsign(sindexl()); }



		// Anksčiau buvo, bet dabar nebėra elemento įdėjimo metodų overload'ų, kurie priimtų
		// value_type&&, nes move semantics neturi prasmės trivially copyable tipams.
		// Modifiers
		constexpr void clear() { r_begin = r_end; }

		constexpr iterator resize(const size_type count) { return r_begin = r_end + count; }
		constexpr iterator resize(const const_iterator pos) { return r_begin = const_cast<iterator>(pos); }

		constexpr iterator pop_back() { return --r_begin; }
		constexpr iterator push_back() { return ++r_begin; }

		constexpr iterator pop_back(const size_type count) { return r_begin -= count; }
		constexpr iterator push_back(const size_type count) { return r_begin += count; }

		// Neišeina emplace_back ir push_back apjungti, nes įsivaizduokime tokį svenarijų, visi masyvo elementai
		// pradžioje sukonstruojami ir mes norime tiesiog rodyklę pastumti, emplace_back iš naujo sukonstruotų elementą.
		template<class... A>
			requires (std::constructible_from<value_type, A...>)
		constexpr iterator emplace_back(A&&... args) {
			return std::ranges::construct_at(push_back(), std::forward<A>(args)...);
		}

		template<assignable_to<reference> V>
		constexpr void insert_back(V &&value) { *push_back() = std::forward<V>(value); }

		constexpr void push(const const_iterator pos) {
			++r_begin;
			std::ranges::copy_backward(pos, const_cast<const_iterator>(r_begin), r_begin + 1);
		}

		template<class... A>
			requires (std::constructible_from<value_type, A...>)
		constexpr iterator emplace(const const_iterator pos, A&&... args) {
			push(pos);
			return std::ranges::construct_at(const_cast<iterator>(pos), std::forward<A>(args)...);
		}

		template<assignable_to<reference> V>
		constexpr void insert(const const_iterator pos, V &&value) {
			push(pos);
			*const_cast<iterator>(pos) = std::forward<V>(value);
		}

		constexpr void erase(const const_iterator pos) {
			std::ranges::copy_n(pos + 1, r_begin - pos, const_cast<iterator>(pos));
			--r_begin;
		}

		template<class... A>
			requires (std::constructible_from<value_type, A...>)
		constexpr iterator fast_emplace(const const_iterator pos, A&&... args) {
			insert_back(*pos);
			return std::ranges::construct_at(const_cast<iterator>(pos), std::forward<A>(args)...);
		}

		template<assignable_to<reference> V>
		constexpr void fast_insert(const const_iterator pos, V &&value) {
			insert_back(*pos);
			*const_cast<iterator>(pos) = std::forward<V>(value);
		}

		constexpr void fast_erase(const const_iterator pos) {
			*const_cast<iterator>(pos) = *r_begin--;
		}

		template<std::ranges::input_range R>
		constexpr void append_range(R &&r) {
			r_begin = std::ranges::copy(r, r_begin + 1).out - 1;
		}



		// Special member functions
		// Kompiliatorius meta klaidą dėl to kaip kintamasis r_end yra inicializuojamas.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
		// Nedarome = default, nes konstrukrotius vis tiek nebus trivial,
		// nes klasė turi kintamųjų su numatytais inicializatoriais.
		constexpr fixed_vector() {}
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
		// Negalime turėti konstruktoriaus, kuris priimtų rodyklę, nes
		// tik po konstruktoriaus įvykdymo galima gauti rodykles.
		constexpr fixed_vector(const std::default_sentinel_t) : r_begin{elements.data()} {}
#pragma GCC diagnostic pop
		constexpr fixed_vector(const size_type count) : r_begin{r_end + count} {}
		// Nereikia konstruktoriaus, kuriame būtų naudojama fill, nes šį funkcionalumą
		// galima simuliuoti naudojant šį konstruktorių pavyzdžiui su repeat_view.
		template<std::ranges::input_range R>
		constexpr fixed_vector(R &&r) : r_begin{std::ranges::copy(r, elements.data()).out - 1} {}
#pragma GCC diagnostic pop



		// Member objects
		std::array<value_type, N> elements;

	protected:
		value_type *const r_end = elements.data() - 1, *r_begin = r_end;
	};

}
