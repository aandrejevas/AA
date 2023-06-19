#pragma once

#include "../metaprogramming/general.hpp"



namespace aa {

	template<wo_cvref_default_initializable D, std::invocable<std::remove_cvref_t<D> &> F>
	AA_CONSTEXPR std::remove_cvref_t<D> make_with_invocable(F &&f = default_value) {
		std::remove_cvref_t<D> d;
		std::invoke(std::forward<F>(f), d);
		return d;
	}

	template<same_as_void = void, class F>
	AA_CONSTEXPR std::remove_cvref_t<function_argument_t<F>> make_with_invocable(F &&f = default_value) {
		return make_with_invocable<std::remove_cvref_t<function_argument_t<F>>>(std::forward<F>(f));
	}

	template<wo_cvref_copy_constructible D, std::remove_cvref_t<D> I, std::invocable<std::remove_cvref_t<D> &> F>
	AA_CONSTEXPR std::remove_cvref_t<D> make_with_invocable(F &&f = default_value) {
		std::remove_cvref_t<D> d = I;
		std::invoke(std::forward<F>(f), d);
		return d;
	}

	template<auto I, same_as_void = void, class F>
	AA_CONSTEXPR std::remove_cvref_t<function_argument_t<F>> make_with_invocable(F &&f = default_value) {
		return make_with_invocable<std::remove_cvref_t<function_argument_t<F>>,
			cast<std::remove_cvref_t<function_argument_t<F>>>(I)>(std::forward<F>(f));
	}

}
