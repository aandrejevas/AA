#pragma once

#include "../metaprogramming/general.hpp"
#include "../algorithm/arithmetic.hpp"
#include "../algorithm/hash.hpp"
#include "../algorithm/find.hpp"
#include "fixed_vector.hpp"
#include "unsafe_subrange.hpp"
#include "bitset_view.hpp"



namespace aa {

	// Negali namespace būti unnamed, nes metama klaida tokiu atveju -Wsubobject-linkage.
	namespace detail {
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
			static consteval size_type max_bucket_index() { return N - 1; }
			static consteval size_type last_bucket_index() { return N - 1; }

			static consteval size_type max_bucket_count() { return N; }
			static consteval size_type bucket_count() { return N; }

			constexpr size_type index(const bucket_pointer bin) const {
				return unsign(bin - bins.data());
			}

			// Neturime end atitikmenų, nes tiesiog reikia naudoti default_sentinel.
			constexpr const_local_iterator begin(const bucket_pointer bin) const {
				return {*bin, product<local_iterator::max_size()>(index(bin))};
			}

			constexpr const_local_iterator begin(const size_type index) const {
				return {bins[index], product<local_iterator::max_size()>(index)};
			}

			static constexpr pure_local_iterator bucket(const bucket_pointer bin) {
				return bucket(*bin);
			}

			static constexpr pure_local_iterator bucket(const bucket_type bin) {
				return {bin};
			}

			static constexpr size_type index(const size_type hash) {
				return quotient<local_iterator::max_size()>(hash);
			}

			static constexpr bucket_type mask(const size_type hash) {
				return int_exp2<bucket_type>(remainder<local_iterator::max_size()>(hash));
			}



			// Observers
			template<hashable_by<const hasher_type &> K>
			constexpr size_type hash(const K &key) const {
				return std::invoke(hasher, key);
			}

			// Kituose konteineriuose vidinio konteinerio nerodome, nes kituose konteineriuose vidinis
			// konteineris galima sakyti yra pats konteineris tai nėra tikslo to daryti.
			constexpr const container_type &buckets() const { return bins; }



			// Lookup
			template<hashable_by<const hasher_type &> K>
			constexpr bool contains(const K &key) const {
				const size_type hash = this->hash(key);
				return bins[index(hash)] & mask(hash);
			}



			// Special member functions
			template<constructible_to<hasher_type> U = hasher_type>
			constexpr fixed_hashes_set_base(U &&h = default_value) : hasher{std::forward<U>(h)} {}



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
	template<regular_unsigned_integral T, size_t N, size_t M, class H = generic_hash<>>
		requires (N >= M || is_numeric_max(M))
	struct fixed_hashes_set : detail::fixed_hashes_set_base<T, N, H> {
		// Member types
		using typename detail::fixed_hashes_set_base<T, N, H>::base_type, base_type::base_type,
			typename base_type::bucket_type, typename base_type::bucket_pointer,
			typename base_type::size_type, typename base_type::difference_type,
			typename base_type::local_iterator, typename base_type::hasher_type;
		using iterator = const bucket_pointer *;
		using const_iterator = iterator;



		// Capacity
		constexpr bool empty() const {
			return dirty.empty();
		}
		constexpr bool single_bucket_dirty() const {
			return dirty.single();
		}
		constexpr bool single() const {
			return single_bucket_dirty() && bucket(dirty.front()).size() == 1;
		}
		constexpr bool all_buckets_dirty() const {
			return dirty.full();
		}
		constexpr bool dirty_buckets_full() const {
			return empty() || unsafe_all_of(dirty_subrange(), [&](const bucket_pointer bin) ->
				bool { return this->bucket(bin).full(); });
		}
		constexpr bool full() const { return all_buckets_dirty() && dirty_buckets_full(); }

		constexpr size_type size() const {
			if (!empty()) {
				size_type sum = 0;
				unsafe_for_each(dirty_subrange(), [&](const bucket_pointer bin) ->
					void { sum += this->bucket(bin).size(); });
				return sum;
			} else return 0;
		}
		constexpr difference_type ssize() const { return sign(size()); }

