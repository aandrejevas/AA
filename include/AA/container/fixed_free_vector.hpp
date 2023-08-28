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
		//
		// Masyvas node_type'ų yra memset'inamas, nes variant default constructor
		// inicializuoja dalykus nors mums tokio funkcionalumo nereikia.
		struct node_type : std::variant<node_type *, value_type> {
			using std::variant<node_type *, value_type>::operator=;
		};

		// Member constants
		static constexpr size_type hole_index = 0, elem_index = 1;

		template<class P1, class P2>
		struct variant_iterator {
			using value_type = P1;
			using difference_type = difference_type;
			using reference = value_type;
			using pointer = value_type;
			using iterator_category = std::random_access_iterator_tag;

			constexpr reference operator*() const { return std::get_if<elem_index>(ptr); }
			constexpr pointer operator->() const { return std::get_if<elem_index>(ptr); }

			constexpr variant_iterator &operator++() { ++ptr; return *this; }
			constexpr variant_iterator operator++(const int) { return {ptr++}; }
			constexpr variant_iterator &operator--() { --ptr; return *this; }
			constexpr variant_iterator operator--(const int) { return {ptr--}; }

			friend constexpr bool operator==(const variant_iterator &l, const variant_iterator &r) { return l.ptr == r.ptr; }
			friend constexpr std::strong_ordering operator<=>(const variant_iterator &l, const variant_iterator &r) { return l.ptr <=> r.ptr; }

			constexpr difference_type operator-(const variant_iterator &r) const { return ptr - r.ptr; }
			constexpr reference operator[](const difference_type n) const { return std::get_if<elem_index>(ptr + n); }
			constexpr variant_iterator operator+(const difference_type n) const { return {ptr + n}; }
			constexpr variant_iterator operator-(const difference_type n) const { return {ptr - n}; }
			constexpr variant_iterator &operator+=(const difference_type n) { ptr += n; return *this; }
			constexpr variant_iterator &operator-=(const difference_type n) { ptr -= n; return *this; }
			friend constexpr variant_iterator operator+(const difference_type n, const variant_iterator &r) { return {n + r.ptr}; }

			consteval variant_iterator() = default;

		protected:
			friend fixed_free_vector;
			using node_type = P2;

			constexpr variant_iterator(node_type *const p) : ptr{p} {}

			// Kintamasis protected, nes prie duomenų patekti reikia specifiniu būdu (žr. operator*).
			node_type *ptr;
		};

	public:
		using iterator = variant_iterator<pointer, node_type>;
		using const_iterator = variant_iterator<const_pointer, const node_type>;



		// Element access
		constexpr pointer operator[](const size_type pos) { return get(pos); }
		constexpr const_pointer operator[](const size_type pos) const { return get(pos); }

		constexpr pointer get(const size_type pos) { return std::get_if<elem_index>(elements.data(pos)); }
		constexpr const_pointer get(const size_type pos) const { return std::get_if<elem_index>(elements.data(pos)); }

		constexpr pointer rget(const size_type pos) { return std::get_if<elem_index>(elements.rdata(pos)); }
		constexpr const_pointer rget(const size_type pos) const { return std::get_if<elem_index>(elements.rdata(pos)); }

		constexpr pointer front() { return std::get_if<elem_index>(elements.data()); }
		constexpr const_pointer front() const { return std::get_if<elem_index>(elements.data()); }

		constexpr pointer back() { return std::get_if<elem_index>(elements.rdata()); }
		constexpr const_pointer back() const { return std::get_if<elem_index>(elements.rdata()); }



		// Iterators
		constexpr iterator begin() { return {elements.begin()}; }
		constexpr const_iterator begin() const { return {elements.begin()}; }

		constexpr iterator end() { return {elements.end()}; }
		constexpr const_iterator end() const { return {elements.end()}; }

		constexpr iterator rbegin() { return {elements.rbegin()}; }
		constexpr const_iterator rbegin() const { return {elements.rbegin()}; }

		constexpr iterator rend() { return {elements.rend()}; }
		constexpr const_iterator rend() const { return {elements.rend()}; }



		// Capacity
		constexpr bool has_holes() const { return first_hole; }

		constexpr size_type holes_count() const {
			if (first_hole) {
				size_type count = 0;
				const node_type *iter = first_hole;
				do {
					++count;
				} while ((iter = std::get<hole_index>(*iter)));
				return count;
			} else return 0;
		}

		constexpr bool full() const { return elements.full(); }
		constexpr bool empty() const { return elements.empty(); }
		constexpr bool single() const { return elements.single(); }

		constexpr size_type size_wo_holes() const { return size() - holes_count(); }
		constexpr difference_type ssize() const { return elements.ssize(); }
		constexpr size_type size() const { return elements.size(); }

		static consteval size_type max_size() { return N; }
		static consteval size_type max_index() { return N - 1; }

		constexpr difference_type sindexl() const { return elements.sindexl(); }
		constexpr size_type indexl() const { return elements.indexl(); }



		// Modifiers
		constexpr void clear() { elements.clear(); first_hole = nullptr; }

		template<ref_invocable<pointer> F>
		constexpr void clear(F &&f) {
			if (!elements.empty())
				unsafe_clear(f);
		}

		template<ref_invocable<pointer> F>
		constexpr void unsafe_clear(F &&f) {
			do {
				std::invoke(f, back());
				elements.pop_back();
			} while (!elements.empty());
			first_hole = nullptr;
		}

		template<class... A>
			requires (std::constructible_from<value_type, A...>)
		constexpr reference emplace(A&&... args) {
			if (first_hole) {
				node_type &hole = *first_hole;
				first_hole = std::get<hole_index>(hole);
				return hole.template emplace<elem_index>(std::forward<A>(args)...);
			} else {
				return elements.push_back()->template emplace<elem_index>(std::forward<A>(args)...);
			}
		}

		template<assignable_to<reference> V>
		constexpr void insert(V &&value) {
			if (first_hole) {
				node_type &hole = *first_hole;
				first_hole = std::get<hole_index>(hole);
				hole = std::forward<V>(value);
			} else {
				*elements.push_back() = std::forward<V>(value);
			}
		}

		constexpr void erase(const size_type pos) {
			node_type *const element = elements.data(pos);
			if (element->index() == elem_index) {
				element->template emplace<hole_index>(first_hole);
				first_hole = element;
			}
		}

		constexpr void erase(const pointer pos) {
			node_type *const element = std::bit_cast<node_type *>(pos);
			element->template emplace<hole_index>(first_hole);
			first_hole = element;
		}



		// Special member functions
		constexpr fixed_free_vector() {}



		// Member objects
	protected:
		fixed_vector<node_type, N> elements;
		node_type *first_hole = nullptr;
	};

}
