#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/io.hpp"
#include "../container/fixed_vector.hpp"
#include "../algorithm/init.hpp"
#include <iostream> // cin, cout
#include <istream> // istream
#include <iterator> // output_iterator_tag, input_iterator_tag
#include <format> // format_to, format_string



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



	using token_type = fixed_vector<char, 100>;



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
	constexpr void printl(const T &s, const format_string_t<T, const A&...> &fmt, const A&... args) {
		std::format_to(ostreambuf_iter{s}, fmt, args...) = '\n';
	}

	template<istream_like T, stream_extractable<T> A1, stream_extractable<T>... A>
	constexpr borrowed_t<T, istream_t<T> &> read(T &&s, A1 &a1, A&... args) {
		return ((cast<istream_t<T> &>(s) >> a1) >> ... >> args);
	}

	template<class T, istream_like S = std::istream &>
	constexpr std::remove_cvref_t<T> read(S &&s = std::cin) {
		return make_with_invocable([&](std::remove_cvref_t<T> &t) -> void { read(s, t); });
	}



	// Čia neatliekamas perfect forwarding, nes atrodo spausdinimui užtenka const kintamųjų. Žinoma gali atsirasti
	// situacijų, kai tai nėra tiesa, pavyzdžiui norint sekti kiek kartų kintamasis buvo išspausdintas. Bet tokiems
	// atvejams klasė gali būti taip parašyta, kad ji ignoruotų const kvalifikatorių.
	template<class... A>
	constexpr void print(const std::format_string<const A&...> &fmt, const A&... args) {
		print(std:cout, fmt, args...);
	}

	template<class... A>
	constexpr void printl(const std::format_string<const A&...> &fmt, const A&... args) {
		printl(std:cout, fmt, args...);
	}

	template<stream_extractable A1, stream_extractable... A>
	constexpr std::istream &read(A1 &a1, A&... args) {
		return read(std::cin, a1, args...);
	}

}
