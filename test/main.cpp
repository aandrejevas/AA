#include <AA/container/fixed_string.hpp>
#include <AA/container/fixed_grid.hpp>
#include <AA/container/fixed_free_vector.hpp>
#include <AA/container/fixed_perfect_hash_set.hpp>
#include <AA/container/fixed_flat_set.hpp>
#include <AA/container/fixed_vector.hpp>
#include <AA/algorithm/arithmetic.hpp>
#include <AA/algorithm/hash.hpp>
#include <AA/algorithm/find.hpp>
#include <AA/algorithm/shift.hpp>
#include <AA/algorithm/permute.hpp>
#include <AA/algorithm/generate.hpp>
#include <AA/algorithm/linear_congruential_generator.hpp>
#include <AA/preprocessor/assert.hpp>
#include <AA/metaprogramming/general.hpp>
#include <AA/system/timekeeper.hpp>
#include <AA/system/parser.hpp>
#include <AA/system/writer.hpp>
#include <AA/system/print.hpp>
#include <AA/system/pathed_stream.hpp>
#include <cstddef> // size_t
#include <cstdlib> // EXIT_SUCCESS
#include <ios> // sync_with_stdio
#include <map> // map
#include <unordered_set> // unordered_set
#include <iterator> // forward_iterator, default_sentinel
#include <ranges> // reverse, iota, contiguous_range, random_access_range, bidirectional_range, subrange
#include <algorithm> // is_sorted, is_permutation, for_each
#include <string> // string
#include <limits> // numeric_limits
#include <iostream> // cout
#include <bit> // bit_cast



using namespace aa;



