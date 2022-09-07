#pragma once

#include "../metaprogramming/general.hpp"
#include "../container/fixed_string.hpp"
#include "../algorithm/arithmetic.hpp"
#include <cstdint> // uint32_t
#include <cstdlib> // abort
#include <utility> // forward
#include <iostream> // cerr, clog



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
	};

	template<size_t N, size_t M>
	source_data(const uint32_t, const uint32_t, const char(&)[N], const char(&)[M])->source_data<N, M>;



	template<source_data D, class S, class... A>
	[[gnu::always_inline]] AA_CONSTEXPR void f_log(S &s, const A&... args) {
		s << D.file_name << '(' << D.line;
		if constexpr (!is_numeric_max(D.column)) {
			s << ':' << D.column;
		}
		s << ") `" << D.function_name << "`: ";

		if constexpr (sizeof...(A)) {
			(s << ... << args) << '\n';
		} else {
			s << "Info logged.\n";
		}
	}

	template<source_data D, class... A>
	[[gnu::always_inline]] AA_CONSTEXPR void log(const A&... args) {
		f_log<D>(std::clog, args...);
	}

	template<source_data D, class S, class... A>
	[[gnu::always_inline, noreturn]] AA_CONSTEXPR void f_abort(S &s, const A&... args) {
		if constexpr (sizeof...(A)) {
			f_log<D>(s, args...);
		} else {
			f_log<D>(s, "Program aborted.");
		}
		std::abort();
	}

	template<source_data D, class... A>
	[[gnu::always_inline, noreturn]] AA_CONSTEXPR void abort(const A&... args) {
		f_abort<D>(std::cerr, args...);
	}

	// TODO: Su c++23 čia galima bus naudoti static operator().
	template<source_data D, bool T = true, nttp_accepting_functor<D> F>
	[[gnu::always_inline]] AA_CONSTEXPR void trace(const bool condition, F &&f) {
		if constexpr (T || !AA_ISDEF_NDEBUG) {
			if (!condition) {
				std::forward<F>(f).template AA_CALL_OPERATOR<D>();
			}
		}
	}

	// Dėl atributo, naudoti šią funkciją turėtų būti tas pats kaip naudoti macro greitaveikos atžvilgiu.
	template<source_data D, bool T = true, class S, class... A>
	[[gnu::always_inline]] AA_CONSTEXPR void f_assert(S &s, const bool condition, const A&... args) {
		if constexpr (T || !AA_ISDEF_NDEBUG) {
			if (!condition) {
				if constexpr (sizeof...(A)) {
					f_abort<D>(s, args...);
				} else {
					f_abort<D>(s, "Assertion failed.");
				}
			}
		}
	}

	template<source_data D, bool T = true, class... A>
	[[gnu::always_inline]] AA_CONSTEXPR void assert(const bool condition, const A&... args) {
		f_assert<D, T>(std::cerr, condition, args...);
	}

}
