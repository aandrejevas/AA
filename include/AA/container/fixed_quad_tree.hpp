#pragma once

#include "../metaprogramming/general.hpp"
#include "../algorithm/arithmetic.hpp"
#include "../algorithm/find.hpp"
#include "fixed_fast_free_vector.hpp"
#include "fixed_array.hpp"
#include "fixed_vector.hpp"
#include "swap_pair.hpp"
#include <cstddef> // ptrdiff_t, size_t
#include <functional> // invoke, identity
#include <utility> // forward, exchange
#include <type_traits> // remove_const_t



namespace aa {

	// https://en.wikipedia.org/wiki/Quadtree
	// https://stackoverflow.com/questions/41946007/efficient-and-well-explained-implementation-of-a-quadtree-for-2d-collision-det
	template<class T, size_t H, size_t N, size_t M = N, storable_vector2_getter<T> L = std::identity>
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
		using position_type = vector2_getter_result_t<value_type, locator_type>;
		using pair_type = pair<array_element_t<position_type>>;

		static AA_CONSTEXPR const size_type leaves_count = int_exp2<2, size_type>(H), phantoms_count = (leaves_count - 1) / 3;

	protected:
		struct query_type {
			size_type i;
			pair_type q;
		};

		struct node_type {
			value_type *element;
			node_type *next;
		};

		using stack_type = fixed_vector<query_type, M>;

	public:
		// Reikia mintyje turėti tokį įdomų scenarijų, kad tame pačiame pass, mes įdedame elementą į lapą, po
		// to išemame elementą ir tada kažką darome, pass bus tas pats, bet first tuo atveju bus nullptr.
		struct leaf {
			// Modifiers
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



			// Lookup
			template<invocable_ref<reference> F, vector2_like T1 = pair_type, vector2_like T2 = pair_type>
			AA_CONSTEXPR void query_range(const T1 &tl, const T2 &br, F &&f, const fixed_quad_tree &t) const {
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



			// Member objects
		protected:
			size_type pass = 0;
			node_type *first = nullptr;
		};



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

		AA_CONSTEXPR const auto &grid() const { return leaves; }



		// Lookup
		AA_CONSTEXPR size_type find_leaf(const value_type &element) const {
			if constexpr (H) {
				size_type i = 0;
				pair_type q = position;
				const position_type &l = locate(element);

				return unsafe_for_each_peel_last(sizes, [&](const pair_type &s) -> void {
					const pair_type m = pair_type{get_x(q) + get_x(s), get_y(q) + get_y(s)};

					if (get_y(l) < get_y(m)) {
						if (get_x(l) < get_x(m)) { {	i = (i << 2) + 1; /*q = q;*/			}
						} else { {						i = (i << 2) + 2; get_x(q) = get_x(m);	} }
					} else {
						if (get_x(l) < get_x(m)) { {	i = (i << 2) + 3; get_y(q) = get_y(m);	}
						} else { {						i = (i << 2) + 4; q = m;				} }
					}
				}, [&](const pair_type &s) -> size_t {
					const pair_type m = pair_type{get_x(q) + get_x(s), get_y(q) + get_y(s)};

					if (get_y(l) < get_y(m)) {
						if (get_x(l) < get_x(m))		return (i << 2) + 1 - phantoms_count;
						else							return (i << 2) + 2 - phantoms_count;
					} else {
						if (get_x(l) < get_x(m))		return (i << 2) + 3 - phantoms_count;
						else							return (i << 2) + 4 - phantoms_count;
					}
				});
			} else {
				return 0;
			}
		}

