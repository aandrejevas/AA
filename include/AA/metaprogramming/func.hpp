#pragma once

#include "general.hpp"
#include <cstddef> // size_t
#include <functional> // hash



namespace aa {

	template<template<class> class H = std::hash>
	struct hash {
		template<hashable_by_template<H> T>
		[[gnu::always_inline]] AA_CONSTEXPR size_t operator()(const T &t) const {
			return H<T>{}(t);
		}

		using is_transparent = void;
	};

	hash()->hash<>;

}
