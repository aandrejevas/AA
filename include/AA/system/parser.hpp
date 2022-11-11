#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/io.hpp"
#include "../container/fixed_immutable_set.hpp"
#include "lexer.hpp"
#include "evaluator.hpp"
#include <string> // string
#include <string_view> // string_view
#include <functional> // invoke



namespace aa {

	template<fixed_immutable_set S, size_t R = 50, class EVAL = generic_evaluator<>, input_stream FILE, instantiation_of<tuple> TUPLE>
	AA_CONSTEXPR void parse(TUPLE &t, FILE &&file, EVAL &&eval = {}) {
		lex<S, R>(file, [&](const size_t index, const std::string &token) -> void {
			t.get(index, [&]<evaluable_by<EVAL &> VALUE>(VALUE & value) -> void {
				std::invoke(eval, value, index, token.operator std::string_view());
			});
		});
	}

}
