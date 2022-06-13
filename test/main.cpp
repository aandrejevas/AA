/*
cd coverage
gcov -k ../main.cpp
del *.gcov

ištrinti:
-nolibc -DNDEBUG
pridėti:
-UNDEBUG -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -D_FORTIFY_SOURCE=3 -fanalyzer -Wanalyzer-too-complex -fanalyzer-call-summaries -fanalyzer-verbosity=0 --coverage -fstack-protector
neveikia (https://github.com/msys2/MINGW-packages/issues/3163):
-fsanitize=address -fsanitize=undefined

regexes:
size_t[^y]
*/

#include <AA/container/static_quad_tree.hpp>
#include <AA/container/static_free_vector.hpp>
#include <AA/container/static_perfect_hash_set.hpp>
#include <AA/container/static_flat_set.hpp>
#include <AA/algorithm/find.hpp>
#include <AA/algorithm/shift.hpp>
#include <AA/algorithm/permute.hpp>
#include <AA/algorithm/generate.hpp>
#include <AA/algorithm/lcg.hpp>
#include <AA/algorithm/repeat.hpp>
#include <AA/preprocessor/assert.hpp>
#include <AA/metaprogramming/general.hpp>
#include <AA/time/timekeeper.hpp>
#include <AA/io/evaluator.hpp>
#include <AA/io/lexer.hpp>
#include <AA/io/writer.hpp>
#include <AA/io/print.hpp>
#include <cstddef> // size_t
#include <cstdint> // uint32_t
#include <cstdlib> // EXIT_SUCCESS
#include <ios> // sync_with_stdio
#include <map> // map
#include <unordered_set> // unordered_set
#include <ranges> // iota, contiguous_range, random_access_range, forward_range
#include <algorithm> // is_sorted, is_permutation
#include <string> // string
#include <limits> // numeric_limits
#include <ostream> // flush, ostream



using namespace aa;



