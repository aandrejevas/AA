#pragma once

#include "../metaprogramming/general.hpp"
#include <cstddef> // size_t, ptrdiff_t
#include <vector> // vector
#include <memory> // allocator
#include <utility> // forward, move
#include <bit> // bit_cast



namespace aa {

	template<class T, class M = std::allocator<T>>
	struct queue {
		// Member types
		using value_type = T;
		using allocator_type = M;
		using size_type = size_t;
		using difference_type = ptrdiff_t;
		using reference = value_type &;
		using const_reference = const value_type &;
		using pointer = value_type *;
		using const_pointer = const value_type *;
		using iterator = std::vector<value_type, allocator_type>::iterator;
		using const_iterator = std::vector<value_type, allocator_type>::const_iterator;
		using reverse_iterator = std::vector<value_type, allocator_type>::reverse_iterator;
		using const_reverse_iterator = std::vector<value_type, allocator_type>::const_reverse_iterator;



		// Element access
		AA_CONSTEXPR reference operator[](const size_type pos) { return elem(pos); }
		AA_CONSTEXPR const_reference operator[](const size_type pos) const { return elem(pos); }

		AA_CONSTEXPR reference elem(const size_type pos) { return *data(pos); }
		AA_CONSTEXPR const_reference elem(const size_type pos) const { return *data(pos); }
		AA_CONSTEXPR const_reference celem(const size_type pos) const { return elem(pos); }

		AA_CONSTEXPR pointer data(const size_type pos) { return data() + pos; }
		AA_CONSTEXPR const_pointer data(const size_type pos) const { return data() + pos; }
		AA_CONSTEXPR const_pointer cdata(const size_type pos) const { return data(pos); }

		AA_CONSTEXPR pointer data() { return elements.data() + front_index; }
		AA_CONSTEXPR const_pointer data() const { return elements.data() + front_index; }
		AA_CONSTEXPR const_pointer cdata() const { return data(); }

		AA_CONSTEXPR reference front() { return *data(); }
		AA_CONSTEXPR const_reference front() const { return *data(); }
		AA_CONSTEXPR const_reference cfront() const { return front(); }

		AA_CONSTEXPR reference back() { return elements.back(); }
		AA_CONSTEXPR const_reference back() const { return elements.back(); }
		AA_CONSTEXPR const_reference cback() const { return back(); }



		// Iterators
		AA_CONSTEXPR iterator begin() { return iterator{data()}; }
		AA_CONSTEXPR const_iterator begin() const { return const_iterator{data()}; }
		AA_CONSTEXPR const_iterator cbegin() const { return begin(); }

		AA_CONSTEXPR iterator end() { return elements.end(); }
		AA_CONSTEXPR const_iterator end() const { return elements.end(); }
		AA_CONSTEXPR const_iterator cend() const { return end(); }

		AA_CONSTEXPR reverse_iterator rbegin() { return elements.rbegin(); }
		AA_CONSTEXPR const_reverse_iterator rbegin() const { return elements.rbegin(); }
		AA_CONSTEXPR const_reverse_iterator crbegin() const { return rbegin(); }

		AA_CONSTEXPR reverse_iterator rend() { return reverse_iterator{begin()}; }
		AA_CONSTEXPR const_reverse_iterator rend() const { return const_reverse_iterator{begin()}; }
		AA_CONSTEXPR const_reverse_iterator crend() const { return rend(); }



		// Capacity
		AA_CONSTEXPR bool empty() const { return elements.empty(); }

		AA_CONSTEXPR size_type size() const { return elements.size() - front_index; }
		AA_CONSTEXPR difference_type ssize() const { return std::bit_cast<difference_type>(size()); }
		AA_CONSTEXPR size_type max_size() const { return elements.max_size(); }
		AA_CONSTEXPR size_type capacity() const { return elements.capacity(); }

		AA_CONSTEXPR void reserve(const size_type cap) { elements.reserve(cap); }
		AA_CONSTEXPR void shrink_to_fit() { elements.shrink_to_fit(); }



		// Observers
		AA_CONSTEXPR const allocator_type &get_allocator() const { return elements.get_allocator(); }



		// Modifiers
		AA_CONSTEXPR void clear() { elements.clear(); front_index = 0; }

		AA_CONSTEXPR void push_back(const value_type &value) { elements.push_back(value); }
		AA_CONSTEXPR void push_back(value_type &&value) { elements.push_back(std::move(value)); }

		template<class... A>
		AA_CONSTEXPR reference emplace_back(A&&... args) { return elements.emplace_back(std::forward<A>(args)...); }

		AA_CONSTEXPR void pop_front() {
			if (++front_index == elements.size())
				clear();
		}



		// Special member functions
		AA_CONSTEXPR queue() {}



		// Member objects
	protected:
		std::vector<value_type, allocator_type> elements;
		// Negalime naudoti rodyklių, nes gali pasikeisti vieta kur elementai yra talpinami.
		size_type front_index = 0;
	};

}
