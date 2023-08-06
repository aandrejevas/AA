#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/io.hpp"
#include "../algorithm/init.hpp"
#include <iostream> // cout, cin
#include <ostream> // ostream, wostream, endl, flush
#include <istream> // istream



namespace aa {

	// Dėl atributo neturėtų nukentėti greitaveika, nes taip tai standartiniai srautai yra visiems pasiekiami
	// ir atrodo nereiktų jų padavinėti per parametrus, bet template argumentai irgi ne alternatyva.
	template<ostream_like T, stream_insertable<T> A1, stream_insertable<T>... A>
	AA_CONSTEXPR borrowed_t<T, ostream_t<T> &> print(T &&s, const A1 &a1, const A&... args) {
		return ((cast<ostream_t<T> &>(s) << a1) << ... << args);
	}

	// Tikriname ar į output_stream galima insert'inti visus paduotus tipus, nes tarp jų
	// gali būti tipų, su kuriais negali susidoroti klasė, kuri paveldi iš basic_ostream.
	//
	// Neturime template parametro, per kurį paduotume eilutės galo simbolį, nes
	// tiesiog galime kaip paprastą parametrą eilutės galo simbolį paduoti.
	template<ostream_like T, stream_insertable<T>... A>
	AA_CONSTEXPR borrowed_t<T, ostream_t<T> &> printl(T &&s, const A&... args) {
		return (cast<ostream_t<T> &>(s) << ... << args) << '\n';
	}

	template<istream_like T, stream_extractable<T> A1, stream_extractable<T>... A>
	AA_CONSTEXPR borrowed_t<T, istream_t<T> &> read(T &&s, A1 &a1, A&... args) {
		return ((cast<istream_t<T> &>(s) >> a1) >> ... >> args);
	}

	template<class T, istream_like S = std::istream &>
	AA_CONSTEXPR std::remove_cvref_t<T> read(S &&s = std::cin) {
		return make_with_invocable([&](std::remove_cvref_t<T> &t) -> void { read(s, t); });
	}



	// Čia neatliekamas perfect forwarding, nes atrodo spausdinimui užtenka const kintamųjų. Žinoma gali atsirasti
	// situacijų, kai tai nėra tiesa, pavyzdžiui norint sekti kiek kartų kintamasis buvo išspausdintas. Bet tokiems
	// atvejams klasė gali būti taip parašyta, kad ji ignoruotų const kvalifikatorių.
	template<stream_insertable A1, stream_insertable... A>
	AA_CONSTEXPR std::ostream &print(const A1 &a1, const A&... args) {
		return print(std::cout, a1, args...);
	}

	template<stream_insertable... A>
	AA_CONSTEXPR std::ostream &printl(const A&... args) {
		return printl(std::cout, args...);
	}

	template<stream_extractable A1, stream_extractable... A>
	AA_CONSTEXPR std::istream &read(A1 &a1, A&... args) {
		return read(std::cin, a1, args...);
	}



	AA_CONSTEXPR std::ostream &endl(std::ostream &os) { return std::endl(os); }
	AA_CONSTEXPR std::wostream &wendl(std::wostream &os) { return std::endl(os); }

	AA_CONSTEXPR std::ostream &flush(std::ostream &os) { return std::flush(os); }
	AA_CONSTEXPR std::wostream &wflush(std::wostream &os) { return std::flush(os); }

}
