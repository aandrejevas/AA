#pragma once

#include "../metaprogramming/general.hpp"
#include <string_view> // string_view
#include <charconv> // from_chars
#include <string> // string



namespace aa {

	template<class>
	struct evaluator;

	// Nėra kreipiama dėmesio į evaluat'inimo procesu įvykusias klaidas standartiniuose evaluator'iuose,
	// nes daroma prielaida, kad naudotojas nenori, kad būtų sustabdyta programa jei buvo tokios klados aptiktos.
	template<arithmetic T>
	struct evaluator<T> {
		AA_CONSTEXPR T operator()(const std::string_view &token) const {
			// Reikia inicializuoti kintamąjį, nes from_chars nebūtinai jį modifikuos.
			T param = 0;
			std::from_chars(token.data(), token.data() + token.length(), param);
			return param;
		}
	};

	// Negalime turėti evaluator<std::string_view>, nes atmintis į kurią rodo token pasikeis.
	template<>
	struct evaluator<std::string> {
		AA_CONSTEXPR std::string operator()(const std::string_view &token) const {
			return std::string{token};
		}
	};

}
