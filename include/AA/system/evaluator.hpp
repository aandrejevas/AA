#pragma once

#include "../metaprogramming/general.hpp"
#include "../algorithm/init.hpp"
#include <cstddef> // size_t
#include <string_view> // string_view
#include <charconv> // from_chars
#include <string> // string
#include <utility> // forward
#include <type_traits> // remove_cvref_t



namespace aa {

	template<not_const>
	struct evaluator;

	// Nėra kreipiama dėmesio į evaluat'inimo procesu įvykusias klaidas standartiniuose evaluator'iuose,
	// nes daroma prielaida, kad naudotojas nenori, kad būtų sustabdyta programa jei buvo tokios klados aptiktos.
	template<not_const_arithmetic T>
	struct evaluator<T> {
		template<size_t = 0>
		AA_CONSTEXPR void operator()(T &t, const std::string_view &token) const {
			// Reikia inicializuoti kintamąjį, nes from_chars nebūtinai jį modifikuos.
			t = numeric_max;
			std::from_chars(token.data(), token.data() + token.length(), t);
		}
	};

	// Negalime turėti evaluator<std::string_view>, nes atmintis į kurią rodo token pasikeis.
	template<>
	struct evaluator<std::string> {
		template<size_t = 0>
		AA_CONSTEXPR void operator()(std::string &t, const std::string_view &token) const {
			t = token;
		}
	};



	template<template<class> class E = evaluator>
	struct generic_evaluator {
		template<size_t I = 0, argument_for_tdc_template<E> T>
		AA_CONSTEXPR void operator()(T &t, const std::string_view &token) const {
			invoke<I>(constant_v<E<T>>, t, token);
		}
	};

	generic_evaluator()->generic_evaluator<>;



	template<class T, size_t I = 0, class EVAL = generic_evaluator<>>
	AA_CONSTEXPR std::remove_cvref_t<T> evaluate(const std::string_view &token, EVAL &&eval = default_value) {
		return make_with_invocable([&](std::remove_cvref_t<T> &t) -> void {
			invoke<I>(std::forward<EVAL>(eval), t, token);
		});
	}

}
