#pragma once

#include "../metaprogramming/general.hpp"
#include <cstdlib> // abort
#include <ostream> // ostream
#include <iostream> // cerr
#include <source_location> // source_location
#include <concepts> // invocable
#include <functional> // invoke
#include <utility> // forward



namespace aa {

	template<class S = std::ostream>
	struct source_logger {
		S &stream = std::cerr;
		const std::source_location &location = std::source_location::current();

		template<class... A>
		AA_CONSTEXPR void log(const A&... args) const {
			stream << location.file_name() << '(' << location.line() << ':' << location.column() << ") `"
				<< location.function_name() << "`: ";
			if constexpr (sizeof...(A)) {
				(stream << ... << args) << '\n';
			} else {
				stream << "Info logged.\n";
			}
		}

		template<class... A>
		[[noreturn]] AA_CONSTEXPR void abort(const A&... args) const {
			if constexpr (sizeof...(A)) {
				log(args...);
			} else {
				log("Program aborted.");
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

		template<bool T = true, class... A>
		AA_CONSTEXPR void assert(const bool condition, const A&... args) const {
			if constexpr (T || !AA_ISDEF_NDEBUG) {
				if (!condition) {
					if constexpr (sizeof...(A)) {
						abort(args...);
					} else {
						abort("Assertion failed.");
					}
				}
			}
		}
	};

	source_logger()->source_logger<std::ostream>;

}
