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
#include <iterator> // bidirectional_iterator_tag
#include <bit> // countr_zero, countl_zero, popcount, has_single_bit, bit_cast



namespace aa {

	// https://en.wikipedia.org/wiki/Perfect_hash_function
	// https://en.wikipedia.org/wiki/Hash_table
	// Konteineris laiko savyje maišos kodus.
	template<std::unsigned_integral T, size_t N, size_t M = N, storable H = std::hash<size_t>>
		requires (N >= M)
	struct fixed_perfect_hash_set {
		// Member types
		// Neturime reference ir pointer tipų, nes konteineris nelaiko elementų, jis juos apskaičiuoja.
		using bucket_type = T;
		using size_type = size_t;
		using difference_type = ptrdiff_t;
		using value_type = size_type;
		using hasher_type = H;
		using bucket_pointer = const bucket_type *;
		using iterator = const bucket_type *const *;
		using const_iterator = iterator;

		struct bucket_iterable {
			// Member types
			using value_type = value_type;
			using size_type = size_type;
			using difference_type = difference_type;

			// Iteratorius skirtas iteruoti pro maišos kodus.
			using const_iterator = struct iterator {
				using value_type = value_type;
				using difference_type = difference_type;
				using iterator_category = std::bidirectional_iterator_tag;

				AA_CONSTEXPR value_type operator*() const {
					return index + bucket->offset;
				}

				AA_CONSTEXPR iterator &operator++() {
					index = unsign<value_type>(std::countr_zero(bucket->word
						& (constant<(std::numeric_limits<bucket_type>::max() << 1)>() << index)));
					return *this;
				}

				AA_CONSTEXPR iterator operator++(const int) {
					const value_type i = index;
					operator++();
					return {bucket, i};
				}

				AA_CONSTEXPR iterator &operator--() {
					index = constant<max_bucket_size() - 1>() - unsign<value_type>(std::countl_zero(bucket->word
						& (constant<(std::numeric_limits<bucket_type>::max() >> 1)>() >> (constant<max_bucket_size() - 1>() - index))));
					return *this;
				}

				AA_CONSTEXPR iterator operator--(const int) {
					const value_type i = index;
					operator--();
					return {bucket, i};
				}

				friend AA_CONSTEXPR bool operator==(const iterator &l, const iterator &r) {
					return l.index == r.index && l.bucket == r.bucket;
				}

				// Iteratoriaus kintamieji ne const, nes racionalu ir reikia palaikyti kopijavimo operatorių.
				const bucket_iterable *bucket;
				value_type index;
			};



			// Element access
			AA_CONSTEXPR value_type front() const { return unsign<value_type>(std::countr_zero(word)) + offset; }
			AA_CONSTEXPR value_type back() const { return unsign<value_type>(std::countl_zero(word)) + offset; }



			// Iterators
			AA_CONSTEXPR const_iterator begin() const { return {this, unsign<value_type>(std::countr_zero(word))}; }
			AA_CONSTEXPR const_iterator end() const { return {this, max_bucket_size()}; }
			AA_CONSTEXPR const_iterator rbegin() const { return {this, unsign<value_type>(std::countl_zero(word))}; }
			AA_CONSTEXPR const_iterator rend() const { return {this, numeric_max}; }



			// Capacity
			AA_CONSTEXPR bool empty() const { return size() == 0; }
			AA_CONSTEXPR bool full() const { return size() == max_size(); }

			AA_CONSTEXPR size_type size() const { return unsign<size_type>(std::popcount(word)); }
			AA_CONSTEXPR difference_type ssize() const { return std::bit_cast<difference_type>(size()); }

			static AA_CONSTEVAL size_type max_size() { return max_bucket_size(); }



			// Member objects
			// Visos funkcijos pažymėtos const todėl nebus netyčia pakeisti šitie kintamieji.
			bucket_type word;
			value_type offset;
		};

		using const_local_iterator = bucket_iterable::const_iterator;
		using local_iterator = bucket_iterable::const_iterator;



		// Element access
		AA_CONSTEXPR bucket_pointer elem(const size_type pos) const { return used_bins.elem(pos); }
		AA_CONSTEXPR bucket_pointer relem(const size_type pos) const { return used_bins.relem(pos); }
		AA_CONSTEXPR const_iterator data(const size_type pos) const { return used_bins.data(pos); }
		AA_CONSTEXPR const_iterator rdata(const size_type pos) const { return used_bins.rdata(pos); }

