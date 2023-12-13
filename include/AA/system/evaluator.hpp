#pragma once

#include "../metaprogramming/general.hpp"
#include "../preprocessor/assert.hpp"
#include "../container/fixed_vector.hpp"
#include <charconv> // from_chars_result, from_chars



namespace aa {

	struct evaluator {
		using traits_type = char_traits;

		fixed_vector<char, 100> token;

		template<arithmetic_or_assignable_from<std::string_view> T>
		constexpr bool operator()(const int c, T &t) {
			switch (c) {
				case ' ': case '\t': case '\n': case '\r':
					if (token.empty()) return true;
					[[fallthrough]];
				case traits_type::eof():
					if constexpr (arithmetic<T>) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
						const std::from_chars_result r = std::from_chars(std::as_const(token).begin(), token.end(), t);
#pragma GCC diagnostic pop
						AA_TRACE_ASSERT(is_default_value(r.ec) && r.ptr == token.end(), "{}", std::to_underlying(r.ec));
					} else {
						t = std::string_view{std::as_const(token)};
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
