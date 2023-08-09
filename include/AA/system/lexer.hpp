#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/io.hpp"
#include "../algorithm/hash.hpp"
#include <string> // string
#include <istream> // istream



namespace aa {

	enum struct lexing_state : size_t {
		BEFORE_KEY, KEY, KEY_SPACE,
		VALUE, SKIP_VALUE
	};

	struct lexer_config {
		// Special member functions
		AA_CONSTEXPR lexer_config(const size_t r1 = 64, const size_t r2 = 64) {
			token.reserve(r1);
			whitespace.reserve(r2);
		}



		// Member objects
		lexing_state state = lexing_state::BEFORE_KEY;
		std::string token, whitespace;
	};

	// https://en.wikipedia.org/wiki/Lexical_analysis
	// Būvo idėja realizuoti escape sequences, bet faile galima tiesiog įterpti pavyzdžiui naują eilutę todėl jų neprireikia.
	// Teksto eilutės reikšmėse pirminiame kode to padaryti negalima todėl tokiame kontekste yra reikalingos escape sequences.
	//
	// Nereikalaujame, kad file kintamasis su savimi neštųsi failo kelią, nes šioje funkcijoje kelio mums nereikia.
	// Patariama pačiam naudoti naudotojui pathed_stream klasę, nes ji automatiškai taip pat patikrina failed state.
	template<instance_of<string_perfect_hash<>> auto H, invocable_constifier<std::tuple_size_v<const_t<H>>, const std::string &> CONSUMER, ref_convertible_to<std::istream &> FILE, wo_ref_same_as<lexer_config> CONFIG = lexer_config>
	AA_CONSTEXPR void lex(FILE &&_file, CONFIG &&config, CONSUMER &&consumer) {
		// Konstruktorius nenustato eofbit jei failas tuščias todėl reikia šio tikrinimo.
		std::istream &file = cast<std::istream &>(_file);
		if (file.peek() == traits_type_in_use_t<std::istream>::eof())
			return;

		// Lexing parameters
		constifier_func_t<CONSUMER> target;

		const auto init_key = [&]() -> void {
			H(config.token, [&]<size_t I> -> void {
				if constexpr (I != std::tuple_size_v<const_t<H>>) {
					// Metodų rodyklių dydis (16 baitai) yra didesnis negu size_t tipo (8 baitai).
					target = constifier_func_v<CONSUMER, I>;
					config.state = lexing_state::VALUE;
				} else {
					config.state = lexing_state::SKIP_VALUE;
				}
			});
		};

		const auto lexer = [&](const int character) -> void {
			switch (config.state) {
				case lexing_state::BEFORE_KEY:
					switch (character) {
						case '=': init_key();
						case ' ': case '\t': case '\n': case '\r': return;
						default:
							config.state = lexing_state::KEY;
							// cast į narrower tipą yra greita operacija ir greitesnės nėra, tik tokio pačio greičio.
							config.token.push_back(static_cast<char>(character));
							return;
					}

				case lexing_state::KEY:
					switch (character) {
						default:
							config.token.push_back(static_cast<char>(character));
							return;
						case ' ': case '\t':
							config.state = lexing_state::KEY_SPACE;
							config.whitespace.push_back(static_cast<char>(character));
							return;
						case '=':
							init_key();
							config.token.clear();
							return;
					}

				case lexing_state::KEY_SPACE:
					switch (character) {
						default:
							config.state = lexing_state::KEY;
							config.token.append(config.whitespace).push_back(static_cast<char>(character));
							config.whitespace.clear();
							return;
						case ' ': case '\t':
							config.whitespace.push_back(static_cast<char>(character));
							return;
						case '=':
							init_key();
							config.token.clear();
							config.whitespace.clear();
							return;
					}

				case lexing_state::VALUE:
					switch (character) {
						default:
							config.token.push_back(static_cast<char>(character));
							return;
						case '\n':
							config.state = lexing_state::BEFORE_KEY;
							(consumer.*target)(config.token);
							config.token.clear();
							return;
					}

				case lexing_state::SKIP_VALUE:
					switch (character) {
						default:
							return;
						case '\n':
							config.state = lexing_state::BEFORE_KEY;
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
		} state = preprocessing_state::NONE;

		do {
			const int_type_in_use_t<std::istream> character = file.get();

			switch (state) {
				case preprocessing_state::NONE:
					switch (character) {
						case '/':
							state = preprocessing_state::CHECK;
							continue;
						default:
							lexer(character);
							continue;
					}

				case preprocessing_state::CHECK:
					switch (character) {
						case '/':
							state = preprocessing_state::COMMENT;
							continue;
						case '*':
							state = preprocessing_state::MULTILINE;
							continue;
						default:
							state = preprocessing_state::NONE;
							lexer('/');
							lexer(character);
							continue;
					}

				case preprocessing_state::COMMENT:
					switch (character) {
						case '\n':
							state = preprocessing_state::NONE;
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
							state = preprocessing_state::CHECKMULTI;
							continue;
						default:
							continue;
					}

				case preprocessing_state::CHECKMULTI:
					switch (character) {
						case '/':
							state = preprocessing_state::NONE;
							continue;
						default:
							state = preprocessing_state::MULTILINE;
							continue;
					}

				default:
					std::unreachable();
			}
		} while (file.peek() != traits_type_in_use_t<std::istream>::eof());
		// Parametro apibrėžimo forma: PARAMETRO_VARDAS =PARAMETRO_REIKŠMĖ'\n'
		// Svarbu nepamiršti, kad jei parametrą bandoma apibrėžti failo pabaigoje,
		// tai failo paskutinis simbolis turės būti '\n', kitaip parametras nebus apibrėžtas.
	}

}
