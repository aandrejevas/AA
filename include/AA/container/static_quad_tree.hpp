#pragma once

#include "../metaprogramming/general.hpp"
#include "../algorithm/arithmetic.hpp"
#include "../algorithm/find.hpp"
#include "static_fast_free_vector.hpp"
#include "static_array.hpp"
#include <cstddef> // ptrdiff_t, size_t
#include <functional> // invoke
#include <utility> // forward
#include <queue> // queue



namespace aa {

	// https://en.wikipedia.org/wiki/Quadtree
	template<class T, storable_vector2_getter<T> L, size_t H, size_t N>
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
		using position_type = vector2_getter_result_t<value_type, locator>;
		using container_type = static_quad_tree<T, L, H, N>;

	protected:
		static AA_CONSTEXPR const size_t leaves_count = int_exp2N<2uz>(H), phantoms_count = (leaves_count - 1) / 3;

		struct query_type {
			size_t i;
			position_type q;
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
			AA_CONSTEXPR void query_range(const position_type &tl, const position_type &br, F &f, const container_type &t) const {
				if (pass == t.pass && first) {
					const auto &[tl_x, tl_y] = tl;
					const auto &[br_x, br_y] = br;
					const node_type *iter = first;
					do {
						const auto &[lx, ly] = t.locate(*iter->element);
						if ((tl_x <= lx) && (lx < br_x) && (tl_y <= ly) && (ly < br_y))
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

		AA_CONSTEXPR const position_type &locate(const value_type &e) const { return std::invoke(locator_func, e); }



		// Lookup
	protected:
		AA_CONSTEXPR size_t find(const value_type &element) const {
			const auto &[l_x, l_y] = locate(element);
			size_t i = 0;

			if constexpr (H) {
				const position_type *size = sizes.data();
				position_type q = position;
				auto &[q_x, q_y] = q;
				do {
					const auto &[s_x, s_y] = *size;
					const position_type m = position_type{q_x + s_x, q_y + s_y};
					const auto &[m_x, m_y] = m;

					if (l_y < m_y) {
						if (l_x < m_x) {
							i = (i << 2) + 1;
							// q = q;
						} else {
							i = (i << 2) + 2;
							q_x = m_x;
						}
					} else {
						if (l_x < m_x) {
							i = (i << 2) + 3;
							q_y = m_y;
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
		AA_CONSTEXPR void query_range(const position_type &tl, const position_type &br, F &&f) {
			const auto &[tl_x, tl_y] = tl;
			const auto &[br_x, br_y] = br;
			queries.emplace(0, position);

			if constexpr (H) {
				const position_type *size = sizes.data();
				do {
					const auto &[s_x, s_y] = *size;
					size_t count = queries.size();
					do {
						const query_type &q = queries.front();
						const auto &[q_x, q_y] = q.q;
						const position_type m = position_type{q_x + s_x, q_y + s_y};
						const auto &[m_x, m_y] = m;

						if (tl_y < m_y) {
							if (tl_x < m_x)
								queries.emplace((q.i << 2) + 1, q.q);
							if (br_x > m_x)
								queries.emplace((q.i << 2) + 2, position_type{m_x, q_y});
						}
						if (br_y > m_y) {
							if (tl_x < m_x)
								queries.emplace((q.i << 2) + 3, position_type{q_x, m_y});
							if (br_x > m_x)
								queries.emplace((q.i << 2) + 4, m);
						}
						queries.pop();
					} while (--count);

					if (size != sizes.rdata()) ++size; else break;
				} while (true);
			}

			do {
				leaves[queries.front().i - phantoms_count].query_range(tl, br, f, *this);
				queries.pop();
			} while (!queries.empty());
		}

		template<invocable_ref<reference> F>
		AA_CONSTEXPR void query(F &&f) const {
			unsafe_for_each(leaves, [&](const leaf &l) -> void {
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
		template<class U = locator>
		AA_CONSTEXPR static_quad_tree(const position_type &pos, const position_type &size, U &&u = {}) : sizes{[&]() {
			static_array<position_type, H> sizes;
			if constexpr (H) {
				{
					const auto &[s_x, s_y] = size;
					sizes.front() = position_type{s_x / two_v<array_element_t<position_type>>, s_y / two_v<array_element_t<position_type>>};
				}
				if constexpr (H != 1) {
					position_type *s = sizes.data() + 1;
					do {
						const auto &[s_x, s_y] = s[-1];
						*s = position_type{s_x / two_v<array_element_t<position_type>>, s_y / two_v<array_element_t<position_type>>};
						if (s != sizes.rdata()) ++s; else break;
					} while (true);
				}
			}
			return sizes;
		}()}, position{pos}, locator_func{std::forward<U>(u)} {}



		// Member objects
		const static_array<position_type, H> sizes;
		const position_type position;

	protected:
		size_t pass = 0;
		std::queue<query_type> queries;
		array_t<leaf, leaves_count> leaves;
		static_fast_free_vector<node_type, N> nodes;
		[[no_unique_address]] const locator locator_func;
	};

}
