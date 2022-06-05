#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/sfml.hpp"
#include "../algorithm/sfml_math.hpp"
#include "static_free_vector.hpp"
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstddef> // ptrdiff_t, size_t
#include <numeric> // accumulate
#include <functional> // invoke
#include <utility> // forward, as_const



namespace aa {

	// https://en.wikipedia.org/wiki/Quadtree
	template<class T, storable_locator<T> L, size_t D, size_t N>
	struct static_quad_tree {
		// Member types
		using value_type = T;
		using locator = L;
		using size_type = size_t;
		using difference_type = ptrdiff_t;
		using reference = value_type &;
		using const_reference = const value_type &;
		using pointer = value_type *;
		using const_pointer = const value_type *;
		using container_type = static_quad_tree<T, L, D, N>;

		struct node_type {
			value_type *element;
			node_type *next;
		};

		template<size_t M>
		struct quad_branch {
			inline static constexpr const size_t decremented_depth = M - 1;

			inline void insert(const pointer e, const sf::Vector2f &l, const sf::FloatRect &r, container_type &t) {
				const sf::Vector2f &s = t.sizes[decremented_depth];

				/**/ if (const sf::FloatRect r_nw = {r.left /***/, r.top /***/, s.x, s.y}; contains(r_nw, l)) nw.insert(e, l, r_nw, t);
				else if (const sf::FloatRect r_ne = {r.left + s.x, r.top /***/, s.x, s.y}; contains(r_ne, l)) ne.insert(e, l, r_ne, t);
				else if (const sf::FloatRect r_sw = {r.left /***/, r.top + s.y, s.x, s.y}; contains(r_sw, l)) sw.insert(e, l, r_sw, t);
				else if (const sf::FloatRect r_se = {r.left + s.x, r.top + s.y, s.x, s.y}; contains(r_se, l)) se.insert(e, l, r_se, t);
			}

			inline void erase(const const_pointer e, const sf::Vector2f &l, const sf::FloatRect &r, container_type &t) {
				const sf::Vector2f &s = t.sizes[decremented_depth];

				/**/ if (const sf::FloatRect r_nw = {r.left /***/, r.top /***/, s.x, s.y}; contains(r_nw, l)) nw.erase(e, l, r_nw, t);
				else if (const sf::FloatRect r_ne = {r.left + s.x, r.top /***/, s.x, s.y}; contains(r_ne, l)) ne.erase(e, l, r_ne, t);
				else if (const sf::FloatRect r_sw = {r.left /***/, r.top + s.y, s.x, s.y}; contains(r_sw, l)) sw.erase(e, l, r_sw, t);
				else if (const sf::FloatRect r_se = {r.left + s.x, r.top + s.y, s.x, s.y}; contains(r_se, l)) se.erase(e, l, r_se, t);
			}

			template<invocable_ref<reference> F>
			inline void query_range(const sf::FloatRect &q, F &f, const sf::FloatRect &r, const container_type &t) const {
				const sf::Vector2f &s = t.sizes[decremented_depth];

				if (const sf::FloatRect r_nw = {r.left /***/, r.top /***/, s.x, s.y}; contains(q, r_nw))
					nw.query(f, t); else if (intersects(q, r_nw)) nw.query_range(q, f, r_nw, t);

				if (const sf::FloatRect r_ne = {r.left + s.x, r.top /***/, s.x, s.y}; contains(q, r_ne))
					ne.query(f, t); else if (intersects(q, r_ne)) ne.query_range(q, f, r_ne, t);

				if (const sf::FloatRect r_sw = {r.left /***/, r.top + s.y, s.x, s.y}; contains(q, r_sw))
					sw.query(f, t); else if (intersects(q, r_sw)) sw.query_range(q, f, r_sw, t);

				if (const sf::FloatRect r_se = {r.left + s.x, r.top + s.y, s.x, s.y}; contains(q, r_se))
					se.query(f, t); else if (intersects(q, r_se)) se.query_range(q, f, r_se, t);
			}

			template<invocable_ref<reference> F>
			inline void query(F &f, const container_type &t) const {
				nw.query(f, t);
				ne.query(f, t);
				sw.query(f, t);
				se.query(f, t);
			}

