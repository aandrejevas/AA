#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/range.hpp"
#include "../metaprogramming/io.hpp"
#include "../algorithm/init.hpp"
#include "evaluator.hpp"
#include <iostream> // cin, cout
#include <istream> // istream
#include <iterator> // output_iterator_tag, input_iterator_tag
#include <format> // format_to, format_string, formatter, format_parse_context, format_context
#include <algorithm> // for_each
#include <ranges> // input_range, range_value_t, empty, begin, drop



namespace aa {

	template<char_traits_like T>
	struct ostreambuf_iter {
		using traits_type = T;
		using int_type = int_type_in_use_t<traits_type>;
		using char_type = char_type_in_use_t<traits_type>;
		using streambuf_type = streambuf_t<ostreambuf_iter>;
		using ostream_type = ostream_t<ostreambuf_iter>;
		using difference_type = ptrdiff_t;
		using value_type = void;
		using reference = void;
		using pointer = void;
		using iterator_category = std::output_iterator_tag;

		constexpr const ostreambuf_iter &operator*() const { return *this; }
		constexpr const ostreambuf_iter &operator++(const int) const { return *this; }
		constexpr ostreambuf_iter &operator++() { return *this; }
		constexpr int_type operator=(const char_type c) const { return file->sputc(c); }

		friend constexpr bool operator==(const ostreambuf_iter &l, const ostreambuf_iter &r) { return l.file == r.file; }

		template<ref_convertible_to<ostream_type &> S>
		constexpr ostreambuf_iter(const S &s) : file{cast<const ostream_type &>(s).rdbuf()} {}

		streambuf_type *file;
	};

	template<ostream_like S>
	ostreambuf_iter(const S &) -> ostreambuf_iter<traits_type_in_use_t<S>>;



	template<char_traits_like T>
	struct istreambuf_iter {
		using traits_type = T;
		using int_type = int_type_in_use_t<traits_type>;
		using char_type = char_type_in_use_t<traits_type>;
		using streambuf_type = streambuf_t<istreambuf_iter>;
		using istream_type = istream_t<istreambuf_iter>;
		using difference_type = ptrdiff_t;
		using value_type = int_type;
		using reference = int_type;
		using pointer = void;
		using iterator_category = std::input_iterator_tag;

		constexpr int_type operator*() const { return file->sgetc(); }
		constexpr istreambuf_iter &operator++() { return (file->sbumpc(), *this); }
		constexpr int_type operator++(const int) const { return file->sbumpc(); }

		friend constexpr bool operator==(const istreambuf_iter &l, const istreambuf_iter &r) { return l.file == r.file; }

		template<ref_convertible_to<istream_type &> S>
		constexpr istreambuf_iter(const S &s) : file{cast<const istream_type &>(s).rdbuf()} {}

		streambuf_type *file;
	};

	template<istream_like S>
	istreambuf_iter(const S &) -> istreambuf_iter<traits_type_in_use_t<S>>;



	// Dėl atributo neturėtų nukentėti greitaveika, nes taip tai standartiniai srautai yra visiems pasiekiami
	// ir atrodo nereiktų jų padavinėti per parametrus, bet template argumentai irgi ne alternatyva.
	template<ostream_like T, class... A>
	constexpr void print(const T &s, const format_string_t<T, const A&...> &fmt, const A&... args) {
		std::format_to(ostreambuf_iter{s}, fmt, args...);
	}

	// Tikriname ar į output_stream galima insert'inti visus paduotus tipus, nes tarp jų
	// gali būti tipų, su kuriais negali susidoroti klasė, kuri paveldi iš basic_ostream.
	//
	// Neturime template parametro, per kurį paduotume eilutės galo simbolį, nes
	// tiesiog galime kaip paprastą parametrą eilutės galo simbolį paduoti.
	template<ostream_like T, class... A>
	constexpr void printl(const T &s, const format_string_t<T, const A&...> &fmt = "", const A&... args) {
		std::format_to(ostreambuf_iter{s}, fmt, args...) = '\n';
	}

	template<ref_convertible_to<std::istream &> S, class EVAL = evaluator, class A>
	constexpr void read(A &a, S &&s, EVAL &&eval = default_value) {
		istreambuf_iter in = {s};
		while (eval(in++));
		eval.evaluate(a);
	}

	template<class T, ref_convertible_to<std::istream &> S, class EVAL = evaluator>
	constexpr std::remove_cvref_t<T> read(S &&s, EVAL &&eval = default_value) {
		return make_with_invocable([&](std::remove_cvref_t<T> &t) -> void { read(t, s, eval); });
	}



	// Čia neatliekamas perfect forwarding, nes atrodo spausdinimui užtenka const kintamųjų. Žinoma gali atsirasti
	// situacijų, kai tai nėra tiesa, pavyzdžiui norint sekti kiek kartų kintamasis buvo išspausdintas. Bet tokiems
	// atvejams klasė gali būti taip parašyta, kad ji ignoruotų const kvalifikatorių.
	template<class... A>
	constexpr void print(const std::format_string<const A&...> &fmt, const A&... args) {
		print(std::cout, fmt, args...);
	}

	template<class... A>
	constexpr void printl(const std::format_string<const A&...> &fmt = "", const A&... args) {
		printl(std::cout, fmt, args...);
	}

	template<class EVAL = evaluator, class A>
	constexpr void read(A &a, EVAL &&eval = default_value) {
		read(a, std::cin, eval);
	}

	template<class T, class EVAL = evaluator>
	constexpr std::remove_cvref_t<T> read(EVAL &&eval = default_value) {
		return read<T>(std::cin, eval);
	}

}



// Kai bus realizuotas P2286R8, nebereikės šios specializacijos.
template<aa::formattable_range R>
struct std::formatter<R> {
	constexpr aa::iterator_in_use_t<std::format_parse_context> parse(const std::format_parse_context &ctx) const {
		return ctx.begin();
	}

	constexpr aa::iterator_in_use_t<std::format_context> format(const R &r, std::format_context &ctx) const {
		if constexpr (std::ranges::input_range<R>) {
			if (std::ranges::empty(r)) {
				return std::format_to(ctx.out(), "[]");
			} else {
				std::format_to(ctx.out(), "[{}", *std::ranges::begin(r));
				std::ranges::for_each(std::views::drop(r, 1), [&](const std::ranges::range_value_t<R> &element) -> void {
					std::format_to(ctx.out(), ", {}", element);
				});
				return std::format_to(ctx.out(), "]");
			}
		} else {
			if constexpr (std::tuple_size_v<R>) {
				std::format_to(ctx.out(), "{{");
				aa::apply<(std::tuple_size_v<R>) - 1>([&]<size_t... I> -> void {
					(std::format_to(ctx.out(), "{}, ", aa::getter_v<I>(r)), ...);
				});
				return std::format_to(ctx.out(), "{}}}", aa::getter_v<(std::tuple_size_v<R>) - 1>(r));
			} else {
				return std::format_to(ctx.out(), "{}");
			}
		}
	}
};
