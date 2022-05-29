#pragma once

#include "../metaprogramming/general.hpp"
#include "../algorithm/arithmetic.hpp"
#include "../algorithm/find.hpp"
#include "static_vector.hpp"
#include <cstddef> // byte, size_t, ptrdiff_t
#include <limits> // numeric_limits
#include <functional> // hash, invoke
#include <concepts> // unsigned_integral
#include <type_traits> // underlying_type_t
#include <utility> // forward
#include <iterator> // forward_iterator_tag
#include <bit> // countr_zero, popcount



namespace aa {

	// https://en.wikipedia.org/wiki/Perfect_hash_function
	template<std::unsigned_integral T, size_t N, size_t M = N, storable H = std::hash<size_t>>
		requires (N >= M)
	struct static_perfect_hash_set {
		// Member types
		// Neturime value_type, reference ir pointer tipų, nes konteineris nelaiko elementų.
		using bucket_type = T;
		using size_type = size_t;
		using difference_type = ptrdiff_t;
		using hasher = H;
		using container_type = static_perfect_hash_set<T, N, M, H>;

		// Iteratorius skirtas iteruoti pro maišos kodus.
		struct iterator {
			// Neturime pointer tipo, nes neturime operator-> metodo.
			using value_type = size_type;
			using difference_type = difference_type;
			using reference = size_type;
			using iterator_category = std::forward_iterator_tag;
			using iterator_type = iterator;

			inline constexpr size_type operator*() const {
				return product<N>(static_cast<size_type>(std::countr_zero(bitset))) + static_cast<size_type>(*pos - bins_begin);
			}

			inline constexpr iterator_type &operator++() {
				// (0 & (0 - 1)) == 0
				bitset &= bitset - 1;
				if (!bitset && pos != rbegin) {
					bitset = **++pos;
				}
				return *this;
			}

			inline constexpr iterator_type operator++(int) {
				const bucket_type b = bitset;
				bitset &= bitset - 1;
				if (!bitset && pos != rbegin) {
					const bucket_type *const *const p = pos;
					bitset = **++pos;
					return {b, p, rbegin, bins_begin};
				} else
					return {b, pos, rbegin, bins_begin};
			}

			friend inline constexpr bool operator==(const iterator_type &l, const iterator_type &r) {
				if (l.bitset && r.bitset) {
					return l.bitset == r.bitset && l.pos == r.pos;
				} else {
					return l.bitset == r.bitset;
				}
			}

			bucket_type bitset;
			const bucket_type *const *pos, *const *rbegin, *bins_begin;
		};
		using const_iterator = iterator;

		struct local_iterator {
			using value_type = size_type;
			using difference_type = difference_type;
			using reference = size_type;
			using iterator_category = std::forward_iterator_tag;
			using iterator_type = local_iterator;

			inline constexpr size_type operator*() const {
				return product<N>(static_cast<size_type>(std::countr_zero(bitset))) + index;
			}

			inline constexpr iterator_type &operator++() {
				bitset &= bitset - 1;
				return *this;
			}

			inline constexpr iterator_type operator++(int) {
				const bucket_type b = bitset;
				bitset &= bitset - 1;
				return {b, index};
			}

			friend inline constexpr bool operator==(const iterator_type &l, const iterator_type &r) {
				if (l.bitset && r.bitset) {
					return l.bitset == r.bitset && l.index == r.index;
				} else {
					return l.bitset == r.bitset;
				}
			}

			bucket_type bitset;
			size_type index;
		};
		using const_local_iterator = local_iterator;



		// Iterators
		inline constexpr iterator begin() const {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
			if (used_bins.empty()) return {};
			else return {*used_bins.front(), used_bins.begin(), used_bins.rbegin(), bins.data()};
#pragma GCC diagnostic pop
		}

		inline constexpr iterator end() const { return {}; }



		// Capacity
		inline constexpr bool empty() const { return used_bins.empty(); }
		inline constexpr bool full() const { return size() == max_size(); }

		inline constexpr size_type size() const {
			size_type sum = 0;
			const bucket_type *const *pos = used_bins.rbegin(), *const *const rend = used_bins.rend();
			while (pos != rend) sum += static_cast<size_type>(std::popcount(**pos--));
			return sum;
		}

