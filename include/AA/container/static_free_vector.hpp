#pragma once

#include "static_vector.hpp"
#include <cstddef> // size_t, ptrdiff_t
#include <variant> // variant, get, get_if
#include <utility> // forward
#include <concepts> // constructible_from
#include <iterator> // random_access_iterator_tag
#include <compare> // strong_ordering
#include <functional> // invoke



namespace aa {

	template<trivially_copyable T, size_t N>
	struct static_free_vector {
		// Member types
		using value_type = T;
		using size_type = size_t;
		using difference_type = ptrdiff_t;
		using reference = value_type &;
		using const_reference = const value_type &;
		using pointer = value_type *;
		using const_pointer = const value_type *;
		using container_type = static_free_vector<T, N>;

		// void* turi būti pirmas tipas, nes masyvo inicializavimo metu, bus inicializuojami visi jo elementai
		// ir variant default kontruktorius inicializuoja pirmą alternatyvą, makes sence, kad by default aktyvi
		// alternatyva būtų void* ir apskritai value_type gali neturėti default konstruktoriaus.
		using node_type = std::variant<void *, value_type>;

	protected:
		inline static constexpr const size_t hole_index = 0, elem_index = 1;

		template<class P1, class P2>
		struct variant_iterator {
			using value_type = P1;
			using difference_type = difference_type;
			using reference = value_type;
			using pointer = value_type;
			using iterator_category = std::random_access_iterator_tag;
			using iterator_type = variant_iterator<P1, P2>;

			inline constexpr reference operator*() const { return std::get_if<elem_index>(ptr); }
			inline constexpr pointer operator->() const { return std::get_if<elem_index>(ptr); }

			inline constexpr iterator_type &operator++() { ++ptr; return *this; }
			inline constexpr iterator_type operator++(int) { return {ptr++}; }
			inline constexpr iterator_type &operator--() { --ptr; return *this; }
			inline constexpr iterator_type operator--(int) { return {ptr--}; }

			friend inline constexpr bool operator==(const iterator_type &l, const iterator_type &r) { return l.ptr == r.ptr; }
			friend inline constexpr std::strong_ordering operator<=>(const iterator_type &l, const iterator_type &r) { return l.ptr <=> r.ptr; }

			inline constexpr difference_type operator-(const iterator_type &r) const { return ptr - r.ptr; }
			inline constexpr reference operator[](const difference_type n) const { return std::get_if<elem_index>(ptr + n); }
			inline constexpr iterator_type operator+(const difference_type n) const { return {ptr + n}; }
			inline constexpr iterator_type operator-(const difference_type n) const { return {ptr - n}; }
			inline constexpr iterator_type &operator+=(const difference_type n) { ptr += n; return *this; }
			inline constexpr iterator_type &operator-=(const difference_type n) { ptr -= n; return *this; }
			friend inline constexpr iterator_type operator+(const difference_type n, const iterator_type &r) { return {n + r.ptr}; }

			inline constexpr variant_iterator() = default;
			inline constexpr variant_iterator(const iterator_type &) = default;

		protected:
			friend container_type;
			using node_type = P2;

			inline constexpr variant_iterator(node_type *const p) : ptr{p} {}

			node_type *ptr;
		};

	public:
		using iterator = variant_iterator<pointer, node_type>;
		using const_iterator = variant_iterator<const_pointer, const node_type>;



		// Element access
		inline constexpr pointer operator[](const size_type pos) { return std::get_if<elem_index>(elements.pointer_at(pos)); }
		inline constexpr const_pointer operator[](const size_type pos) const { return std::get_if<elem_index>(elements.pointer_at(pos)); }

		inline constexpr pointer at(const size_type pos) { return std::get_if<elem_index>(elements.pointer_at(pos)); }
		inline constexpr const_pointer at(const size_type pos) const { return std::get_if<elem_index>(elements.pointer_at(pos)); }
		inline constexpr const_pointer c_at(const size_type pos) const { return at(pos); }

		inline constexpr pointer r_at(const size_type pos) { return std::get_if<elem_index>(elements.rpointer_at(pos)); }
		inline constexpr const_pointer r_at(const size_type pos) const { return std::get_if<elem_index>(elements.rpointer_at(pos)); }
		inline constexpr const_pointer cr_at(const size_type pos) const { return r_at(pos); }

		inline constexpr pointer front() { return std::get_if<elem_index>(elements.data()); }
		inline constexpr const_pointer front() const { return std::get_if<elem_index>(elements.data()); }
		inline constexpr const_pointer cfront() const { return front(); }

		inline constexpr pointer back() { return std::get_if<elem_index>(elements.rdata()); }
		inline constexpr const_pointer back() const { return std::get_if<elem_index>(elements.rdata()); }
		inline constexpr const_pointer cback() const { return back(); }



		// Iterators
		inline constexpr iterator begin() { return {elements.begin()}; }
		inline constexpr const_iterator begin() const { return {elements.begin()}; }
		inline constexpr const_iterator cbegin() const { return begin(); }

		inline constexpr iterator end() { return {elements.end()}; }
		inline constexpr const_iterator end() const { return {elements.end()}; }
		inline constexpr const_iterator cend() const { return end(); }

		inline constexpr iterator rbegin() { return {elements.rbegin()}; }
		inline constexpr const_iterator rbegin() const { return {elements.rbegin()}; }
		inline constexpr const_iterator crbegin() const { return rbegin(); }

		inline constexpr iterator rend() { return {elements.rend()}; }
		inline constexpr const_iterator rend() const { return {elements.rend()}; }
		inline constexpr const_iterator crend() const { return rend(); }



		// Capacity
		inline constexpr bool has_holes() const { return first_hole; }

		inline constexpr bool empty() const { return elements.empty(); }
		inline constexpr bool full() const { return elements.full(); }

		inline constexpr difference_type ssize() const { return elements.ssize(); }
		inline constexpr size_type size() const { return elements.size(); }

		static inline consteval size_type max_size() { return N; }



		// Modifiers
		inline constexpr void clear() { elements.clear(); first_hole = nullptr; }

		template<invocable_ref<pointer> F>
		inline constexpr void clear(F &&f) {
			if (!elements.empty())
				unsafe_clear(f);
		}

		template<invocable_ref<pointer> F>
		inline constexpr void unsafe_clear(F &&f) {
			do {
				std::invoke(f, back());
				elements.pop_back();
			} while (!elements.empty());
			first_hole = nullptr;
		}

		template<class... A>
			requires (std::constructible_from<value_type, A...>)
		inline constexpr reference emplace(A&&... args) {
			if (first_hole) {
				node_type *const hole = first_hole;
				first_hole = static_cast<node_type *>(std::get<hole_index>(*hole));
				return hole->template emplace<elem_index>(std::forward<A>(args)...);
			} else {
				elements.push_back();
				return elements.back().template emplace<elem_index>(std::forward<A>(args)...);
			}
		}

		inline constexpr void erase(const size_type pos) {
			node_type *const element = elements.pointer_at(pos);
			if (element->index() == elem_index) {
				element->template emplace<hole_index>(first_hole);
				first_hole = element;
			}
		}

		inline void erase(const pointer pos) {
			node_type *const element = reinterpret_cast<node_type *>(pos);
			element->template emplace<hole_index>(first_hole);
			first_hole = element;
		}



		// Special member functions
		inline constexpr static_free_vector() {}



		// Member objects
	protected:
		static_vector<node_type, N> elements;
		node_type *first_hole = nullptr;
	};

}
