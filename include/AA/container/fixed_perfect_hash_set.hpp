#pragma once

#include "../metaprogramming/general.hpp"
#include "../algorithm/arithmetic.hpp"
#include "../algorithm/find.hpp"
#include "fixed_vector.hpp"
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
	struct fixed_perfect_hash_set {
		// Member types
		// Neturime value_type, reference ir pointer tipų, nes konteineris nelaiko elementų.
		using bucket_type = T;
		using size_type = size_t;
		using difference_type = ptrdiff_t;
		using hasher_type = H;

		// Iteratorius skirtas iteruoti pro maišos kodus.
		struct iterator {
			// Neturime pointer tipo, nes neturime operator-> metodo.
			using value_type = size_type;
			using difference_type = difference_type;
			using reference = value_type;
			using iterator_category = std::forward_iterator_tag;

			AA_CONSTEXPR reference operator*() const {
				return product<N>(unsign<size_type>(std::countr_zero(bitset))) + unsign<size_type>(*pos - bins_begin);
			}

			AA_CONSTEXPR iterator &operator++() {
				// (0 & (0 - 1)) == 0
				bitset &= bitset - 1;
				if (!bitset && pos != rbegin) {
					bitset = **++pos;
				}
				return *this;
			}

			AA_CONSTEXPR iterator operator++(int) {
				const bucket_type b = bitset;
				bitset &= bitset - 1;
				if (!bitset && pos != rbegin) {
					const bucket_type *const *const p = pos;
					bitset = **++pos;
					return {b, p, rbegin, bins_begin};
				} else
					return {b, pos, rbegin, bins_begin};
			}

			friend AA_CONSTEXPR bool operator==(const iterator &l, const iterator &r) {
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
			using reference = value_type;
			using iterator_category = std::forward_iterator_tag;

			AA_CONSTEXPR reference operator*() const {
				return product<N>(unsign<size_type>(std::countr_zero(bitset))) + index;
			}

			AA_CONSTEXPR local_iterator &operator++() {
				bitset &= bitset - 1;
				return *this;
			}

			AA_CONSTEXPR local_iterator operator++(int) {
				const bucket_type b = bitset;
				bitset &= bitset - 1;
				return {b, index};
			}

			friend AA_CONSTEXPR bool operator==(const local_iterator &l, const local_iterator &r) {
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
		AA_CONSTEXPR iterator begin() const {
			if (used_bins.empty()) return {};
			else return {*used_bins.front(), used_bins.begin(), used_bins.rbegin(), bins.data()};
		}

		AA_CONSTEXPR iterator end() const { return {}; }



		// Capacity
		AA_CONSTEXPR bool empty() const { return used_bins.empty(); }
		AA_CONSTEXPR bool full() const { return size() == max_size(); }

		AA_CONSTEXPR size_type size() const {
			size_type sum = 0;
			const bucket_type *const *pos = used_bins.rbegin(), *const *const rend = used_bins.rend();
			while (pos != rend) sum += unsign<size_type>(std::popcount(**pos--));
			return sum;
		}

		// Šis konteineris gali talpinti tik mažesnias reikšmes už max_size,
		// nebent būtų naudojamas ne tas hasher, kuris yra naudojamas pagal nutylėjimą.
		static AA_CONSTEVAL size_type max_size() {
			return sizeof(bucket_type[M][std::numeric_limits<std::underlying_type_t<std::byte>>::digits]);
		}



		// Bucket interface
		AA_CONSTEXPR size_type to_index(const bucket_type *const bin) const { return unsign<size_type>(bin - bins.data()); }

		AA_CONSTEXPR size_type index_at(const size_type pos) const { return to_index(used_bins.at(pos)); }
		AA_CONSTEXPR size_type index_rat(const size_type pos) const { return to_index(used_bins.rat(pos)); }

		AA_CONSTEXPR size_type front_index() const { return to_index(used_bins.front()); }
		AA_CONSTEXPR size_type back_index() const { return to_index(used_bins.back()); }

		AA_CONSTEXPR local_iterator begin(const size_type n) const { return {bins[n], n}; }
		AA_CONSTEXPR local_iterator end(const size_type) const { return {}; }

		AA_CONSTEXPR size_type bucket_size(const size_type n) const { return unsign<size_type>(std::popcount(bins[n])); }

		AA_CONSTEXPR size_type bucket_count() const { return used_bins.size(); }

		static AA_CONSTEVAL size_type max_bucket_count() { return M; }

		[[gnu::always_inline]] static AA_CONSTEXPR size_type bucket(const size_type hash) { return remainder<N>(hash); }
		[[gnu::always_inline]] static AA_CONSTEXPR bucket_type bit(const size_type hash) { return int_exp2N<1uz, bucket_type>(quotient<N>(hash)); }



		// Observers
		template<hashable_by<hasher_type> K>
		[[gnu::always_inline]] AA_CONSTEXPR size_type hash(const K &key) const {
			return std::invoke(hasher, key);
		}

		// Kituose konteineriuose vidinio konteinerio nerodome, nes kituose konteineriuose vidinis
		// konteineris galima sakyti yra pats konteineris tai nėra tikslo to daryti.
		AA_CONSTEXPR const array_t<bucket_type, N> &buckets() const { return bins; }



		// Lookup
		template<hashable_by<hasher_type> K>
		AA_CONSTEXPR bool contains(const K &key) const {
			const size_type hash = this->hash(key);
			return bins[bucket(hash)] & bit(hash);
		}



		// Modifiers
		AA_CONSTEXPR void clear() {
			// Nedarome used_bins.clear(), nes vis tiek reiktų iteruoti per visus elementus ir tam reiktų įsivesti
			// lokalų kintamąjį, pamąsčiau kam tai daryti jei galime naudoti jau fixed_vector klasėje esantį kintamąjį.
			if (!used_bins.empty())
				unsafe_clear();
		}

		AA_CONSTEXPR void unsafe_clear() {
			do {
				*used_bins.back() = 0;
				used_bins.pop_back();
			} while (!used_bins.empty());
		}

		template<hashable_by<hasher_type> K>
		AA_CONSTEXPR void insert(const K &key) {
			const size_type hash = this->hash(key);
			bucket_type &bin = bins[bucket(hash)];
			if (!bin) used_bins.insert_back(&bin);
			bin |= bit(hash);
		}

		template<hashable_by<hasher_type> K>
		AA_CONSTEXPR void erase(const K &key) {
			const size_type hash = this->hash(key);
			bucket_type &bin = bins[bucket(hash)];
			if (bin) {
				bin &= ~bit(hash);
				if (!bin) used_bins.fast_erase(aa::find_last(used_bins, &bin));
			}
		}

		template<hashable_by<hasher_type> K>
		AA_CONSTEXPR void insert_or_erase(const K &key) {
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
		template<class U = hasher_type>
		AA_CONSTEXPR fixed_perfect_hash_set(U &&h = {}) : hasher{std::forward<U>(h)} {}



		// Member objects
	protected:
		array_t<bucket_type, N> bins = {};
		fixed_vector<bucket_type *, M> used_bins;

	public:
		[[no_unique_address]] const hasher_type hasher;
	};

}