		template<invocable_ref<size_type> F, vector2_like T1 = pair_type, vector2_like T2 = pair_type>
		AA_CONSTEXPR void find_leaves(const T1 &tl, const T2 &br, F &&f) const {
			if constexpr (H) {
				get_0(queries).emplace_back(0, position);

				unsafe_for_each_peel_last(sizes, [&](const pair_type &s) -> void {
					stack_type &in_stack = get_0(queries), &out_stack = get_1(queries);
					do {
						const query_type &q = in_stack.back();
						const pair_type m = pair_type{get_x(q.q) + get_x(s), get_y(q.q) + get_y(s)};

						if (get_y(tl) < get_y(m)) {
							if (get_x(tl) < get_x(m))	out_stack.emplace_back((q.i << 2) + 1, q.q);
							if (get_x(br) > get_x(m))	out_stack.emplace_back((q.i << 2) + 2, pair_type{get_x(m), get_y(q.q)});
						}
						if (get_y(br) > get_y(m)) {
							if (get_x(tl) < get_x(m))	out_stack.emplace_back((q.i << 2) + 3, pair_type{get_x(q.q), get_y(m)});
							if (get_x(br) > get_x(m))	out_stack.emplace_back((q.i << 2) + 4, m);
						}
						in_stack.pop_back();
					} while (!in_stack.empty());
					queries.swap();
				}, [&](const pair_type &s) -> void {
					stack_type &in_stack = get_0(queries);
					do {
						const query_type &q = in_stack.back();
						const pair_type m = pair_type{get_x(q.q) + get_x(s), get_y(q.q) + get_y(s)};

						if (get_y(tl) < get_y(m)) {
							if (get_x(tl) < get_x(m))	std::invoke(f, (q.i << 2) + 1 - phantoms_count);
							if (get_x(br) > get_x(m))	std::invoke(f, (q.i << 2) + 2 - phantoms_count);
						}
						if (get_y(br) > get_y(m)) {
							if (get_x(tl) < get_x(m))	std::invoke(f, (q.i << 2) + 3 - phantoms_count);
							if (get_x(br) > get_x(m))	std::invoke(f, (q.i << 2) + 4 - phantoms_count);
						}
						in_stack.pop_back();
					} while (!in_stack.empty());
				});
			} else {
				std::invoke(f, 0);
			}
		}

		template<invocable_ref<reference> F, vector2_like T1 = pair_type, vector2_like T2 = pair_type>
		AA_CONSTEXPR void query_range(const T1 &tl, const T2 &br, F &&f) const {
			find_leaves(tl, br, [&](const size_type i) -> void {
				leaves[i].query_range(tl, br, f, *this);
			});
		}

		template<invocable_ref<reference> F, vector2_like T1 = pair_type, vector2_like T2 = pair_type>
		AA_CONSTEXPR void query_loose_range(const T1 &tl, const T2 &br, F &&f) const {
			find_leaves(tl, br, [&](const size_type i) -> void {
				leaves[i].query(f, *this);
			});
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
		template<class U = locator_type, vector2_like T1 = pair_type, vector2_like T2 = pair_type>
		AA_CONSTEXPR fixed_quad_tree(const T1 &pos, const T2 &size, U &&u = {})
			: sizes{[&](std::remove_const_t<decltype(sizes)> &init_sizes) -> void
		{
			if constexpr (H) {
				unsafe_for_each_peel_first(init_sizes, [&](pair_type &s) -> void {
					s = pair_type{halve(get_x(size)), halve(get_y(size))};
				}, [](pair_type &s) -> void {
					const pair_type &prev_s = (&s)[-1];
					s = pair_type{halve(get_x(prev_s)), halve(get_y(prev_s))};
				});
			}
		}}, position{get_x(pos), get_y(pos)}, locator{std::forward<U>(u)} {}



		// Member objects
		const fixed_array<pair_type, H> sizes;
		const pair_type position;

	protected:
		size_type pass = 0;
		// Čia greičiausias konteineris, nes kitų svarstytų konteinerių metodai naudoja sąlygos sakinius.
		mutable swap_pair<stack_type> queries;
		array_t<leaf, leaves_count> leaves;
		fixed_fast_free_vector<node_type, N> nodes;

	public:
		[[no_unique_address]] const locator_type locator;
	};

}