		AA_CONSTEXPR const_iterator data() const { return used_bins.data(); }
		AA_CONSTEXPR const_iterator rdata() const { return used_bins.rdata(); }
		AA_CONSTEXPR bucket_pointer front() const { return used_bins.front(); }
		AA_CONSTEXPR bucket_pointer back() const { return used_bins.back(); }



		// Iterators
		// Iteratoriai skirti iteruoti pro buckets, o ne elementus. Nestandartinis sprendimas, bet ir nėra taip,
		// kad laiko konteineris elementus. Toks sprendimas leidžia teisingai iteruoti pro visus maišos kodus.
		AA_CONSTEXPR const_iterator begin() const { return used_bins.begin(); }
		AA_CONSTEXPR const_iterator end() const { return used_bins.end(); }
		AA_CONSTEXPR const_iterator rbegin() const { return used_bins.rbegin(); }
		AA_CONSTEXPR const_iterator rend() const { return used_bins.rend(); }



		// Capacity
		AA_CONSTEXPR bool empty() const { return used_bins.empty(); }
		AA_CONSTEXPR bool full() const { return size() == max_size(); }

		AA_CONSTEXPR size_type size() const {
			size_type sum = 0;
			unsafe_for_each(*this, [&sum](const bucket_pointer bin) -> void {
				sum += unsign<size_type>(std::popcount(*bin));
			});
			return sum;
		}
		AA_CONSTEXPR difference_type ssize() const { return std::bit_cast<difference_type>(size()); }

		// Jei N=M, tada šis konteineris gali talpinti tik mažesnias reikšmes už max_size,
		// nebent būtų naudojamas ne tas hasher, kuris yra naudojamas pagal nutylėjimą.
		static AA_CONSTEVAL size_type max_size() {
			return max_bucket_size() * max_bucket_count();
		}



		// Bucket interface
		// Šita funkcija reikalinga jei naudotojas pasirinktų dirbti tiesiogiai su buckets.
		AA_CONSTEXPR size_type index(const bucket_pointer bin) const {
			return std::bit_cast<size_type>(bin - bins.data());
		}

		AA_CONSTEXPR bucket_iterable bucket(const bucket_pointer bin) const {
			return {*bin, product<max_bucket_size()>(index(bin))};
		}


		AA_CONSTEXPR size_type bucket_size(const bucket_pointer bin) const {
			return unsign<size_type>(std::popcount(*bin));
		}

		// Funkcija turi gražinti dvejeto laipsnį dėl greitaveikos sumetimų.
		static AA_CONSTEVAL size_type max_bucket_size() {
			return std::numeric_limits<bucket_type>::digits;
		}

		// Nors tuo sunku patikėti, bet gal gali negražinti ko reikia funkcija todėl daromas tikrinimas.
		static_assert(std::has_single_bit(max_bucket_size()));


		AA_CONSTEXPR size_type bucket_count() const {
			return used_bins.size();
		}

		static AA_CONSTEVAL size_type max_bucket_count() {
			return M;
		}


		[[gnu::always_inline]] static AA_CONSTEXPR size_type index(const size_type hash) {
			return quotient<max_bucket_size()>(hash);
		}

		[[gnu::always_inline]] static AA_CONSTEXPR bucket_type mask(const size_type hash) {
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
			return bins[index(hash)] & mask(hash);
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
			bucket_type &bin = bins[index(hash)];
			if (!bin) used_bins.insert_back(&bin);
			bin |= mask(hash);
		}

		template<hashable_by<hasher_type> K>
		AA_CONSTEXPR void erase(const K &key) {
			const size_type hash = this->hash(key);
			bucket_type &bin = bins[index(hash)];
			// Reikia sąlygos, nes kitaip būtų bandoma pašalinti nenaudojamą bucket.
			if (bin) {
				bin &= ~mask(hash);
				if (!bin) used_bins.fast_erase(aa::unsafe_find_last(used_bins, &bin));
			}
		}

		template<hashable_by<hasher_type> K>
		AA_CONSTEXPR void insert_or_erase(const K &key) {
			const size_type hash = this->hash(key);
			bucket_type &bin = bins[index(hash)];
			if (bin) {
				bin ^= mask(hash);
				if (!bin) used_bins.fast_erase(aa::unsafe_find_last(used_bins, &bin));
			} else {
				used_bins.insert_back(&bin);
				bin |= mask(hash);
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
