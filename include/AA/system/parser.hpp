#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/io.hpp"
#include "../preprocessor/assert.hpp"
#include "../algorithm/arithmetic.hpp"
#include "../algorithm/hash.hpp"
#include "../algorithm/init.hpp"
#include "lexer.hpp"
#include "evaluator.hpp"
#include "source.hpp"
#include <cstddef> // size_t
#include <string> // string
#include <type_traits> // remove_const_t, remove_reference_t
#include <limits> // numeric_limits
#include <bit> // countr_one
#include <utility> // tuple_size_v



namespace aa {

	template<class EVAL = generic_evaluator<>, not_const_instance_of_twtp<param_lexer> LEXER, input_stream FILE, same_tuple_size_as<LEXER> TUPLE>
	AA_CONSTEXPR void parse(TUPLE &t, FILE &&file, LEXER &&lexer, EVAL &&eval = constant<EVAL>()) {
		lex(file, lexer, [&]<size_t I>(const std::string & token) -> void {
			invoke<I>(eval, constant<getter<I>>()(t), token);
		});
	}

	template<class EVAL = generic_evaluator<>, not_const_instance_of_twtp<param_lexer> LEXER, input_stream FILE, same_tuple_size_as<LEXER> TUPLE>
		requires (std::tuple_size_v<TUPLE> <= std::numeric_limits<size_t>::digits)
	AA_CONSTEXPR void safe_parse(TUPLE &t, FILE &&file, LEXER &&lexer, EVAL &&eval = constant<EVAL>()) {
		const size_t index = unsign<size_t>(std::countr_one(make_with_invocable<size_t>([&](size_t &bitset) -> void { {
				bitset = 0;
				lex(file, lexer, [&]<size_t I>(const std::string & token) -> void {
					bitset |= constant<int_exp2(I)>();
					invoke<I>(eval, constant<getter<I>>()(t), token);
				});
			}})));
		trace<AA_SOURCE_DATA>(index == std::tuple_size_v<TUPLE>, [&]<class D>() -> void {
			std::remove_reference_t<LEXER>::hasher_type
				::get(index, []<size_t, auto NAME>() -> void { abort<D>("Parameter `", NAME, "` not found."); });
		});
	}

	template<class TUPLE, class EVAL = generic_evaluator<>, not_const_instance_of_twtp<param_lexer> LEXER, input_stream FILE>
	AA_CONSTEXPR std::remove_const_t<TUPLE> parse(FILE &&file, LEXER &&lexer, EVAL &&eval = constant<EVAL>()) {
		return make_with_invocable<TUPLE>([&](std::remove_const_t<TUPLE> &t) -> void {
			parse(t, file, lexer, eval);
		});
	}

	template<class TUPLE, class EVAL = generic_evaluator<>, not_const_instance_of_twtp<param_lexer> LEXER, input_stream FILE>
	AA_CONSTEXPR std::remove_const_t<TUPLE> safe_parse(FILE &&file, LEXER &&lexer, EVAL &&eval = constant<EVAL>()) {
		return make_with_invocable<TUPLE>([&](std::remove_const_t<TUPLE> &t) -> void {
			safe_parse(t, file, lexer, eval);
		});
	}

}
