#pragma once

#include "../metaprogramming/general.hpp"
#include "fixed_vector.hpp"
#include <memory> // construct_at



namespace aa {

	// https://en.wikipedia.org/wiki/Circular_buffer
	template<trivially_copyable T, size_t N>
	struct fixed_circular_vector {
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

		constexpr pointer rdata() { return full() ? r_begin : r_curr; }
		constexpr const_pointer rdata() const { return full() ? r_begin : r_curr; }

		constexpr pointer tdata() { return r_curr; }
		constexpr const_pointer tdata() const { return r_curr; }

		constexpr reference front() { return *data(); }
		constexpr const_reference front() const { return *data(); }

		constexpr reference back() { return *rdata(); }
		constexpr const_reference back() const { return *rdata(); }

		constexpr reference top() { return *tdata(); }
		constexpr const_reference top() const { return *tdata(); }



		// Iterators
		constexpr iterator begin() { return data(); }
		constexpr const_iterator begin() const { return data(); }

		constexpr const_iterator end() const { return rdata() + 1; }

		constexpr iterator rbegin() { return rdata(); }
		constexpr const_iterator rbegin() const { return rdata(); }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
		constexpr const_iterator rend() const { return data() - 1; }
#pragma GCC diagnostic pop



		// Capacity
		constexpr bool empty() const { return r_curr == r_end; }
		constexpr bool single() const { return !full() && r_curr == data(); }
		constexpr bool full() const { return is_full; }

		constexpr difference_type ssize() const { return full() ? N : (r_curr - r_end); }
		constexpr size_type size() const { return unsign(ssize()); }

		static consteval size_type max_size() { return N; }
		static consteval size_type max_index() { return N - 1; }

		constexpr difference_type sindexl() const { return full() ? max_index() : (r_curr - data()); }
		constexpr size_type indexl() const { return unsign(sindexl()); }



		// Modifiers
		constexpr void clear() {
			is_full = false;
			r_curr = r_end;
		}

		constexpr iterator push() {
			if (r_curr == r_begin) {
				is_full = true;
				return r_curr = data();
			} else {
				return ++r_curr;
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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
		constexpr fixed_circular_vector() {}
#pragma GCC diagnostic pop



		// Member objects
	protected:
		fixed_vector<value_type, N> elements;
		bool is_full = false;
		value_type *const r_end = elements.data() - 1, *const r_begin = elements.data() + max_index(), *r_curr = r_end;
	};

}