			quad_branch<decremented_depth> nw, ne, sw, se;
		};

		// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=85282#c17, GCC bug, neleidžia kompiliatorius full specialization
		// declare'inti in class scope tai apieiname problemą declare'indami partial specialization.
		//
		// Reikia mintyje turėti tokį įdomų scenarijų, kad tame pačiame pass, mes įdedame elementą į lapą, po
		// to išemame elementą ir tada kažką darome, pass bus tas pats, bet first tuo atveju bus nullptr.
		template<size_t M>
			requires (!M)
		struct quad_branch<M> {
			inline void insert(const pointer e, const sf::Vector2f &, const sf::FloatRect &, container_type &t) {
				if (pass != t.pass) {
					pass = t.pass;
					first = &t.nodes.emplace(e, nullptr);
				} else {
					first = &t.nodes.emplace(e, first);
				}
			}

			inline void erase(const const_pointer e, const sf::Vector2f &, const sf::FloatRect &, container_type &t) {
				if (pass == t.pass && first) {
					if (first->element == e) {
						// Reikia išsaugoti sekančio elemento adresą, nes ištrynus
						// šį elementą, bus ištrintas ir sekančio elemento adresas.
						node_type *const next = first->next;
						t.nodes.erase(first);
						first = next;
					} else {
						node_type *iter = first;
						while (iter->next) {
							if (iter->next->element == e) {
								node_type *const next = iter->next->next;
								t.nodes.erase(iter->next);
								iter->next = next;
								return;
							}
							iter = iter->next;
						}
					}
				}
			}

			template<invocable_ref<reference> F>
			inline void query_range(const sf::FloatRect &q, F &f, const sf::FloatRect &, const container_type &t) const {
				if (pass == t.pass && first) {
					const node_type *iter = first;
					do {
						if (contains(q, t.locate(*iter->element)))
							std::invoke(f, *iter->element);
					} while ((iter = iter->next));
				}
			}

			template<invocable_ref<reference> F>
			inline void query(F &f, const container_type &t) const {
				if (pass == t.pass && first) {
					const node_type *iter = first;
					do {
						std::invoke(f, *iter->element);
					} while ((iter = iter->next));
				}
			}

			size_t pass = 0;
			node_type *first = nullptr;
		};



		// Capacity
		inline constexpr bool empty() const { return nodes.empty(); }
		inline constexpr bool full() const { return nodes.full(); }

		inline constexpr difference_type ssize() const { return nodes.ssize(); }
		inline constexpr size_type size() const { return nodes.size(); }

		static inline consteval size_type max_size() { return N; }



		// Observers
		inline constexpr const locator &locator_function() const { return locator_func; }

		inline constexpr const sf::Vector2f &locate(const value_type &e) const { return std::invoke(locator_func, e); }



		// Lookup
		template<invocable_ref<reference> F>
		inline void query_range(const sf::FloatRect &range, F &&f) const {
			trunk.query_range(range, f, rect, *this);
		}

		template<invocable_ref<reference> F>
		inline void query(F &&f) const {
			trunk.query(f, *this);
		}



		// Modifiers
		inline constexpr void clear() {
			nodes.clear();
			++pass;
		}

		inline void insert(value_type &element) {
			trunk.insert(&element, locate(std::as_const(element)), rect, *this);
		}

		inline void erase(const value_type &element) {
			trunk.erase(&element, locate(element), rect, *this);
		}



		// Special member functions
		template<class U>
		inline constexpr static_quad_tree(const sf::Vector2f &position, const sf::Vector2f &size, U &&u = {}) : sizes{[&]() {
			array_t<sf::Vector2f, D> sizes;
			if constexpr (D) {
				std::accumulate(sizes.rbegin(), sizes.rend(), size,
					[](const sf::Vector2f &size, sf::Vector2f &s) { return (s = size * 0.5f); });
			}
			return sizes;
		}()}, rect{position, size}, locator_func{std::forward<U>(u)} {}



		// Member objects
		const array_t<sf::Vector2f, D> sizes;
		const sf::FloatRect rect;

	protected:
		size_t pass = 0;
		quad_branch<D> trunk;
		static_free_vector<node_type, N> nodes;
		[[no_unique_address]] const locator locator_func;
	};

}
