#pragma once

#include "../metaprogramming/general.hpp"
#include "../algorithm/find.hpp"
#include "../algorithm/arithmetic.hpp"
#include "fixed_fast_free_vector.hpp"
#include "fixed_vector.hpp"
#include "unsafe_subrange.hpp"
#include <algorithm> // min, max



namespace aa {

	// https://stackoverflow.com/questions/41946007/efficient-and-well-explained-implementation-of-a-quadtree-for-2d-collision-det
	// Klasė neturi iteratorių, nes ji pati savyje nelaiko elementų, reiktų iteruoti struktūrą, kuri juos laiko.
	template<class T, size_t W, size_t H, size_t N, arithmetic_array_getter<T, 2> L = std::identity, bool ERASABLE = false>
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
		using position_type = arithmetic_array_getter_result_t<value_type, locator_type>;
		using pair_type = pair<array_element_t<position_type>>;

		// Member constants
		static AA_CONSTEXPR const array_element_t<position_type> min_loc = 0;
		static AA_CONSTEXPR const size_type first_pass = 1;

	protected:
		struct node_type {
			value_type *element;
			node_type *next;
		};

		using container_type = std::conditional_t<ERASABLE, fixed_fast_free_vector<node_type, N>, fixed_vector<node_type, N>>;

		template<class... A>
		AA_CONSTEXPR node_type *emplace(A&&... args) {
			if constexpr (ERASABLE)		return nodes.emplace(std::forward<A>(args)...);
			else						return nodes.emplace_back(std::forward<A>(args)...);
		}

	public:
		struct leaf {
			// Modifiers
			AA_CONSTEXPR void insert(const pointer e, fixed_grid &t) {
				if (empty(t)) {
					pass = t.pass;
					first = t.emplace(e, nullptr);
				} else {
					first = t.emplace(e, first);
				}
			}

