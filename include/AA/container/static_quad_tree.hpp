#pragma once

#include "../metaprogramming/general.hpp"
#include "../algorithm/arithmetic.hpp"
#include "../algorithm/find.hpp"
#include "static_fast_free_vector.hpp"
#include "static_array.hpp"
#include "queue.hpp"
#include <cstddef> // ptrdiff_t, size_t
#include <functional> // invoke
#include <utility> // forward
#include <queue> // queue



namespace aa {

	template<class T, storable_vector2_getter<T> L>
	struct query_result {
		using size_type = size_t;
		using position_type = vector2_getter_result_t<T, L>;

		size_type i;
		position_type q;
	};

	// https://en.wikipedia.org/wiki/Quadtree
	template<class T, storable_vector2_getter<T> L, size_t H, size_t N, class C = queue<query_result<T, L>>>
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
		using query_result = query_result<value_type, locator>;
		using position_type = query_result::position_type;
		using container_type = static_quad_tree<T, L, H, N>;

	protected:
		static AA_CONSTEXPR const size_type leaves_count = int_exp2N<2uz>(H), phantoms_count = (leaves_count - 1) / 3;

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
					const node_type *iter = first;
					do {
						const position_type &l = t.locate(*iter->element);
						if ((get_x(tl) <= get_x(l)) && (get_x(l) < get_x(br))
						 && (get_y(tl) <= get_y(l)) && (get_y(l) < get_y(br)))
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

			size_type pass = 0;
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
		AA_CONSTEXPR size_type get_pass() const { return pass; }

		AA_CONSTEXPR const locator &locator_function() const { return locator_func; }

		AA_CONSTEXPR const position_type &locate(const value_type &e) const { return std::invoke(locator_func, e); }



		// Lookup
	protected:
		AA_CONSTEXPR size_type find_leaf(const value_type &element) const {
			if constexpr (H) {
				size_type i = 0;
				position_type q = position;
				const position_type &l = locate(element);
				const position_type *size = sizes.data();
				do {
					const position_type &s = *size;
					const position_type m = position_type{get_x(q) + get_x(s), get_y(q) + get_y(s)};

					if (get_y(l) < get_y(m)) {
						if (get_x(l) < get_x(m)) {
							i = (i << 2) + 1;
							// q = q;
						} else {
							i = (i << 2) + 2;
							get_x(q) = get_x(m);
						}
					} else {
						if (get_x(l) < get_x(m)) {
							i = (i << 2) + 3;
							get_y(q) = get_y(m);
						} else {
							i = (i << 2) + 4;
							q = m;
						}
					}
					if (size != sizes.rdata()) ++size; else break;
				} while (true);

				return i - phantoms_count;
			} else
				return 0;
		}

		AA_CONSTEXPR void find_leaves(const position_type &tl, const position_type &br) const {
			queries.emplace(0, position);

			if constexpr (H) {
				const position_type *size = sizes.data();
				do {
					const position_type &s = *size;
					size_type count = queries.size();
					do {
						const query_result &q = queries.front();
						const position_type m = position_type{get_x(q.q) + get_x(s), get_y(q.q) + get_y(s)};

						if (get_y(tl) < get_y(m)) {
							if (get_x(tl) < get_x(m))
								queries.emplace((q.i << 2) + 1, q.q);
							if (get_x(br) > get_x(m))
								queries.emplace((q.i << 2) + 2, position_type{get_x(m), get_y(q.q)});
						}
						if (get_y(br) > get_y(m)) {
							if (get_x(tl) < get_x(m))
								queries.emplace((q.i << 2) + 3, position_type{get_x(q.q), get_y(m)});
							if (get_x(br) > get_x(m))
								queries.emplace((q.i << 2) + 4, m);
						}
						queries.pop();
					} while (--count);

					if (size != sizes.rdata()) ++size; else break;
				} while (true);
			}
		}

	public:
		template<invocable_ref<reference> F>
		AA_CONSTEXPR void query_range(const position_type &tl, const position_type &br, F &&f) const {
			find_leaves(tl, br);
			do {
				leaves[queries.front().i - phantoms_count].query_range(tl, br, f, *this);
				queries.pop();
			} while (!queries.empty());
		}

		template<invocable_ref<reference> F>
		AA_CONSTEXPR void query_loose_range(const position_type &tl, const position_type &br, F &&f) const {
			find_leaves(tl, br);
			do {
				leaves[queries.front().i - phantoms_count].query(f, *this);
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
			leaves[find_leaf(element)].insert(&element, *this);
		}

		AA_CONSTEXPR void erase(const value_type &element) {
			leaves[find_leaf(element)].erase(&element, *this);
		}



		// Special member functions
		template<class U = locator>
		AA_CONSTEXPR static_quad_tree(const position_type &pos, const position_type &size, U &&u = {}) : sizes{[&]() {
			static_array<position_type, H> init_sizes;
			if constexpr (H) {
				init_sizes.front() = position_type{
					get_x(size) / two_v<array_element_t<position_type>>,
					get_y(size) / two_v<array_element_t<position_type>>
				};
				if constexpr (H != 1) {
					position_type *iter_s = init_sizes.data() + 1;
					do {
						const position_type &s = iter_s[-1];
						*iter_s = position_type{
							get_x(s) / two_v<array_element_t<position_type>>,
							get_y(s) / two_v<array_element_t<position_type>>
						};
						if (iter_s != init_sizes.rdata()) ++iter_s; else break;
					} while (true);
				}
			}
			return init_sizes;
		}()}, position{pos}, locator_func{std::forward<U>(u)} {}



		// Member objects
		const static_array<position_type, H> sizes;
		const position_type position;

	protected:
		size_type pass = 0;
		mutable std::queue<query_result, C> queries;
		array_t<leaf, leaves_count> leaves;
		static_fast_free_vector<node_type, N> nodes;
		[[no_unique_address]] const locator locator_func;
	};

}
