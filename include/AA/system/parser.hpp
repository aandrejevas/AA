#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/io.hpp"
#include "../preprocessor/assert.hpp"
#include "../algorithm/arithmetic.hpp"
#include "../algorithm/hash.hpp"
#include "../algorithm/init.hpp"
#include "lexer.hpp"
#include "evaluator.hpp"
#include <cstddef> // size_t
#include <string> // string
#include <string_view> // string_view
#include <functional> // invoke
#include <type_traits> // remove_const_t
#include <utility> // tuple_size_v
#include <limits> // numeric_limits
#include <bit> // countr_one



namespace aa {

	template<string_perfect_hash H, size_t R = 50, class EVAL = generic_evaluator<>, input_stream FILE, instantiation_of<tuple> TUPLE>
	AA_CONSTEXPR void parse(TUPLE &t, FILE &&file, EVAL &&eval = {}) {
		lex<H, R>(file, [&](const size_t index, const std::string &token) -> void {
			{
				t.get(index, [&]<size_t I, class VALUE>(VALUE & value) -> void {
					invoke<I>(eval, value, token.operator std::string_view());
				});
			}});
	}

	template<string_perfect_hash H, size_t R = 50, class EVAL = generic_evaluator<>, input_stream FILE, instantiation_of<tuple> TUPLE>
		requires (H.max() <= std::numeric_limits<size_t>::digits)
	AA_CONSTEXPR void safe_parse(TUPLE &t, FILE &&file, EVAL &&eval = {}) {
		size_t bitset = 0;
		lex<H, R>(file, [&](const size_t index, const std::string &token) -> void {
			{
				t.get(index, [&]<size_t I, class VALUE>(VALUE & value) -> void {
					bitset |= constant<int_exp2(I)>();
					invoke<I>(eval, value, token.operator std::string_view());
				});
			}});
		AA_TRACE_ASSERT(std::countr_one(bitset) == H.max());
	}

	template<instantiation_of<tuple> TUPLE, string_perfect_hash H, size_t R = 50, class EVAL = generic_evaluator<>, input_stream FILE>
	AA_CONSTEXPR std::remove_const_t<TUPLE> parse(FILE &&file, EVAL &&eval = {}) {
		return create_with_invocable<TUPLE>([&](std::remove_const_t<TUPLE> &t) -> void {
			parse<H, R>(t, file, eval);
		});
	}

	template<instantiation_of<tuple> TUPLE, string_perfect_hash H, size_t R = 50, class EVAL = generic_evaluator<>, input_stream FILE>
	AA_CONSTEXPR std::remove_const_t<TUPLE> safe_parse(FILE &&file, EVAL &&eval = {}) {
		return create_with_invocable<TUPLE>([&](std::remove_const_t<TUPLE> &t) -> void {
			safe_parse<H, R>(t, file, eval);
		});
	}

}
