#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/io.hpp"
#include "../algorithm/hash.hpp"
#include "../container/fixed_vector.hpp"
#include "read.hpp"
#include <streambuf> // streambuf



namespace aa {

	// https://en.wikipedia.org/wiki/Lexical_analysis
	// Būvo idėja realizuoti escape sequences, bet faile galima tiesiog įterpti pavyzdžiui naują eilutę todėl jų neprireikia.
	// Teksto eilutės reikšmėse pirminiame kode to padaryti negalima todėl tokiame kontekste yra reikalingos escape sequences.
	//
	// Nereikalaujame, kad file kintamasis su savimi neštųsi failo kelią, nes šioje funkcijoje kelio mums nereikia.
	// Patariama pačiam naudoti naudotojui pathed_stream klasę, nes ji automatiškai taip pat patikrina failed state.
	template<string_perfect_hash H, invocable_r_constifier<std::tuple_size_v<const_t<H>>, bool, int> CONSUMER>
	constexpr void lex(std::streambuf &file, CONSUMER &&consumer) {
		// Lexing parameters
		constifier_func_t<CONSUMER> target;

		enum struct lexing_state : size_t {
			KEY,
			VALUE, SKIP_VALUE
		} phase2 = lexing_state::KEY;

		// Nereikia token kaip parametro turėti, nes nieko nekainuoja jį sukonstruoti.
		fixed_vector<char, 100> token;

		const auto lexer = [&](const int character) -> void {
			switch (phase2) {
				case lexing_state::KEY:
					switch (character) {
						default:
							// cast į narrower tipą yra greita operacija ir greitesnės nėra, tik tokio pačio greičio.
							token.insert_back(char_traits_t::to_char_type(character));
							return;
						case ' ': case '\t': case '\n': case '\r': case char_traits_t::eof():
							return;
						case '=':
							H(token, [&]<size_t I> -> void {
								if constexpr (I != std::tuple_size_v<const_t<H>>) {
									// Metodų rodyklių dydis (16 baitai) yra didesnis negu size_t tipo (8 baitai).
									target = constifier_func_v<CONSUMER, I>;
									phase2 = lexing_state::VALUE;
								} else {
									phase2 = lexing_state::SKIP_VALUE;
								}
							});
							token.clear();
							return;
					}

				case lexing_state::VALUE:
					if ((consumer.*target)(character)) return; else {
						switch (character) {
							default:
								phase2 = lexing_state::SKIP_VALUE;
								return;
							case '\n':
								phase2 = lexing_state::KEY;
								return;
						}
					}

				case lexing_state::SKIP_VALUE:
					switch (character) {
						default:
							return;
						case '\n':
							phase2 = lexing_state::KEY;
							return;
					}
			}
			std::unreachable();
		};

		// Lexing comments
		enum struct preprocessing_state : size_t {
			NONE,
			CHECK,
			COMMENT,
			MULTILINE,
			CHECKMULTI
		} phase1 = preprocessing_state::NONE;

		const istreambuf_iter in = {&file};
		do {
			const int character = *in;

			switch (phase1) {
				case preprocessing_state::NONE:
					switch (character) {
						case '/':
							phase1 = preprocessing_state::CHECK;
							goto CONTINUE;
						default:
							lexer(character);
							goto CONTINUE;
					}

				case preprocessing_state::CHECK:
					switch (character) {
						case '/':
							phase1 = preprocessing_state::COMMENT;
							goto CONTINUE;
						case '*':
							phase1 = preprocessing_state::MULTILINE;
							goto CONTINUE;
						default:
							phase1 = preprocessing_state::NONE;
							lexer('/');
							lexer(character);
							goto CONTINUE;
					}

				case preprocessing_state::COMMENT:
					switch (character) {
						case '\n':
							phase1 = preprocessing_state::NONE;
							// Pasibaigus paprastam komentarui vis tiek į lekserį turime nusiųsti '\n' simbolį,
							// nes kitaip gali lekseris nepastebėti, kad pasibaigė parametro reikšmės leksema.
							lexer('\n');
							goto CONTINUE;
						default:
							goto CONTINUE;
					}

				case preprocessing_state::MULTILINE:
					switch (character) {
						case '*':
							phase1 = preprocessing_state::CHECKMULTI;
							goto CONTINUE;
						default:
							goto CONTINUE;
					}

				case preprocessing_state::CHECKMULTI:
					switch (character) {
						case '/':
							phase1 = preprocessing_state::NONE;
							goto CONTINUE;
						default:
							phase1 = preprocessing_state::MULTILINE;
							goto CONTINUE;
					}
			}
			// Neturime default case su unreachable, nes tokiu atveju pasikeitus enum,
			// kompiliatorius mūsų neperspėtų apie case'us, kurie būtų nepanaudoti.
			std::unreachable();

		CONTINUE:
			switch (character) { case char_traits_t::eof(): return; }
		} while ((in++, true));
		// Parametro apibrėžimo forma: PARAMETRO_VARDAS =PARAMETRO_REIKŠMĖ'\n'
	}

}
