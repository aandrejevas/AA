#pragma once

#include "../metaprogramming/general.hpp"
#include "../algorithm/arithmetic.hpp"
#include "../algorithm/hash.hpp"
#include "../algorithm/find.hpp"
#include "fixed_vector.hpp"
#include "unsafe_subrange.hpp"
#include <cstring> // memset
#include <cstddef> // size_t, ptrdiff_t
#include <limits> // numeric_limits
#include <functional> // invoke
#include <utility> // forward
#include <iterator> // bidirectional_iterator_tag, forward_iterator_tag, default_sentinel_t
#include <bit> // countr_zero, countl_zero, popcount, bit_cast
#include <type_traits> // conditional_t
#include <concepts> // same_as



namespace aa {

	// https://en.wikipedia.org/wiki/Perfect_hash_function
	// https://en.wikipedia.org/wiki/Hash_table
	// Konteineris laiko savyje maišos kodus.
	// Neturi klasė iteratoriu, nes nežinome, kurie buckets naudojami, dirty regione taip pat ne visi naudojami.
	template<regular_unsigned_integral T, size_t N, size_t M = 0, storable H = generic_hash<>>
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
		using container_type = array_t<bucket_type, N>;

		// Member constants
		static AA_CONSTEXPR const bucket_type full_mask = numeric_max, empty_mask = numeric_min,
			left_shifted_mask = (full_mask << 1), right_shifted_mask = (full_mask >> 1);
		static AA_CONSTEXPR const bool is_space_efficient = !M;

	protected:
		struct dirty_region {
			size_type f = numeric_max, l = numeric_min;
		};

	public:
		struct bucket_iterable {
			// Member types
			using value_type = value_type;
			using size_type = size_type;
			using difference_type = difference_type;

