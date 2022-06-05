#pragma once

#include "../metaprogramming/general.hpp"
#include "../algorithm/find.hpp"
#include "static_vector.hpp"
#include <cstddef> // size_t, ptrdiff_t
#include <functional> // invoke, less
#include <utility> // forward
#include <type_traits> // conditional_t



namespace aa {

	// https://en.wikipedia.org/wiki/Set_(abstract_data_type)
	template<trivially_copyable T, size_t N, storable_relation<T> C = std::ranges::less, bool MULTISET = false>
	struct static_flat_set {
		// Member types
		using value_type = T;
		using size_type = size_t;
		using difference_type = ptrdiff_t;
		using key_compare = C;
		using reference = value_type &;
		using const_reference = const value_type &;
		using pointer = value_type *;
		using const_pointer = const value_type *;
		using iterator = const_pointer;
		using const_iterator = const_pointer;
		using container_type = static_flat_set<T, N, C, MULTISET>;



		// Element access
		inline constexpr const_reference operator[](const size_type pos) const { return elements[pos]; }
		inline constexpr const_reference at(const size_type pos) const { return elements.at(pos); }
		inline constexpr const_reference r_at(const size_type pos) const { return elements.r_at(pos); }
		inline constexpr const_pointer pointer_at(const size_type pos) const { return elements.pointer_at(pos); }
		inline constexpr const_pointer rpointer_at(const size_type pos) const { return elements.rpointer_at(pos); }

		inline constexpr const_pointer data() const { return elements.data(); }
		inline constexpr const_pointer rdata() const { return elements.rdata(); }
		inline constexpr const_reference front() const { return elements.front(); }
		inline constexpr const_reference back() const { return elements.back(); }



		// Iterators
		inline constexpr const_iterator begin() const { return elements.begin(); }
		inline constexpr const_iterator end() const { return elements.end(); }
		inline constexpr const_iterator rbegin() const { return elements.rbegin(); }
		inline constexpr const_iterator rend() const { return elements.rend(); }



		// Capacity
		inline constexpr bool empty() const { return elements.empty(); }
		inline constexpr bool full() const { return elements.full(); }

		inline constexpr difference_type ssize() const { return elements.ssize(); }
		inline constexpr size_type size() const { return elements.size(); }

		static inline consteval size_type max_size() { return N; }



		// Observers
		inline constexpr const key_compare &key_comp() const { return comparer; }

		template<class K1, in_relation_with<K1, key_compare> K2>
		inline constexpr bool compare(const K1 &key1, const K2 &key2) const { return std::invoke(comparer, key1, key2); }



		// Lookup
		template<in_relation_with<value_type, key_compare> K>
		inline constexpr const_iterator lower_bound(const K &key) const {
			return aa::lower_bound(elements, key, comparer);
		}

		template<in_relation_with<value_type, key_compare> K>
		inline constexpr const_iterator upper_bound(const K &key) const {
			return aa::upper_bound(elements, key, comparer);
		}

		template<in_relation_with<value_type, key_compare> K>
		inline constexpr const_iterator find(const K &key) const {
			if (empty() || compare(back(), key)) {
				return nullptr;
			} else {
				const const_iterator pos = lower_bound(key);
				return !compare(key, *pos) ? pos : nullptr;
			}
		}

		template<in_relation_with<value_type, key_compare> K>
		inline constexpr bool contains(const K &key) const {
			return (empty() || compare(back(), key))
				? false : !compare(key, *lower_bound(key));
		}



		// Modifiers
		inline constexpr void clear() { elements.clear(); }
		inline constexpr void clear(const value_type &value) { elements.resize(elements.begin()); elements.back() = value; }

		// Neturime, kaip static_vector turi, resize funkcijų, nes jomis naudotojas gali
		// padidinti savavališkai elementų kiekį ir taip sugadinti elementų tvarką.

		inline constexpr iterator pop_back() { return elements.pop_back(); }
		inline constexpr iterator pop_back(const size_type count) { return elements.pop_back(count); }

		inline std::conditional_t<MULTISET, void, bool> insert(const value_type &value) {
			if constexpr (MULTISET) {
				elements.insert(upper_bound(value), value);
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
					const const_iterator pos = lower_bound(value);
					if (compare(value, *pos)) {
						elements.insert(pos, value);
						return true;
					} else return false;
				}
			}
		}

		template<in_relation_with<value_type, key_compare> K>
		inline void erase(const K &key) {
			if (empty() || compare(back(), key)) {
				return;
			} else {
				const const_iterator pos = lower_bound(key);
				if (!compare(key, *pos))
					elements.erase(pos);
			}
		}

		inline void erase(const const_iterator pos) {
			elements.erase(pos);
		}



		// Special member functions
		// Nėra esmės turėti default konstruktoriaus, nes comparer vis tiek reikėtų inicializuoti,
		// nes comparer tipas yra const. Perfect forwarding naudojame, kad palaikyti move semantics
		// ir pass by reference, jei parametras būtų const& tai neišeitų palaikyti move semantics.
		template<class U = key_compare>
		inline constexpr static_flat_set(U &&c = {}) : comparer{std::forward<U>(c)} {}
		template<class U = key_compare>
		inline constexpr static_flat_set(const value_type &value, U &&c = {})
			: elements{elements.begin()}, comparer{std::forward<U>(c)} { elements.back() = value; }



		// Member objects
	protected:
		static_vector<T, N> elements;
		[[no_unique_address]] const key_compare comparer;
	};

	template<class T, size_t N, class C = std::ranges::less>
	using static_flat_multiset = static_flat_set<T, N, C, true>;

}
