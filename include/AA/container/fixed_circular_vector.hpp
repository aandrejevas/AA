#pragma once

#include "../metaprogramming/general.hpp"
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
		AA_CONSTEXPR reference operator[](const size_type pos) { return get(pos); }
		AA_CONSTEXPR const_reference operator[](const size_type pos) const { return get(pos); }

		AA_CONSTEXPR reference get(const size_type pos) { return *data(pos); }
		AA_CONSTEXPR const_reference get(const size_type pos) const { return *data(pos); }
		AA_CONSTEXPR const_reference cget(const size_type pos) const { return get(pos); }

		AA_CONSTEXPR reference rget(const size_type pos) { return *rdata(pos); }
		AA_CONSTEXPR const_reference rget(const size_type pos) const { return *rdata(pos); }
		AA_CONSTEXPR const_reference crget(const size_type pos) const { return rget(pos); }

		AA_CONSTEXPR pointer data(const size_type pos) { return data() + pos; }
		AA_CONSTEXPR const_pointer data(const size_type pos) const { return data() + pos; }
		AA_CONSTEXPR const_pointer cdata(const size_type pos) const { return data(pos); }

		AA_CONSTEXPR pointer rdata(const size_type pos) { return rdata() - pos; }
		AA_CONSTEXPR const_pointer rdata(const size_type pos) const { return rdata() - pos; }
		AA_CONSTEXPR const_pointer crdata(const size_type pos) const { return rdata(pos); }

		AA_CONSTEXPR pointer data() { return elements.data(); }
		AA_CONSTEXPR const_pointer data() const { return elements.data(); }
		AA_CONSTEXPR const_pointer cdata() const { return data(); }

		AA_CONSTEXPR pointer rdata() { return full() ? r_begin : r_curr; }
		AA_CONSTEXPR const_pointer rdata() const { return full() ? r_begin : r_curr; }
		AA_CONSTEXPR const_pointer crdata() const { return rdata(); }

		AA_CONSTEXPR pointer tdata() { return r_curr; }
		AA_CONSTEXPR const_pointer tdata() const { return r_curr; }
		AA_CONSTEXPR const_pointer ctdata() const { return tdata(); }

		AA_CONSTEXPR reference front() { return *data(); }
		AA_CONSTEXPR const_reference front() const { return *data(); }
		AA_CONSTEXPR const_reference cfront() const { return front(); }

		AA_CONSTEXPR reference back() { return *rdata(); }
		AA_CONSTEXPR const_reference back() const { return *rdata(); }
		AA_CONSTEXPR const_reference cback() const { return back(); }

		AA_CONSTEXPR reference top() { return *tdata(); }
		AA_CONSTEXPR const_reference top() const { return *tdata(); }
		AA_CONSTEXPR const_reference ctop() const { return top(); }



		// Iterators
		AA_CONSTEXPR iterator begin() { return data(); }
		AA_CONSTEXPR const_iterator begin() const { return data(); }
		AA_CONSTEXPR const_iterator cbegin() const { return begin(); }

		AA_CONSTEXPR iterator end() { return rdata() + 1; }
		AA_CONSTEXPR const_iterator end() const { return rdata() + 1; }
		AA_CONSTEXPR const_iterator cend() const { return end(); }

		AA_CONSTEXPR iterator rbegin() { return rdata(); }
		AA_CONSTEXPR const_iterator rbegin() const { return rdata(); }
		AA_CONSTEXPR const_iterator crbegin() const { return rbegin(); }

		AA_CONSTEXPR iterator rend() { return r_end; }
		AA_CONSTEXPR const_iterator rend() const { return r_end; }
		AA_CONSTEXPR const_iterator crend() const { return rend(); }



		// Capacity
		AA_CONSTEXPR bool empty() const { return r_curr == r_end; }
		AA_CONSTEXPR bool single() const { return !full() && r_curr == data(); }
		AA_CONSTEXPR bool full() const { return is_full; }

		AA_CONSTEXPR difference_type ssize() const { return full() ? N : (r_curr - r_end); }
		AA_CONSTEXPR size_type size() const { return unsign(ssize()); }

		static AA_CONSTEVAL size_type max_size() { return N; }
		static AA_CONSTEVAL size_type max_index() { return N - 1; }

		AA_CONSTEXPR difference_type sindexl() const { return full() ? max_index() : (r_curr - data()); }
		AA_CONSTEXPR size_type indexl() const { return unsign(sindexl()); }



		// Modifiers
		AA_CONSTEXPR void clear() {
			is_full = false;
			r_curr = r_end;
		}

		AA_CONSTEXPR iterator push() {
			if (r_curr == r_begin) {
				is_full = true;
				return r_curr = data();
			} else {
				return ++r_curr;
			}
		}

		template<class... A>
			requires (std::constructible_from<value_type, A...>)
		AA_CONSTEXPR void emplace(A&&... args) {
			std::ranges::construct_at(push(), std::forward<A>(args)...);
		}

		template<assignable_to<reference> V>
		AA_CONSTEXPR void insert(V &&value) {
			*push() = std::forward<V>(value);
		}



		// Special member functions
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
		AA_CONSTEXPR fixed_circular_vector() {}
#pragma GCC diagnostic pop



		// Member objects
		std::array<value_type, N> elements;

	protected:
		bool is_full = false;
		value_type *const r_end = elements.data() - 1, *const r_begin = elements.data() + max_index(), *r_curr = r_end;
	};

}
