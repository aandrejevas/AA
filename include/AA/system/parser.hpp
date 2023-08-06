#pragma once

#include "../metaprogramming/general.hpp"
#include "../preprocessor/assert.hpp"
#include "../algorithm/hash.hpp"
#include "../algorithm/init.hpp"
#include "lexer.hpp"
#include "evaluator.hpp"
#include "source.hpp"
#include <string> // string



namespace aa {

	template<instance_of<string_perfect_hash<>> auto H, class EVAL = generic_evaluator<>, class CONFIG, class FILE, same_tuple_size_as<const_t<H>> TUPLE>
	AA_CONSTEXPR void parse(TUPLE &t, FILE &&file, CONFIG &&config, EVAL &&eval = default_value) {
		lex<H>(file, config, [&]<size_t I>(const std::string &token) -> void {
			invoke<I>(eval, default_value_v<getter<I>>(t), token);
		});
	}

	template<instance_of<string_perfect_hash<>> auto H, class EVAL = generic_evaluator<>, class CONFIG, class FILE, same_tuple_size_as<const_t<H>> TUPLE>
		requires (std::tuple_size_v<TUPLE> <= numeric_digits_v<size_t>)
	AA_CONSTEXPR void safe_parse(TUPLE &t, FILE &&file, CONFIG &&config, EVAL &&eval = default_value) {
		const size_t index = unsign<size_t>(std::countr_one(make_with_invocable<0uz>([&](size_t &bitset) -> void {
			lex<H>(file, config, [&]<size_t I>(const std::string &token) -> void {
				bitset |= const_v<int_exp2(I)>;
				invoke<I>(eval, default_value_v<getter<I>>(t), token);
			});
		})));
		if (index != std::tuple_size_v<TUPLE>) {
			abort<AA_SOURCE_DATA>("Parameter (index: ", index, ") not found.");
		}
	}

	template<instance_of<string_perfect_hash<>> auto H, same_tuple_size_as<const_t<H>> TUPLE, class EVAL = generic_evaluator<>, class CONFIG, class FILE>
	AA_CONSTEXPR std::remove_cvref_t<TUPLE> parse(FILE &&file, CONFIG &&config, EVAL &&eval = default_value) {
		return make_with_invocable([&](std::remove_cvref_t<TUPLE> &t) -> void {
			parse<H>(t, file, config, eval);
		});
	}

	template<instance_of<string_perfect_hash<>> auto H, same_tuple_size_as<const_t<H>> TUPLE, class EVAL = generic_evaluator<>, class CONFIG, class FILE>
	AA_CONSTEXPR std::remove_cvref_t<TUPLE> safe_parse(FILE &&file, CONFIG &&config, EVAL &&eval = default_value) {
		return make_with_invocable([&](std::remove_cvref_t<TUPLE> &t) -> void {
			safe_parse<H>(t, file, config, eval);
		});
	}

}
