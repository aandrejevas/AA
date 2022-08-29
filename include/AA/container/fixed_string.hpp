#pragma once

#include "../metaprogramming/general.hpp"
#include <cstddef> // size_t, ptrdiff_t
#include <iterator> // reverse_iterator
#include <algorithm> // copy_n
#include <string_view> // basic_string_view
#include <ostream> // basic_ostream



namespace aa {

	// https://en.wikipedia.org/wiki/Null-terminated_string
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
		using view_type = std::basic_string_view<value_type>;



		// Element access
		AA_CONSTEXPR reference operator[](const size_type pos) { return elem(pos); }
		AA_CONSTEXPR const_reference operator[](const size_type pos) const { return elem(pos); }

		AA_CONSTEXPR reference elem(const size_type pos) { return *data(pos); }
		AA_CONSTEXPR const_reference elem(const size_type pos) const { return *data(pos); }
		AA_CONSTEXPR const_reference celem(const size_type pos) const { return elem(pos); }

		AA_CONSTEXPR pointer data(const size_type pos) { return data() + pos; }
		AA_CONSTEXPR const_pointer data(const size_type pos) const { return data() + pos; }
		AA_CONSTEXPR const_pointer cdata(const size_type pos) const { return data(pos); }

		AA_CONSTEXPR pointer data() { return elements.data(); }
		AA_CONSTEXPR const_pointer data() const { return elements.data(); }
		AA_CONSTEXPR const_pointer cdata() const { return data(); }

		AA_CONSTEXPR reference front() { return *data(); }
		AA_CONSTEXPR const_reference front() const { return *data(); }
		AA_CONSTEXPR const_reference cfront() const { return front(); }

		AA_CONSTEXPR reference back() { return elem(max_index()); }
		AA_CONSTEXPR const_reference back() const { return elem(max_index()); }
		AA_CONSTEXPR const_reference cback() const { return back(); }

		AA_CONSTEXPR operator view_type() const { return view_type{data(), size()}; }



		// Iterators
		AA_CONSTEXPR iterator begin() { return elements.begin(); }
		AA_CONSTEXPR const_iterator begin() const { return elements.begin(); }
		AA_CONSTEXPR const_iterator cbegin() const { return begin(); }

		AA_CONSTEXPR iterator end() { return elements.begin() + size(); }
		AA_CONSTEXPR const_iterator end() const { return elements.begin() + size(); }
		AA_CONSTEXPR const_iterator cend() const { return end(); }

		AA_CONSTEXPR reverse_iterator rbegin() { return reverse_iterator{end()}; }
		AA_CONSTEXPR const_reverse_iterator rbegin() const { return const_reverse_iterator{end()}; }
		AA_CONSTEXPR const_reverse_iterator crbegin() const { return rbegin(); }

		AA_CONSTEXPR reverse_iterator rend() { return elements.rend(); }
		AA_CONSTEXPR const_reverse_iterator rend() const { return elements.rend(); }
		AA_CONSTEXPR const_reverse_iterator crend() const { return rend(); }



		// Capacity
		static AA_CONSTEVAL bool empty() { return !size(); }
		static AA_CONSTEVAL difference_type ssize() { return size(); }
		static AA_CONSTEVAL size_type size() { return N - 1; }
		static AA_CONSTEVAL size_type max_size() { return N; }
		static AA_CONSTEVAL size_type max_index() { return N - 2; }



		// Input/output
		template<class C, class CT>
		friend AA_CONSTEXPR std::basic_ostream<C, CT> &operator<<(std::basic_ostream<C, CT> &os, const basic_fixed_string &str) {
			return os.write(str.data(), str.size());
		}



		// Special member functions
		AA_CONSTEXPR basic_fixed_string(const value_type *const cstring) {
			std::ranges::copy_n(cstring, N, elements.data());
		}



		// Member objects
		array_t<value_type, N> elements;
	};



	template<class T, size_t N>
	basic_fixed_string(const T(&)[N])->basic_fixed_string<T, N>;

	template<size_t N>
	using fixed_string = basic_fixed_string<char, N>;

}