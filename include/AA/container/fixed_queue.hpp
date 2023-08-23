#pragma once

#include "../metaprogramming/general.hpp"
#include "fixed_vector.hpp"
#include <memory> // construct_at
#include <ranges> // input_range



namespace aa {

	// https://en.wikipedia.org/wiki/Queue_(abstract_data_type)
	template<trivially_copyable T, size_t N>
	struct fixed_queue {
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

		constexpr reference rget(const size_type pos) { return elements.rget(pos); }
		constexpr const_reference rget(const size_type pos) const { return elements.rget(pos); }

		constexpr pointer data(const size_type pos) { return data() + pos; }
		constexpr const_pointer data(const size_type pos) const { return data() + pos; }

		constexpr pointer rdata(const size_type pos) { return elements.rdata(pos); }
		constexpr const_pointer rdata(const size_type pos) const { return elements.rdata(pos); }

		constexpr pointer data() { return f_begin; }
		constexpr const_pointer data() const { return f_begin; }

		constexpr pointer rdata() { return elements.rdata(); }
		constexpr const_pointer rdata() const { return elements.rdata(); }

		constexpr reference front() { return *data(); }
		constexpr const_reference front() const { return *data(); }

		constexpr reference back() { return elements.back(); }
		constexpr const_reference back() const { return elements.back(); }



		// Iterators
		constexpr iterator begin() { return data(); }
		constexpr const_iterator begin() const { return data(); }

		constexpr iterator end() { return elements.end(); }
		constexpr const_iterator end() const { return elements.end(); }

		constexpr iterator rbegin() { return elements.rbegin(); }
		constexpr const_iterator rbegin() const { return elements.rbegin(); }

		constexpr iterator rend() { return data() - 1; }
		constexpr const_iterator rend() const { return data() - 1; }



		// Capacity
		constexpr bool empty() const { return elements.empty(); }
		constexpr bool single() const { return f_begin == elements.rdata(); }
		constexpr bool full() const { return elements.full(); }

		constexpr difference_type ssize() const { return sindexl() + 1; }
		constexpr size_type size() const { return indexl() + 1; }

		static consteval size_type max_size() { return N; }
		static consteval size_type max_index() { return N - 1; }

		constexpr difference_type sindexl() const { return elements.rdata() - f_begin; }
		constexpr size_type indexl() const { return unsign(sindexl()); }



		// Modifiers
		constexpr void clear() { elements.clear(); f_begin = elements.data(); }

		template<class... A>
			requires (std::constructible_from<value_type, A...>)
		constexpr iterator emplace_back(A&&... args) { return elements.emplace_back(std::forward<A>(args)...); }

		template<assignable_to<reference> V>
		constexpr void insert_back(V &&value) { elements.insert_back(std::forward<V>(value)); }

		constexpr void push_back() { elements.push_back(); }

		constexpr void pop_back() {
			if (single())
				clear(); else elements.pop_back();
		}

		template<class... A>
			requires (std::constructible_from<value_type, A...>)
		constexpr iterator emplace_front(A&&... args) { return std::ranges::construct_at(push_front(), std::forward<A>(args)...); }

		template<assignable_to<reference> V>
		constexpr void insert_front(V &&value) { *push_front() = std::forward<V>(value); }

		constexpr void push_front() { --f_begin; }

		constexpr void pop_front() {
			if (single())
				clear(); else ++f_begin;
		}

		template<class... A>
			requires (std::constructible_from<value_type, A...>)
		constexpr iterator emplace(const const_iterator pos, A&&... args) { return elements.emplace(pos, std::forward<A>(args)...); }

		template<assignable_to<reference> V>
		constexpr void insert(const const_iterator pos, V &&value) { elements.insert(pos, std::forward<V>(value)); }

		constexpr void push(const const_iterator pos) { elements.push(pos); }

		constexpr void erase(const const_iterator pos) {
			if (single())
				clear(); else elements.erase(pos);
		}



		// Special member functions
		constexpr fixed_queue() {}
		template<std::ranges::input_range R>
		constexpr fixed_queue(R &&r) : elements{r} {}



		// Member objects
	protected:
		// Konteineris paslėptas, nes naudotojas galėtų pavyzdžiui išvalyti konteinerį ir taip sutrugdyti šios klasės veikimui.
		fixed_vector<value_type, N> elements;
		value_type *f_begin = elements.data();
	};

}
