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
#include <string> // string



namespace aa {

	template<class EVAL = generic_evaluator<>, not_const_instance_of_twttp<lexer_config> CONFIG, char_input_stream FILE, same_tuple_size_as<CONFIG> TUPLE>
	AA_CONSTEXPR void parse(TUPLE &t, FILE &&file, CONFIG &&config, EVAL &&eval = default_value) {
		lex(file, lexer{config, ([&]<size_t I>(const std::string & token) -> void {
			invoke<I>(eval, constant_v<getter<I>>(t), token);
		})});
	}

	template<class EVAL = generic_evaluator<>, not_const_instance_of_twttp<lexer_config> CONFIG, char_input_stream FILE, same_tuple_size_as<CONFIG> TUPLE>
		requires (std::tuple_size_v<TUPLE> <= std::numeric_limits<size_t>::digits)
	AA_CONSTEXPR void safe_parse(TUPLE &t, FILE &&file, CONFIG &&config, EVAL &&eval = default_value) {
		const size_t index = unsign<size_t>(std::countr_one(make_with_invocable([&](size_t &bitset) -> void { {
				bitset = 0;
				lex(file, lexer{config, ([&]<size_t I>(const std::string & token) -> void {
					bitset |= const_v<int_exp2(I)>;
					invoke<I>(eval, constant_v<getter<I>>(t), token);
				})});
			}})));
		if (index != std::tuple_size_v<TUPLE>) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
			constify<TUPLE>(index, []<size_t I>() -> void {
#pragma GCC diagnostic pop
				abort<AA_SOURCE_DATA>("Parameter `", std::remove_reference_t<CONFIG>::template get<I>(), "` (index: ", I, ") not found.");
			});
		}
	}

	template<class TUPLE, class EVAL = generic_evaluator<>, not_const_instance_of_twttp<lexer_config> CONFIG, char_input_stream FILE>
	AA_CONSTEXPR std::remove_cvref_t<TUPLE> parse(FILE &&file, CONFIG &&config, EVAL &&eval = default_value) {
		return make_with_invocable([&](std::remove_cvref_t<TUPLE> &t) -> void {
			parse(t, file, config, eval);
		});
	}

	template<class TUPLE, class EVAL = generic_evaluator<>, not_const_instance_of_twttp<lexer_config> CONFIG, char_input_stream FILE>
	AA_CONSTEXPR std::remove_cvref_t<TUPLE> safe_parse(FILE &&file, CONFIG &&config, EVAL &&eval = default_value) {
		return make_with_invocable([&](std::remove_cvref_t<TUPLE> &t) -> void {
			safe_parse(t, file, config, eval);
		});
	}

}
