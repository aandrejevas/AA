#pragma once

#include "../metaprogramming/general.hpp"
#include "fixed_vector.hpp"
#include <memory> // construct_at



namespace aa {

	// https://en.wikipedia.org/wiki/Circular_buffer
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpadded"
	template<trivially_copyable T, size_t N>
	struct fixed_circular_vector {
#pragma GCC diagnostic pop
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
		constexpr reference operator[](const size_type pos) { return elements.get(pos); }
		constexpr const_reference operator[](const size_type pos) const { return elements.get(pos); }

		constexpr reference get(const size_type pos) { return elements.get(pos); }
		constexpr const_reference get(const size_type pos) const { return elements.get(pos); }

		constexpr reference rget(const size_type pos) { return *rdata(pos); }
		constexpr const_reference rget(const size_type pos) const { return *rdata(pos); }

		constexpr pointer data(const size_type pos) { return elements.data(pos); }
		constexpr const_pointer data(const size_type pos) const { return elements.data(pos); }

		constexpr pointer rdata(const size_type pos) { return rdata() - pos; }
		constexpr const_pointer rdata(const size_type pos) const { return rdata() - pos; }

		constexpr pointer data() { return elements.data(); }
		constexpr const_pointer data() const { return elements.data(); }

		constexpr pointer rdata() { return is_full ? data(max_index()) : tdata(); }
		constexpr const_pointer rdata() const { return is_full ? data(max_index()) : tdata(); }

		constexpr pointer tdata() { return elements.rdata(); }
		constexpr const_pointer tdata() const { return elements.rdata(); }

		constexpr reference front() { return elements.front(); }
		constexpr const_reference front() const { return elements.front(); }

		constexpr reference back() { return *rdata(); }
		constexpr const_reference back() const { return *rdata(); }

		constexpr reference top() { return elements.back(); }
		constexpr const_reference top() const { return elements.back(); }



		// Iterators
		constexpr iterator begin() { return elements.begin(); }
		constexpr const_iterator begin() const { return elements.begin(); }

		constexpr const_iterator end() const { return rdata() + 1; }

		constexpr iterator rbegin() { return rdata(); }
		constexpr const_iterator rbegin() const { return rdata(); }

		constexpr const_iterator rend() const { return elements.rend(); }



		// Capacity
		constexpr bool empty() const { return elements.empty(); }
		constexpr bool single() const {
			if constexpr (N == 1)	return elements.single();
			else					return !is_full && elements.single();
		}
		constexpr bool full() const { return is_full; }

		constexpr size_type size() const { return is_full ? max_size() : elements.size(); }
		constexpr size_type last_index() const { return is_full ? max_index() : elements.last_index(); }

		static consteval size_type max_size() { return N; }
		static consteval size_type max_index() { return N - 1; }



		// Modifiers
		constexpr void clear() {
			is_full = false;
			elements.clear();
		}

		constexpr iterator push() {
			if (elements.full()) {
				is_full = true;
				return elements.resize(data());
			} else {
				return elements.push_back();
			}
		}

		template<class... A>
			requires (std::constructible_from<value_type, A...>)
		constexpr void emplace(A&&... args) {
			std::ranges::construct_at(push(), std::forward<A>(args)...);
		}

		template<assignable_to<reference> V>
		constexpr void insert(V &&value) {
			*push() = std::forward<V>(value);
		}



		// Special member functions
		constexpr fixed_circular_vector() {}



		// Member objects
	protected:
		fixed_vector<value_type, N> elements;
		bool is_full = false;
	};

}
