#pragma once

#include "../metaprogramming/general.hpp"
#include "../preprocessor/assert.hpp"
#include "../algorithm/hash.hpp"
#include "lexer.hpp"
#include "evaluator.hpp"



namespace aa {

	// Į evaluate nepaduodame indekso, nes naudotojas galėtų tiesiog naudoti skirtingus tipus.
	template<string_perfect_hash H, class EVAL = evaluator, int_input_iterator T = istreambuf_iter<>, tuple_evaluable_by<EVAL &, std::tuple_size_v<const_t<H>>> TUPLE>
	constexpr void parse(TUPLE &t, T i, EVAL &&eval = default_value) {
		lex<H>(i, [&]<size_t I>(const int c) -> bool {
			return eval(c, get_v<I>(t));
		});
	}

	template<string_perfect_hash H, class EVAL = evaluator, int_input_iterator T = istreambuf_iter<>, tuple_evaluable_by<EVAL &, std::tuple_size_v<const_t<H>>> TUPLE>
		requires (std::tuple_size_v<TUPLE> <= numeric_digits_v<size_t>)
	constexpr void safe_parse(TUPLE &t, T i, EVAL &&eval = default_value) {
		const size_t index = unsign<size_t>(std::countr_one(make_with_invocable<default_value>([&](size_t &bitset) -> void {
			lex<H>(i, [&]<size_t I>(const int c) -> bool {
				if (eval(c, get_v<I>(t))) return true; else {
					bitset |= const_v<int_exp2(I)>;
					return false;
				}
			});
		})));
		AA_TRACE_ASSERT(index == std::tuple_size_v<TUPLE>, "Parameter (index: {}) not found.", index);
	}

	template<string_perfect_hash H, not_cvref TUPLE, evaluator_for_tuple<TUPLE, std::tuple_size_v<const_t<H>>> EVAL = evaluator, int_input_iterator T = istreambuf_iter<>>
	constexpr TUPLE parse(T i, EVAL &&eval = default_value) {
		return make_with_invocable([&](TUPLE &t) -> void {
			parse<H>(t, i, eval);
		});
	}

	template<string_perfect_hash H, not_cvref TUPLE, evaluator_for_tuple<TUPLE, std::tuple_size_v<const_t<H>>> EVAL = evaluator, int_input_iterator T = istreambuf_iter<>>
	constexpr TUPLE safe_parse(T i, EVAL &&eval = default_value) {
		return make_with_invocable([&](TUPLE &t) -> void {
			safe_parse<H>(t, i, eval);
		});
	}

}
