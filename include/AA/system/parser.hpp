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
#include <type_traits> // remove_const_t
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
		size_t bitset = 0;
		lex(file, lexer, [&]<size_t I>(const std::string & token) -> void {
			bitset |= constant<int_exp2(I)>();
			invoke<I>(eval, constant<getter<I>>()(t), token);
		});
		AA_TRACE_ASSERT(std::countr_one(bitset) == std::tuple_size_v<TUPLE>/*,
			"Parameter `", type_name<T>(), " - ", name, "` not found."*/);
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
