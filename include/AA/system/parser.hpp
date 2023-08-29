#pragma once

#include "../metaprogramming/general.hpp"
#include "../preprocessor/assert.hpp"
#include "../algorithm/hash.hpp"
#include "../algorithm/init.hpp"
#include "lexer.hpp"
#include "evaluator.hpp"
#include "source.hpp"
#include "print.hpp"



namespace aa {

	// config parametrui nenurodome numatytos reikšmės, nes ji nėra constexpr reikšmė.
	template<instance_of<string_perfect_hash<>> auto H, class EVAL = const generic_evaluator<>, class FILE, same_tuple_size_as<const_t<H>> TUPLE>
	constexpr void parse(TUPLE &t, const FILE &file, EVAL &&eval = default_value) {
		lex<H>(file, config, [&]<size_t I>(const token_type &token) -> void {
			invoke<I>(eval, getter_v<I>(t), token);
		});
	}

	template<instance_of<string_perfect_hash<>> auto H, class EVAL = const generic_evaluator<>, class FILE, same_tuple_size_as<const_t<H>> TUPLE>
		requires (std::tuple_size_v<TUPLE> <= numeric_digits_v<size_t>)
	constexpr void safe_parse(TUPLE &t, const FILE &file, EVAL &&eval = default_value) {
		const size_t index = unsign<size_t>(std::countr_one(make_with_invocable<0uz>([&](size_t &bitset) -> void {
			lex<H>(file, config, [&]<size_t I>(const token_type &token) -> void {
				bitset |= const_v<int_exp2(I)>;
				invoke<I>(eval, getter_v<I>(t), token);
			});
		})));
		if (index != std::tuple_size_v<TUPLE>) {
			abort<AA_SOURCE_DATA>("Parameter (index: ", index, ") not found.");
		}
	}

	template<instance_of<string_perfect_hash<>> auto H, same_tuple_size_as<const_t<H>> TUPLE, class EVAL = const generic_evaluator<>, class FILE>
	constexpr std::remove_cvref_t<TUPLE> parse(const FILE &file, EVAL &&eval = default_value) {
		return make_with_invocable([&](std::remove_cvref_t<TUPLE> &t) -> void {
			parse<H>(t, file, config, eval);
		});
	}

	template<instance_of<string_perfect_hash<>> auto H, same_tuple_size_as<const_t<H>> TUPLE, class EVAL = const generic_evaluator<>, class FILE>
	constexpr std::remove_cvref_t<TUPLE> safe_parse(const FILE &file, EVAL &&eval = default_value) {
		return make_with_invocable([&](std::remove_cvref_t<TUPLE> &t) -> void {
			safe_parse<H>(t, file, config, eval);
		});
	}

}
