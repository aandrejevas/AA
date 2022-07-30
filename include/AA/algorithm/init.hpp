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

	// Sakoma, kad D turi būti trivial, nes šiuo atveju konstruktorius neturi jokio darbo atlikti.
	// Kitaip klasės kintamieji bus inicializuoti ir jie bus vėl inicializuoti memcpy procedūros.
	template<trivial D, trivially_copyable S>
	[[gnu::always_inline]] AA_CONSTEXPR std::remove_const_t<D> create_with_memcpy(const S &s, const size_t n) {
		std::remove_const_t<D> d;
		std::memcpy(std::addressof(d), std::addressof(s), n);
		return d;
	}

	template<trivial D>
	[[gnu::always_inline]] AA_CONSTEXPR std::remove_const_t<D> create_with_memset(const int v, const size_t n) {
		std::remove_const_t<D> d;
		std::memset(std::addressof(d), v, n);
		return d;
	}

	template<trivially_default_constructible D, std::invocable<D &> F>
	[[gnu::always_inline]] AA_CONSTEXPR std::remove_const_t<D> create_with_lambda(F &&f) {
		std::remove_const_t<D> d;
		std::invoke(std::forward<F>(f), d);
		return d;
	}

}