int main() {
	static_assert(std::numeric_limits<size_t>::digits == 64 && sizeof(size_t) == 8);
	std::ios_base::sync_with_stdio(false);

	pascal_lcg g;
	println(g.curr(), std::flush<std::ostream::char_type, std::ostream::traits_type>);
	timekeeper tttt;
	tttt.start();
	{
		const pascal_lcg initial_g = g;
		std::map<int, size_t> m;

		unsafe_for_each(std::views::iota(1uz, 5001uz), [&](const size_t i) -> void {
			++m[int_distribution<int>(g, static_cast<size_t>(-5), 10)];
			pascal_lcg copy_g = initial_g;
			copy_g.jump(i);
			const size_t d = initial_g.dist(copy_g);
			AA_TRACE_ASSERT(copy_g.curr() == g.curr() && i == d, i, ' ', d);
		});
		println(make_range_writer(m, pair_inserter{}));

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
		println(type_name<std::string>());
		println(type_name<uint32_t>());
		println(type_name<double>());
		const lexer<evaluator<std::string>, evaluator<uint32_t>, evaluator<double>> l = {"params.txt"};

		AA_TRACE_ASSERT(l.get_param<std::string>("TEST_1") == "text");
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
		AA_TRACE_ASSERT(l.get_param<double>("TEST_2") == 22.5);
#pragma GCC diagnostic pop
		AA_TRACE_ASSERT(l.get_param<uint32_t>("TEST_3") == 45);
		AA_TRACE_ASSERT(l.get_params().size() == 3);
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
			partial_shuffle_counting_sort(a.data(), a.data() + 10, a.data() + 4, g, s.data());
			AA_TRACE_ASSERT(std::ranges::is_sorted(a.data(), a.data() + 5));
			//println(make_range_writer(a));
		});

		counting_sort(a, s.data());
		cshift_right(a, b);
		unsafe_for_each(std::views::iota(0uz, 10uz), [&](const size_t i) -> void {
			AA_TRACE_ASSERT(a[i] == b[(i + 1) % 10]);
		});

		AA_TRACE_ASSERT(*lower_bound(a, 5) == 5 && *upper_bound(a, 5) == 6);
	}
	{
		static_flat_multiset<size_t, 500> a;

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
		static_perfect_hash_set<size_t, 1'000> a;
		println(a.max_bucket_count(), ' ', a.max_size());

		// Insert test
		repeat(100'000, [&]() {
			const size_t c = int_distribution(g, a.max_size());
			b.insert(c);
			a.insert(c);
		});
		AA_TRACE_ASSERT(b.size() == a.size());

		// Iterator test
		unsafe_for_each(a, [&](const size_t c) {
			AA_TRACE_ASSERT(b.contains(c));
			b.erase(c);
		});
		AA_TRACE_ASSERT(b.empty());

		// Clear test
		a.unsafe_clear();
		unsafe_for_each(a.buckets(), [&](const size_t i) -> void { AA_TRACE_ASSERT(!i); });

		// Bucket test
		repeat(100, [&]() {
			const size_t c = int_distribution(g, 64) * a.max_bucket_count();
			b.insert(c);
			a.insert(c);
		});
		AA_TRACE_ASSERT(b.size() == a.bucket_size(0));

		// Local iterator test
		unsafe_for_each(a.begin(0), a.end(0), [&](const size_t c) {
			AA_TRACE_ASSERT(b.contains(c));
			b.erase(c);
		});
		AA_TRACE_ASSERT(b.empty());

		// Erase test
		repeat(100'000, [&]() {
			a.insert(int_distribution(g, a.max_size()));
		});
		do {
			a.erase(*a.begin(a.index_at(int_distribution(g, a.bucket_count()))));
		} while (!a.empty());
		unsafe_for_each(a.buckets(), [&](const size_t i) -> void { AA_TRACE_ASSERT(!i); });

		static_assert(std::ranges::forward_range<decltype(a)>);
	}
	{
		static_free_vector<size_t, 50'000> a;

		repeat(a.max_size(), [&]() { a.emplace(a.size()); });
		unsafe_for_each(std::views::iota(0uz, a.size() >> 1), [&](const size_t i) { a.erase(a[i]); AA_TRACE_ASSERT(!a[i]); });
		unsafe_for_each(std::views::iota(a.size() >> 1, a.size()), [&](const size_t i) { AA_TRACE_ASSERT(*a[i] == i); });

		AA_TRACE_ASSERT(a.max_size() == a.size());
		repeat(a.size() >> 1, [&]() { a.emplace(a.size()); });
		AA_TRACE_ASSERT(a.max_size() == a.size());

		unsafe_for_each(std::views::iota(0uz, a.size() >> 1), [&](const size_t i) { AA_TRACE_ASSERT(*a[i] == a.size()); });
		unsafe_for_each(std::views::iota(a.size() >> 1, a.size()), [&](const size_t i) { AA_TRACE_ASSERT(*a[i] == i); });

		static_assert(std::ranges::random_access_range<decltype(a)>);
	}
	{
		using quad_tree_type = static_quad_tree<array_t<float, 2>, std::identity, 5, 500>;
		quad_tree_type tree = {{0, 0}, {100, 100}};
		println(make_range_writer(tree.sizes, pair_inserter{}));
		static_vector<quad_tree_type::value_type, tree.max_size()> positions;

		{
			repeat(tree.max_size(), [&]() {
				positions.emplace_back(quad_tree_type::value_type{real_distribution<float>(g, 25.), real_distribution<float>(g, 25.)});
				tree.insert(positions.back());
			});
			size_t sum = 0;
			tree.query_range({0, 0}, {25, 25}, [&](const quad_tree_type::value_type &) { ++sum; });
			AA_TRACE_ASSERT(tree.max_size() == sum);
		}
		{
			repeat(tree.max_size() >> 1, [&]() {
				tree.erase(positions.back());
				positions.pop_back();
			});
			size_t sum = 0;
			tree.query_range({0, 0}, {25, 25}, [&](const quad_tree_type::value_type &) { ++sum; });
			AA_TRACE_ASSERT((tree.max_size() >> 1) == sum);
		}
	}
	tttt.stop();
	println(tttt.elapsed());

	return EXIT_SUCCESS;
}
