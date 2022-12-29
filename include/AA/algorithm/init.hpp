#pragma once

#include "../metaprogramming/general.hpp"
#include <cstddef> // size_t
#include <cstring> // memcpy, memset
#include <type_traits> // remove_const_t
#include <concepts> // invocable
#include <functional> // invoke
#include <utility> // forward
#include <memory> // addressof



namespace aa {

	template<not_const_default_initializable D, std::invocable<std::remove_const_t<D> &> F>
	AA_CONSTEXPR std::remove_const_t<D> create_with_invocable(F &&f) {
		std::remove_const_t<D> d;
		std::invoke(std::forward<F>(f), d);
		return d;
	}

	// Sakoma, kad D turi būti trivial, nes šiuo atveju konstruktorius neturi jokio darbo atlikti.
	// Kitaip klasės kintamieji bus inicializuoti ir jie bus vėl inicializuoti memcpy procedūros.
	template<trivial D, trivially_copyable S>
	AA_CONSTEXPR std::remove_const_t<D> create_with_memcpy(const S &s, const size_t n) {
		return create_with_invocable<D>([&](std::remove_const_t<D> &d) -> void {
			std::memcpy(std::addressof(d), std::addressof(s), n);
		});
	}

	template<trivial D>
	AA_CONSTEXPR std::remove_const_t<D> create_with_memset(const int v, const size_t n) {
		return create_with_invocable<D>([&](std::remove_const_t<D> &d) -> void {
			std::memset(std::addressof(d), v, n);
		});
	}

}
