#pragma once

#include "../metaprogramming/general.hpp"
#include "../algorithm/arithmetic.hpp"
#include "../algorithm/hash.hpp"
#include "../algorithm/find.hpp"
#include "fixed_vector.hpp"
#include "unsafe_subrange.hpp"
#include "bitset_view.hpp"
#include <algorithm> // fill



namespace aa {

	namespace {
		template<regular_unsigned_integral T, size_t N, class H>
		struct fixed_hashes_set_base {
			// Member types
			// Neturime reference ir pointer tipų, nes konteineris nelaiko elementų, jis juos apskaičiuoja.
			using bucket_type = T;
			using size_type = size_t;
			using difference_type = ptrdiff_t;
			using value_type = size_type;
			using hasher_type = H;
			using bucket_pointer = const bucket_type *;
			using container_type = std::array<bucket_type, N>;
			using base_type = fixed_hashes_set_base;
			using local_iterator = bitset_view<bucket_type, value_type>;
			using const_local_iterator = local_iterator;
			using pure_local_iterator = bitset_view<bucket_type>;



			// Bucket interface
			AA_CONSTEXPR size_type index(const bucket_pointer bin) const {
				return std::bit_cast<size_type>(bin - bins.data());
			}

			// Neturime end atitikmenų, nes tiesiog reikia naudoti default_sentinel.
			AA_CONSTEXPR const_local_iterator begin(const bucket_pointer bin) const {
				return {*bin, product<local_iterator::max_size()>(index(bin))};
			}

			AA_CONSTEXPR const_local_iterator begin(const size_type index) const {
				return {bins[index], product<local_iterator::max_size()>(index)};
			}

			static AA_CONSTEXPR pure_local_iterator bucket(const bucket_pointer bin) {
				return bucket(*bin);
			}

			static AA_CONSTEXPR pure_local_iterator bucket(const bucket_type bin) {
				return {bin};
			}

			static AA_CONSTEXPR size_type index(const size_type hash) {
				return quotient<local_iterator::max_size()>(hash);
			}

			static AA_CONSTEXPR bucket_type mask(const size_type hash) {
				return int_exp2<bucket_type>(remainder<local_iterator::max_size()>(hash));
			}



			// Observers
			template<hashable_by<const hasher_type &> K>
			AA_CONSTEXPR size_type hash(const K &key) const {
				return std::invoke(hasher, key);
			}

			// Kituose konteineriuose vidinio konteinerio nerodome, nes kituose konteineriuose vidinis
			// konteineris galima sakyti yra pats konteineris tai nėra tikslo to daryti.
			AA_CONSTEXPR const container_type &buckets() const { return bins; }



			// Lookup
			template<hashable_by<const hasher_type &> K>
			AA_CONSTEXPR bool contains(const K &key) const {
				const size_type hash = this->hash(key);
				return bins[index(hash)] & mask(hash);
			}



			// Special member functions
			template<constructible_to<hasher_type> U = hasher_type>
			AA_CONSTEXPR fixed_hashes_set_base(U &&h = default_value) : hasher{std::forward<U>(h)} {}



			// Member objects
		protected:
			container_type bins = default_value;

		public:
			[[no_unique_address]] const hasher_type hasher;
		};
	}

	// https://en.wikipedia.org/wiki/Perfect_hash_function
	// https://en.wikipedia.org/wiki/Hash_table
	// Konteineris laiko savyje maišos kodus.
	// Neturi klasė iteratoriu, nes nežinome, kurie buckets naudojami, dirty regione taip pat ne visi naudojami.
	template<regular_unsigned_integral T, size_t N, size_t M = 0, class H = generic_hash<>>
		requires (N >= M || is_numeric_max(M))
	struct fixed_hashes_set : fixed_hashes_set_base<T, N, H> {
		using typename fixed_hashes_set_base<T, N, H>::base_type, base_type::base_type,
			typename base_type::bucket_type, typename base_type::bucket_pointer,
			typename base_type::size_type, typename base_type::difference_type,
			typename base_type::local_iterator, typename base_type::hasher_type;
		using iterator = const bucket_pointer *;
		using const_iterator = iterator;



		// Capacity
		AA_CONSTEXPR bool empty() const {
			return dirty.empty();
		}
		AA_CONSTEXPR bool single_bucket_dirty() const {
			return dirty.single();
		}
		AA_CONSTEXPR bool single() const {
			return single_bucket_dirty() && bucket(dirty.front()).size() == 1;
		}
		AA_CONSTEXPR bool all_buckets_dirty() const {
			return dirty.full();
		}
		AA_CONSTEXPR bool dirty_buckets_full() const {
			return empty() || unsafe_all_of(dirty_subrange(), [&](const bucket_pointer bin) ->
				bool { return this->bucket(bin).full(); });
		}
		AA_CONSTEXPR bool full() const { return all_buckets_dirty() && dirty_buckets_full(); }

