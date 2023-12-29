#pragma once

#include "../metaprogramming/general.hpp"
#include "../preprocessor/assert.hpp"
#include "evaluator.hpp"
#include <iostream> // cin
#include <fstream> // filebuf
#include <ios> // ios_base, streamsize



namespace aa {

	template<char_traits_like T = char_traits>
	struct istreambuf_iter {
		using traits_type = T;
		using int_type = int_type_in_use_t<traits_type>;
		using char_type = char_type_in_use_t<traits_type>;
		using streambuf_type = streambuf_t<istreambuf_iter>;
		template<size_t N>
		using span_type = span_t<istreambuf_iter, N>;
		using difference_type = ptrdiff_t;
		using value_type = int_type;
		using reference = int_type;
		using pointer = void;
		using iterator_category = std::input_iterator_tag;

		struct proxy {
			constexpr int_type operator*() const { return value; }

			const int_type value;
		};

		template<size_t N>
		constexpr std::streamsize operator()(const span_type<N> c) const { return file->sgetn(c.data(), c.size()); }
		constexpr int_type operator*() const { return file->sgetc(); }
		constexpr istreambuf_iter &operator++() { return (file->sbumpc(), *this); }
		constexpr proxy operator++(const int) const { return {file->sbumpc()}; }
		constexpr istreambuf_iter &operator--() { return (file->sungetc(), *this); }
		constexpr proxy operator--(const int) const { return {file->sungetc()}; }
		constexpr int sync() const { return file->pubsync(); }

		friend constexpr bool operator==(const istreambuf_iter l, const istreambuf_iter r) { return l.file == r.file; }

		streambuf_type *file = std::cin.rdbuf();
	};

	template<char_traits_like T>
	istreambuf_iter(std::basic_streambuf<char_type_in_use_t<T>, T> *const) -> istreambuf_iter<T>;



	// eval turi būti paduotas kaip parametras, nes yra situacijų, kai EVAL gali būti ne paprasta klasė.
	template<class EVAL = evaluator, int_input_iterator I = istreambuf_iter<>, evaluable_by<EVAL &> A>
	constexpr I read(A &a, I i = default_value, EVAL &&eval = default_value) {
		do {
			const int character = *i;
			switch (character) {
				case char_traits::eof():
					eval(char_traits::eof(), a);
					break;
				default:
					if (eval(character, a)) continue; else break;
			}
			return i;
		} while ((++i, true));
	}

	template<not_cvref T, evaluator_for<T> EVAL = evaluator, int_input_iterator I = istreambuf_iter<>>
	constexpr pair<T, I> read(I i = default_value, EVAL &&eval = default_value) {
		return make_with_invocable([&](pair<T, I> &t) -> void {
			get_1(t) = read(get_0(t), i, eval);
		});
	}

	template<int DELIM = '\n', int_input_iterator I = istreambuf_iter<>>
	constexpr I ignore(I i) {
		do {
			switch (*i) {
				case char_traits::eof():	return i;
				case DELIM:					return ++i;
			}
		} while ((++i, true));
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



	namespace detail {
		[[gnu::constructor]] constexpr void init() {
			std::ios_base::sync_with_stdio(false);
			AA_IF_DEBUG(AA_LOG({}, "Init function called."));
		}
	}

}