		// Šis konteineris gali talpinti tik mažesnias reikšmes už max_size,
		// nebent būtų naudojamas ne tas hasher, kuris yra naudojamas pagal nutylėjimą.
		static inline consteval size_type max_size() {
			return std::numeric_limits<std::underlying_type_t<std::byte>>::digits * sizeof(bucket_type[M]);
		}



		// Bucket interface
		inline constexpr size_type to_index(const bucket_type *const bin) const { return static_cast<size_type>(bin - bins.data()); }

		inline constexpr size_type index_at(const size_type pos) const { return to_index(used_bins.at(pos)); }
		inline constexpr size_type index_rat(const size_type pos) const { return to_index(used_bins.rat(pos)); }

		inline constexpr size_type front_index() const { return to_index(used_bins.front()); }
		inline constexpr size_type back_index() const { return to_index(used_bins.back()); }

		inline constexpr local_iterator begin(const size_type n) const { return {bins[n], n}; }
		inline constexpr local_iterator end(const size_type) const { return {}; }

		inline constexpr size_type bucket_size(const size_type n) const { return static_cast<size_type>(std::popcount(bins[n])); }

		inline constexpr size_type bucket_count() const { return used_bins.size(); }

		static inline consteval size_type max_bucket_count() { return M; }

		[[gnu::always_inline]] static inline constexpr size_type bucket(const size_type hash) { return remainder<N>(hash); }
		[[gnu::always_inline]] static inline constexpr bucket_type bit(const size_type hash) { return int_exp2<bucket_type>(quotient<N>(hash)); }



		// Observers
		inline constexpr const hasher &hash_function() const { return hasher_func; }

		template<hashable_by<hasher> K>
		[[gnu::always_inline]] inline constexpr size_type hash(const K &key) const { return std::invoke(hasher_func, key); }

		// Kituose konteineriuose vidinio konteinerio nerodome, nes kituose konteineriuose vidinis
		// konteineris galima sakyti yra pats konteineris tai nėra tikslo to daryti.
		inline constexpr const array_t<bucket_type, N> &buckets() const { return bins; }



		// Lookup
		template<hashable_by<hasher> K>
		inline constexpr bool contains(const K &key) const {
			const size_type hash = this->hash(key);
			return bins[bucket(hash)] & bit(hash);
		}



		// Modifiers
		inline constexpr void clear() {
			// Nedarome used_bins.clear(), nes vis tiek reiktų iteruoti per visus elementus ir tam reiktų įsivesti
			// lokalų kintamąjį, pamąsčiau kam tai daryti jei galime naudoti jau static_vector klasėje esantį kintamąjį.
			while (!used_bins.empty()) {
				*used_bins.back() = 0;
				used_bins.pop_back();
			}
		}

		inline constexpr void unsafe_clear() {
			do {
				*used_bins.back() = 0;
				used_bins.pop_back();
			} while (!used_bins.empty());
		}

		template<hashable_by<hasher> K>
		inline constexpr void insert(const K &key) {
			const size_type hash = this->hash(key);
			bucket_type &bin = bins[bucket(hash)];
			if (!bin) used_bins.insert_back(&bin);
			bin |= bit(hash);
		}

		template<hashable_by<hasher> K>
		inline constexpr void erase(const K &key) {
			const size_type hash = this->hash(key);
			bucket_type &bin = bins[bucket(hash)];
			if (bin) {
				bin &= ~bit(hash);
				if (!bin) used_bins.fast_erase(aa::find_last(used_bins, &bin));
			}
		}

		template<hashable_by<hasher> K>
		inline constexpr void insert_or_erase(const K &key) {
			const size_type hash = this->hash(key);
			bucket_type &bin = bins[bucket(hash)];
			if (bin) {
				bin ^= bit(hash);
				if (!bin) used_bins.fast_erase(aa::find_last(used_bins, &bin));
			} else {
				used_bins.insert_back(&bin);
				bin |= bit(hash);
			}
		}



		// Special member functions
		inline constexpr static_perfect_hash_set() {}
		template<class U>
		inline constexpr static_perfect_hash_set(U &&h) : hasher_func{std::forward<U>(h)} {}



		// Member objects
	protected:
		static_vector<bucket_type *, M> used_bins;
		array_t<bucket_type, N> bins = {};
		[[no_unique_address]] const hasher hasher_func = {};
	};

}
