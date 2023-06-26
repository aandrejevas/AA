#pragma once

#include "../metaprogramming/general.hpp"



namespace aa {

	template<out_unary_invocable F>
	AA_CONSTEXPR std::remove_reference_t<function_argument_t<F>> make_with_invocable(F &&f = default_value) {
		std::remove_reference_t<function_argument_t<F>> d;
		std::invoke(std::forward<F>(f), d);
		return d;
	}

	template<auto I, out_unary_invocable F>
	AA_CONSTEXPR std::remove_reference_t<function_argument_t<F>> make_with_invocable(F &&f = default_value) {
		std::remove_reference_t<function_argument_t<F>> d = const_v<cast<std::remove_reference_t<function_argument_t<F>>>(I)>;
		std::invoke(std::forward<F>(f), d);
		return d;
	}

	template<out_unary_invocable F, class... A>
		requires (std::constructible_from<std::remove_reference_t<function_argument_t<F>>, A...>)
	AA_CONSTEXPR std::remove_reference_t<function_argument_t<F>> make_with_invocable(F &&f = default_value, A&&... args) {
		std::remove_reference_t<function_argument_t<F>> d = std::remove_reference_t<function_argument_t<F>>(std::forward<A>(args)...);
		std::invoke(std::forward<F>(f), d);
		return d;
	}

}
