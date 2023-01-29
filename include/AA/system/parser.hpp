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
#include <type_traits> // remove_const_t, remove_reference_t
#include <limits> // numeric_limits
#include <bit> // countr_one



namespace aa {

	template<class EVAL = generic_evaluator<>, not_const_instance_of_twtp<param_lexer> LEXER, input_stream FILE, not_const_instance_of_twtp<tuple> TUPLE>
	AA_CONSTEXPR void parse(TUPLE &t, FILE &&file, LEXER &&lexer, EVAL &&eval = {}) {
		lex(file, lexer, [&]<size_t I>(const std::string & token) -> void {
			invoke<I>(eval, t.template get<I>(), token);
		});
	}

	template<class EVAL = generic_evaluator<>, not_const_instance_of_twtp<param_lexer> LEXER, input_stream FILE, not_const_instance_of_twtp<tuple> TUPLE>
		requires (std::remove_reference_t<LEXER>::hasher_type::max() <= std::numeric_limits<size_t>::digits)
	AA_CONSTEXPR void safe_parse(TUPLE &t, FILE &&file, LEXER &&lexer, EVAL &&eval = {}) {
		size_t bitset = 0;
		lex(file, lexer, [&]<size_t I>(const std::string & token) -> void {
			bitset |= constant<int_exp2(I)>();
			invoke<I>(eval, t.template get<I>(), token);
		});
		AA_TRACE_ASSERT(std::countr_one(bitset) == std::remove_reference_t<LEXER>::hasher_type::max()/*,
			"Parameter `", type_name<T>(), " - ", name, "` not found."*/);
	}

	template<instance_of_twtp<tuple> TUPLE, class EVAL = generic_evaluator<>, not_const_instance_of_twtp<param_lexer> LEXER, input_stream FILE>
	AA_CONSTEXPR std::remove_const_t<TUPLE> parse(FILE &&file, LEXER &&lexer, EVAL &&eval = {}) {
		return make_with_invocable<TUPLE>([&](std::remove_const_t<TUPLE> &t) -> void {
			parse(t, file, lexer, eval);
		});
	}

	template<instance_of_twtp<tuple> TUPLE, class EVAL = generic_evaluator<>, not_const_instance_of_twtp<param_lexer> LEXER, input_stream FILE>
	AA_CONSTEXPR std::remove_const_t<TUPLE> safe_parse(FILE &&file, LEXER &&lexer, EVAL &&eval = {}) {
		return make_with_invocable<TUPLE>([&](std::remove_const_t<TUPLE> &t) -> void {
			safe_parse(t, file, lexer, eval);
		});
	}

}