		// Daugybos nepaverčiame į postūmio operaciją, nes kompiliavimo metu ilgiau užtruktų nustatyti ar galime daryti
		// postūmį negu tiesiog įvykdyti daugybos operaciją, programos veikimo laikui irgi nepadėtų tokios kostrukcijos.
		//
		// Šis konteineris gali talpinti tik pavyzdžiui mažesnius sveikuosius skaičius už max_size
		// nebent būtų naudojamas ne tas hasher, kuris yra naudojamas pagal nutylėjimą.
		static consteval size_type max_size() {
			return local_iterator::max_size() * max_dirty_bucket_count();
		}



		// Bucket interface
		static consteval size_type max_dirty_bucket_count() { return M; }
		constexpr size_type dirty_bucket_count() const { return dirty.size(); }



		// Observers
		constexpr aa::unsafe_subrange<const_iterator> dirty_subrange() const {
			return {dirty};
		}



		// Modifiers
		constexpr void clear() {
			if (!empty()) {
				unsafe_clear();
			}
		}

		constexpr void unsafe_clear() {
			do {
				*dirty.back() = default_value;
				dirty.pop_back();
			} while (!dirty.empty());
		}

		constexpr void clear_bucket(const size_type index) {
			bucket_type &bin = this->bins[index];
			if (bin) {
				bin = numeric_min;
				dirty.fast_erase(aa::unsafe_find_last(dirty, &bin));
			}
		}

		constexpr void fill_bucket(const size_type index) {
			bucket_type &bin = this->bins[index];
			if (!bin) dirty.insert_back(&bin);
			bin = numeric_max;
		}

		template<hashable_by<const hasher_type &> K>
		constexpr void insert(const K &key) {
			const size_type hash = this->hash(key);
			bucket_type &bin = this->bins[this->index(hash)];
			if (!bin) {
				dirty.insert_back(&bin);
				bin = this->mask(hash);
			} else bin |= this->mask(hash);
		}