			AA_CONSTEXPR void erase(const const_pointer e, fixed_grid &t) requires (ERASABLE) {
				if (!empty(t)) {
					if (first->element == e) {
						t.nodes.erase(std::exchange(first, first->next));
						// Reikia mintyje turėti tokį įdomų scenarijų, kad tame pačiame pass, mes įdedame elementą į lapą, po
						// to išemame elementą ir tada kažką darome, pass bus tas pats, bet first tuo atveju bus nullptr.
						if (first == nullptr) {
							pass = 0;
						}
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
			template<invocable_ref<reference> F, array_similar_to<pair_type> P1 = pair_type, array_similar_to<pair_type> P2 = pair_type>
			AA_CONSTEXPR void query_range(const P1 &tl, const P2 &br, F &&f, const fixed_grid &t) const {
				if (!empty(t)) {
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
				if (!empty(t)) {
					const node_type *iter = first;
					do {
						std::invoke(f, *iter->element);
					} while ((iter = iter->next));
				}
			}



			// Observers
			AA_CONSTEXPR size_type get_pass() const { return pass; }

			AA_CONSTEXPR bool empty(const fixed_grid &t) const { return pass != t.pass; }



			// Special member functions
			AA_CONSTEVAL leaf() = default;



			// Member objects
		protected:
			size_type pass;
			node_type *first;
		};



		// Capacity
		AA_CONSTEXPR bool empty() const { return nodes.empty(); }
		AA_CONSTEXPR bool single() const { return nodes.single(); }
		AA_CONSTEXPR bool full() const { return nodes.full(); }

		AA_CONSTEXPR difference_type ssize() const { return nodes.ssize(); }
		AA_CONSTEXPR size_type size() const { return nodes.size(); }

		static AA_CONSTEVAL size_type max_size() { return N; }
		static AA_CONSTEVAL size_type row_size() { return W; }
		static AA_CONSTEVAL size_type col_size() { return H; }
		static AA_CONSTEVAL size_type last_row_index() { return W - 1; }
		static AA_CONSTEVAL size_type last_col_index() { return H - 1; }



		// Observers
		AA_CONSTEXPR size_type get_pass() const { return pass; }

		AA_CONSTEXPR const position_type &locate(const value_type &e) const {
			return std::invoke(locator, e);
		}



		// Lookup
		template<array_similar_to<pair_type> P = pair_type>
		AA_CONSTEXPR leaf &find_leaf(const P &pos) {
			return const_cast<leaf &>(std::as_const(*this).find_leaf(pos));
		}

		template<array_similar_to<pair_type> P = pair_type>
		AA_CONSTEXPR const leaf &find_leaf(const P &pos) const {
			return
				leaves[unsign_cast<size_type>(get_y(pos) / get_h(leaf_size))][unsign_cast<size_type>(get_x(pos) / get_w(leaf_size))];
		}

		template<invocable_ref<leaf &> F, array_similar_to<pair_type> P1 = pair_type, array_similar_to<pair_type> P2 = pair_type>
		AA_CONSTEXPR void find_leaves(const P1 &tl, const P2 &br, F &&f) {
			std::as_const(*this).find_leaves(tl, br, [&](const leaf &l) -> void {
				std::invoke(f, const_cast<leaf &>(l));
			});
		}

		template<invocable_ref<const leaf &> F, array_similar_to<pair_type> P1 = pair_type, array_similar_to<pair_type> P2 = pair_type>
		AA_CONSTEXPR void find_leaves(const P1 &tl, const P2 &br, F &&f) const {
			if (get_x(br) < min_loc || get_y(br) < min_loc || get_x(max_loc) < get_x(tl) || get_y(max_loc) < get_y(tl)) return;

			const size_type
				jb = unsign_cast<size_type>(std::ranges::max(get_x(tl), min_loc) / get_w(leaf_size)),
				ib = unsign_cast<size_type>(std::ranges::max(get_y(tl), min_loc) / get_h(leaf_size)),
				je = unsign_cast<size_type>(std::ranges::min(get_x(br), get_x(max_loc)) / get_w(leaf_size)),
				ie = unsign_cast<size_type>(std::ranges::min(get_y(br), get_y(max_loc)) / get_h(leaf_size));

			unsafe_for_each(unsafe_subrange{
				leaves.data() + ib,
				leaves.data() + ie
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

		template<invocable_ref<reference> F, array_similar_to<pair_type> P1 = pair_type, array_similar_to<pair_type> P2 = pair_type>
		AA_CONSTEXPR void query_range(const P1 &tl, const P2 &br, F &&f) const {
			find_leaves(tl, br, [&](const leaf &l) -> void {
				l.query_range(tl, br, f, *this);
			});
		}

		template<invocable_ref<reference> F, array_similar_to<pair_type> P1 = pair_type, array_similar_to<pair_type> P2 = pair_type>
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

		AA_CONSTEXPR void erase(const value_type &element) requires (ERASABLE) {
			find_leaf(locate(element)).erase(&element, *this);
		}



		// Special member functions
		template<constructible_to<locator_type> U = locator_type, array_similar_to<pair_type> P = pair_type>
		AA_CONSTEXPR fixed_grid(const P &size, U &&u = default_value)
			: fixed_grid{size, {product<W>(get_w(size)) - 1, product<H>(get_h(size)) - 1}, std::forward<U>(u)} {}

		template<constructible_to<locator_type> U = locator_type, array_similar_to<pair_type> P1 = pair_type, array_similar_to<pair_type> P2 = pair_type>
		AA_CONSTEXPR fixed_grid(const P1 &size, const P2 &l, U &&u = default_value)
			: leaf_size{get_w(size), get_h(size)}, max_loc{get_x(l), get_y(l)}, locator{std::forward<U>(u)} {}



		// Member objects
		const pair_type leaf_size, max_loc;
		array_t<leaf, W, H> leaves;

	protected:
		container_type nodes;
		size_type pass = first_pass;

	public:
		[[no_unique_address]] const locator_type locator;
	};

	template<class T, size_t W, size_t H, size_t N, class L = std::identity>
	using fixed_erasable_grid = fixed_grid<T, W, H, N, L, true>;

}
