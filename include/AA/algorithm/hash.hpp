#pragma once

#include "../metaprogramming/general.hpp"
#include "arithmetic.hpp"
#include <cstddef> // size_t
#include <functional> // hash



namespace aa {

	template<template<class> class H = std::hash>
	struct generic_hash {
		template<hashable_by_template<H> T>
		[[gnu::always_inline]] AA_CONSTEXPR size_t operator()(const T &t) const {
			return H<T>{}(t);
		}

		static AA_CONSTEVAL size_t min() { return numeric_min; }
		static AA_CONSTEVAL size_t max() { return numeric_max; }

		using is_transparent = void;
	};

	generic_hash()->generic_hash<>;



	template<size_t N, template<class> class H = std::hash>
	struct mod_generic_hash {
		template<hashable_by_template<H> T>
		[[gnu::always_inline]] AA_CONSTEXPR size_t operator()(const T &t) const {
			return remainder<N>(H<T>{}(t));
		}

		static AA_CONSTEVAL size_t min() { return 0; }
		static AA_CONSTEVAL size_t max() { return N - 1; }

		using is_transparent = void;
	};

}
