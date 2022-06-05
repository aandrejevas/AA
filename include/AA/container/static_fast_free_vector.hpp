#pragma once

#include "static_vector.hpp"
#include <cstddef> // size_t, ptrdiff_t
#include <utility> // forward
#include <concepts> // constructible_from
#include <memory> // construct_at



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
		using container_type = static_fast_free_vector<T, N>;

		struct node_type {
			union {
				node_type *next;
				value_type element;
			};
		};



		// Capacity
		inline constexpr bool has_holes() const { return first_hole; }

		inline constexpr bool empty() const { return elements.empty(); }
		inline constexpr bool full() const { return elements.full(); }

		inline constexpr difference_type ssize() const { return elements.ssize(); }
		inline constexpr size_type size() const { return elements.size(); }

		static inline consteval size_type max_size() { return N; }



		// Modifiers
		inline constexpr void clear() { elements.clear(); first_hole = nullptr; }

		template<class... A>
			requires (std::constructible_from<value_type, A...>)
		inline constexpr pointer emplace(A&&... args) {
			if (first_hole) {
				node_type *const hole = first_hole;
				first_hole = hole->next;
				return std::ranges::construct_at(&hole->element, std::forward<A>(args)...);
			} else {
				return std::ranges::construct_at(&elements.push_back()->element, std::forward<A>(args)...);
			}
		}

		inline void erase(const pointer pos) {
			node_type *const hole = reinterpret_cast<node_type *>(pos);
			hole->next = first_hole;
			first_hole = hole;
		}



		// Special member functions
		inline constexpr static_fast_free_vector() {}



		// Member objects
	protected:
		static_vector<node_type, N> elements;
		node_type *first_hole = nullptr;
	};

}
