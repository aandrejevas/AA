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
		// const managed<std::FILE *, ::_unlock_file> _ = stream;

		// https://stackoverflow.com/questions/76106361/stdformating-stdchrono-seconds-without-fractional-digits
		std::print(stream, "{} [{} {}] {}:{}:{} '{}': ",
			std::chrono::system_clock::now(),
			::_getpid(), std::this_thread::get_id(),
			l.file_name(), l.line(), l.column(), l.function_name());

		if constexpr (std::invocable<F, std::FILE * const &>) {
			if constexpr (!invocable_r<F, void, std::FILE * const &>) {
				std::invoke(std::forward<F>(f), stream);
			}
			std::invoke(std::forward<F>(f), stream);
		} else {
			std::print(stream, "{}", std::forward<F>(f));
		}
		// return stream;

		std::println(stream);
		std::fflush(stream);
		::_unlock_file(stream);
	}

	template<class F = std::identity>
	constexpr bool alway(const bool cond,
		F && f = default_value,
		std::FILE * const stream = stdout,
		std::source_location && l = std::source_location::current())
	{
		if (!cond) log(std::forward<F>(f), stream, std::move(l));
		return !cond;
	}

	template<class F = std::identity>
	constexpr std::expected never(const bool cond,
		F && f = default_value,
		std::FILE * const stream = stdout,
		std::source_location && l = std::source_location::current())
	{
		if (cond) return log(std::forward<F>(f), stream, std::move(l));
		return cond;
	}

}
