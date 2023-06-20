#pragma once

#include "../metaprogramming/general.hpp"



namespace aa {

	template<unary_invocable_with_out_arg F>
	AA_CONSTEXPR std::remove_reference_t<function_argument_t<F>> make_with_invocable(F &&f = default_value) {
		std::remove_reference_t<function_argument_t<F>> d;
		std::invoke(std::forward<F>(f), d);
		return d;
	}

	template<auto I, unary_invocable_with_out_arg F>
	AA_CONSTEXPR std::remove_reference_t<function_argument_t<F>> make_with_invocable(F &&f = default_value) {
		std::remove_reference_t<function_argument_t<F>> d = const_v<cast<std::remove_reference_t<function_argument_t<F>>>(I)>;
		std::invoke(std::forward<F>(f), d);
		return d;
	}

	template<unary_invocable_with_out_arg F, class... A>
		requires (std::constructible_from<std::remove_reference_t<function_argument_t<F>>, A...>)
	AA_CONSTEXPR std::remove_reference_t<function_argument_t<F>> make_with_invocable(F &&f = default_value, A&&... args) {
		std::remove_reference_t<function_argument_t<F>> d = std::remove_reference_t<function_argument_t<F>>(std::forward<A>(args)...);
		std::invoke(std::forward<F>(f), d);
		return d;
	}

}
