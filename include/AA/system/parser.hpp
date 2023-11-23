#pragma once

#include "../metaprogramming/general.hpp"
#include "../preprocessor/assert.hpp"
#include "../algorithm/hash.hpp"
#include "../algorithm/init.hpp"
#include "lexer.hpp"
#include "evaluator.hpp"
#include <streambuf> // streambuf



namespace aa {

	// Į evaluate nepaduodame indekso, nes naudotojas galėtų tiesiog naudoti skirtingus tipus.
	template<string_perfect_hash H, class EVAL = evaluator, same_tuple_size_as<const_t<H>> TUPLE>
	constexpr void parse(TUPLE &t, std::streambuf &file, EVAL &&eval) {
		lex<H>(file, [&]<size_t I>(const int c) -> bool {
			return eval(c, getter_v<I>(t));
		});
	}

	template<string_perfect_hash H, class EVAL = evaluator, same_tuple_size_as<const_t<H>> TUPLE>
		requires (std::tuple_size_v<TUPLE> <= numeric_digits_v<size_t>)
	constexpr void safe_parse(TUPLE &t, std::streambuf &file, EVAL &&eval) {
		const size_t index = unsign<size_t>(std::countr_one(make_with_invocable<0uz>([&](size_t &bitset) -> void {
			lex<H>(file, [&]<size_t I>(const int c) -> bool {
				if (eval(c, getter_v<I>(t))) return true; else {
					bitset |= const_v<int_exp2(I)>;
					return false;
				}
			});
		})));
		AA_TRACE_ASSERT(index == std::tuple_size_v<TUPLE>, "Parameter (index: {}) not found.", index);
	}

	template<string_perfect_hash H, same_tuple_size_as<const_t<H>> TUPLE, class EVAL = evaluator>
	constexpr std::remove_cvref_t<TUPLE> parse(std::streambuf &file, EVAL &&eval) {
		return make_with_invocable([&](std::remove_cvref_t<TUPLE> &t) -> void {
			parse<H>(t, file, eval);
		});
	}

	template<string_perfect_hash H, same_tuple_size_as<const_t<H>> TUPLE, class EVAL = evaluator>
	constexpr std::remove_cvref_t<TUPLE> safe_parse(std::streambuf &file, EVAL &&eval) {
		return make_with_invocable([&](std::remove_cvref_t<TUPLE> &t) -> void {
			safe_parse<H>(t, file, eval);
		});
	}

}
