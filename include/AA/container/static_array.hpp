#pragma once

#include "../metaprogramming/general.hpp"
#include <cstddef> // size_t, ptrdiff_t
#include <concepts> // invocable
#include <functional> // invoke
#include <utility> // forward



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



		// Element access
		AA_CONSTEXPR reference operator[](const size_type pos) { return at(pos); }
		AA_CONSTEXPR const_reference operator[](const size_type pos) const { return at(pos); }

		AA_CONSTEXPR reference at(const size_type pos) { return *pointer_at(pos); }
		AA_CONSTEXPR const_reference at(const size_type pos) const { return *pointer_at(pos); }
		AA_CONSTEXPR const_reference c_at(const size_type pos) const { return at(pos); }

		AA_CONSTEXPR reference r_at(const size_type pos) { return *rpointer_at(pos); }
		AA_CONSTEXPR const_reference r_at(const size_type pos) const { return *rpointer_at(pos); }
		AA_CONSTEXPR const_reference cr_at(const size_type pos) const { return r_at(pos); }

		AA_CONSTEXPR pointer pointer_at(const size_type pos) { return data() + pos; }
		AA_CONSTEXPR const_pointer pointer_at(const size_type pos) const { return data() + pos; }
		AA_CONSTEXPR const_pointer cpointer_at(const size_type pos) const { return pointer_at(pos); }

		AA_CONSTEXPR pointer rpointer_at(const size_type pos) { return r_begin - pos; }
		AA_CONSTEXPR const_pointer rpointer_at(const size_type pos) const { return r_begin - pos; }
		AA_CONSTEXPR const_pointer crpointer_at(const size_type pos) const { return rpointer_at(pos); }

		AA_CONSTEXPR pointer data() { return elements.data(); }
		AA_CONSTEXPR const_pointer data() const { return elements.data(); }
		AA_CONSTEXPR const_pointer cdata() const { return data(); }

		AA_CONSTEXPR pointer rdata() { return r_begin; }
		AA_CONSTEXPR const_pointer rdata() const { return r_begin; }
		AA_CONSTEXPR const_pointer crdata() const { return rdata(); }

		AA_CONSTEXPR reference front() { return *data(); }
		AA_CONSTEXPR const_reference front() const { return *data(); }
		AA_CONSTEXPR const_reference cfront() const { return front(); }

		AA_CONSTEXPR reference back() { return *r_begin; }
		AA_CONSTEXPR const_reference back() const { return *r_begin; }
		AA_CONSTEXPR const_reference cback() const { return back(); }



		// Iterators
		AA_CONSTEXPR iterator begin() { return data(); }
		AA_CONSTEXPR const_iterator begin() const { return data(); }
		AA_CONSTEXPR const_iterator cbegin() const { return begin(); }

		AA_CONSTEXPR iterator end() { return r_begin + 1; }
		AA_CONSTEXPR const_iterator end() const { return r_begin + 1; }
		AA_CONSTEXPR const_iterator cend() const { return end(); }

		AA_CONSTEXPR iterator rbegin() { return r_begin; }
		AA_CONSTEXPR const_iterator rbegin() const { return r_begin; }
		AA_CONSTEXPR const_iterator crbegin() const { return rbegin(); }

		AA_CONSTEXPR iterator rend() { return data() - 1; }
		AA_CONSTEXPR const_iterator rend() const { return data() - 1; }
		AA_CONSTEXPR const_iterator crend() const { return rend(); }



		// Capacity
		static AA_CONSTEVAL bool empty() { return !N; }
		static AA_CONSTEVAL difference_type ssize() { return N; }
		static AA_CONSTEVAL size_type size() { return N; }
		static AA_CONSTEVAL size_type max_size() { return N; }



		// Special member functions
		AA_CONSTEXPR fixed_array() {}

		template<std::invocable<fixed_array &> F>
		AA_CONSTEXPR fixed_array(F &&f) { std::invoke(std::forward<F>(f), *this); }



		// Member objects
		array_t<value_type, N> elements;

	protected:
		// Šitas kintamasis turi būti paslėptas, nes kitaip jis suteiktų galimybę naudotojui keisti const elementus.
		value_type *const r_begin = elements.data() + constant_v<N - 1>;
	};

}
