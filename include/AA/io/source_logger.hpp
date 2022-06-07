#pragma once

#include <cstdlib> // abort
#include <ostream> // ostream
#include <iostream> // cerr
#include <source_location> // source_location



namespace aa {

	template<class S = std::ostream>
	struct source_logger {
		S &stream = std::cerr;
		const std::source_location location = std::source_location::current();

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
		[[noreturn]] inline void abort(const A&... args) const {
			if constexpr (sizeof...(A)) {
				log(args...);
			} else {
				log("Program aborted.");
			}
			std::abort();
		}
	};

}
