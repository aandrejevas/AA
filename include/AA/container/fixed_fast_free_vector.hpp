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
		constexpr bool has_holes() const { return first_hole; }

		constexpr size_type holes_count() const {
			if (first_hole) {
				size_type count = 0;
				const node_type *iter = first_hole;
				do {
					++count;
				} while ((iter = iter->next));
				return count;
			} else return 0;
		}

		constexpr bool full() const { return elements.full(); }
		constexpr bool empty() const { return elements.empty(); }
		constexpr bool single() const { return elements.single(); }

		constexpr size_type size_wo_holes() const { return size() - holes_count(); }
		constexpr size_type size() const { return elements.size(); }
		constexpr size_type last_index() const { return elements.last_index(); }

		static consteval size_type max_size() { return N; }
		static consteval size_type max_index() { return N - 1; }



		// Modifiers
		constexpr void clear() { elements.clear(); first_hole = nullptr; }

		template<class... A>
			requires (std::constructible_from<value_type, A...>)
		constexpr pointer emplace(A&&... args) {
			if (first_hole) {
				node_type &hole = *first_hole;
				first_hole = hole.next;
				return std::ranges::construct_at(&hole.element, std::forward<A>(args)...);
			} else {
				return std::ranges::construct_at(&elements.push_back()->element, std::forward<A>(args)...);
			}
		}

		template<assignable_to<reference> V>
		constexpr void insert(V &&value) {
			if (first_hole) {
				node_type &hole = *first_hole;
				first_hole = hole.next;
				hole.element = std::forward<V>(value);
			} else {
				elements.push_back()->element = std::forward<V>(value);
			}
		}

		constexpr void erase(const pointer pos) {
			node_type *const hole = std::bit_cast<node_type *>(pos);
			hole->next = first_hole;
			first_hole = hole;
		}



		// Special member functions
		constexpr fixed_fast_free_vector() {}



		// Member objects
	protected:
		fixed_vector<node_type, N> elements;
		node_type *first_hole = nullptr;
	};

}
