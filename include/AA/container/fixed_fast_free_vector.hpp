#pragma once

#include "../metaprogramming/general.hpp"
#include "fixed_vector.hpp"
#include <memory> // construct_at



namespace aa {

	template<trivially_copyable T, size_t N>
	struct fixed_fast_free_vector {
		// Member types
		using value_type = T;
		using size_type = size_t;
		using difference_type = ptrdiff_t;
		using reference = value_type &;
		using const_reference = const value_type &;
		using pointer = value_type *;
		using const_pointer = const value_type *;

	protected:
		struct node_type {
			union {
				node_type *next;
				value_type element;
			};
		};



		// Capacity
	public:
		AA_CONSTEXPR bool has_holes() const { return first_hole; }

		AA_CONSTEXPR size_type holes_count() const {
			if (first_hole) {
				size_type count = 0;
				const node_type *iter = first_hole;
				do {
					++count;
				} while ((iter = iter->next));
				return count;
			} else return 0;
		}

		AA_CONSTEXPR bool full() const { return elements.full(); }
		AA_CONSTEXPR bool empty() const { return elements.empty(); }
		AA_CONSTEXPR bool single() const { return elements.single(); }

		AA_CONSTEXPR size_type size_wo_holes() const { return size() - holes_count(); }
		AA_CONSTEXPR difference_type ssize() const { return elements.ssize(); }
		AA_CONSTEXPR size_type size() const { return elements.size(); }

		static AA_CONSTEVAL size_type max_size() { return N; }
		static AA_CONSTEVAL size_type max_index() { return N - 1; }

		AA_CONSTEXPR difference_type sindexl() const { return elements.sindexl(); }
		AA_CONSTEXPR size_type indexl() const { return elements.indexl(); }



		// Modifiers
		AA_CONSTEXPR void clear() { elements.clear(); first_hole = nullptr; }

		template<class... A>
			requires (std::constructible_from<value_type, A...>)
		AA_CONSTEXPR pointer emplace(A&&... args) {
			if (first_hole) {
				node_type &hole = *first_hole;
				first_hole = hole.next;
				return std::ranges::construct_at(&hole.element, std::forward<A>(args)...);
			} else {
				return std::ranges::construct_at(&elements.push_back()->element, std::forward<A>(args)...);
			}
		}

		template<assignable_to<reference> V>
		AA_CONSTEXPR void insert(V &&value) {
			if (first_hole) {
				node_type &hole = *first_hole;
				first_hole = hole.next;
				hole.element = std::forward<V>(value);
			} else {
				elements.push_back()->element = std::forward<V>(value);
			}
		}

		AA_CONSTEXPR void erase(const pointer pos) {
			node_type *const hole = std::bit_cast<node_type *>(pos);
			hole->next = first_hole;
			first_hole = hole;
		}



		// Special member functions
		AA_CONSTEXPR fixed_fast_free_vector() {}



		// Member objects
	protected:
		fixed_vector<node_type, N> elements;
		node_type *first_hole = nullptr;
	};

}