int main() {
	// Galima būtų naudoti funkcijų atributus (pvz. const, pure) ir restricted kvalifikatorių, bet visą tai yra compiler extensions.
	// Sakykime nenaudojimas minėtų galimybių skatina ieškoti kitų būdų kaip gauti tokį patį performance.

	{
		static_assert(std::numeric_limits<size_t>::digits == 64 && sizeof(size_t) == 8);
		static_assert(int_log2<2>(1uz) == 0 && int_log2<2>(4uz) == 1 && int_log2<2>(16uz) == 2);
		static_assert(int_log2<3>(1uz) == 0 && int_log2<3>(8uz) == 1 && int_log2<3>(64uz) == 2);
		static_assert(int_log2<4>(1uz) == 0 && int_log2<4>(16uz) == 1 && int_log2<4>(256uz) == 2);
	}

	std::ios_base::sync_with_stdio(false);

	timekeeper tttt;
	tttt.start();
	linear_congruential_generator g;
	{
		log<AA_SOURCE_DATA>();
		log<AA_SOURCE_DATA>("Hello world!");
		log<AA_SOURCE_DATA>(std::cout);
		log<AA_SOURCE_DATA>(std::cout, "Hello world!");
		//print();
		printl();
		//print(std::cout);
		printl(std::cout);
		print("Hello world!\n");
		printl("Hello world!");
		print(std::cout, "Hello world!\n");
		printl(std::cout, "Hello world!");
		printl(g.curr());
	}
	{
		const linear_congruential_generator initial_g = g;
		std::map<int, size_t> m;

		unsafe_for_each(std::views::iota(1uz, 5001uz), [&](const size_t i) -> void {
			++m[int_distribution<int>(g, static_cast<size_t>(-5), 10)];
			linear_congruential_generator copy_g = initial_g;
			copy_g.jump(i);
			const size_t d = initial_g.dist(copy_g);
			AA_TRACE_ASSERT(copy_g.curr() == g.curr() && i == d, i, ' ', d);
		});
		printl(range_writer{m, tuple_inserter{}});

		repeat(5000, [&]() {
			g.prev();
		});
		AA_TRACE_ASSERT(g.curr() == initial_g.curr());

		//g.seed();
		//while (true) {
		//	const double a = real_distribution(g);
		//	AA_TRACE_ASSERT(0 <= a && a < 1, a);
		//}
	}
	{
		const auto a = parse<tuple<std::string, double, prev_unsigned_t<size_t>>,
			constant<m_string_perfect_hash<"TEST_1"_fs, "TEST_2"_fs, "TEST_3"_fs>>()>(pathed_ifstream{"params.txt"}.get());

		AA_TRACE_ASSERT(a.get<0>() == "text");
		AA_TRACE_ASSERT(std::bit_cast<size_t>(a.get<1>()) == std::bit_cast<size_t>(22.5));
		AA_TRACE_ASSERT(a.get<2>() == 45);
	}
	{
		array_t<int, 10> a = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}, b = a;
		array_t<bool, 10> s = {};

		repeat(100, [&]() {
			shuffle(a, g);
			AA_TRACE_ASSERT(std::ranges::is_permutation(a, b));
			counting_sort(a, s.data());
			AA_TRACE_ASSERT(std::ranges::is_sorted(a));
		});

		repeat(100, [&]() {
			partial_shuffle_counting_sort(a, a.data() + 4, g, s.data());
			AA_TRACE_ASSERT(std::ranges::is_sorted(a.data(), a.data() + 5));
		});

		counting_sort(a, s.data());
		cshift_right(a, b);
		unsafe_for_each(std::views::iota(0uz, 10uz), [&](const size_t i) -> void {
			AA_TRACE_ASSERT(a[i] == b[(i + 1) % 10]);
		});

		AA_TRACE_ASSERT(*unsafe_lower_bound(a, 5) == 5 && *unsafe_upper_bound(a, 5) == 6);
	}
	{
		fixed_flat_multiset<size_t, 500> a;

		repeat(100, [&]() {
			a.insert(g());
			AA_TRACE_ASSERT(std::ranges::is_sorted(a));
		});

		do {
			a.erase(a[int_distribution(g, a.size())]);
			AA_TRACE_ASSERT(std::ranges::is_sorted(a));
		} while (!a.empty());

		static_assert(std::ranges::contiguous_range<decltype(a)>);
	}
	{
		std::unordered_set<size_t> b;
		fixed_perfect_hash_set<size_t, 1'000> a;
		printl(a.bucket_count(), ' ', a.bucket_max_size(), ' ', a.max_size());

		// Insert test
		repeat(100'000, [&]() {
			const size_t c = int_distribution(g, a.max_size());
			b.insert(c);
			a.insert(c);
		});
		AA_TRACE_ASSERT(b.size() == a.size());

		// Iterator test
		unsafe_for_each(a.dirty_subrange(), [&](const size_t &d) {
			std::ranges::for_each(std::views::reverse(a.bucket(&d)), [&](const size_t c) {
				AA_TRACE_ASSERT(b.erase(c));
			});
		});
		AA_TRACE_ASSERT(b.empty());

		// Clear test
		a.unsafe_clear();
		unsafe_for_each(a.buckets(), [](const size_t i) -> void { AA_TRACE_ASSERT(!i); });

		// Bucket test
		a.fill_bucket(0uz);
		unsafe_for_each(std::views::iota(0uz, a.bucket_max_size()), [&](const size_t i) { b.insert(i); });
		AA_TRACE_ASSERT(b.size() == a.bucket_size(a.buckets()[0]) && a.dirty_buckets_full());

		// Local iterator test
		std::ranges::for_each(std::ranges::subrange{a.begin(0uz), std::default_sentinel}, [&](const size_t c) {
			AA_TRACE_ASSERT(b.erase(c));
		});
		AA_TRACE_ASSERT(b.empty());

		// Erase test
		repeat(100'000, [&]() {
			a.insert(int_distribution(g, a.max_size()));
		});
		bool use_f = true; do {
			a.erase(a.bucket(use_f ? a.first_dirty_index() : a.last_dirty_index()).front());
			if (!a.empty()) use_f = !use_f; else break;
		} while (true);
		unsafe_for_each(a.buckets(), [](const size_t i) -> void { AA_TRACE_ASSERT(!i); });

		static_assert(std::ranges::bidirectional_range<decltype(a)::bucket_iterable>);
		static_assert(std::forward_iterator<decltype(a)::local_iterator>);
	}
	{
		fixed_free_vector<size_t, 50'000> a;

		unsafe_for_each(std::views::iota(0uz, a.max_size()), [&](const size_t i) {
			AA_TRACE_ASSERT(!a[i]); a.insert(i); AA_TRACE_ASSERT(a[i]); AA_TRACE_ASSERT(*a[i] == i);
		});
		AA_TRACE_ASSERT(a.full() && !a.has_holes());

		unsafe_for_each(a, [&](size_t *const ptr) { a.erase(ptr); });
		AA_TRACE_ASSERT(a.full() && a.empty());

		unsafe_for_each(std::views::iota(0uz, a.max_size()) | std::views::reverse, [&](const size_t i) {
			AA_TRACE_ASSERT(!a[i]); a.insert(i); AA_TRACE_ASSERT(a[i]); AA_TRACE_ASSERT(*a[i] == i);
		});
		AA_TRACE_ASSERT(a.full() && !a.has_holes());

		static_assert(std::ranges::random_access_range<decltype(a)>);
	}
	{
		using grid_type = fixed_erasable_grid<array_t<double, 2>, 100, 100, 500>;
		grid_type tree = {{1, 1}};
		fixed_vector<grid_type::value_type, tree.max_size()> positions;

		{
			repeat(tree.max_size(), [&]() {
				positions.emplace_back(grid_type::value_type{real_distribution<double>(g, 25.),
					norm_map<0>(real_distribution<double>(g, 50., 10.), 50., 10., 25.)});
				tree.insert(positions.back());
			});
			size_t sum = 0;
			tree.query_range({-25, -25}, {25, 25}, [&](const grid_type::value_type &) { ++sum; });
			AA_TRACE_ASSERT(tree.max_size() == sum);
		}
		{
			repeat(tree.max_size() >> 1, [&]() {
				tree.erase(positions.back());
				positions.pop_back();
			});
			size_t sum = 0;
			tree.query_range({-25, -25}, {25, 25}, [&](const grid_type::value_type &) { ++sum; });
			AA_TRACE_ASSERT((tree.max_size() >> 1) == sum);
		}
	}
	tttt.stop();
	print(tttt.elapsed(), endl);

	return EXIT_SUCCESS;
}
