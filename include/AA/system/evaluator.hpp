#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/io.hpp"
#include "../preprocessor/assert.hpp"
#include "../container/fixed_vector.hpp"
#include <charconv> // from_chars_result, from_chars



namespace aa {

	struct evaluator {
		using traits_type = char_traits_t;

		fixed_vector<char, 100> token;

		template<arithmetic T>
		constexpr bool operator()(const int c, T &t) {
			switch (c) {
				case ' ': case '\t': case '\n': case '\r':
					if (token.empty()) return true;
					[[fallthrough]];
				case traits_type::eof(): {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
					const std::from_chars_result r = std::from_chars(std::as_const(token).begin(), token.end(), t);
#pragma GCC diagnostic pop
					AA_TRACE_ASSERT(is_default_value(r.ec) && r.ptr == token.end(), "{}", std::to_underlying(r.ec));
					token.clear();
					return false;
				} default:
					token.insert_back(traits_type::to_char_type(c));
					return true;
			}
		}
	};

}
