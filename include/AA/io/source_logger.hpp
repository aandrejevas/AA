#pragma once

#include "../metaprogramming/general.hpp"
#include "print.hpp"
#include <cstdint> // uint32_t
#include <cstdlib> // abort
#include <source_location> // source_location
#include <type_traits> // invoke_result_t
#include <functional> // invoke
#include <utility> // forward



namespace aa {

	// Klasės reikia, nes source_location klasės negalima naudoti kaip non type template parameter
	// ir taip pat yra truputį keista man, kad minėtos klasės duomenys pasiekiami tik per metodus.
	struct source_data {
		// Member objects
		const uint32_t column, line;
		const char *const file_name, *const function_name;



		// Special member functions
		AA_CONSTEVAL source_data(const std::source_location &l = std::source_location::current())
			: column{l.column()}, line{l.line()}, file_name{l.file_name()}, function_name{l.function_name()} {}

		AA_CONSTEVAL source_data(const uint32_t c, const uint32_t l = __LINE__, const char *const fi = __FILE__, const char *const fu = __func__)
			: column{c}, line{l}, file_name{fi}, function_name{fu} {}
	};



	// Negalime iškelti template parametro į funkcijas, nes tokiu atveju būtų labai nepatogu jas naudoti.
	// Patogu naudoti šią klasę tik dėl sukurto macro, ne alternatyva kiekvienai funkcijai kurti po macro.
	template<source_data D>
	struct logger {
		// Static member functions
		template<auto S = &get_clog, class... A>
			requires (function_pointer<decltype(S)>)
		static AA_CONSTEXPR void log(const A&... args) {
			std::invoke_result_t<decltype(S)> stream = S();
			stream << D.file_name << '(' << D.line << ':' << D.column << ") `" << D.function_name << "`: ";
			if constexpr (sizeof...(A)) {
				(stream << ... << args) << '\n';
			} else {
				stream << "Info logged.\n";
			}
		}

		template<auto S = &get_cerr, class... A>
			requires (function_pointer<decltype(S)>)
		[[noreturn]] static AA_CONSTEXPR void abort(const A&... args) {
			if constexpr (sizeof...(A)) {
				log<S>(args...);
			} else {
				log<S>("Program aborted.");
			}
			std::abort();
		}

		template<bool T = true, class F>
		static AA_CONSTEXPR void trace(const bool condition, F &&f) {
			if constexpr (T || !AA_ISDEF_NDEBUG) {
				if (!condition) {
					std::forward<F>(f).template AA_CALL_OPERATOR<logger>();
				}
			}
		}

		template<auto S = &get_cerr, bool T = true, class... A>
			requires (function_pointer<decltype(S)>)
		static AA_CONSTEXPR void assert(const bool condition, const A&... args) {
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
