#pragma once

#include "../metaprogramming/general.hpp"
#include "arithmetic.hpp"
#include <cstddef> // size_t
#include <functional> // invoke, hash



namespace aa {

	template<template<class> class H = std::hash>
	struct hash {
		template<hashable_by_template<H> T>
		[[gnu::always_inline]] AA_CONSTEXPR size_t operator()(const T &t) const {
			return std::invoke(H<T>{}, t);
		}

		static AA_CONSTEVAL size_t min() { return numeric_min; }
		static AA_CONSTEVAL size_t max() { return numeric_max; }

		using is_transparent = void;
	};

	hash()->hash<>;



	template<size_t N, template<class> class H = std::hash>
	struct mod_hash : hash<H> {
		template<hashable_by_template<H> T>
		[[gnu::always_inline]] AA_CONSTEXPR size_t operator()(const T &t) const {
			return remainder<N>(hash<H>::operator()(t));
		}

		static AA_CONSTEVAL size_t max() { return N - 1; }
	};

}
