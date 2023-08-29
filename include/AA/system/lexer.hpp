#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/io.hpp"
#include "../algorithm/hash.hpp"
#include "print.hpp"
#include <istream> // istream



namespace aa {

	// https://en.wikipedia.org/wiki/Lexical_analysis
	// Būvo idėja realizuoti escape sequences, bet faile galima tiesiog įterpti pavyzdžiui naują eilutę todėl jų neprireikia.
	// Teksto eilutės reikšmėse pirminiame kode to padaryti negalima todėl tokiame kontekste yra reikalingos escape sequences.
	//
	// Nereikalaujame, kad file kintamasis su savimi neštųsi failo kelią, nes šioje funkcijoje kelio mums nereikia.
	// Patariama pačiam naudoti naudotojui pathed_stream klasę, nes ji automatiškai taip pat patikrina failed state.
	template<instance_of<string_perfect_hash<>> auto H, invocable_constifier<std::tuple_size_v<const_t<H>>, const token_type &> CONSUMER, ref_convertible_to<std::istream &> FILE>
	constexpr void lex(const FILE &file, CONSUMER &&consumer) {
		// Lexing parameters
		constifier_func_t<CONSUMER> target;

		enum struct lexing_state : size_t {
			BEFORE_KEY, KEY, KEY_SPACE,
			VALUE, SKIP_VALUE
		} phase2 = lexing_state::BEFORE_KEY;

		token_type token, whitespace;

		const auto init_key = [&]() -> void {
			H(token, [&]<size_t I> -> void {
				if constexpr (I != std::tuple_size_v<const_t<H>>) {
					// Metodų rodyklių dydis (16 baitai) yra didesnis negu size_t tipo (8 baitai).
					target = constifier_func_v<CONSUMER, I>;
					phase2 = lexing_state::VALUE;
				} else {
					phase2 = lexing_state::SKIP_VALUE;
				}
			});
		};

		const auto lexer = [&](const int character) -> void {
			switch (phase2) {
				case lexing_state::BEFORE_KEY:
					switch (character) {
						case '=': init_key();
						case ' ': case '\t': case '\n': case '\r': return;
						default:
							phase2 = lexing_state::KEY;
							// cast į narrower tipą yra greita operacija ir greitesnės nėra, tik tokio pačio greičio.
							token.insert_back(static_cast<char>(character));
							return;
					}

				case lexing_state::KEY:
					switch (character) {
						default:
							token.insert_back(static_cast<char>(character));
							return;
						case ' ': case '\t':
							phase2 = lexing_state::KEY_SPACE;
							whitespace.insert_back(static_cast<char>(character));
							return;
						case '=':
							init_key();
							token.clear();
							return;
					}

				case lexing_state::KEY_SPACE:
					switch (character) {
						default:
							phase2 = lexing_state::KEY;
							token.append_range(whitespace);
							token.insert_back(static_cast<char>(character));
							whitespace.clear();
							return;
						case ' ': case '\t':
							whitespace.insert_back(static_cast<char>(character));
							return;
						case '=':
							init_key();
							token.clear();
							whitespace.clear();
							return;
					}

				case lexing_state::VALUE:
					switch (character) {
						default:
							token.insert_back(static_cast<char>(character));
							return;
						case '\n':
							phase2 = lexing_state::BEFORE_KEY;
							(consumer.*target)(std::as_const(token));
							token.clear();
							return;
					}

				case lexing_state::SKIP_VALUE:
					switch (character) {
						default:
							return;
						case '\n':
							phase2 = lexing_state::BEFORE_KEY;
							return;
					}

				default:
					std::unreachable();
			}
		};

		// Lexing comments
		enum struct preprocessing_state : size_t {
			NONE,
			CHECK,
			COMMENT,
			MULTILINE,
			CHECKMULTI
		} phase1 = preprocessing_state::NONE;

		istreambuf_iter in = {file};
		do {
			const int_type_in_use_t<std::istream> character = in++;

			using traits_type = traits_type_in_use_t<std::istream>;
			if (traits_type::eq_int_type(character, traits_type::eof())) break;

			switch (phase1) {
				case preprocessing_state::NONE:
					switch (character) {
						case '/':
							phase1 = preprocessing_state::CHECK;
							continue;
						default:
							lexer(character);
							continue;
					}

				case preprocessing_state::CHECK:
					switch (character) {
						case '/':
							phase1 = preprocessing_state::COMMENT;
							continue;
						case '*':
							phase1 = preprocessing_state::MULTILINE;
							continue;
						default:
							phase1 = preprocessing_state::NONE;
							lexer('/');
							lexer(character);
							continue;
					}

				case preprocessing_state::COMMENT:
					switch (character) {
						case '\n':
							phase1 = preprocessing_state::NONE;
							// Pasibaigus paprastam komentarui vis tiek į lekserį turime nusiųsti '\n' simbolį,
							// nes kitaip gali lekseris nepastebėti, kad pasibaigė parametro reikšmės leksema.
							lexer('\n');
							continue;
						default:
							continue;
					}

				case preprocessing_state::MULTILINE:
					switch (character) {
						case '*':
							phase1 = preprocessing_state::CHECKMULTI;
							continue;
						default:
							continue;
					}

				case preprocessing_state::CHECKMULTI:
					switch (character) {
						case '/':
							phase1 = preprocessing_state::NONE;
							continue;
						default:
							phase1 = preprocessing_state::MULTILINE;
							continue;
					}

				default:
					std::unreachable();
			}
		} while (true);
		// Parametro apibrėžimo forma: PARAMETRO_VARDAS =PARAMETRO_REIKŠMĖ'\n'
		// Svarbu nepamiršti, kad jei parametrą bandoma apibrėžti failo pabaigoje,
		// tai failo paskutinis simbolis turės būti '\n', kitaip parametras nebus apibrėžtas.
	}

}
