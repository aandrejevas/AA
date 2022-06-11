#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/sfml.hpp"
#include "../algorithm/arithmetic.hpp"
#include "../algorithm/find.hpp"
#include "static_fast_free_vector.hpp"
#include "static_array.hpp"
#include <SFML/System/Vector2.hpp>
#include <cstddef> // ptrdiff_t, size_t
#include <functional> // invoke
#include <utility> // forward
#include <queue> // queue



namespace aa {

	// https://en.wikipedia.org/wiki/Quadtree
	template<class T, storable_vec2_getter<T> L, size_t H, size_t N>
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
		using container_type = static_quad_tree<T, L, H, N>;

	protected:
		static AA_CONSTEXPR const size_t leaves_count = int_exp2N<2uz>(H), phantoms_count = (leaves_count - 1) / 3;

		struct query_type {
			size_t i;
			float x, y;
		};

		struct node_type {
			value_type *element;
			node_type *next;
		};

		// Reikia mintyje turėti tokį įdomų scenarijų, kad tame pačiame pass, mes įdedame elementą į lapą, po
		// to išemame elementą ir tada kažką darome, pass bus tas pats, bet first tuo atveju bus nullptr.
		struct leaf {
			AA_CONSTEXPR void insert(const pointer e, container_type &t) {
				if (pass != t.pass) {
					pass = t.pass;
					first = t.nodes.emplace(e, nullptr);
				} else {
					first = t.nodes.emplace(e, first);
				}
			}

			AA_CONSTEXPR void erase(const const_pointer e, container_type &t) {
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
			AA_CONSTEXPR void query_range(const sf::Vector2f &tl, const sf::Vector2f &br, F &f, const container_type &t) const {
				if (pass == t.pass && first) {
					const node_type *iter = first;
					do {
						const sf::Vector2f &l = t.locate(*iter->element);
						if ((tl.x <= l.x) && (l.x < br.x) && (tl.y <= l.y) && (l.y < br.y))
							std::invoke(f, *iter->element);
					} while ((iter = iter->next));
				}
			}

			template<invocable_ref<reference> F>
			AA_CONSTEXPR void query(F &f, const container_type &t) const {
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
	public:



		// Capacity
		AA_CONSTEXPR bool empty() const { return nodes.empty(); }
		AA_CONSTEXPR bool full() const { return nodes.full(); }

		AA_CONSTEXPR difference_type ssize() const { return nodes.ssize(); }
		AA_CONSTEXPR size_type size() const { return nodes.size(); }

		static AA_CONSTEVAL size_type max_size() { return N; }



		// Observers
		AA_CONSTEXPR size_t get_pass() const { return pass; }

		AA_CONSTEXPR const locator &locator_function() const { return locator_func; }

		AA_CONSTEXPR const sf::Vector2f &locate(const value_type &e) const { return std::invoke(locator_func, e); }



		// Lookup
	protected:
		AA_CONSTEXPR size_t find(const value_type &element) const {
			const sf::Vector2f &l = locate(element);
			size_t i = 0;

			if constexpr (H) {
				const sf::Vector2f *size = sizes.data();
				sf::Vector2f q = position;
				do {
					const sf::Vector2f m = q + *size;

					if (l.y < m.y) {
						if (l.x < m.x) {
							i = (i << 2) + 1;
							// q = q;
						} else {
							i = (i << 2) + 2;
							q.x = m.x;
						}
					} else {
						if (l.x < m.x) {
							i = (i << 2) + 3;
							q.y = m.y;
						} else {
							i = (i << 2) + 4;
							q = m;
						}
					}
					if (size != sizes.rdata()) ++size; else break;
				} while (true);
			}

			return i - phantoms_count;
		}

	public:
		template<invocable_ref<reference> F>
		AA_CONSTEXPR void query_range(const sf::Vector2f &tl, const sf::Vector2f &br, F &&f) {
			queue.emplace(0, position.x, position.y);

			if constexpr (H) {
				const sf::Vector2f *size = sizes.data();
				do {
					const sf::Vector2f &s = *size;
					size_t count = queue.size();
					do {
						const query_type &q = queue.front();
						const sf::Vector2f m = {q.x + s.x, q.y + s.y};

						if (tl.y < m.y) {
							if (tl.x < m.x)
								queue.emplace((q.i << 2) + 1, q.x, q.y);
							if (br.x > m.x)
								queue.emplace((q.i << 2) + 2, m.x, q.y);
						}
						if (br.y > m.y) {
							if (tl.x < m.x)
								queue.emplace((q.i << 2) + 3, q.x, m.y);
							if (br.x > m.x)
								queue.emplace((q.i << 2) + 4, m.x, m.y);
						}
						queue.pop();
					} while (--count);

					if (size != sizes.rdata()) ++size; else break;
				} while (true);
			}

			do {
				leaves[queue.front().i - phantoms_count].query_range(tl, br, f, *this);
				queue.pop();
			} while (!queue.empty());
		}

		template<invocable_ref<reference> F>
		AA_CONSTEXPR void query(F &&f) const {
			unsafe_for_each(leaves, [&](const leaf &l) {
				l.query(f, *this);
			});
		}



		// Modifiers
		AA_CONSTEXPR void clear() {
			nodes.clear();
			++pass;
		}

		AA_CONSTEXPR void insert(value_type &element) {
			leaves[find(element)].insert(&element, *this);
		}

		AA_CONSTEXPR void erase(const value_type &element) {
			leaves[find(element)].erase(&element, *this);
		}



		// Special member functions
		template<class U>
		AA_CONSTEXPR static_quad_tree(const sf::Vector2f &pos, const sf::Vector2f &size, U &&u = {}) : sizes{[&]() {
			static_array<sf::Vector2f, H> sizes;
			if constexpr (H) {
				sizes.front() = size * 0.5f;
				if constexpr (H != 1) {
					sf::Vector2f *s = sizes.data() + 1;
					do {
						*s = s[-1] * 0.5f;
						if (s != sizes.rdata()) ++s; else break;
					} while (true);
				}
			}
			return sizes;
		}()}, position{pos}, locator_func{std::forward<U>(u)} {}



		// Member objects
		const static_array<sf::Vector2f, H> sizes;
		const sf::Vector2f position;

	protected:
		size_t pass = 0;
		std::queue<query_type> queue;
		array_t<leaf, leaves_count> leaves;
		static_fast_free_vector<node_type, N> nodes;
		[[no_unique_address]] const locator locator_func;
	};

}
