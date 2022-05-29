#pragma once

#include "../metaprogramming/general.hpp"
#include "static_free_vector.hpp"
#include <cstddef> // size_t
#include <numeric> // accumulate
#include <functional> // invoke
#include <utility> // forward
#include <iterator> // reverse_iterator
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>



namespace aa {

	// https://en.wikipedia.org/wiki/Quadtree
	template<class L, size_t D, size_t N>
	struct static_quad_tree {
		// Member types
		using locator = L;
		using container_type = static_quad_tree<L, D, N>;

		struct value_type {
			void *element;
			value_type **leaf, *next;
		};

		template<size_t M>
		struct quad_branch {
			inline static constexpr const size_t decremented_depth = M - 1;

			inline void insert(void *const e, const sf::Vector2f &l, const sf::FloatRect &r, container_type &t) {
				const sf::Vector2f &s = t.sizes[decremented_depth];

				/**/ if (const sf::FloatRect r_nw = {r.left /***/, r.top /***/, s.x, s.y}; r_nw.contains(l)) nw.insert(e, l, r_nw, t);
				else if (const sf::FloatRect r_ne = {r.left + s.x, r.top /***/, s.x, s.y}; r_ne.contains(l)) ne.insert(e, l, r_ne, t);
				else if (const sf::FloatRect r_sw = {r.left /***/, r.top + s.y, s.x, s.y}; r_sw.contains(l)) sw.insert(e, l, r_sw, t);
				else if (const sf::FloatRect r_se = {r.left + s.x, r.top + s.y, s.x, s.y}; r_se.contains(l)) se.insert(e, l, r_se, t);
			}

			template<class F>
			inline void query_range(const sf::FloatRect &q, F &f, const sf::FloatRect &r, const container_type &t) const {
				const sf::Vector2f &s = t.sizes[decremented_depth];

				if (const sf::FloatRect r_nw = {r.left /***/, r.top /***/, s.x, s.y}; r_nw.intersects(q)) nw.query_range(q, f, r_nw, t);
				if (const sf::FloatRect r_ne = {r.left + s.x, r.top /***/, s.x, s.y}; r_ne.intersects(q)) ne.query_range(q, f, r_ne, t);
				if (const sf::FloatRect r_sw = {r.left /***/, r.top + s.y, s.x, s.y}; r_sw.intersects(q)) sw.query_range(q, f, r_sw, t);
				if (const sf::FloatRect r_se = {r.left + s.x, r.top + s.y, s.x, s.y}; r_se.intersects(q)) se.query_range(q, f, r_se, t);
			}

			quad_branch<decremented_depth> nw, ne, sw, se;
		};

		// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=85282#c17, GCC bug, neleidžia kompiliatorius full specialization
		// declare'inti in class scope tai apieiname problemą declare'indami partial specialization.
		template<size_t M>
			requires (!M)
		struct quad_branch<M> {
			inline void insert(void *const e, const sf::Vector2f &, const sf::FloatRect &, container_type &t) {
				t.elements.emplace(e, &first, first);
				first = t.elements.back();
			}

			template<class F>
			inline void query_range(const sf::FloatRect &q, F &f, const sf::FloatRect &, const container_type &t) const {
				if (first) {
					const value_type *iter = first;
					do {
						if (q.contains(t.locate(iter->element)))
							std::invoke(f, iter->element);
					} while ((iter = iter->next));
				}
			}

			value_type *first = nullptr;
		};



		// Capacity
		inline constexpr bool empty() const { return elements.empty(); }
		inline constexpr bool full() const { return elements.full(); }

		inline constexpr difference_type ssize() const { return elements.ssize(); }
		inline constexpr size_type size() const { return elements.size(); }

		static inline consteval size_type max_size() { return N; }



		// Observers
		inline constexpr const locator &locator_function() const { return locator_func; }

		inline constexpr const sf::Vector2f &locate(const void *const e) const { return std::invoke(locator_func, e); }



		// Lookup
		template<class F>
		inline void query_range(const sf::FloatRect &range, F &&f) const {
			trunk.query_range(range, f, rect, *this);
		}



		// Modifiers
		inline void insert(void *const element) {
			trunk.insert(element, locate(element), rect, *this);
		}



		// Special member functions
		template<class U>
		inline constexpr static_quad_tree(const sf::Vector2f &position, const sf::Vector2f &size, U &&u = {}) : sizes{[&]() {
			array_t<sf::Vector2f, D> sizes;
			if constexpr (D) {
				std::accumulate(std::reverse_iterator{&(sizes.back() = size)}, sizes.rend(), size,
					[](const sf::Vector2f &size, sf::Vector2f &s) { return (s = size * 0.5f); });
			}
			return sizes;
		}()}, rect{position, size}, locator_func{std::forward<U>(u)} {}



		// Member objects
		const array_t<sf::Vector2f, D> sizes;
		const sf::FloatRect rect;

	protected:
		quad_branch<D> trunk;
		static_free_vector<value_type, N> elements;
		[[no_unique_address]] const locator locator_func;
	};

}
