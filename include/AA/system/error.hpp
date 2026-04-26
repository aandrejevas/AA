#pragma once

#include "../metaprogramming/general.hpp"
#include "../container/managed.hpp"
#include <cstdio>
#include <source_location>
#include <chrono>
#include <thread>
#include <print>
#include <expected>



namespace aa {

	// Neturime versijos, kuri naudotų std::longjmp, nes labai nepatogu naudoti tuos įrankius ir jie sumažina greitaveiką.
	// Atrodo, kad nėra bibliotekos, kuri galėtų pakeisti šitą funkciją. Arba jos nenaudoja std::source_location, arba jos nepateikia galimybės išspausdinti stulpelį, arba jos naudoja macros, arba jos neturi sąlyginio spausdinimo.
	// We do not print the current working directory bc we would need to allocate memory to get it. And it is not really useful.
	// We do not print the name of the executable or the command line bc those should be printed once at the beginning of the program.
	template<class F = std::identity>
	constexpr void log(
		F && f = default_value,
		std::FILE * const stream = stdout,
		std::source_location && l = std::source_location::current())
	{
		::_lock_file(stream);

		// https://stackoverflow.com/questions/76106361/stdformating-stdchrono-seconds-without-fractional-digits
		std::print(stream, "{} [{} {}] {}:{}:{} '{}': ",
			std::chrono::system_clock::now(),
			::_getpid(), std::this_thread::get_id(),
			l.file_name(), l.line(), l.column(), l.function_name());

		if constexpr (std::invocable<F, std::FILE *>) {
			std::invoke(std::forward<F>(f), auto{stream});
		} else {
			std::print(stream, "{}", std::forward<F>(f));
		}

		std::println(stream);
		std::fflush(stream);

		::_unlock_file(stream);
	}

	template<class U, class F = std::identity>
	constexpr auto never(const bool cond,
		U && u,
		F && f = default_value,
		std::FILE * const stream = stdout,
		std::source_location && l = std::source_location::current())
	{
		if (cond) {
			log(std::forward<F>(f), stream, std::move(l));

			if constexpr (invocable_not_r<U, void>
				)	return std::expected<void, std::invoke_result_t<U>>{std::unexpect, std::invoke(std::forward<U>(u))};
			else	return std::expected<void, U>{std::unexpect, std::forward<U>(u)};
		} else {
			if constexpr (invocable_not_r<U, void>
				)	return c(std::expected<void, std::invoke_result_t<U>>{std::in_place});
			else	return c(std::expected<void, U>{std::in_place});
		}
	}

	template<class U, class F = std::identity>
	constexpr auto alway(const bool cond,
		U && u,
		F && f = default_value,
		std::FILE * const stream = stdout,
		std::source_location && l = std::source_location::current())
	{
		return never(!cond, std::forward<U>(u), std::forward<F>(f), stream, std::move(l));
	}

}
