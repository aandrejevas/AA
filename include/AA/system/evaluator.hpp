#pragma once

#include "../metaprogramming/general.hpp"
#include "../container/fixed_vector.hpp"
#include <charconv> // from_chars



namespace aa {

	struct evaluator {
		fixed_vector<char, 100> token;

		constexpr bool operator()(const int c) {
			switch (c) {
				case ' ': case '\t': case '\n': case '\r':
					return token.empty();
				default:
					token.insert_back(static_cast<char>(c));
					return true;
			}
		}

		template<size_t = 0, arithmetic T>
		constexpr void evaluate(T &t) {
			// Įvykus klaidai from_chars kintamojo nemodifikuos, bet t privalo būti modifikuotas.
			if (!is_default_value(std::from_chars(token.begin(), token.end(), t).ec)) {
				t = numeric_max;
			}
			token.clear();
		}
	};

}
