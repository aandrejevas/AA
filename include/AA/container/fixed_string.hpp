#pragma once

#include "../metaprogramming/general.hpp"
#include <cstddef> // size_t, ptrdiff_t
#include <cstring> // memcpy
#include <iterator> // reverse_iterator



namespace aa {

	// https://en.wikipedia.org/wiki/Array_data_structure
	template<trivially_copyable T, size_t N>
	struct basic_fixed_string {
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
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;



		// Element access
		AA_CONSTEXPR reference operator[](const size_type pos) { return at(pos); }
		AA_CONSTEXPR const_reference operator[](const size_type pos) const { return at(pos); }

		AA_CONSTEXPR reference at(const size_type pos) { return *pointer_at(pos); }
		AA_CONSTEXPR const_reference at(const size_type pos) const { return *pointer_at(pos); }
		AA_CONSTEXPR const_reference c_at(const size_type pos) const { return at(pos); }

		AA_CONSTEXPR pointer pointer_at(const size_type pos) { return data() + pos; }
		AA_CONSTEXPR const_pointer pointer_at(const size_type pos) const { return data() + pos; }
		AA_CONSTEXPR const_pointer cpointer_at(const size_type pos) const { return pointer_at(pos); }

		AA_CONSTEXPR pointer data() { return elements.data(); }
		AA_CONSTEXPR const_pointer data() const { return elements.data(); }
		AA_CONSTEXPR const_pointer cdata() const { return data(); }

		AA_CONSTEXPR reference front() { return *data(); }
		AA_CONSTEXPR const_reference front() const { return *data(); }
		AA_CONSTEXPR const_reference cfront() const { return front(); }

		AA_CONSTEXPR reference back() { return at(constant_v<N - 1>); }
		AA_CONSTEXPR const_reference back() const { return at(constant_v<N - 1>); }
		AA_CONSTEXPR const_reference cback() const { return back(); }



		// Iterators
		AA_CONSTEXPR iterator begin() { return elements.begin(); }
		AA_CONSTEXPR const_iterator begin() const { return elements.begin(); }
		AA_CONSTEXPR const_iterator cbegin() const { return begin(); }

		AA_CONSTEXPR iterator end() { return elements.begin() + N; }
		AA_CONSTEXPR const_iterator end() const { return elements.begin() + N; }
		AA_CONSTEXPR const_iterator cend() const { return end(); }

		AA_CONSTEXPR reverse_iterator rbegin() { return reverse_iterator{end()}; }
		AA_CONSTEXPR const_reverse_iterator rbegin() const { return const_reverse_iterator{end()}; }
		AA_CONSTEXPR const_reverse_iterator crbegin() const { return rbegin(); }

		AA_CONSTEXPR reverse_iterator rend() { return elements.rend(); }
		AA_CONSTEXPR const_reverse_iterator rend() const { return elements.rend(); }
		AA_CONSTEXPR const_reverse_iterator crend() const { return rend(); }



		// Capacity
		static AA_CONSTEVAL bool empty() { return !N; }
		static AA_CONSTEVAL difference_type ssize() { return N; }
		static AA_CONSTEVAL size_type size() { return N; }
		static AA_CONSTEVAL size_type max_size() { return N; }



		// Special member functions
		AA_CONSTEXPR basic_fixed_string(const value_type(&cstring)[N + 1]) {
			std::memcpy(elements.data(), cstring, constant_v<(N + 1) * sizeof(value_type)>);
		}



		// Member objects
	protected:
		array_t<value_type, N + 1> elements;
	};



	template<class T, size_t N>
	basic_fixed_string(const T(&)[N])->basic_fixed_string<T, N - 1>;

	template<size_t N>
	using fixed_string = basic_fixed_string<char, N>;

}
