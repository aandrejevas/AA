#pragma once

#include "../metaprogramming/general.hpp"
#include <memory> // unique_ptr, make_unique_for_overwrite



namespace aa {

	// Neturime fixed_array konteinerio, nes talpinimas pabaigos rodyklės nepagreitintų funkcijų, nes
	// greitaveika nenukenčia prie adreso pridėjus skaičių, kuris yra žinomas kompiliavimo metu.

	// https://en.wikipedia.org/wiki/Array_data_structure
	template<class T>
	struct alloc_array {
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

		constexpr pointer data() { return elements.get(); }
		constexpr const_pointer data() const { return elements.get(); }

		constexpr pointer rdata() { return const_cast<iterator>(r_begin); }
		constexpr const_pointer rdata() const { return r_begin; }

		constexpr reference front() { return *data(); }
		constexpr const_reference front() const { return *data(); }

		constexpr reference back() { return *rdata(); }
		constexpr const_reference back() const { return *rdata(); }



		// Iterators
		constexpr iterator begin() { return data(); }
		constexpr const_iterator begin() const { return data(); }

		constexpr const_iterator end() const { return rdata() + 1; }

		constexpr iterator rbegin() { return rdata(); }
		constexpr const_iterator rbegin() const { return rdata(); }

		constexpr const_iterator rend() const { return data() - 1; }



		// Capacity
		constexpr bool empty() const { return r_begin == rend(); }
		constexpr bool single() const { return r_begin == data(); }

		constexpr size_type size() const { return unsign(r_begin - rend()); }
		constexpr size_type last_index() const { return unsign(r_begin - data()); }

		static consteval size_type max_size() { return numeric_max; }
		static consteval size_type max_index() { return max_size() - 1; }



		// Special member functions
		constexpr alloc_array()
			: r_begin{nullptr} {}
		constexpr alloc_array(alloc_array<value_type> &&o)
			: elements{std::move(o.elements)}, r_begin{o.r_begin} {}
		constexpr alloc_array(const size_t size)
			: elements{std::make_unique_for_overwrite<value_type[]>(size)}, r_begin{rend() + size} {}



		// Member objects
	protected:
		std::unique_ptr<value_type[]> elements;

	public:
		const value_type *const r_begin;
	};

}
