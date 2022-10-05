#pragma once

#include "../metaprogramming/general.hpp"
#include "../algorithm/arithmetic.hpp"
#include "../algorithm/find.hpp"
#include "fixed_fast_free_vector.hpp"
#include "fixed_array.hpp"
#include "queue.hpp"
#include <cstddef> // ptrdiff_t, size_t
#include <functional> // invoke, identity
#include <utility> // forward, exchange
#include <queue> // queue



namespace aa {

	// https://en.wikipedia.org/wiki/Quadtree
	template<class T, size_t H, size_t N, storable_vector2_getter<T> L = std::identity>
	struct fixed_quad_tree {
		// Member types
		using value_type = T;
		using locator_type = L;
		using size_type = size_t;
		using difference_type = ptrdiff_t;
		using reference = value_type &;
		using const_reference = const value_type &;
		using pointer = value_type *;
		using const_pointer = const value_type *;
		using position_type = vector2_getter_result_t<T, L>;
		using pair_type = array_t<array_element_t<position_type>, 2>;

	protected:
		static AA_CONSTEXPR const size_type leaves_count = int_exp2<2, size_type>(H), phantoms_count = (leaves_count - 1) / 3;

		struct query_type {
			size_type i;
			pair_type q;
		};

		struct node_type {
			value_type *element;
			node_type *next;
		};

		// Reikia mintyje turėti tokį įdomų scenarijų, kad tame pačiame pass, mes įdedame elementą į lapą, po
		// to išemame elementą ir tada kažką darome, pass bus tas pats, bet first tuo atveju bus nullptr.
		struct leaf {
			AA_CONSTEXPR void insert(const pointer e, fixed_quad_tree &t) {
				if (pass != t.pass) {
					pass = t.pass;
					first = t.nodes.emplace(e, nullptr);
				} else {
					first = t.nodes.emplace(e, first);
				}
			}

			AA_CONSTEXPR void erase(const const_pointer e, fixed_quad_tree &t) {
				if (pass == t.pass && first) {
					if (first->element == e) {
						t.nodes.erase(std::exchange(first, first->next));
					} else {
						node_type *iter = first;
						while (iter->next) {
							if (iter->next->element == e) {
								t.nodes.erase(std::exchange(iter->next, iter->next->next));
								return;
							}
							iter = iter->next;
						}
					}
				}
			}

			template<invocable_ref<reference> F>
			AA_CONSTEXPR void query_range(const pair_type &tl, const pair_type &br, F &f, const fixed_quad_tree &t) const {
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
			AA_CONSTEXPR void query(F &f, const fixed_quad_tree &t) const {
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

		[[gnu::always_inline]] AA_CONSTEXPR const position_type &locate(const value_type &e) const {
			return std::invoke(locator, e);
		}



		// Lookup
	protected:
		AA_CONSTEXPR size_type find_leaf(const value_type &element) const {
			if constexpr (H) {
				size_type i = 0;
				pair_type q = position;
				const position_type &l = locate(element);
				const pair_type *size = sizes.data();
				do {
					const pair_type &s = *size;
					const pair_type m = pair_type{get_x(q) + get_x(s), get_y(q) + get_y(s)};

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

		AA_CONSTEXPR void find_leaves(const pair_type &tl, const pair_type &br) const {
			queries.emplace(0, position);

			if constexpr (H) {
				const pair_type *size = sizes.data();
				do {
					const pair_type &s = *size;
					size_type count = queries.size();
					do {
						const query_type &q = queries.front();
						const pair_type m = pair_type{get_x(q.q) + get_x(s), get_y(q.q) + get_y(s)};

						if (get_y(tl) < get_y(m)) {
							if (get_x(tl) < get_x(m))
								queries.emplace((q.i << 2) + 1, q.q);
							if (get_x(br) > get_x(m))
								queries.emplace((q.i << 2) + 2, pair_type{get_x(m), get_y(q.q)});
						}
						if (get_y(br) > get_y(m)) {
							if (get_x(tl) < get_x(m))
								queries.emplace((q.i << 2) + 3, pair_type{get_x(q.q), get_y(m)});
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
		AA_CONSTEXPR void query_range(const pair_type &tl, const pair_type &br, F &&f) const {
			find_leaves(tl, br);
			do {
				leaves[queries.front().i - phantoms_count].query_range(tl, br, f, *this);
				queries.pop();
			} while (!queries.empty());
		}

		template<invocable_ref<reference> F>
		AA_CONSTEXPR void query_loose_range(const pair_type &tl, const pair_type &br, F &&f) const {
			find_leaves(tl, br);
			do {
				leaves[queries.front().i - phantoms_count].query(f, *this);
				queries.pop();
			} while (!queries.empty());
		}

		template<invocable_ref<reference> F>
		AA_CONSTEXPR void query_all(F &&f) const {
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
		template<class U = locator_type>
		AA_CONSTEXPR fixed_quad_tree(const pair_type &pos, const pair_type &size, U &&u = {})
			: sizes{[&](fixed_array<pair_type, H> &init_sizes) -> void
		{
			if constexpr (H) {
				init_sizes.front() = pair_type{halve(get_x(size)), halve(get_y(size))};
				if constexpr (H != 1) {
					pair_type *iter_s = init_sizes.data() + 1;
					do {
						const pair_type &prev_s = iter_s[-1];
						*iter_s = pair_type{halve(get_x(prev_s)), halve(get_y(prev_s))};
						if (iter_s != init_sizes.rdata()) ++iter_s; else break;
					} while (true);
				}
			}
		}}, position{pos}, locator{std::forward<U>(u)} {}



		// Member objects
		const fixed_array<pair_type, H> sizes;
		const pair_type position;

	protected:
		size_type pass = 0;
		mutable std::queue<query_type, aa::queue<query_type>> queries;
		array_t<leaf, leaves_count> leaves;
		fixed_fast_free_vector<node_type, N> nodes;

	public:
		[[no_unique_address]] const locator_type locator;
	};

}
