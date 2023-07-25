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
		AA_CONSTEXPR reference operator[](const size_type pos) { return get(pos); }
		AA_CONSTEXPR const_reference operator[](const size_type pos) const { return get(pos); }

		AA_CONSTEXPR reference get(const size_type pos) { return *data(pos); }
		AA_CONSTEXPR const_reference get(const size_type pos) const { return *data(pos); }
		AA_CONSTEXPR const_reference cget(const size_type pos) const { return get(pos); }

		AA_CONSTEXPR reference rget(const size_type pos) { return elements.rget(pos); }
		AA_CONSTEXPR const_reference rget(const size_type pos) const { return elements.rget(pos); }
		AA_CONSTEXPR const_reference crget(const size_type pos) const { return rget(pos); }

		AA_CONSTEXPR pointer data(const size_type pos) { return data() + pos; }
		AA_CONSTEXPR const_pointer data(const size_type pos) const { return data() + pos; }
		AA_CONSTEXPR const_pointer cdata(const size_type pos) const { return data(pos); }

		AA_CONSTEXPR pointer rdata(const size_type pos) { return elements.rdata(pos); }
		AA_CONSTEXPR const_pointer rdata(const size_type pos) const { return elements.rdata(pos); }
		AA_CONSTEXPR const_pointer crdata(const size_type pos) const { return rdata(pos); }

		AA_CONSTEXPR pointer data() { return f_begin; }
		AA_CONSTEXPR const_pointer data() const { return f_begin; }
		AA_CONSTEXPR const_pointer cdata() const { return data(); }

		AA_CONSTEXPR pointer rdata() { return elements.rdata(); }
		AA_CONSTEXPR const_pointer rdata() const { return elements.rdata(); }
		AA_CONSTEXPR const_pointer crdata() const { return rdata(); }

		AA_CONSTEXPR reference front() { return *data(); }
		AA_CONSTEXPR const_reference front() const { return *data(); }
		AA_CONSTEXPR const_reference cfront() const { return front(); }

		AA_CONSTEXPR reference back() { return elements.back(); }
		AA_CONSTEXPR const_reference back() const { return elements.back(); }
		AA_CONSTEXPR const_reference cback() const { return back(); }



		// Iterators
		AA_CONSTEXPR iterator begin() { return data(); }
		AA_CONSTEXPR const_iterator begin() const { return data(); }
		AA_CONSTEXPR const_iterator cbegin() const { return begin(); }

		AA_CONSTEXPR iterator end() { return elements.end(); }
		AA_CONSTEXPR const_iterator end() const { return elements.end(); }
		AA_CONSTEXPR const_iterator cend() const { return end(); }

		AA_CONSTEXPR iterator rbegin() { return elements.rbegin(); }
		AA_CONSTEXPR const_iterator rbegin() const { return elements.rbegin(); }
		AA_CONSTEXPR const_iterator crbegin() const { return rbegin(); }

		AA_CONSTEXPR iterator rend() { return data() - 1; }
		AA_CONSTEXPR const_iterator rend() const { return data() - 1; }
		AA_CONSTEXPR const_iterator crend() const { return rend(); }



		// Capacity
		AA_CONSTEXPR bool empty() const { return elements.empty(); }
		AA_CONSTEXPR bool single() const { return f_begin == elements.rdata(); }
		AA_CONSTEXPR bool full() const { return elements.full(); }

		AA_CONSTEXPR difference_type ssize() const { return last_sindex() + 1; }
		AA_CONSTEXPR size_type size() const { return last_index() + 1; }

		static AA_CONSTEVAL size_type max_size() { return N; }
		static AA_CONSTEVAL size_type max_index() { return N - 1; }

		AA_CONSTEXPR difference_type last_sindex() const { return elements.rdata() - f_begin; }
		AA_CONSTEXPR size_type last_index() const { return unsign(last_sindex()); }



		// Modifiers
		AA_CONSTEXPR void clear() { elements.clear(); f_begin = elements.data(); }

		template<class... A>
			requires (std::constructible_from<value_type, A...>)
		AA_CONSTEXPR iterator emplace_back(A&&... args) { return elements.emplace_back(std::forward<A>(args)...); }

		template<assignable_to<reference> V>
		AA_CONSTEXPR void insert_back(V &&value) { elements.insert_back(std::forward<V>(value)); }

		AA_CONSTEXPR void push_back() { elements.push_back(); }

		AA_CONSTEXPR void pop_back() {
			if (single())
				clear(); else elements.pop_back();
		}

		template<class... A>
			requires (std::constructible_from<value_type, A...>)
		AA_CONSTEXPR iterator emplace_front(A&&... args) { return std::ranges::construct_at(push_front(), std::forward<A>(args)...); }

		template<assignable_to<reference> V>
		AA_CONSTEXPR void insert_front(V &&value) { *push_front() = std::forward<V>(value); }

		AA_CONSTEXPR void push_front() { --f_begin; }

		AA_CONSTEXPR void pop_front() {
			if (single())
				clear(); else ++f_begin;
		}

		template<class... A>
			requires (std::constructible_from<value_type, A...>)
		AA_CONSTEXPR iterator emplace(const const_iterator pos, A&&... args) { return elements.emplace(pos, std::forward<A>(args)...); }

		template<assignable_to<reference> V>
		AA_CONSTEXPR void insert(const const_iterator pos, V &&value) { elements.insert(pos, std::forward<V>(value)); }

		AA_CONSTEXPR void push(const const_iterator pos) { elements.push(pos); }

		AA_CONSTEXPR void erase(const const_iterator pos) {
			if (single())
				clear(); else elements.erase(pos);
		}



		// Special member functions
		AA_CONSTEXPR fixed_queue() {}
		template<std::ranges::input_range R>
		AA_CONSTEXPR fixed_queue(R &&r) : elements{r} {}



		// Member objects
	protected:
		// Konteineris paslėptas, nes naudotojas galėtų pavyzdžiui išvalyti konteinerį ir taip sutrugdyti šios klasės veikimui.
		fixed_vector<value_type, N> elements;
		value_type *f_begin = elements.data();
	};

}
