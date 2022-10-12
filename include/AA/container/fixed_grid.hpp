#pragma once

#include "../metaprogramming/general.hpp"
#include "../algorithm/find.hpp"
#include "fixed_fast_free_vector.hpp"
#include "unsafe_subrange.hpp"
#include <cstddef> // ptrdiff_t, size_t
#include <functional> // invoke, identity
#include <utility> // forward, exchange, as_const



namespace aa {

	// https://stackoverflow.com/questions/41946007/efficient-and-well-explained-implementation-of-a-quadtree-for-2d-collision-det
	template<class T, size_t W, size_t H, size_t N, storable_vector2_getter<T> L = std::identity>
	struct fixed_grid {
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

	protected:
		struct node_type {
			value_type *element;
			node_type *next;
		};

	public:
		// Reikia mintyje turėti tokį įdomų scenarijų, kad tame pačiame pass, mes įdedame elementą į lapą, po
		// to išemame elementą ir tada kažką darome, pass bus tas pats, bet first tuo atveju bus nullptr.
		struct leaf {
			// Modifiers
			AA_CONSTEXPR void insert(const pointer e, fixed_grid &t) {
				if (pass != t.pass) {
					pass = t.pass;
					first = t.nodes.emplace(e, nullptr);
				} else {
					first = t.nodes.emplace(e, first);
				}
			}

			AA_CONSTEXPR void erase(const const_pointer e, fixed_grid &t) {
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
			template<invocable_ref<reference> F, vector2_similar_to<pair_type> P1 = pair_type, vector2_similar_to<pair_type> P2 = pair_type>
			AA_CONSTEXPR void query_range(const P1 &tl, const P2 &br, F &&f, const fixed_grid &t) const {
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
			AA_CONSTEXPR void query(F &f, const fixed_grid &t) const {
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



		// Lookup
		template<vector2_similar_to<pair_type> P = pair_type>
		AA_CONSTEXPR leaf &find_leaf(const P &pos) {
			return const_cast<leaf &>(std::as_const(*this).find_leaf(pos));
		}

		template<vector2_similar_to<pair_type> P = pair_type>
		AA_CONSTEXPR const leaf &find_leaf(const P &pos) const {
			return
				leaves[unsign_cast<size_type>(get_y(pos) / get_h(leaf_size))][unsign_cast<size_type>(get_x(pos) / get_w(leaf_size))];
		}

		template<invocable_ref<leaf &> F, vector2_similar_to<pair_type> P1 = pair_type, vector2_similar_to<pair_type> P2 = pair_type>
		AA_CONSTEXPR void find_leaves(const P1 &tl, const P2 &br, F &&f) {
			std::as_const(*this).find_leaves(tl, br, [&](const leaf &l) -> void {
				std::invoke(f, const_cast<leaf &>(l));
			});
		}

		template<invocable_ref<const leaf &> F, vector2_similar_to<pair_type> P1 = pair_type, vector2_similar_to<pair_type> P2 = pair_type>
		AA_CONSTEXPR void find_leaves(const P1 &tl, const P2 &br, F &&f) const {
			const size_type
				jb = unsign_cast<size_type>(get_x(tl) / get_w(leaf_size)),
				je = unsign_cast<size_type>(get_x(br) / get_w(leaf_size));

			unsafe_for_each(unsafe_subrange{
				leaves.data() + unsign_cast<size_type>(get_y(tl) / get_h(leaf_size)),
				leaves.data() + unsign_cast<size_type>(get_y(br) / get_h(leaf_size))
			},
			[&](const array_t<leaf, W> &row) -> void {
				unsafe_for_each(unsafe_subrange{
					row.data() + jb,
					row.data() + je
				},
				[&](const leaf &l) -> void {
					std::invoke(f, l);
				});
			});
		}

		template<invocable_ref<reference> F, vector2_similar_to<pair_type> P1 = pair_type, vector2_similar_to<pair_type> P2 = pair_type>
		AA_CONSTEXPR void query_range(const P1 &tl, const P2 &br, F &&f) const {
			find_leaves(tl, br, [&](const leaf &l) -> void {
				l.query_range(tl, br, f, *this);
			});
		}

		template<invocable_ref<reference> F, vector2_similar_to<pair_type> P1 = pair_type, vector2_similar_to<pair_type> P2 = pair_type>
		AA_CONSTEXPR void query_loose_range(const P1 &tl, const P2 &br, F &&f) const {
			find_leaves(tl, br, [&](const leaf &l) -> void {
				l.query(f, *this);
			});
		}



		// Modifiers
		AA_CONSTEXPR void clear() {
			nodes.clear();
			++pass;
		}

		AA_CONSTEXPR void insert(value_type &element) {
			find_leaf(locate(element)).insert(&element, *this);
		}

		AA_CONSTEXPR void erase(const value_type &element) {
			find_leaf(locate(element)).erase(&element, *this);
		}



		// Special member functions
		template<class U = locator_type, vector2_similar_to<pair_type> P = pair_type>
		AA_CONSTEXPR fixed_grid(const P &size, U &&u = {})
			: leaf_size{get_w(size), get_h(size)}, locator{std::forward<U>(u)} {}



		// Member objects
		const pair_type leaf_size;
		array_t<leaf, W, H> leaves;

	protected:
		fixed_fast_free_vector<node_type, N> nodes;
		size_type pass = 0;

	public:
		[[no_unique_address]] const locator_type locator;
	};

}
