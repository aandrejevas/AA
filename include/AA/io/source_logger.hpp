#pragma once

#include "../metaprogramming/general.hpp"
#include "print.hpp"
#include <cstdlib> // abort
#include <source_location> // source_location
#include <type_traits> // invoke_result_t
#include <concepts> // invocable
#include <functional> // invoke
#include <utility> // forward



namespace aa {

	// Negalime šios klasės metodų paversti funkcijomis, nes neišeitų
	// suderinti numatyto source_location parametro su parametrų grupe.
	//
	// Klasės metodai yra templated, o ne pati klasė, nes įsivaizduokime scenarijų, kad reikia
	// į du failus surašyti informaciją, to neeitų lengvai padaryti jei klasė būtų templated.
	struct source_logger {
		const std::source_location &location = std::source_location::current();

		template<auto S = &get_clog, class... A>
			requires (function_pointer<decltype(S)>)
		AA_CONSTEXPR void log(const A&... args) const {
			std::invoke_result_t<decltype(S)> stream = S();
			stream << location.file_name() << '(' << location.line() << ':' << location.column() << ") `"
				<< location.function_name() << "`: ";
			if constexpr (sizeof...(A)) {
				(stream << ... << args) << '\n';
			} else {
				stream << "Info logged.\n";
			}
		}

		template<auto S = &get_cerr, class... A>
			requires (function_pointer<decltype(S)>)
		[[noreturn]] AA_CONSTEXPR void abort(const A&... args) const {
			if constexpr (sizeof...(A)) {
				log<S>(args...);
			} else {
				log<S>("Program aborted.");
			}
			std::abort();
		}

		template<bool T = true, std::invocable<const source_logger &> F>
		AA_CONSTEXPR void trace(const bool condition, F &&f) const {
			if constexpr (T || !AA_ISDEF_NDEBUG) {
				if (!condition) {
					std::invoke(std::forward<F>(f), *this);
				}
			}
		}

		template<auto S = &get_cerr, bool T = true, class... A>
			requires (function_pointer<decltype(S)>)
		AA_CONSTEXPR void assert(const bool condition, const A&... args) const {
			if constexpr (T || !AA_ISDEF_NDEBUG) {
				if (!condition) {
					if constexpr (sizeof...(A)) {
						abort<S>(args...);
					} else {
						abort<S>("Assertion failed.");
					}
				}
			}
		}
	};

}
