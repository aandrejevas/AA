#pragma once

#include "../metaprogramming/general.hpp"
#include "../algorithm/find.hpp"
#include "fixed_vector.hpp"
#include <cstddef> // size_t, ptrdiff_t
#include <functional> // invoke, less
#include <utility> // forward
#include <type_traits> // conditional_t



namespace aa {

	// https://en.wikipedia.org/wiki/Set_(abstract_data_type)
	template<trivially_copyable T, size_t N, storable_relation_for<T> C = std::ranges::less, bool MULTISET = false>
	struct fixed_flat_set {
		// Member types
		using value_type = T;
		using size_type = size_t;
		using difference_type = ptrdiff_t;
		using comparer_type = C;
		using reference = value_type &;
		using const_reference = const value_type &;
		using pointer = value_type *;
		using const_pointer = const value_type *;
		using iterator = const_pointer;
		using const_iterator = const_pointer;



		// Element access
		AA_CONSTEXPR const_reference operator[](const size_type pos) const { return elements[pos]; }
		AA_CONSTEXPR const_reference elem(const size_type pos) const { return elements.elem(pos); }
		AA_CONSTEXPR const_reference relem(const size_type pos) const { return elements.relem(pos); }
		AA_CONSTEXPR const_pointer data(const size_type pos) const { return elements.data(pos); }
		AA_CONSTEXPR const_pointer rdata(const size_type pos) const { return elements.rdata(pos); }

		AA_CONSTEXPR const_pointer data() const { return elements.data(); }
		AA_CONSTEXPR const_pointer rdata() const { return elements.rdata(); }
		AA_CONSTEXPR const_reference front() const { return elements.front(); }
		AA_CONSTEXPR const_reference back() const { return elements.back(); }



		// Iterators
		AA_CONSTEXPR const_iterator begin() const { return elements.begin(); }
		AA_CONSTEXPR const_iterator end() const { return elements.end(); }
		AA_CONSTEXPR const_iterator rbegin() const { return elements.rbegin(); }
		AA_CONSTEXPR const_iterator rend() const { return elements.rend(); }



		// Capacity
		AA_CONSTEXPR bool empty() const { return elements.empty(); }
		AA_CONSTEXPR bool full() const { return elements.full(); }

		AA_CONSTEXPR difference_type ssize() const { return elements.ssize(); }
		AA_CONSTEXPR size_type size() const { return elements.size(); }

		static AA_CONSTEVAL size_type max_size() { return N; }

		AA_CONSTEXPR size_type max_index() const { return elements.max_index(); }



		// Observers
		template<class K1, in_relation_with<K1, comparer_type> K2>
		[[gnu::always_inline]] AA_CONSTEXPR bool compare(const K1 &key1, const K2 &key2) const {
			return std::invoke(comparer, key1, key2);
		}



		// Lookup
		template<in_relation_with<value_type, comparer_type> K>
		AA_CONSTEXPR const_iterator unsafe_lower_bound(const K &key) const {
			return aa::unsafe_lower_bound(elements, key, comparer);
		}

		template<in_relation_with<value_type, comparer_type> K>
		AA_CONSTEXPR const_iterator unsafe_upper_bound(const K &key) const {
			return aa::unsafe_upper_bound(elements, key, comparer);
		}

		template<in_relation_with<value_type, comparer_type> K>
		AA_CONSTEXPR const_iterator find(const K &key) const {
			if (empty() || compare(back(), key)) {
				return nullptr;
			} else {
				const const_iterator pos = unsafe_lower_bound(key);
				return !compare(key, *pos) ? pos : nullptr;
			}
		}

		template<in_relation_with<value_type, comparer_type> K>
		AA_CONSTEXPR bool contains(const K &key) const {
			return (empty() || compare(back(), key))
				? false : !compare(key, *unsafe_lower_bound(key));
		}



		// Modifiers
		AA_CONSTEXPR void clear() { elements.clear(); }
		AA_CONSTEXPR void clear(const value_type &value) { *elements.resize(elements.begin()) = value; }

		// Neturime, kaip fixed_vector turi, resize funkcijų, nes jomis naudotojas galėtų
		// padidinti savavališkai elementų kiekį ir taip sugadinti elementų tvarką.

		AA_CONSTEXPR const_iterator pop_back() { return elements.pop_back(); }
		AA_CONSTEXPR const_iterator pop_back(const size_type count) { return elements.pop_back(count); }

		AA_CONSTEXPR std::conditional_t<MULTISET, void, bool> insert(const value_type &value) {
			if constexpr (MULTISET) {
				if (empty()) {
					clear(value);
				} else {
					elements.insert(unsafe_upper_bound(value), value);
				}
			} else {
				// Nors reikia kiekvieną kartą daryti papildomą tikrinimą ar konteineris nėra tuščias,
				// vis tiek pasirinkta tokia realizacija, nes kartais išvengiamas lower_bound iškvietimas.
				if (empty()) {
					clear(value);
					return true;
				} else if (compare(back(), value)) {
					elements.insert_back(value);
					return true;
				} else {
					const const_iterator pos = unsafe_lower_bound(value);
					if (compare(value, *pos)) {
						elements.insert(pos, value);
						return true;
					} else return false;
				}
			}
		}

		template<in_relation_with<value_type, comparer_type> K>
		AA_CONSTEXPR void erase(const K &key) {
			if (empty() || compare(back(), key)) {
				return;
			} else {
				const const_iterator pos = unsafe_lower_bound(key);
				if (!compare(key, *pos))
					elements.erase(pos);
			}
		}

		AA_CONSTEXPR void erase(const const_iterator pos) {
			elements.erase(pos);
		}



		// Special member functions
		// Nėra esmės turėti default konstruktoriaus, nes comparer vis tiek reikėtų inicializuoti,
		// nes comparer tipas yra const. Perfect forwarding naudojame, kad palaikyti move semantics
		// ir pass by reference, jei parametras būtų const& tai neišeitų palaikyti move semantics.
		template<class U = comparer_type>
		AA_CONSTEXPR fixed_flat_set(U &&c = {}) : comparer{std::forward<U>(c)} {}

		template<class U = comparer_type>
		AA_CONSTEXPR fixed_flat_set(const value_type &value, U &&c = {})
			: elements{elements.begin()}, comparer{std::forward<U>(c)} { elements.back() = value; }



		// Member objects
	protected:
		fixed_vector<value_type, N> elements;

	public:
		[[no_unique_address]] const comparer_type comparer;
	};

	template<class T, size_t N, class C = std::ranges::less>
	using fixed_flat_multiset = fixed_flat_set<T, N, C, true>;

}
