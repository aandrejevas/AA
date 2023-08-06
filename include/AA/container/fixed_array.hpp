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
		AA_CONSTEXPR reference get() requires (I < N) { return get(I); }
		template<size_type I>
		AA_CONSTEXPR const_reference get() const requires (I < N) { return get(I); }

		AA_CONSTEXPR reference operator[](const size_type pos) { return get(pos); }
		AA_CONSTEXPR const_reference operator[](const size_type pos) const { return get(pos); }

		AA_CONSTEXPR reference get(const size_type pos) { return *data(pos); }
		AA_CONSTEXPR const_reference get(const size_type pos) const { return *data(pos); }

		AA_CONSTEXPR reference rget(const size_type pos) { return *rdata(pos); }
		AA_CONSTEXPR const_reference rget(const size_type pos) const { return *rdata(pos); }

		AA_CONSTEXPR pointer data(const size_type pos) { return data() + pos; }
		AA_CONSTEXPR const_pointer data(const size_type pos) const { return data() + pos; }

		AA_CONSTEXPR pointer rdata(const size_type pos) { return rdata() - pos; }
		AA_CONSTEXPR const_pointer rdata(const size_type pos) const { return rdata() - pos; }

		AA_CONSTEXPR pointer data() { return elements.data(); }
		AA_CONSTEXPR const_pointer data() const { return elements.data(); }

		AA_CONSTEXPR pointer rdata() { return r_begin; }
		AA_CONSTEXPR const_pointer rdata() const { return r_begin; }

		AA_CONSTEXPR reference front() { return *data(); }
		AA_CONSTEXPR const_reference front() const { return *data(); }

		AA_CONSTEXPR reference back() { return *rdata(); }
		AA_CONSTEXPR const_reference back() const { return *rdata(); }



		// Iterators
		AA_CONSTEXPR iterator begin() { return data(); }
		AA_CONSTEXPR const_iterator begin() const { return data(); }

		AA_CONSTEXPR iterator end() { return rdata() + 1; }
		AA_CONSTEXPR const_iterator end() const { return rdata() + 1; }

		AA_CONSTEXPR iterator rbegin() { return rdata(); }
		AA_CONSTEXPR const_iterator rbegin() const { return rdata(); }

		AA_CONSTEXPR iterator rend() { return data() - 1; }
		AA_CONSTEXPR const_iterator rend() const { return data() - 1; }



		// Capacity
		static AA_CONSTEVAL bool empty() { return !N; }
		static AA_CONSTEVAL bool single() { return N == 1; }
		static AA_CONSTEVAL size_type size() { return N; }
		static AA_CONSTEVAL size_type max_size() { return N; }
		static AA_CONSTEVAL size_type tuple_size() { return N; }
		static AA_CONSTEVAL size_type indexl() { return N - 1; }
		static AA_CONSTEVAL size_type max_index() { return N - 1; }



		// Special member functions
		AA_CONSTEXPR fixed_array() {}
		template<std::ranges::input_range R>
		AA_CONSTEXPR fixed_array(R &&r) { std::ranges::copy(r, elements.data()); }



		// Member objects
		container_type elements;

	protected:
		// Šitas kintamasis turi būti paslėptas, nes kitaip jis suteiktų galimybę naudotojui keisti const elementus.
		value_type *const r_begin = elements.data() + indexl();
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
		AA_CONSTEXPR reference operator[](const size_type pos) { return get(pos); }
		AA_CONSTEXPR const_reference operator[](const size_type pos) const { return get(pos); }

		AA_CONSTEXPR reference get(const size_type pos) { return *data(pos); }
		AA_CONSTEXPR const_reference get(const size_type pos) const { return *data(pos); }

		AA_CONSTEXPR reference rget(const size_type pos) { return *rdata(pos); }
		AA_CONSTEXPR const_reference rget(const size_type pos) const { return *rdata(pos); }

		AA_CONSTEXPR pointer data(const size_type pos) { return data() + pos; }
		AA_CONSTEXPR const_pointer data(const size_type pos) const { return data() + pos; }

		AA_CONSTEXPR pointer rdata(const size_type pos) { return rdata() - pos; }
		AA_CONSTEXPR const_pointer rdata(const size_type pos) const { return rdata() - pos; }

		AA_CONSTEXPR pointer data() { return elements.get(); }
		AA_CONSTEXPR const_pointer data() const { return elements.get(); }

		AA_CONSTEXPR pointer rdata() { return r_begin; }
		AA_CONSTEXPR const_pointer rdata() const { return r_begin; }

		AA_CONSTEXPR reference front() { return *data(); }
		AA_CONSTEXPR const_reference front() const { return *data(); }

		AA_CONSTEXPR reference back() { return *rdata(); }
		AA_CONSTEXPR const_reference back() const { return *rdata(); }



		// Iterators
		AA_CONSTEXPR iterator begin() { return data(); }
		AA_CONSTEXPR const_iterator begin() const { return data(); }

		AA_CONSTEXPR iterator end() { return rdata() + 1; }
		AA_CONSTEXPR const_iterator end() const { return rdata() + 1; }

		AA_CONSTEXPR iterator rbegin() { return rdata(); }
		AA_CONSTEXPR const_iterator rbegin() const { return rdata(); }

		AA_CONSTEXPR iterator rend() { return data() - 1; }
		AA_CONSTEXPR const_iterator rend() const { return data() - 1; }



		// Capacity
		AA_CONSTEXPR bool empty() const { return r_begin < data(); }
		AA_CONSTEXPR bool single() const { return r_begin == data(); }

		AA_CONSTEXPR difference_type ssize() const { return sindexl() + 1; }
		AA_CONSTEXPR size_type size() const { return indexl() + 1; }
		AA_CONSTEXPR difference_type sindexl() const { return r_begin - data(); }
		AA_CONSTEXPR size_type indexl() const { return unsign(sindexl()); }

		static AA_CONSTEVAL size_type max_size() { return numeric_max; }
		static AA_CONSTEVAL size_type max_index() { return max_size() - 1; }



		// Special member functions
		AA_CONSTEXPR fixed_array()
			: r_begin{nullptr} {}
		AA_CONSTEXPR fixed_array(alloc_array<value_type> &&o)
			: elements{std::move(o.elements)}, r_begin{o.r_begin} {}
		AA_CONSTEXPR fixed_array(const size_t size)
			: elements{std::make_unique_for_overwrite<value_type[]>(size)}, r_begin{elements.get() + size - 1} {}



		// Member objects
	protected:
		std::unique_ptr<value_type[]> elements;
		value_type *const r_begin;
	};

}