			// Iteratorius skirtas iteruoti pro maišos kodus.
			// Čia iteratorius suteikiantis pilną funkcionalumą, jei reikia greitaveikos naudoti local_iterator.
			using const_iterator = struct iterator {
				using value_type = value_type;
				using difference_type = difference_type;
				using iterator_category = std::bidirectional_iterator_tag;

				AA_CONSTEXPR value_type operator*() const {
					return index + bucket->offset;
				}

				AA_CONSTEXPR iterator &operator++() {
					if (is_numeric_max(index)) {
						index = unsign<value_type>(std::countr_zero(bucket->word));
					} else {
						index = unsign<value_type>(std::countr_zero(bucket->word & (left_shifted_mask << index)));
					}
					return *this;
				}

				AA_CONSTEXPR iterator operator++(const int) {
					const value_type i = index;
					operator++();
					return {bucket, i};
				}

				AA_CONSTEXPR iterator &operator--() {
					if (index == max_size()) {
						index = last_index() - unsign<value_type>(std::countl_zero(bucket->word));
					} else {
						index = last_index() - unsign<value_type>(std::countl_zero(bucket->word
							& (right_shifted_mask >> (last_index() - index))));
					}
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
			AA_CONSTEXPR value_type back() const { return (last_index() - unsign<value_type>(std::countl_zero(word))) + offset; }



			// Iterators
			AA_CONSTEXPR const_iterator begin() const { return {this, unsign<value_type>(std::countr_zero(word))}; }
			AA_CONSTEXPR const_iterator end() const { return {this, max_size()}; }
			AA_CONSTEXPR const_iterator rbegin() const { return {this, last_index() - unsign<value_type>(std::countl_zero(word))}; }
			AA_CONSTEXPR const_iterator rend() const { return {this, numeric_max}; }



			// Capacity
			AA_CONSTEXPR bool empty() const { return bucket_empty(word); }
			AA_CONSTEXPR bool full() const { return bucket_full(word); }

			AA_CONSTEXPR size_type size() const { return bucket_size(word); }
			AA_CONSTEXPR difference_type ssize() const { return bucket_ssize(word); }

			static AA_CONSTEVAL size_type max_size() { return bucket_max_size(); }
			static AA_CONSTEVAL size_type last_index() { return bucket_last_index(); }



			// Member objects
			// Visos funkcijos pažymėtos const todėl nebus netyčia pakeisti šitie kintamieji.
			bucket_type word;
			value_type offset;
		};

		using const_local_iterator = struct local_iterator {
			using value_type = value_type;
			using difference_type = difference_type;
			using iterator_category = std::forward_iterator_tag;

			AA_CONSTEXPR value_type operator*() const {
				return unsign<value_type>(std::countr_zero(bitset)) + offset;
			}

			AA_CONSTEXPR local_iterator &operator++() {
				// (0 & (0 - 1)) == 0
				bitset &= bitset - 1;
				return *this;
			}

			AA_CONSTEXPR local_iterator operator++(const int) {
				const bucket_type b = bitset;
				bitset &= bitset - 1;
				return {b, offset};
			}

			friend AA_CONSTEXPR bool operator==(const local_iterator &l, const local_iterator &r) {
				return l.bitset == r.bitset && l.offset == r.offset;
			}

			friend AA_CONSTEXPR bool operator==(const local_iterator &l, const std::default_sentinel_t &) {
				return !l.bitset;
			}

			explicit AA_CONSTEXPR operator bool() const { return bitset; }

			bucket_type bitset;
			value_type offset;
		};



		// Capacity
		AA_CONSTEXPR bool empty() const {
			if constexpr (is_space_efficient)	return is_numeric_min(dirty.l);
			else								return dirty.empty();
		}
		AA_CONSTEXPR bool all_buckets_dirty() const {
			if constexpr (is_space_efficient)	return dirty.f == 0 && dirty.l == max_bucket_index();
			else								return dirty.full();
		}
		AA_CONSTEXPR bool dirty_buckets_full() const {
			if constexpr (is_space_efficient)	return empty() || unsafe_all_of(dirty_subrange(), bucket_full<bucket_type>);
			else								return empty() || unsafe_all_of(dirty, bucket_full<bucket_pointer>);
		}
		AA_CONSTEXPR bool full() const { return all_buckets_dirty() && dirty_buckets_full(); }

		AA_CONSTEXPR size_type size() const {
			if (!empty()) {
				size_type sum = 0;
				if constexpr (is_space_efficient) {
					unsafe_for_each(dirty_subrange(), [&sum](const bucket_type bin) -> void { sum += bucket_size(bin); });
				} else {
					unsafe_for_each(dirty, [&sum](const bucket_pointer bin) -> void { sum += bucket_size(bin); });
				}
				return sum;
			} else return 0;
		}
		AA_CONSTEXPR difference_type ssize() const { return std::bit_cast<difference_type>(size()); }

		// Daugybos nepaverčiame į postūmio operaciją, nes kompiliavimo metu ilgiau užtruktų nustatyti ar galime daryti
		// postūmį negu tiesiog įvykdyti daugybos operaciją, programos veikimo laikui irgi nepadėtų tokios kostrukcijos.
		//
		// Šis konteineris gali talpinti tik pavyzdžiui mažesnius sveikuosius skaičius už max_size
		// nebent būtų naudojamas ne tas hasher, kuris yra naudojamas pagal nutylėjimą.
		static AA_CONSTEVAL size_type max_size() {
			return bucket_max_size() * max_bucket_count();
		}



		// Bucket interface
		AA_CONSTEXPR size_type index(const bucket_pointer bin) const {
			return std::bit_cast<size_type>(bin - bins.data());
		}

		AA_CONSTEXPR bucket_iterable bucket(const bucket_pointer bin) const {
			return {*bin, product<bucket_max_size()>(index(bin))};
		}

		AA_CONSTEXPR bucket_iterable bucket(const size_type index) const {
			return {bins[index], product<bucket_max_size()>(index)};
		}

		// Neturime end atitikmenų, nes tiesiog reikia naudoti default_sentinel.
		AA_CONSTEXPR const_local_iterator begin(const bucket_pointer bin) const {
			return {*bin, product<bucket_max_size()>(index(bin))};
		}

		AA_CONSTEXPR const_local_iterator begin(const size_type index) const {
			return {bins[index], product<bucket_max_size()>(index)};
		}


		template<same_as_any<bucket_type, bucket_pointer> U>
		static AA_CONSTEXPR bool bucket_empty(const U bin) {
			if constexpr (std::same_as<U, bucket_type>)	return is_numeric_min(bin);
			else										return is_numeric_min(*bin);
		}

		template<same_as_any<bucket_type, bucket_pointer> U>
		static AA_CONSTEXPR bool bucket_full(const U bin) {
			if constexpr (std::same_as<U, bucket_type>)	return is_numeric_max(bin);
			else										return is_numeric_max(*bin);
		}

		template<same_as_any<bucket_type, bucket_pointer> U>
		static AA_CONSTEXPR size_type bucket_size(const U bin) {
			if constexpr (std::same_as<U, bucket_type>)	return unsign<size_type>(std::popcount(bin));
			else										return unsign<size_type>(std::popcount(*bin));
		}

		template<same_as_any<bucket_type, bucket_pointer> U>
		static AA_CONSTEXPR difference_type bucket_ssize(const U bin) {
			if constexpr (std::same_as<U, bucket_type>)	return std::bit_cast<difference_type>(bucket_size(bin));
			else										return std::bit_cast<difference_type>(bucket_size(*bin));
		}

		// Funkcija turi gražinti dvejeto laipsnį dėl greitaveikos sumetimų.
		static AA_CONSTEVAL size_type bucket_max_size() {
			return std::numeric_limits<bucket_type>::digits;
		}

		static AA_CONSTEVAL size_type bucket_last_index() {
			return bucket_max_size() - 1;
		}


		static AA_CONSTEVAL size_type max_bucket_index() {
			if constexpr (is_space_efficient) return N - 1; else return M - 1;
		}

		static AA_CONSTEVAL size_type last_bucket_index() requires (is_space_efficient) { return N - 1; }
		AA_CONSTEXPR size_type last_bucket_index() const requires (!is_space_efficient) { return dirty.last_index(); }

		static AA_CONSTEVAL size_type max_bucket_count() {
			if constexpr (is_space_efficient) return N; else return M;
		}

		static AA_CONSTEVAL size_type bucket_count() requires (is_space_efficient) { return N; }
		AA_CONSTEXPR size_type bucket_count() const requires (!is_space_efficient) { return dirty.size(); }


		static AA_CONSTEXPR size_type index(const size_type hash) {
			return quotient<bucket_max_size()>(hash);
		}

		static AA_CONSTEXPR bucket_type mask(const size_type hash) {
			return int_exp2<bucket_type>(remainder<bucket_max_size()>(hash));
		}



		// Observers
		template<hashable_by<const hasher_type &> K>
		AA_CONSTEXPR size_type hash(const K &key) const {
			return std::invoke(hasher, key);
		}

		// Kituose konteineriuose vidinio konteinerio nerodome, nes kituose konteineriuose vidinis
		// konteineris galima sakyti yra pats konteineris tai nėra tikslo to daryti.
		AA_CONSTEXPR const container_type &buckets() const { return bins; }

		AA_CONSTEXPR size_type first_dirty_index() const requires (is_space_efficient) { return dirty.f; }
		AA_CONSTEXPR size_type last_dirty_index() const requires (is_space_efficient) { return dirty.l; }

		AA_CONSTEXPR aa::unsafe_subrange<bucket_pointer> dirty_subrange() const requires (is_space_efficient) {
			return {bins.data() + dirty.f, bins.data() + dirty.l};
		}



		// Lookup
		template<hashable_by<const hasher_type &> K>
		AA_CONSTEXPR bool contains(const K &key) const {
			const size_type hash = this->hash(key);
			return bins[index(hash)] & mask(hash);
		}



		// Modifiers
	protected:
		AA_CONSTEXPR void mark_not_dirty() requires (is_space_efficient) {
			dirty.f = numeric_max;
			dirty.l = numeric_min;
		}

		AA_CONSTEXPR void contract_dirty_region(const size_type index) requires (is_space_efficient) {
			const bool f_not_dirty = (index == dirty.f), l_not_dirty = (index == dirty.l);
			if (f_not_dirty && l_not_dirty) mark_not_dirty();
			else if (f_not_dirty) {
				bucket_pointer f = bins.data() + (dirty.f + 1);
				const bucket_pointer l = bins.data() + dirty.l;
				do {
					if (!bucket_empty(*f)) { dirty.f = this->index(f); return; }
					if (f != l) ++f; else break;
				} while (true);
				mark_not_dirty();
			} else if (l_not_dirty) {
				bucket_pointer l = bins.data() + (dirty.l - 1);
				const bucket_pointer f = bins.data() + dirty.f;
				do {
					if (!bucket_empty(*l)) { dirty.l = this->index(l); return; }
					if (l != f) --l; else break;
				} while (true);
				mark_not_dirty();
			}
		}

		AA_CONSTEXPR void expand_dirty_region(const size_type index) requires (is_space_efficient) {
			// Patikrinau, čia greičiausia teisinga realizacija.
			if (dirty.f > index) dirty.f = index;
			if (dirty.l < index) dirty.l = index;
		}

	public:
		AA_CONSTEXPR void clear() {
			if (!empty()) {
				unsafe_clear();
			}
		}

		AA_CONSTEXPR void unsafe_clear() {
			if constexpr (is_space_efficient) {
				std::memset(bins.data() + dirty.f, empty_mask, size_of<bucket_type>(dirty.l - dirty.f + 1));
				mark_not_dirty();
			} else {
				do {
					*dirty.back() = empty_mask;
					dirty.pop_back();
				} while (!dirty.empty());
			}
		}

		AA_CONSTEXPR void clear_bucket(const size_type index) {
			if constexpr (is_space_efficient) {
				bins[index] = empty_mask;
				contract_dirty_region(index);
			} else {
				bucket_type &bin = bins[index];
				if (bin) {
					bin = empty_mask;
					dirty.fast_erase(aa::unsafe_find_last(dirty, &bin));
				}
			}
		}

		AA_CONSTEXPR void fill_bucket(const size_type index) {
			if constexpr (is_space_efficient) {
				expand_dirty_region(index);
				bins[index] = full_mask;
			} else {
				bucket_type &bin = bins[index];
				if (!bin) dirty.insert_back(&bin);
				bin = full_mask;
			}
		}

		template<hashable_by<const hasher_type &> K>
		AA_CONSTEXPR void insert(const K &key) {
			if constexpr (is_space_efficient) {
				const size_type hash = this->hash(key), index = this->index(hash);
				expand_dirty_region(index);
				bins[index] |= mask(hash);
			} else {
				const size_type hash = this->hash(key);
				bucket_type &bin = bins[index(hash)];
				if (!bin) {
					dirty.insert_back(&bin);
					bin = mask(hash);
				} else bin |= mask(hash);
			}
		}

		template<hashable_by<const hasher_type &> K>
		AA_CONSTEXPR void erase(const K &key) {
			if constexpr (is_space_efficient) {
				const size_type hash = this->hash(key), index = this->index(hash);
				if (!(bins[index] &= ~mask(hash)))
					contract_dirty_region(index);
			} else {
				const size_type hash = this->hash(key);
				bucket_type &bin = bins[index(hash)];
				// Reikia sąlygos, nes kitaip būtų bandoma pašalinti nenaudojamą bucket.
				if (bin) {
					if (!(bin &= ~mask(hash)))
						dirty.fast_erase(aa::unsafe_find_last(dirty, &bin));
				}
			}
		}



		// Special member functions
		template<class U = hasher_type>
		AA_CONSTEXPR fixed_perfect_hash_set(U &&h = {}) : hasher{std::forward<U>(h)} {}



		// Member objects
	protected:
		container_type bins = {};
		std::conditional_t<is_space_efficient, dirty_region, fixed_vector<bucket_type *, M>> dirty;

	public:
		[[no_unique_address]] const hasher_type hasher;
	};

}
