#pragma once

#include "../metaprogramming/general.hpp"
#include <cstddef> // size_t
#include <cstring> // memcpy
#include <type_traits> // remove_const_t
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

}
