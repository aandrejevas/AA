#pragma once

#include "../metaprogramming/general.hpp"
#include "../algorithm/hash.hpp"
#include "../algorithm/init.hpp"
#include <cstddef> // size_t, ptrdiff_t
#include <functional> // invoke, equal_to
#include <utility> // forward
#include <iterator> // reverse_iterator



namespace aa {

	// https://en.wikipedia.org/wiki/Set_(abstract_data_type)
	// https://en.wikipedia.org/wiki/Perfect_hash_function#Minimal_perfect_hash_function
	// Naudotojo atsakomybė pateikti minimal perfect hash function. Lengviausia kažkokį switch parašyti atskiram atvejui.
	// Nepatogu, kad A argumentai turi būti gale, bet šią problemą pilnai išsprendžiame su aliases.
	template<class H, auto... A>
		requires (storable_hasher_for<H, decltype(A)...>)
	struct fixed_immutable_set : pack<A...> {
		// Member types
		using size_type = size_t;
		using difference_type = ptrdiff_t;
		using value_type = size_type;
		using hasher_type = H;
		using reference = value_type &;
		using const_reference = const value_type &;
		using pointer = value_type *;
		using const_pointer = const value_type *;
		using iterator = const_pointer;
		using const_iterator = const_pointer;
		using reverse_iterator = std::reverse_iterator<const_pointer>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		using container_type = array_t<size_type, sizeof...(A)>;



		// Element access
		AA_CONSTEXPR const_reference operator[](const size_type pos) const { return elem(pos); }
		AA_CONSTEXPR const_reference elem(const size_type pos) const { return *data(pos); }
		AA_CONSTEXPR const_pointer data(const size_type pos) const { return data() + pos; }

		AA_CONSTEXPR const_pointer data() const { return indices.data(); }
		AA_CONSTEXPR const_reference front() const { return indices.front(); }
		AA_CONSTEXPR const_reference back() const { return indices.back(); }



		// Iterators
		AA_CONSTEXPR const_iterator begin() const { return indices.begin(); }
		AA_CONSTEXPR const_iterator end() const { return indices.end(); }
		AA_CONSTEXPR const_reverse_iterator rbegin() const { return indices.rbegin(); }
		AA_CONSTEXPR const_reverse_iterator rend() const { return indices.rend(); }



		// Capacity
		static AA_CONSTEVAL bool empty() { return !sizeof...(A); }
		static AA_CONSTEVAL difference_type ssize() { return sizeof...(A); }
		static AA_CONSTEVAL size_type size() { return sizeof...(A); }
		static AA_CONSTEVAL size_type max_size() { return sizeof...(A); }
		static AA_CONSTEVAL size_type max_index() { return sizeof...(A) - 1; }



		// Observers
		template<hashable_by<hasher_type> K>
		[[gnu::always_inline]] AA_CONSTEXPR size_type hash(const K &key) const {
			return std::invoke(hasher, key);
		}

		AA_CONSTEXPR bool valid() const {
			return apply<sizeof...(A)>([&]<size_type... I>() -> bool { return (... && !is_numeric_max(indices[I])); });
		}



		// Lookup
		template<hashable_by<hasher_type> K>
		AA_CONSTEXPR size_type index(const K &key) const {
			return indices[hash(key)];
		}

		template<class F, hashable_by<hasher_type> K>
		AA_CONSTEXPR void context(const K &key, F &&f = {}) const {
			this->get(index(key), std::forward<F>(f));
		}

		template<class E = std::ranges::equal_to, hashable_by<hasher_type> K>
		AA_CONSTEXPR bool contains(const K &key, const E &e = {}) const {
			bool r;
			context(key, [&]<in_relation_with<K, E> T>(const T & a) -> void {
				r = std::invoke(e, a, key);
			});
			return r;
		}



		// Special member functions
		template<class U = hasher_type>
		AA_CONSTEXPR fixed_immutable_set(U &&h = {}) : hasher{std::forward<U>(h)}, indices{
				create_with_invocable<container_type>([&](container_type &c) -> void
		{
			c.fill(numeric_max);
			// Neišeina čia jokio assert daryti at compile time tai joks ir nedaromas.
			// https://stackoverflow.com/questions/62185058/why-isnt-this-known-at-compile-time-in-a-consteval-constructor
			apply<sizeof...(A)>([&]<size_type... I>() -> void { ((c[hash(A)] = I), ...); });
		})} {}



		// Member objects
		[[no_unique_address]] const hasher_type hasher;

		// Galėtų konteineris būti static, bet tada panaikintume galimybę hashinti template parametrus su runtime hasher.
		const container_type indices;
	};

	fixed_immutable_set(allow_ctad)->fixed_immutable_set<mod_hash<0>>;



	template<auto... A>
	using h_fixed_immutable_set = fixed_immutable_set<hash<>, A...>;

	template<auto... A>
	using mh_fixed_immutable_set = fixed_immutable_set<mod_hash<sizeof...(A)>, A...>;

}
