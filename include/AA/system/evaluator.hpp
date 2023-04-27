#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/range.hpp"
#include "../algorithm/init.hpp"
#include <charconv> // from_chars
#include <string> // string
#include <system_error> // errc
#include <ranges> // data, size



namespace aa {

	template<not_const>
	struct evaluator;

	// Nėra kreipiama dėmesio į evaluat'inimo procesu įvykusias klaidas standartiniuose evaluator'iuose,
	// nes daroma prielaida, kad naudotojas nenori, kad būtų sustabdyta programa jei buvo tokios klados aptiktos.
	template<not_const_arithmetic T>
	struct evaluator<T> {
		template<size_t = 0, char_range U>
		AA_CONSTEXPR void operator()(T &t, const U &token) const {
			// Įvykus klaidai from_chars kintamojo nemodifikuos, bet t privalo būti modifikuotas.
			if (std::from_chars(std::ranges::data(token), get_data_end(token), t).ec != constant_v<std::errc>) {
				t = numeric_max;
			}
		}
	};

	// Negalime turėti evaluator<std::string_view>, nes atmintis į kurią rodo token pasikeis.
	template<>
	struct evaluator<std::string> {
		template<size_t = 0, char_range U>
		AA_CONSTEXPR void operator()(std::string &t, const U &token) const {
			t.assign(std::ranges::data(token), std::ranges::size(token));
		}
	};



	template<template<class> class E = evaluator>
	struct generic_evaluator {
		template<size_t I = 0, argument_for_tdc_template<E> T, char_range U>
		AA_CONSTEXPR void operator()(T &t, const U &token) const {
			invoke<I>(constant_v<E<T>>, t, token);
		}
	};

	generic_evaluator()->generic_evaluator<>;



	template<class T, size_t I = 0, char_range U, class EVAL = generic_evaluator<>>
	AA_CONSTEXPR std::remove_cvref_t<T> evaluate(const U &token, EVAL &&eval = default_value) {
		return make_with_invocable([&](std::remove_cvref_t<T> &t) -> void {
			invoke<I>(std::forward<EVAL>(eval), t, token);
		});
	}

}
