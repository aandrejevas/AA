#pragma once

#include "../metaprogramming/general.hpp"
#include <cstddef> // size_t, ptrdiff_t



namespace aa {

	// https://en.wikipedia.org/wiki/Array_data_structure
	template<trivially_copyable T, size_t N>
	struct fixed_array {
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
		using container_type = array_t<value_type, N>;



		// Element access
		AA_CONSTEXPR reference operator[](const size_type pos) { return elem(pos); }
		AA_CONSTEXPR const_reference operator[](const size_type pos) const { return elem(pos); }

		AA_CONSTEXPR reference elem(const size_type pos) { return *data(pos); }
		AA_CONSTEXPR const_reference elem(const size_type pos) const { return *data(pos); }
		AA_CONSTEXPR const_reference celem(const size_type pos) const { return elem(pos); }

		AA_CONSTEXPR reference relem(const size_type pos) { return *rdata(pos); }
		AA_CONSTEXPR const_reference relem(const size_type pos) const { return *rdata(pos); }
		AA_CONSTEXPR const_reference crelem(const size_type pos) const { return relem(pos); }

		AA_CONSTEXPR pointer data(const size_type pos) { return data() + pos; }
		AA_CONSTEXPR const_pointer data(const size_type pos) const { return data() + pos; }
		AA_CONSTEXPR const_pointer cdata(const size_type pos) const { return data(pos); }

		AA_CONSTEXPR pointer rdata(const size_type pos) { return rdata() - pos; }
		AA_CONSTEXPR const_pointer rdata(const size_type pos) const { return rdata() - pos; }
		AA_CONSTEXPR const_pointer crdata(const size_type pos) const { return rdata(pos); }

		AA_CONSTEXPR pointer data() { return elements.data(); }
		AA_CONSTEXPR const_pointer data() const { return elements.data(); }
		AA_CONSTEXPR const_pointer cdata() const { return data(); }

		AA_CONSTEXPR pointer rdata() { return r_begin; }
		AA_CONSTEXPR const_pointer rdata() const { return r_begin; }
		AA_CONSTEXPR const_pointer crdata() const { return rdata(); }

		AA_CONSTEXPR reference front() { return *data(); }
		AA_CONSTEXPR const_reference front() const { return *data(); }
		AA_CONSTEXPR const_reference cfront() const { return front(); }

		AA_CONSTEXPR reference back() { return *rdata(); }
		AA_CONSTEXPR const_reference back() const { return *rdata(); }
		AA_CONSTEXPR const_reference cback() const { return back(); }



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

		AA_CONSTEXPR iterator rend() { return data() - 1; }
		AA_CONSTEXPR const_iterator rend() const { return data() - 1; }
		AA_CONSTEXPR const_iterator crend() const { return rend(); }



		// Capacity
		static AA_CONSTEVAL bool empty() { return !N; }
		static AA_CONSTEVAL difference_type ssize() { return N; }
		static AA_CONSTEVAL size_type size() { return N; }
		static AA_CONSTEVAL size_type max_size() { return N; }
		static AA_CONSTEVAL size_type max_index() { return N - 1; }



		// Special member functions
		AA_CONSTEXPR fixed_array() {}
		AA_CONSTEXPR fixed_array(const container_type &e) : elements{e} {}



		// Member objects
		container_type elements;

	protected:
		// Šitas kintamasis turi būti paslėptas, nes kitaip jis suteiktų galimybę naudotojui keisti const elementus.
		value_type *const r_begin = elements.data() + max_index();
	};

}
