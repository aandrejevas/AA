#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/io.hpp"
#include "../preprocessor/assert.hpp"
#include "../algorithm/init.hpp"
#include "evaluator.hpp"
#include <iostream> // cin
#include <fstream> // filebuf
#include <ios> // ios_base
#include <iterator> // input_iterator_tag
#include <streambuf> // streambuf



namespace aa {

	template<char_traits_like T>
	struct istreambuf_iter {
		using traits_type = T;
		using int_type = int_type_in_use_t<traits_type>;
		using char_type = char_type_in_use_t<traits_type>;
		using streambuf_type = streambuf_t<istreambuf_iter>;
		using difference_type = ptrdiff_t;
		using value_type = int_type;
		using reference = int_type;
		using pointer = void;
		using iterator_category = std::input_iterator_tag;

		constexpr int_type operator*() const { return file->sgetc(); }
		constexpr istreambuf_iter &operator++() { return (file->sbumpc(), *this); }
		constexpr void operator++(const int) const { file->sbumpc(); }
		constexpr istreambuf_iter &operator--() { return (file->sungetc(), *this); }
		constexpr void operator--(const int) const { file->sungetc(); }

		friend constexpr bool operator==(const istreambuf_iter &l, const istreambuf_iter &r) { return l.file == r.file; }

		constexpr istreambuf_iter(streambuf_type &s) : file{&s} {}

		streambuf_type *file;
	};

	template<streambuf_like S>
	istreambuf_iter(const S &) -> istreambuf_iter<traits_type_in_use_t<S>>;



	// eval turi būti paduotas kaip parametras, nes yra situacijų, kai EVAL gali būti ne paprasta klasė.
	template<class EVAL = evaluator, evaluable_by<EVAL &>... A>
		requires (!!sizeof...(A))
	constexpr void read(std::streambuf &s, EVAL &&eval, A&... a) {
		const istreambuf_iter in = {s};
		(([&] -> void {
			do {
				const int character = *in;
				switch (character) {
					case char_traits_t::eof():
						eval(char_traits_t::eof(), a);
						return;
					default:
						if (eval(character, a)) continue; else return;
				}
			} while ((in++, true));
		})(), ...);
	}

	template<class T, evaluator_for<T> EVAL = evaluator>
	constexpr std::remove_cvref_t<T> read(EVAL &&eval, std::streambuf &s = *std::cin.rdbuf()) {
		return make_with_invocable([&](std::remove_cvref_t<T> &t) -> void { read(s, eval, t); });
	}

	template<class EVAL = evaluator, evaluable_by<EVAL &>... A>
		requires (!!sizeof...(A))
	constexpr void read(EVAL &&eval, A&... a) {
		read(*std::cin.rdbuf(), eval, a...);
	}



	template<std::ios_base::openmode MODE, class T>
	constexpr std::filebuf make_filebuf(const T &path) {
		std::filebuf file;
		AA_TRACE_ASSERT(file.open(path, MODE));
		return file;
	}

	template<std::ios_base::openmode MODE = std::ios_base::out, class T>
	constexpr std::filebuf make_ofilebuf(const T &path) {
		return make_filebuf<MODE | std::ios_base::out>(path);
	}

	template<std::ios_base::openmode MODE = std::ios_base::in, class T>
	constexpr std::filebuf make_ifilebuf(const T &path) {
		return make_filebuf<MODE | std::ios_base::in>(path);
	}

}
