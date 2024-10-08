#pragma once

#include "../metaprogramming/general.hpp"
#include "../algorithm/find.hpp"
#include "fixed_vector.hpp"



namespace aa {

	// https://en.wikipedia.org/wiki/Set_(abstract_data_type)
	template<trivially_copyable T, size_t N, relation_for<T> C = std::ranges::less, bool MULTISET = false>
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
		constexpr const_reference operator[](const size_type pos) const { return elements.get(pos); }
		constexpr const_reference get(const size_type pos) const { return elements.get(pos); }
		constexpr const_reference rget(const size_type pos) const { return elements.rget(pos); }
		constexpr const_pointer data(const size_type pos) const { return elements.data(pos); }
		constexpr const_pointer rdata(const size_type pos) const { return elements.rdata(pos); }

		constexpr const_pointer data() const { return elements.data(); }
		constexpr const_pointer rdata() const { return elements.rdata(); }
		constexpr const_reference front() const { return elements.front(); }
		constexpr const_reference back() const { return elements.back(); }



		// Iterators
		constexpr const_iterator begin() const { return elements.begin(); }
		constexpr const_iterator end() const { return elements.end(); }
		constexpr const_iterator rbegin() const { return elements.rbegin(); }
		constexpr const_iterator rend() const { return elements.rend(); }



		// Capacity
		constexpr bool empty() const { return elements.empty(); }
		constexpr bool single() const { return elements.single(); }
		constexpr bool full() const { return elements.full(); }

		constexpr size_type size() const { return elements.size(); }
		constexpr size_type last_index() const { return elements.last_index(); }

		static consteval size_type max_size() { return N; }
		static consteval size_type max_index() { return N - 1; }



		// Observers
		template<class K1, in_relation_with<K1, const comparer_type &> K2>
		constexpr bool compare(const K1 &key1, const K2 &key2) const {
			return std::invoke(comparer, key1, key2);
		}



		// Lookup
		template<in_relation_with<value_type, const comparer_type &> K>
		constexpr const_iterator unsafe_lower_bound(const K &key) const {
			return aa::unsafe_lower_bound(elements, key, comparer);
		}

		template<in_relation_with<value_type, const comparer_type &> K>
		constexpr const_iterator unsafe_upper_bound(const K &key) const {
			return aa::unsafe_upper_bound(elements, key, comparer);
		}

		template<in_relation_with<value_type, const comparer_type &> K>
		constexpr const_iterator find(const K &key) const {
			if (empty())	return nullptr;
			else			return unsafe_find(key);
		}

		template<in_relation_with<value_type, const comparer_type &> K>
		constexpr const_iterator unsafe_find(const K &key) const {
			if (compare(back(), key)) {
				return nullptr;
			} else {
				const const_iterator pos = unsafe_lower_bound(key);
				if (!compare(key, *pos))	return pos;
				else						return nullptr;
			}
		}

		template<in_relation_with<value_type, const comparer_type &> K>
		constexpr bool contains(const K &key) const {
			if (empty())	return false;
			else			return unsafe_contains(key);
		}

		template<in_relation_with<value_type, const comparer_type &> K>
		constexpr bool unsafe_contains(const K &key) const {
			if (compare(back(), key))	return false;
			else						return !compare(key, *unsafe_lower_bound(key));
		}



		// Modifiers
		constexpr void clear() { elements.clear(); }

		template<assignable_to<reference> V>
		constexpr void clear(V &&value) { *elements.resize(elements.begin()) = std::forward<V>(value); }

		// Neturime, kaip fixed_vector turi, resize funkcijų, nes jomis naudotojas galėtų
		// padidinti savavališkai elementų kiekį ir taip sugadinti elementų tvarką.

		constexpr const_iterator pop_back() { return elements.pop_back(); }
		constexpr const_iterator pop_back(const size_type count) { return elements.pop_back(count); }

		template<in_relation_with_and_assignable_to<value_type, const comparer_type &> V>
		constexpr bool insert(const V &value) {
			if (empty())	return (clear(value), true);
			else			return unsafe_insert(value);
		}

		template<in_relation_with_and_assignable_to<value_type, const comparer_type &> V>
		constexpr bool unsafe_insert(const V &value) {
			if constexpr (MULTISET) {
				elements.insert(unsafe_upper_bound(value), value);
				return true;
			} else {
				// Nors reikia kiekvieną kartą daryti papildomą tikrinimą ar konteineris nėra tuščias,
				// vis tiek pasirinkta tokia realizacija, nes kartais išvengiamas lower_bound iškvietimas.
				if (compare(back(), value)) {
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

		template<in_relation_with<value_type, const comparer_type &> K>
		constexpr bool erase(const K &key) {
			if (empty())	return false;
			else			return unsafe_erase(key);
		}

		template<in_relation_with<value_type, const comparer_type &> K>
		constexpr bool unsafe_erase(const K &key) {
			if (compare(back(), key)) {
				return false;
			} else {
				const const_iterator pos = unsafe_lower_bound(key);
				if (!compare(key, *pos)) {
					elements.erase(pos);
					return true;
				} else return false;
			}
		}

		constexpr void erase(const const_iterator pos) {
			elements.erase(pos);
		}



		// Special member functions
		// Nėra esmės turėti default konstruktoriaus, nes comparer vis tiek reikėtų inicializuoti,
		// nes comparer tipas yra const. Perfect forwarding naudojame, kad palaikyti move semantics
		// ir pass by reference, jei parametras būtų const& tai neišeitų palaikyti move semantics.
		template<constructible_to<comparer_type> U = comparer_type>
		constexpr fixed_flat_set(U &&c = default_value) : comparer{std::forward<U>(c)} {}

		template<assignable_to<reference> V, constructible_to<comparer_type> U = comparer_type>
		constexpr fixed_flat_set(V &&value, U &&c = default_value)
			: elements{std::placeholders::_1}, comparer{std::forward<U>(c)} { elements.back() = std::forward<V>(value); }



		// Member objects
	protected:
		fixed_vector<value_type, N> elements;

	public:
		// Konteinerių invocables turi būti const, nes nėra logiška, kad juos galėtume pakeisti.
		// Į funkcijas paduodami invocables negali būti pakeisti todėl jie neturi būti const.
		[[no_unique_address]] const comparer_type comparer;
	};

	template<class T, size_t N, class C = std::ranges::less>
	using fixed_flat_multiset = fixed_flat_set<T, N, C, true>;

}
