#pragma once

#include "../metaprogramming/general.hpp"
#include <iostream> // cout
#include <format> // format_to, format_string, formatter, format_parse_context, format_context
#include <ios> // streamsize



namespace aa {

	template<char_traits_like T = char_traits>
	struct ostreambuf_iter {
		using traits_type = T;
		using int_type = int_type_in_use_t<traits_type>;
		using char_type = char_type_in_use_t<traits_type>;
		using streambuf_type = streambuf_t<ostreambuf_iter>;
		using view_type = string_view_t<ostreambuf_iter>;
		using difference_type = ptrdiff_t;
		using value_type = void;
		using reference = void;
		using pointer = void;
		using iterator_category = std::output_iterator_tag;

		constexpr const ostreambuf_iter &operator*() const { return *this; }
		constexpr const ostreambuf_iter &operator++(const int) const { return *this; }
		constexpr ostreambuf_iter &operator++() { return *this; }
		constexpr int_type operator=(const char_type c) const { return file->sputc(c); }
		constexpr std::streamsize operator=(const view_type c) const { return file->sputn(c.data(), c.size()); }
		constexpr int flush() const { return file->pubsync(); }

		friend constexpr bool operator==(const ostreambuf_iter l, const ostreambuf_iter r) { return l.file == r.file; }

		streambuf_type *file = std::cout.rdbuf();
	};

	template<char_traits_like T>
	ostreambuf_iter(std::basic_streambuf<char_type_in_use_t<T>, T> *const) -> ostreambuf_iter<T>;



	// Dėl atributo neturėtų nukentėti greitaveika, nes taip tai standartiniai srautai yra visiems pasiekiami
	// ir atrodo nereiktų jų padavinėti per parametrus, bet template argumentai irgi ne alternatyva.
	template<char_output_iterator I = ostreambuf_iter<>, class... A>
	constexpr I print(I i, const std::format_string<const A&...> fmt, const A&... args) {
		return std::format_to(i, fmt, args...);
	}

	// Tikriname ar į output_stream galima insert'inti visus paduotus tipus, nes tarp jų
	// gali būti tipų, su kuriais negali susidoroti klasė, kuri paveldi iš basic_ostream.
	//
	// Neturime template parametro, per kurį paduotume eilutės galo simbolį, nes
	// tiesiog galime kaip paprastą parametrą eilutės galo simbolį paduoti.
	template<char_output_iterator I = ostreambuf_iter<>, class... A>
	constexpr I printl(I i, const std::format_string<const A&...> fmt = "", const A&... args) {
		*(i = std::format_to(i, fmt, args...)) = '\n';
		return ++i;
	}

	// Čia neatliekamas perfect forwarding, nes atrodo spausdinimui užtenka const kintamųjų. Žinoma gali atsirasti
	// situacijų, kai tai nėra tiesa, pavyzdžiui norint sekti kiek kartų kintamasis buvo išspausdintas. Bet tokiems
	// atvejams klasė gali būti taip parašyta, kad ji ignoruotų const kvalifikatorių.
	template<class... A>
	constexpr ostreambuf_iter<> print(const std::format_string<const A&...> fmt, const A&... args) {
		return print({}, fmt, args...);
	}

	template<class... A>
	constexpr ostreambuf_iter<> printl(const std::format_string<const A&...> fmt = "", const A&... args) {
		return printl({}, fmt, args...);
	}

}



// Kai bus realizuotas P2286R8, nebereikės šios specializacijos.
template<aa::formattable_range R>
struct std::formatter<R> {
	static constexpr aa::iterator_in_use_t<std::format_parse_context> parse(const std::format_parse_context &ctx) {
		return ctx.begin();
	}

	static constexpr aa::iterator_in_use_t<std::format_context> format(const R &r, std::format_context &ctx) {
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
					(std::format_to(ctx.out(), "{}, ", aa::get_v<I>(r)), ...);
				});
				return std::format_to(ctx.out(), "{}}}", aa::get_v<(std::tuple_size_v<R>) - 1>(r));
			} else {
				return std::format_to(ctx.out(), "{{}}");
			}
		}
	}
};
