#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/io.hpp"
#include "../algorithm/hash.hpp"
#include "lexer.hpp"
#include "evaluator.hpp"
#include <string> // string
#include <string_view> // string_view
#include <functional> // invoke



namespace aa {

	template<string_perfect_hash H, size_t R = 50, class EVAL = generic_evaluator<>, input_stream FILE, instantiation_of<tuple> TUPLE>
	AA_CONSTEXPR void parse(TUPLE &t, FILE &&file, EVAL &&eval = {}) {
		lex<H, R>(file, [&](const size_t index, const std::string &token) -> void {
			t.get(index, [&]<evaluable_by<EVAL &> VALUE>(VALUE & value) -> void {
				std::invoke(eval, value, index, token.operator std::string_view());
			});
		});
	}

}
