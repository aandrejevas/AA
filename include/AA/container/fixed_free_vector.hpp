#pragma once

#include "../metaprogramming/general.hpp"
#include "fixed_vector.hpp"
#include <iterator> // random_access_iterator_tag
#include <compare> // strong_ordering



namespace aa {

	template<trivially_copyable T, size_t N>
	struct fixed_free_vector {
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
		struct node_type {
			using union_type = std::variant<node_type *, value_type>;

			union_type v;
		};

		using union_type = typename node_type::union_type;

		// Member constants
		static AA_CONSTEXPR const size_type hole_index = 0, elem_index = 1;

		template<class P1, class P2>
		struct variant_iterator {
			using value_type = P1;
			using difference_type = difference_type;
			using reference = value_type;
			using pointer = value_type;
			using iterator_category = std::random_access_iterator_tag;

			AA_CONSTEXPR reference operator*() const { return get_elem(ptr); }
			AA_CONSTEXPR pointer operator->() const { return get_elem(ptr); }

			AA_CONSTEXPR variant_iterator &operator++() { ++ptr; return *this; }
			AA_CONSTEXPR variant_iterator operator++(const int) { return {ptr++}; }
			AA_CONSTEXPR variant_iterator &operator--() { --ptr; return *this; }
			AA_CONSTEXPR variant_iterator operator--(const int) { return {ptr--}; }

			friend AA_CONSTEXPR bool operator==(const variant_iterator &l, const variant_iterator &r) { return l.ptr == r.ptr; }
			friend AA_CONSTEXPR std::strong_ordering operator<=>(const variant_iterator &l, const variant_iterator &r) { return l.ptr <=> r.ptr; }

			AA_CONSTEXPR difference_type operator-(const variant_iterator &r) const { return ptr - r.ptr; }
			AA_CONSTEXPR reference operator[](const difference_type n) const { return get_elem(ptr + n); }
			AA_CONSTEXPR variant_iterator operator+(const difference_type n) const { return {ptr + n}; }
			AA_CONSTEXPR variant_iterator operator-(const difference_type n) const { return {ptr - n}; }
			AA_CONSTEXPR variant_iterator &operator+=(const difference_type n) { ptr += n; return *this; }
			AA_CONSTEXPR variant_iterator &operator-=(const difference_type n) { ptr -= n; return *this; }
			friend AA_CONSTEXPR variant_iterator operator+(const difference_type n, const variant_iterator &r) { return {n + r.ptr}; }

			AA_CONSTEVAL variant_iterator() = default;

		protected:
			friend fixed_free_vector;
			using node_type = P2;

			AA_CONSTEXPR variant_iterator(node_type *const p) : ptr{p} {}

			// Kintamasis protected, nes prie duomenų patekti reikia specifiniu būdu (žr. operator*).
			node_type *ptr;
		};

	public:
		using iterator = variant_iterator<pointer, node_type>;
		using const_iterator = variant_iterator<const_pointer, const node_type>;



		// Element access
	protected:
		static AA_CONSTEXPR union_type &unwrap(node_type *const n) { return n->v; }
		static AA_CONSTEXPR const union_type &unwrap(const node_type *const n) { return n->v; }

		static AA_CONSTEXPR pointer get_elem(node_type *const n) { return std::get_if<elem_index>(&unwrap(n)); }
		static AA_CONSTEXPR const_pointer get_elem(const node_type *const n) { return std::get_if<elem_index>(&unwrap(n)); }

	public:
		AA_CONSTEXPR pointer operator[](const size_type pos) { return get(pos); }
		AA_CONSTEXPR const_pointer operator[](const size_type pos) const { return get(pos); }

		AA_CONSTEXPR pointer get(const size_type pos) { return get_elem(elements.data(pos)); }
		AA_CONSTEXPR const_pointer get(const size_type pos) const { return get_elem(elements.data(pos)); }
		AA_CONSTEXPR const_pointer cget(const size_type pos) const { return get(pos); }

		AA_CONSTEXPR pointer rget(const size_type pos) { return get_elem(elements.rdata(pos)); }
		AA_CONSTEXPR const_pointer rget(const size_type pos) const { return get_elem(elements.rdata(pos)); }
		AA_CONSTEXPR const_pointer crget(const size_type pos) const { return rget(pos); }

		AA_CONSTEXPR pointer front() { return get_elem(elements.data()); }
		AA_CONSTEXPR const_pointer front() const { return get_elem(elements.data()); }
		AA_CONSTEXPR const_pointer cfront() const { return front(); }

		AA_CONSTEXPR pointer back() { return get_elem(elements.rdata()); }
		AA_CONSTEXPR const_pointer back() const { return get_elem(elements.rdata()); }
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

		AA_CONSTEXPR size_type holes_count() const {
			if (first_hole) {
				size_type count = 0;
				const node_type *iter = first_hole;
				do {
					++count;
				} while ((iter = std::get<hole_index>(unwrap(iter))));
				return count;
			} else return 0;
		}

		AA_CONSTEXPR bool full() const { return elements.full(); }
		AA_CONSTEXPR bool empty() const { return elements.empty(); }
		AA_CONSTEXPR bool single() const { return elements.single(); }

		AA_CONSTEXPR difference_type ssize_wo_holes() const { return sign(size()); }
		AA_CONSTEXPR size_type size_wo_holes() const { return size() - holes_count(); }
		AA_CONSTEXPR difference_type ssize() const { return elements.ssize(); }
		AA_CONSTEXPR size_type size() const { return elements.size(); }

		static AA_CONSTEVAL size_type max_size() { return N; }
		static AA_CONSTEVAL size_type max_index() { return N - 1; }

		AA_CONSTEXPR difference_type sindexl() const { return elements.sindexl(); }
		AA_CONSTEXPR size_type indexl() const { return elements.indexl(); }



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
				union_type &hole = unwrap(first_hole);
				first_hole = std::get<hole_index>(hole);
				return hole.template emplace<elem_index>(std::forward<A>(args)...);
			} else {
				return unwrap(elements.push_back()).template emplace<elem_index>(std::forward<A>(args)...);
			}
		}

		template<assignable_to<reference> V>
		AA_CONSTEXPR void insert(V &&value) {
			if (first_hole) {
				union_type &hole = unwrap(first_hole);
				first_hole = std::get<hole_index>(hole);
				hole = std::forward<V>(value);
			} else {
				unwrap(elements.push_back()) = std::forward<V>(value);
			}
		}

		AA_CONSTEXPR void erase(const size_type pos) {
			node_type *const element = elements.data(pos);
			if (unwrap(element).index() == elem_index) {
				unwrap(element).template emplace<hole_index>(first_hole);
				first_hole = element;
			}
		}

		AA_CONSTEXPR void erase(const pointer pos) {
			node_type *const element = std::bit_cast<node_type *>(pos);
			unwrap(element).template emplace<hole_index>(first_hole);
			first_hole = element;
		}



		// Special member functions
		AA_CONSTEXPR fixed_free_vector() {}



		// Member objects
	protected:
		fixed_vector<node_type, N> elements;
		node_type *first_hole = nullptr;
	};

}
