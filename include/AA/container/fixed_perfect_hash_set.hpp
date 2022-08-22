#pragma once

#include "../metaprogramming/general.hpp"
#include "../algorithm/arithmetic.hpp"
#include "../algorithm/find.hpp"
#include "fixed_vector.hpp"
#include <cstddef> // size_t, ptrdiff_t
#include <limits> // numeric_limits
#include <functional> // hash, invoke
#include <concepts> // unsigned_integral
#include <utility> // forward
#include <iterator> // forward_iterator_tag
#include <bit> // countr_zero, popcount, has_single_bit



namespace aa {

	// https://en.wikipedia.org/wiki/Perfect_hash_function
	// https://en.wikipedia.org/wiki/Hash_table
	template<std::unsigned_integral T, size_t N, size_t M = N, storable H = std::hash<size_t>>
		requires (N >= M)
	struct fixed_perfect_hash_set {
		// Funkcija max_bucket_size turi gražinti dvejeto laipsnį dėl greitaveikos sumetimų.
		// Nors tuo sunku patikėti, bet gal gali negražinti ko reikia funkcija todėl daromas tikrinimas.
		static_assert(std::has_single_bit(max_bucket_size()));

		// Member types
		// Neturime value_type, reference ir pointer tipų, nes konteineris nelaiko elementų.
		using bucket_type = T;
		using size_type = size_t;
		using difference_type = ptrdiff_t;
		using hasher_type = H;

		// Iteratoriai skirti iteruoti pro maišos kodus.
		using const_iterator = struct iterator {
			// Neturime pointer tipo, nes neturime operator-> metodo.
			using value_type = size_type;
			using difference_type = difference_type;
			using reference = value_type;
			using iterator_category = std::forward_iterator_tag;

			AA_CONSTEXPR reference operator*() const {
				return unsign<size_type>(std::countr_zero(bitset)) + product<max_bucket_size()>(unsign<size_type>(*pos - bins_begin));
			}

			AA_CONSTEXPR iterator &operator++() {
				// (0 & (0 - 1)) == 0
				bitset &= bitset - 1;
				if (!bitset && pos != rbegin) {
					bitset = **++pos;
				}
				return *this;
			}

			AA_CONSTEXPR iterator operator++(const int) {
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
				l.bitset == r.bitset && l.pos == r.pos;
			}

			explicit AA_CONSTEXPR operator bool() const { return bitset; }

			bucket_type bitset;
			const bucket_type *const *pos, *const *rbegin, *bins_begin;
		};

		using const_local_iterator = struct local_iterator {
			using value_type = size_type;
			using difference_type = difference_type;
			using reference = value_type;
			using iterator_category = std::forward_iterator_tag;

			AA_CONSTEXPR reference operator*() const {
				return unsign<size_type>(std::countr_zero(bitset)) + index;
			}

			AA_CONSTEXPR local_iterator &operator++() {
				bitset &= bitset - 1;
				return *this;
			}

			AA_CONSTEXPR local_iterator operator++(const int) {
				const bucket_type b = bitset;
				bitset &= bitset - 1;
				return {b, index};
			}

			friend AA_CONSTEXPR bool operator==(const local_iterator &l, const local_iterator &r) {
				return l.bitset == r.bitset && l.index == r.index;
			}

			explicit AA_CONSTEXPR operator bool() const { return bitset; }

			// Jokie iteratorių kintamieji ne const, nes racionalu ir reikia palaikyti kopijavimo operatorių.
			bucket_type bitset;
			size_type index;
		};



		// Iterators
		AA_CONSTEXPR const_iterator begin() const {
			if (used_bins.empty()) return end();
			else return {*used_bins.front(), used_bins.begin(), used_bins.rbegin(), bins.data()};
		}

		// Pridera neštis šią visą šią informaciją end iteratoriui gautam iš šios klasės objekto.
		// Galime įsivaizduoti, kad tai normalus iteratorius gautas po iteravimo proceso.
		// Funkcija operator++(const int) taip pat gražina "perteklinius duomenis" kartais.
		AA_CONSTEXPR const_iterator end() const {
			return {0, used_bins.rbegin(), used_bins.rbegin(), bins.data()};
		}



		// Capacity
		AA_CONSTEXPR bool empty() const { return used_bins.empty(); }
		AA_CONSTEXPR bool full() const { return size() == max_size(); }

		AA_CONSTEXPR size_type size() const {
			size_type sum = 0;
			const bucket_type *const *pos = used_bins.rbegin(), *const *const rend = used_bins.rend();
			while (pos != rend) sum += unsign<size_type>(std::popcount(**pos--));
			return sum;
		}

		// Jei N=M, tada šis konteineris gali talpinti tik mažesnias reikšmes už max_size,
		// nebent būtų naudojamas ne tas hasher, kuris yra naudojamas pagal nutylėjimą.
		static AA_CONSTEVAL size_type max_size() {
			return max_bucket_size() * max_bucket_count();
		}



		// Bucket interface
		AA_CONSTEXPR size_type to_index(const bucket_type *const bin) const { return unsign<size_type>(bin - bins.data()); }

		AA_CONSTEXPR size_type index_at(const size_type pos) const { return to_index(used_bins.at(pos)); }
		AA_CONSTEXPR size_type index_rat(const size_type pos) const { return to_index(used_bins.rat(pos)); }

		AA_CONSTEXPR size_type front_index() const { return to_index(used_bins.front()); }
		AA_CONSTEXPR size_type back_index() const { return to_index(used_bins.back()); }

		AA_CONSTEXPR const_local_iterator begin(const size_type n) const { return {bins[n], product<max_bucket_size()>(n)}; }
		AA_CONSTEXPR const_local_iterator end(const size_type n) const { return {0, product<max_bucket_size()>(n)}; }


		AA_CONSTEXPR size_type bucket_size(const size_type n) const {
			return unsign<size_type>(std::popcount(bins[n]));
		}

		static AA_CONSTEVAL size_type max_bucket_size() {
			return std::numeric_limits<bucket_type>::digits;
		}


		AA_CONSTEXPR size_type bucket_count() const {
			return used_bins.size();
		}

		static AA_CONSTEVAL size_type max_bucket_count() {
			return M;
		}


		[[gnu::always_inline]] static AA_CONSTEXPR size_type bucket(const size_type hash) {
			return quotient<max_bucket_size()>(hash);
		}

		[[gnu::always_inline]] static AA_CONSTEXPR bucket_type bit(const size_type hash) {
			return int_exp2<bucket_type>(remainder<max_bucket_size()>(hash));
		}



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
			// Reikia sąlygos, nes kitaip būtų bandoma pašalinti nenaudojamą bucket.
			if (bin) {
				bin &= ~bit(hash);
				if (!bin) used_bins.fast_erase(aa::unsafe_find_last(used_bins, &bin));
			}
		}

		template<hashable_by<hasher_type> K>
		AA_CONSTEXPR void insert_or_erase(const K &key) {
			const size_type hash = this->hash(key);
			bucket_type &bin = bins[bucket(hash)];
			if (bin) {
				bin ^= bit(hash);
				if (!bin) used_bins.fast_erase(aa::unsafe_find_last(used_bins, &bin));
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
		// Yra galimybė nenaudoti šios struktūros, o naudoti du kintamuosius, kurie žymėtų intervalą, kuris yra nešvarus.
		// Bet tokia realizacija prognuozuoju, kad lėtai dirbtų su labai pasiskirsčiusiais maišos kodais.
		fixed_vector<bucket_type *, M> used_bins;

	public:
		[[no_unique_address]] const hasher_type hasher;
	};

}
