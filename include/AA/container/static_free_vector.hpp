#pragma once

#include "../metaprogramming/general.hpp"
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

	protected:
		// node_type tipas paslepiamas, nes nėra jokio viešo funkcionalumo kuriame dalyvautų tipas.
		//
		// void* turi būti pirmas tipas, nes masyvo inicializavimo metu, bus inicializuojami visi jo elementai
		// ir variant default kontruktorius inicializuoja pirmą alternatyvą, makes sence, kad by default aktyvi
		// alternatyva būtų void* ir apskritai value_type gali neturėti default konstruktoriaus.
		using node_type = std::variant<void *, value_type>;

		static AA_CONSTEXPR const size_t hole_index = 0, elem_index = 1;

		template<class P1, class P2>
		struct variant_iterator {
			using value_type = P1;
			using difference_type = difference_type;
			using reference = value_type;
			using pointer = value_type;
			using iterator_category = std::random_access_iterator_tag;

			AA_CONSTEXPR reference operator*() const { return std::get_if<elem_index>(ptr); }
			AA_CONSTEXPR pointer operator->() const { return std::get_if<elem_index>(ptr); }

			AA_CONSTEXPR variant_iterator &operator++() { ++ptr; return *this; }
			AA_CONSTEXPR variant_iterator operator++(int) { return {ptr++}; }
			AA_CONSTEXPR variant_iterator &operator--() { --ptr; return *this; }
			AA_CONSTEXPR variant_iterator operator--(int) { return {ptr--}; }

			friend AA_CONSTEXPR bool operator==(const variant_iterator &l, const variant_iterator &r) { return l.ptr == r.ptr; }
			friend AA_CONSTEXPR std::strong_ordering operator<=>(const variant_iterator &l, const variant_iterator &r) { return l.ptr <=> r.ptr; }

			AA_CONSTEXPR difference_type operator-(const variant_iterator &r) const { return ptr - r.ptr; }
			AA_CONSTEXPR reference operator[](const difference_type n) const { return std::get_if<elem_index>(ptr + n); }
			AA_CONSTEXPR variant_iterator operator+(const difference_type n) const { return {ptr + n}; }
			AA_CONSTEXPR variant_iterator operator-(const difference_type n) const { return {ptr - n}; }
			AA_CONSTEXPR variant_iterator &operator+=(const difference_type n) { ptr += n; return *this; }
			AA_CONSTEXPR variant_iterator &operator-=(const difference_type n) { ptr -= n; return *this; }
			friend AA_CONSTEXPR variant_iterator operator+(const difference_type n, const variant_iterator &r) { return {n + r.ptr}; }

			AA_CONSTEXPR variant_iterator() = default;
			AA_CONSTEXPR variant_iterator(const variant_iterator &) = default;

		protected:
			friend static_free_vector;
			using node_type = P2;

			AA_CONSTEXPR variant_iterator(node_type *const p) : ptr{p} {}

			node_type *ptr;
		};

	public:
		using iterator = variant_iterator<pointer, node_type>;
		using const_iterator = variant_iterator<const_pointer, const node_type>;



		// Element access
		AA_CONSTEXPR pointer operator[](const size_type pos) { return std::get_if<elem_index>(elements.pointer_at(pos)); }
		AA_CONSTEXPR const_pointer operator[](const size_type pos) const { return std::get_if<elem_index>(elements.pointer_at(pos)); }

		AA_CONSTEXPR pointer at(const size_type pos) { return std::get_if<elem_index>(elements.pointer_at(pos)); }
		AA_CONSTEXPR const_pointer at(const size_type pos) const { return std::get_if<elem_index>(elements.pointer_at(pos)); }
		AA_CONSTEXPR const_pointer c_at(const size_type pos) const { return at(pos); }

		AA_CONSTEXPR pointer r_at(const size_type pos) { return std::get_if<elem_index>(elements.rpointer_at(pos)); }
		AA_CONSTEXPR const_pointer r_at(const size_type pos) const { return std::get_if<elem_index>(elements.rpointer_at(pos)); }
		AA_CONSTEXPR const_pointer cr_at(const size_type pos) const { return r_at(pos); }

		AA_CONSTEXPR pointer front() { return std::get_if<elem_index>(elements.data()); }
		AA_CONSTEXPR const_pointer front() const { return std::get_if<elem_index>(elements.data()); }
		AA_CONSTEXPR const_pointer cfront() const { return front(); }

		AA_CONSTEXPR pointer back() { return std::get_if<elem_index>(elements.rdata()); }
		AA_CONSTEXPR const_pointer back() const { return std::get_if<elem_index>(elements.rdata()); }
		AA_CONSTEXPR const_pointer cback() const { return back(); }



		// Iterators
		AA_CONSTEXPR iterator begin() { return {elements.begin()}; }
		AA_CONSTEXPR const_iterator begin() const { return {elements.begin()}; }
		AA_CONSTEXPR const_iterator cbegin() const { return begin(); }

		AA_CONSTEXPR iterator end() { return {elements.end()}; }
		AA_CONSTEXPR const_iterator end() const { return {elements.end()}; }
		AA_CONSTEXPR const_iterator cend() const { return end(); }

		AA_CONSTEXPR iterator rbegin() { return {elements.rbegin()}; }
		AA_CONSTEXPR const_iterator rbegin() const { return {elements.rbegin()}; }
		AA_CONSTEXPR const_iterator crbegin() const { return rbegin(); }

		AA_CONSTEXPR iterator rend() { return {elements.rend()}; }
		AA_CONSTEXPR const_iterator rend() const { return {elements.rend()}; }
		AA_CONSTEXPR const_iterator crend() const { return rend(); }



		// Capacity
		AA_CONSTEXPR bool has_holes() const { return first_hole; }

		AA_CONSTEXPR bool empty() const { return elements.empty(); }
		AA_CONSTEXPR bool full() const { return elements.full(); }

		AA_CONSTEXPR difference_type ssize() const { return elements.ssize(); }
		AA_CONSTEXPR size_type size() const { return elements.size(); }

		static AA_CONSTEVAL size_type max_size() { return N; }



		// Modifiers
		AA_CONSTEXPR void clear() { elements.clear(); first_hole = nullptr; }

		template<invocable_ref<pointer> F>
		AA_CONSTEXPR void clear(F &&f) {
			if (!elements.empty())
				unsafe_clear(f);
		}

		template<invocable_ref<pointer> F>
		AA_CONSTEXPR void unsafe_clear(F &&f) {
			do {
				std::invoke(f, back());
				elements.pop_back();
			} while (!elements.empty());
			first_hole = nullptr;
		}

		template<class... A>
			requires (std::constructible_from<value_type, A...>)
		AA_CONSTEXPR reference emplace(A&&... args) {
			if (first_hole) {
				node_type *const hole = first_hole;
				first_hole = static_cast<node_type *>(std::get<hole_index>(*hole));
				return hole->template emplace<elem_index>(std::forward<A>(args)...);
			} else {
				return elements.push_back()->template emplace<elem_index>(std::forward<A>(args)...);
			}
		}

		AA_CONSTEXPR void erase(const size_type pos) {
			node_type *const element = elements.pointer_at(pos);
			if (element->index() == elem_index) {
				element->template emplace<hole_index>(first_hole);
				first_hole = element;
			}
		}

		AA_CONSTEXPR void erase(const pointer pos) {
			node_type *const element = reinterpret_cast<node_type *>(pos);
			element->template emplace<hole_index>(first_hole);
			first_hole = element;
		}



		// Special member functions
		AA_CONSTEXPR static_free_vector() {}



		// Member objects
	protected:
		static_vector<node_type, N> elements;
		node_type *first_hole = nullptr;
	};

}
