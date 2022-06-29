#pragma once

#include "../metaprogramming/general.hpp"
#include "static_vector.hpp"
#include <cstddef> // size_t, ptrdiff_t
#include <utility> // forward
#include <concepts> // constructible_from
#include <memory> // construct_at
#include <bit> // bit_cast



namespace aa {

	template<trivially_copyable T, size_t N>
	struct static_fast_free_vector {
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
	public:



		// Capacity
		AA_CONSTEXPR bool has_holes() const { return first_hole; }

		AA_CONSTEXPR bool empty() const { return elements.empty(); }
		AA_CONSTEXPR bool full() const { return elements.full(); }

		AA_CONSTEXPR difference_type ssize() const { return elements.ssize(); }
		AA_CONSTEXPR size_type size() const { return elements.size(); }

		static AA_CONSTEVAL size_type max_size() { return N; }



		// Modifiers
		AA_CONSTEXPR void clear() { elements.clear(); first_hole = nullptr; }

		template<class... A>
			requires (std::constructible_from<value_type, A...>)
		AA_CONSTEXPR pointer emplace(A&&... args) {
			if (first_hole) {
				node_type *const hole = first_hole;
				first_hole = hole->next;
				return std::ranges::construct_at(&hole->element, std::forward<A>(args)...);
			} else {
				return std::ranges::construct_at(&elements.push_back()->element, std::forward<A>(args)...);
			}
		}

		AA_CONSTEXPR void insert(const value_type &value) {
			if (first_hole) {
				node_type *const hole = first_hole;
				first_hole = hole->next;
				hole->element = value;
			} else {
				elements.push_back()->element = value;
			}
		}

		AA_CONSTEXPR void erase(const pointer pos) {
			node_type *const hole = std::bit_cast<node_type *>(pos);
			hole->next = first_hole;
			first_hole = hole;
		}



		// Special member functions
		AA_CONSTEXPR static_fast_free_vector() {}



		// Member objects
	protected:
		static_vector<node_type, N> elements;
		node_type *first_hole = nullptr;
	};

}
