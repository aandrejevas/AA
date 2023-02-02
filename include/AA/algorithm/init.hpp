#pragma once

#include "../metaprogramming/general.hpp"
#include <cstddef> // size_t
#include <cstring> // memcpy, memset
#include <type_traits> // remove_cvref_t
#include <concepts> // invocable
#include <functional> // invoke
#include <utility> // forward
#include <memory> // addressof



namespace aa {

	template<fcn_arg_default_initializable F>
	AA_CONSTEXPR std::remove_cvref_t<function_argument_t<F>> make_with_invocable(F &&f = constant<F>()) {
		std::remove_cvref_t<function_argument_t<F>> d;
		std::invoke(std::forward<F>(f), d);
		return d;
	}

	// Sakoma, kad D turi būti trivial, nes šiuo atveju konstruktorius neturi jokio darbo atlikti.
	// Kitaip klasės kintamieji bus inicializuoti ir jie bus vėl inicializuoti memcpy procedūros.
	template<trivial D, trivially_copyable S>
	AA_CONSTEXPR std::remove_cvref_t<D> make_with_memcpy(const S &s, const size_t n) {
		return make_with_invocable([&](std::remove_cvref_t<D> &d) -> void {
			std::memcpy(std::addressof(d), std::addressof(s), n);
		});
	}

	template<trivial D>
	AA_CONSTEXPR std::remove_cvref_t<D> make_with_memset(const int v, const size_t n) {
		return make_with_invocable([&](std::remove_cvref_t<D> &d) -> void {
			std::memset(std::addressof(d), v, n);
		});
	}

}