		AA_CONSTEXPR size_type size() const {
			if (!empty()) {
				size_type sum = 0;
				unsafe_for_each(dirty_subrange(), [&](const bucket_pointer bin) ->
					void { sum += this->bucket(bin).size(); });
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
			return local_iterator::max_size() * max_bucket_count();
		}



		// Bucket interface
		static AA_CONSTEVAL size_type max_bucket_index() { return M - 1; }
		AA_CONSTEXPR size_type last_bucket_index() const { return dirty.last_index(); }

		static AA_CONSTEVAL size_type max_bucket_count() { return M; }
		AA_CONSTEXPR size_type bucket_count() const { return dirty.size(); }



		// Observers
		AA_CONSTEXPR aa::unsafe_subrange<const_iterator> dirty_subrange() const {
			return {dirty};
		}



		// Modifiers
	public:
		AA_CONSTEXPR void clear() {
			if (!empty()) {
				unsafe_clear();
			}
		}

		AA_CONSTEXPR void unsafe_clear() {
			do {
				*dirty.back() = zero_v<bucket_type>;
				dirty.pop_back();
			} while (!dirty.empty());
		}

		AA_CONSTEXPR void clear_bucket(const size_type index) {
			bucket_type &bin = this->bins[index];
			if (bin) {
				bin = numeric_min;
				dirty.fast_erase(aa::unsafe_find_last(dirty, &bin));
			}
		}

		AA_CONSTEXPR void fill_bucket(const size_type index) {
			bucket_type &bin = this->bins[index];
			if (!bin) dirty.insert_back(&bin);
			bin = numeric_max;
		}

		template<hashable_by<const hasher_type &> K>
		AA_CONSTEXPR void insert(const K &key) {
			const size_type hash = this->hash(key);
			bucket_type &bin = this->bins[this->index(hash)];
			if (!bin) {
				dirty.insert_back(&bin);
				bin = this->mask(hash);
			} else bin |= this->mask(hash);
		}

		template<hashable_by<const hasher_type &> K>
		AA_CONSTEXPR void erase(const K &key) {
			const size_type hash = this->hash(key);
			bucket_type &bin = this->bins[this->index(hash)];
			// Reikia sąlygos, nes kitaip būtų bandoma pašalinti nenaudojamą bucket.
			if (bin) {
				if (!(bin &= ~this->mask(hash)))
					dirty.fast_erase(aa::unsafe_find_last(dirty, &bin));
			}
		}



		// Member objects
	protected:
		fixed_vector<bucket_type *, M> dirty;
	};

	template<class T, size_t N, class H>
	struct fixed_hashes_set<T, N, 0, H> : fixed_hashes_set_base<T, N, H> {
		using typename fixed_hashes_set_base<T, N, H>::base_type, base_type::base_type,
			typename base_type::bucket_type, typename base_type::bucket_pointer,
			typename base_type::size_type, typename base_type::difference_type,
			typename base_type::local_iterator, typename base_type::hasher_type;
		using iterator = bucket_pointer;
		using const_iterator = iterator;



		// Capacity
		AA_CONSTEXPR bool empty() const {
			return is_numeric_min(dirty.l);
		}
		AA_CONSTEXPR bool single_bucket_dirty() const {
			return dirty.f == dirty.l;
		}
		AA_CONSTEXPR bool single() const {
			return single_bucket_dirty() && bucket(this->bins[dirty.f]).size() == 1;
		}
		AA_CONSTEXPR bool all_buckets_dirty() const {
			return dirty.f == 0 && dirty.l == max_bucket_index();
		}
		AA_CONSTEXPR bool dirty_buckets_full() const {
			return empty() || unsafe_all_of(dirty_subrange(), [&](const bucket_type bin) ->
				bool { return this->bucket(bin).full(); });
		}
		AA_CONSTEXPR bool full() const { return all_buckets_dirty() && dirty_buckets_full(); }

		AA_CONSTEXPR size_type size() const {
			if (!empty()) {
				size_type sum = 0;
				unsafe_for_each(dirty_subrange(), [&](const bucket_type bin) ->
					void { sum += this->bucket(bin).size(); });
				return sum;
			} else return 0;
		}
		AA_CONSTEXPR difference_type ssize() const { return std::bit_cast<difference_type>(size()); }

		static AA_CONSTEVAL size_type max_size() {
			return local_iterator::max_size() * max_bucket_count();
		}



		// Bucket interface
		static AA_CONSTEVAL size_type max_bucket_index() { return N - 1; }
		static AA_CONSTEVAL size_type last_bucket_index() { return N - 1; }

		static AA_CONSTEVAL size_type max_bucket_count() { return N; }
		static AA_CONSTEVAL size_type bucket_count() { return N; }



		// Observers
		AA_CONSTEXPR size_type first_dirty_index() const { return dirty.f; }
		AA_CONSTEXPR size_type last_dirty_index() const { return dirty.l; }

