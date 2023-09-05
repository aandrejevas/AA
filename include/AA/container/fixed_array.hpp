#pragma once

#include "../metaprogramming/general.hpp"
#include <memory> // unique_ptr, make_unique_for_overwrite
#include <algorithm> // copy



namespace aa {

	// https://en.wikipedia.org/wiki/Array_data_structure
	template<class T, size_t N>
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
		using container_type = std::array<value_type, N>;



		// Element access
		template<size_type I>
		constexpr reference get() requires (std::cmp_less(I, N)) { return get(I); }
		template<size_type I>
		constexpr const_reference get() const requires (std::cmp_less(I, N)) { return get(I); }

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

		constexpr pointer rdata() { return r_begin; }
		constexpr const_pointer rdata() const { return r_begin; }

		constexpr reference front() { return *data(); }
		constexpr const_reference front() const { return *data(); }

		constexpr reference back() { return *rdata(); }
		constexpr const_reference back() const { return *rdata(); }



		// Iterators
		constexpr iterator begin() { return data(); }
		constexpr const_iterator begin() const { return data(); }

		constexpr iterator end() { return rdata() + 1; }
		constexpr const_iterator end() const { return rdata() + 1; }

		constexpr iterator rbegin() { return rdata(); }
		constexpr const_iterator rbegin() const { return rdata(); }

		constexpr iterator rend() { return data() - 1; }
		constexpr const_iterator rend() const { return data() - 1; }



		// Capacity
		static consteval bool empty() { return !N; }
		static consteval bool single() { return N == 1; }
		static consteval size_type size() { return N; }
		static consteval size_type max_size() { return N; }
		static consteval size_type tuple_size() { return N; }
		static consteval size_type indexl() { return N - 1; }
		static consteval size_type max_index() { return N - 1; }



		// Special member functions
		constexpr fixed_array() {}
		template<std::ranges::input_range R>
		constexpr fixed_array(R &&r) { std::ranges::copy(r, elements.data()); }



		// Member objects
		container_type elements;

	protected:
		// Šitas kintamasis turi būti paslėptas, nes kitaip jis suteiktų galimybę naudotojui keisti const elementus.
		value_type *const r_begin = elements.data() + max_index();
	};



	template<class T>
	using alloc_array = fixed_array<T, numeric_max>;

	template<class T>
	struct fixed_array<T, numeric_max> {
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

		constexpr pointer rdata() { return r_begin; }
		constexpr const_pointer rdata() const { return r_begin; }

		constexpr reference front() { return *data(); }
		constexpr const_reference front() const { return *data(); }

		constexpr reference back() { return *rdata(); }
		constexpr const_reference back() const { return *rdata(); }



		// Iterators
		constexpr iterator begin() { return data(); }
		constexpr const_iterator begin() const { return data(); }

		constexpr iterator end() { return rdata() + 1; }
		constexpr const_iterator end() const { return rdata() + 1; }

		constexpr iterator rbegin() { return rdata(); }
		constexpr const_iterator rbegin() const { return rdata(); }

		constexpr iterator rend() { return data() - 1; }
		constexpr const_iterator rend() const { return data() - 1; }



		// Capacity
		constexpr bool empty() const { return r_begin < data(); }
		constexpr bool single() const { return r_begin == data(); }

		constexpr difference_type ssize() const { return sindexl() + 1; }
		constexpr size_type size() const { return indexl() + 1; }
		constexpr difference_type sindexl() const { return r_begin - data(); }
		constexpr size_type indexl() const { return unsign(sindexl()); }

		static consteval size_type max_size() { return numeric_max; }
		static consteval size_type max_index() { return max_size() - 1; }



		// Special member functions
		constexpr fixed_array()
			: r_begin{nullptr} {}
		constexpr fixed_array(alloc_array<value_type> &&o)
			: elements{std::move(o.elements)}, r_begin{o.r_begin} {}
		constexpr fixed_array(const size_t size)
			: elements{std::make_unique_for_overwrite<value_type[]>(size)}, r_begin{(elements.get() - 1) + size} {}



		// Member objects
	protected:
		std::unique_ptr<value_type[]> elements;
		value_type *const r_begin;
	};

}