		template<hashable_by<const hasher_type &> K>
		constexpr void erase(const K &key) {
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



	template<class T, size_t N, class H = generic_hash<>>
	using fixed_small_hashes_set = fixed_hashes_set<T, N, 0, H>;

	template<class T, size_t N, class H>
	struct fixed_hashes_set<T, N, 0, H> : detail::fixed_hashes_set_base<T, N, H> {
		// Member types
		using typename detail::fixed_hashes_set_base<T, N, H>::base_type, base_type::base_type,
			typename base_type::bucket_type, typename base_type::bucket_pointer,
			typename base_type::size_type, typename base_type::difference_type,
			typename base_type::local_iterator, typename base_type::hasher_type;
		using iterator = bucket_pointer;
		using const_iterator = iterator;



		// Capacity
		constexpr bool empty() const {
			return dirty.empty();
		}
		constexpr bool single_bucket_dirty() const {
			return dirty.degenerate();
		}
		constexpr bool single() const {
			return single_bucket_dirty() && bucket(dirty.min()).size() == 1;
		}
		constexpr bool all_buckets_dirty() const {
			return dirty.eq(this->bins.data(), get_rbegin(this->bins));
		}
		constexpr bool dirty_buckets_full() const {
			return empty() || unsafe_all_of(dirty_subrange(), [&](const bucket_type bin) ->
				bool { return this->bucket(bin).full(); });
		}
		constexpr bool full() const { return all_buckets_dirty() && dirty_buckets_full(); }

		constexpr size_type size() const {
			if (!empty()) {
				size_type sum = 0;
				unsafe_for_each(dirty_subrange(), [&](const bucket_type bin) ->
					void { sum += this->bucket(bin).size(); });
				return sum;
			} else return 0;
		}
		constexpr difference_type ssize() const { return sign(size()); }

		static consteval size_type max_size() {
			return local_iterator::max_size() * base_type::max_bucket_count();
		}



		// Observers
		constexpr aa::unsafe_subrange<const_iterator> dirty_subrange() const {
			return {dirty.min(), dirty.max()};
		}



		// Modifiers
	protected:
		constexpr void shrink_dirty_region(const bucket_pointer bin) {
			const bool f_not_dirty = dirty.min_eq(bin), l_not_dirty = dirty.max_eq(bin);
			if (f_not_dirty && l_not_dirty) {
				dirty.reset(this->bins.data(), get_rbegin(this->bins));

			} else if (f_not_dirty) {
				while (this->bucket(++dirty.min()).empty());

			} else if (l_not_dirty) {
				while (this->bucket(--dirty.max()).empty());
			}
		}

	public:
		constexpr void clear() {
			if (!empty()) {
				unsafe_clear();
			}
		}

		constexpr void unsafe_clear() {
			std::ranges::fill(dirty.min(), dirty.max() + 1, default_v<bucket_type>);
			dirty.reset(this->bins.data(), get_rbegin(this->bins));
		}

		constexpr void clear_bucket(const size_type index) {
			bucket_type &bin = this->bins[index];
			bin = numeric_min;
			shrink_dirty_region(&bin);
		}

		constexpr void fill_bucket(const size_type index) {
			bucket_type &bin = this->bins[index];
			dirty.expand(&bin);
			bin = numeric_max;
		}

		template<hashable_by<const hasher_type &> K>
		constexpr void insert(const K &key) {
			const size_type hash = this->hash(key);
			bucket_type &bin = this->bins[this->index(hash)];
			dirty.expand(&bin);
			bin |= this->mask(hash);
		}

		template<hashable_by<const hasher_type &> K>
		constexpr void erase(const K &key) {
			const size_type hash = this->hash(key);
			bucket_type &bin = this->bins[this->index(hash)];
			if (!(bin &= ~this->mask(hash)))
				shrink_dirty_region(&bin);
		}



		// Member objects
	protected:
		interval<bucket_type *> dirty = {this->bins.data(), get_rbegin(this->bins)};
	};



	template<class T, size_t N, class H = generic_hash<>>
	using fixed_fast_hashes_set = fixed_hashes_set<T, N, numeric_max, H>;

	template<class T, size_t N, class H>
	struct fixed_hashes_set<T, N, numeric_max, H> : detail::fixed_hashes_set_base<T, N, H> {
		// Member types
		using typename detail::fixed_hashes_set_base<T, N, H>::base_type, base_type::base_type,
			typename base_type::bucket_type, typename base_type::bucket_pointer,
			typename base_type::size_type, typename base_type::difference_type,
			typename base_type::local_iterator, typename base_type::hasher_type;
		using iterator = bucket_pointer;
		using const_iterator = iterator;



		// Capacity
		constexpr bool empty() const {
			return unsafe_all_of(this->bins, [&](const bucket_type bin) ->
				bool { return this->bucket(bin).empty(); });
		}
		constexpr bool all_buckets_dirty() const {
			return unsafe_all_of(this->bins, [&](const bucket_type bin) ->
				bool { return this->bucket(bin); });
		}
		constexpr bool full() const {
			return unsafe_all_of(this->bins, [&](const bucket_type bin) ->
				bool { return this->bucket(bin).full(); });
		}

		constexpr size_type size() const {
			size_type sum = 0;
			unsafe_for_each(this->bins, [&](const bucket_type bin) ->
				void { sum += this->bucket(bin).size(); });
			return sum;
		}
		constexpr difference_type ssize() const { return sign(size()); }

		static consteval size_type max_size() {
			return local_iterator::max_size() * base_type::max_bucket_count();
		}



		// Modifiers
		constexpr void clear() {
			std::ranges::fill(this->bins, default_v<bucket_type>);
		}

		constexpr void clear_bucket(const size_type index) {
			this->bins[index] = numeric_min;
		}

		constexpr void fill_bucket(const size_type index) {
			this->bins[index] = numeric_max;
		}

		template<hashable_by<const hasher_type &> K>
		constexpr void insert(const K &key) {
			const size_type hash = this->hash(key);
			this->bins[this->index(hash)] |= this->mask(hash);
		}

		template<hashable_by<const hasher_type &> K>
		constexpr void erase(const K &key) {
			const size_type hash = this->hash(key);
			this->bins[this->index(hash)] &= ~this->mask(hash);
		}
	};

}