		AA_CONSTEXPR aa::unsafe_subrange<const_iterator> dirty_subrange() const {
			return {this->bins.data() + dirty.f, this->bins.data() + dirty.l};
		}



		// Modifiers
	protected:
		AA_CONSTEXPR void mark_not_dirty() {
			dirty.f = numeric_max;
			dirty.l = numeric_min;
		}

		AA_CONSTEXPR void contract_dirty_region(const size_type index) {
			const bool f_not_dirty = (index == dirty.f), l_not_dirty = (index == dirty.l);
			if (f_not_dirty && l_not_dirty) mark_not_dirty();
			else if (f_not_dirty) {
				bucket_pointer f = this->bins.data() + (dirty.f + 1);
				do {
					if (!this->bucket(f).empty()) { dirty.f = this->index(f); return; }
				} while ((++f, true));
			} else if (l_not_dirty) {
				bucket_pointer l = this->bins.data() + (dirty.l - 1);
				do {
					if (!this->bucket(l).empty()) { dirty.l = this->index(l); return; }
				} while ((--l, true));
			}
		}

		AA_CONSTEXPR void expand_dirty_region(const size_type index) {
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
			std::ranges::fill(this->bins.data() + dirty.f,
				std::as_const(this->bins).data() + dirty.l + 1, zero_v<bucket_type>);
			mark_not_dirty();
		}

		AA_CONSTEXPR void clear_bucket(const size_type index) {
			this->bins[index] = numeric_min;
			contract_dirty_region(index);
		}

		AA_CONSTEXPR void fill_bucket(const size_type index) {
			expand_dirty_region(index);
			this->bins[index] = numeric_max;
		}

		template<hashable_by<const hasher_type &> K>
		AA_CONSTEXPR void insert(const K &key) {
			const size_type hash = this->hash(key), index = this->index(hash);
			expand_dirty_region(index);
			this->bins[index] |= this->mask(hash);
		}

		template<hashable_by<const hasher_type &> K>
		AA_CONSTEXPR void erase(const K &key) {
			const size_type hash = this->hash(key), index = this->index(hash);
			if (!(this->bins[index] &= ~this->mask(hash)))
				contract_dirty_region(index);
		}



		// Member objects
	protected:
		struct dirty_region_type {
			size_type f = numeric_max, l = numeric_min;
		} dirty;
	};

	template<class T, size_t N, class H>
	struct fixed_hashes_set<T, N, numeric_max, H> : fixed_hashes_set_base<T, N, H> {
		using typename fixed_hashes_set_base<T, N, H>::base_type, base_type::base_type,
			typename base_type::bucket_type, typename base_type::bucket_pointer,
			typename base_type::size_type, typename base_type::difference_type,
			typename base_type::local_iterator, typename base_type::hasher_type;
		using iterator = bucket_pointer;
		using const_iterator = iterator;



		// Capacity
		AA_CONSTEXPR bool empty() const {
			return unsafe_all_of(this->bins, [&](const bucket_type bin) ->
				bool { return this->bucket(bin).empty(); });
		}
		AA_CONSTEXPR bool all_buckets_dirty() const {
			return unsafe_all_of(this->bins, [&](const bucket_type bin) ->
				bool { return this->bucket(bin); });
		}
		AA_CONSTEXPR bool full() const {
			return unsafe_all_of(this->bins, [&](const bucket_type bin) ->
				bool { return this->bucket(bin).full(); });
		}

		AA_CONSTEXPR size_type size() const {
			size_type sum = 0;
			unsafe_for_each(this->bins, [&](const bucket_type bin) ->
				void { sum += this->bucket(bin).size(); });
			return sum;
		}
		AA_CONSTEXPR difference_type ssize() const { return std::bit_cast<difference_type>(size()); }

		static AA_CONSTEVAL size_type max_size() {
			return local_iterator::max_size() * max_bucket_count();
		}



		// Bucket interface
		static AA_CONSTEVAL size_type max_bucket_index() { return N - 1; }
		static AA_CONSTEVAL size_type last_bucket_index() { return N - 1; }

		static AA_CONSTEVAL size_type max_bucket_count() { return N; }
		static AA_CONSTEVAL size_type bucket_count() { return N; }



		// Modifiers
		AA_CONSTEXPR void clear() {
			this->bins.fill(zero_v<bucket_type>);
		}

		AA_CONSTEXPR void clear_bucket(const size_type index) {
			this->bins[index] = numeric_min;
		}

		AA_CONSTEXPR void fill_bucket(const size_type index) {
			this->bins[index] = numeric_max;
		}

		template<hashable_by<const hasher_type &> K>
		AA_CONSTEXPR void insert(const K &key) {
			const size_type hash = this->hash(key);
			this->bins[this->index(hash)] |= this->mask(hash);
		}

		template<hashable_by<const hasher_type &> K>
		AA_CONSTEXPR void erase(const K &key) {
			const size_type hash = this->hash(key);
			this->bins[this->index(hash)] &= ~this->mask(hash);
		}
	};

}
