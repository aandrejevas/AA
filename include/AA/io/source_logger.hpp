#pragma once

#include "../metaprogramming/general.hpp"
#include "../container/fixed_string.hpp"
#include "../algorithm/arithmetic.hpp"
#include "print.hpp"
#include <cstdint> // uint32_t
#include <cstdlib> // abort
#include <type_traits> // invoke_result_t
#include <utility> // forward



namespace aa {

	// Klasės reikia, nes source_location klasės negalima naudoti kaip non type template parameter
	// ir taip pat yra truputį keista man, kad minėtos klasės duomenys pasiekiami tik per metodus.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpadded"
	template<size_t N, size_t M>
	struct source_data {
#pragma GCC diagnostic pop
		// Member objects
		const uint32_t column, line;
		const fixed_string<N> file_name;
		const fixed_string<M> function_name;



		// Special member functions
		AA_CONSTEVAL source_data(const uint32_t c, const uint32_t l, const char *const fi, const char *const fu)
			: column{c}, line{l}, file_name{fi}, function_name{fu} {}
	};

	template<size_t N, size_t M>
	source_data(const uint32_t, const uint32_t, const char(&)[N], const char(&)[M])->source_data<N - 1, M - 1>;



	template<source_data D, auto S = &get_clog, class... A>
		requires (function_pointer<decltype(S)>)
	AA_CONSTEXPR void log(const A&... args) {
		std::invoke_result_t<decltype(S)> stream = S();

		stream << D.file_name << '(' << D.line;
		if constexpr (!is_max(D.column)) {
			stream << ':' << D.column;
		}
		stream << ") `" << D.function_name << "`: ";

		if constexpr (sizeof...(A)) {
			(stream << ... << args) << '\n';
		} else {
			stream << "Info logged.\n";
		}
	}

	template<source_data D, auto S = &get_cerr, class... A>
		requires (function_pointer<decltype(S)>)
	[[noreturn]] AA_CONSTEXPR void abort(const A&... args) {
		if constexpr (sizeof...(A)) {
			log<D, S>(args...);
		} else {
			log<D, S>("Program aborted.");
		}
		std::abort();
	}

	template<source_data D, bool T = true, class F>
	[[gnu::always_inline]] AA_CONSTEXPR void trace(const bool condition, F &&f) {
		if constexpr (T || !AA_ISDEF_NDEBUG) {
			if (!condition) {
				std::forward<F>(f).template AA_CALL_OPERATOR<D>();
			}
		}
	}

	// Dėl atributo, naudoti šią funkciją turėtų būti tas pats kaip naudoti macro greitaveikos atžvilgiu.
	template<source_data D, auto S = &get_cerr, bool T = true, class... A>
		requires (function_pointer<decltype(S)>)
	[[gnu::always_inline]] AA_CONSTEXPR void assert(const bool condition, const A&... args) {
		if constexpr (T || !AA_ISDEF_NDEBUG) {
			if (!condition) {
				if constexpr (sizeof...(A)) {
					abort<D, S>(args...);
				} else {
					abort<D, S>("Assertion failed.");
				}
			}
		}
	}

}
