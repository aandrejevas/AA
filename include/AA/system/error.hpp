#pragma once

#include "../metaprogramming/general.hpp"
#include <csetjmp>
#include <cstdio>
#include <source_location>
#include <chrono>
#include <print>



namespace aa {

	template<placeholder_like auto = std::placeholders::_1>
	constinit thread_local std::jmp_buf catcher;

	template<class F>
	constexpr void log(
		F && f = default_value,
		std::FILE * const stream = stdout,
		std::source_location && l = std::source_location::current())
	{
		// https://stackoverflow.com/questions/76106361/stdformating-stdchrono-seconds-without-fractional-digits
		std::print(stream, "{} {}({}:{}) '{}': ",
			std::chrono::system_clock::now(),
			l.file_name(), l.line(), l.column(), l.function_name());

		if constexpr (std::invocable<F, std::FILE * const &>) {
			std::invoke(std::forward<F>(f), stream);
		} else {
			std::invoke(std::forward<F>(f));
		}

		std::println(stream);
	}

	template<class F>
	constexpr bool check(const bool cond,
		F && f = default_value,
		std::FILE * const stream = stdout,
		std::source_location && l = std::source_location::current())
	{
		if (cond)		return false;
		else			return (log(std::forward<F>(f), stream, std::move(l)), true);
	}

	template<int STATUS = true, class F>
		requires (!!STATUS)
	[[noreturn]] constexpr void die(
		F && f = default_value,
		std::FILE * const stream = stdout, std::jmp_buf & eh = catcher<>,
		std::source_location && l = std::source_location::current())
	{
		log(std::forward<F>(f), stream, std::move(l));

		std::longjmp(eh, STATUS);
	}

	template<int STATUS = true, class F>
	constexpr void assert(const bool cond,
		F && f = default_value,
		std::FILE * const stream = stdout, std::jmp_buf & eh = catcher<>,
		std::source_location && l = std::source_location::current())
	{
		if (!cond)		die<STATUS>(std::forward<F>(f), stream, eh, std::move(l));
	}

}
