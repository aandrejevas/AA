#pragma once

#include "../metaprogramming/general.hpp"
#include "../container/fixed_vector.hpp"
#include <charconv> // from_chars
#include <string> // char_traits



namespace aa {

	struct evaluator {
		using traits_type = std::char_traits<char>;

		fixed_vector<char, 100> token;

		template<arithmetic T>
		constexpr bool operator()(const int c, T &t) {
			switch (c) {
				case ' ': case '\t': case '\n': case '\r':
					if (token.empty()) return true;
					[[fallthrough]];
				case traits_type::eof():
					// Įvykus klaidai from_chars kintamojo nemodifikuos, bet t privalo būti modifikuotas.
					if (!is_default_value(std::from_chars(std::as_const(token).begin(), token.end(), t).ec)) {
						t = numeric_max;
					}
					token.clear();
					return false;
				default:
					token.insert_back(traits_type::to_char_type(c));
					return true;
			}
		}
	};

}
