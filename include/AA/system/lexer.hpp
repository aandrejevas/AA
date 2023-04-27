#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/io.hpp"
#include "../algorithm/hash.hpp"
#include <string> // string



namespace aa {

	// Lexing parameters
	// Būvo idėja realizuoti escape sequences, bet faile galima tiesiog įterpti pavyzdžiui naują eilutę todėl jų neprireikia.
	// Teksto eilutės reikšmėse pirminiame kode to padaryti negalima todėl tokiame kontekste yra reikalingos escape sequences.
	template<instance_of_twnttp<string_perfect_hash> H>
	struct param_lexer : H::pack_type {
		// Special member functions
		AA_CONSTEXPR param_lexer(const size_t r1 = 50, const size_t r2 = 50) {
			token.reserve(r1);
			whitespace.reserve(r2);
		}



		// Member types
		using hasher_type = H;



		// Member objects
	protected:
		enum struct lexing_state : size_t {
			BEFORE_KEY, KEY, KEY_SPACE,
			VALUE, SKIP_VALUE
		} state = lexing_state::BEFORE_KEY;
		std::string token, whitespace;
		size_t index;



		// Member functions
		AA_CONSTEXPR void init_key() {
			constant_v<hasher_type>(token, [&]<size_t I>() -> void {
				if constexpr (I != std::tuple_size_v<hasher_type>) {
					// Anksčiau čia kopijuodavome metodų rodykles, o ne indeksus, bet buvo labai nepatogu talpinti
					// tą rodyklę ir metodų rodyklių dydis (16 baitų) yra didesnis negu size_t tipo (8 baitai).
					index = I;
					state = lexing_state::VALUE;
				} else
					state = lexing_state::SKIP_VALUE;
			});
		}

	public:
		template<class C>
		AA_CONSTEXPR void operator()(const int character, C &&consumer) {
			switch (state) {
				case lexing_state::BEFORE_KEY:
					switch (character) {
						case '=': init_key();
						case ' ': case '\t': case '\n': case '\r': return;
						default:
							state = lexing_state::KEY;
							// cast į narrower tipą yra greita operacija ir greitesnės nėra, tik tokio pačio greičio.
							token.push_back(static_cast<char>(character));
							return;
					}

				case lexing_state::KEY:
					switch (character) {
						default:
							token.push_back(static_cast<char>(character));
							return;
						case ' ': case '\t':
							state = lexing_state::KEY_SPACE;
							whitespace.push_back(static_cast<char>(character));
							return;
						case '=':
							init_key();
							token.clear();
							return;
					}

				case lexing_state::KEY_SPACE:
					switch (character) {
						default:
							state = lexing_state::KEY;
							token.append(whitespace).push_back(static_cast<char>(character));
							whitespace.clear();
							return;
						case ' ': case '\t':
							whitespace.push_back(static_cast<char>(character));
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
							token.push_back(static_cast<char>(character));
							return;
						case '\n':
							state = lexing_state::BEFORE_KEY;
							constify<hasher_type>(index, std::forward<C>(consumer), std::as_const(token));
							token.clear();
							return;
					}

				case lexing_state::SKIP_VALUE:
					switch (character) {
						default:
							return;
						case '\n':
							state = lexing_state::BEFORE_KEY;
							return;
					}

				default:
					std::unreachable();
			}
		}
	};

	// https://en.wikipedia.org/wiki/Lexical_analysis
	// Nereikalaujame, kad file kintamasis su savimi neštųsi failo kelią, nes šioje funkcijoje kelio mums nereikia.
	// Patariama pačiam naudoti naudotojui pathed_stream klasę, nes ji automatiškai taip pat patikrina failed state.
	template<class C, not_const_instance_of_twttp<param_lexer> LEXER, input_stream FILE>
	AA_CONSTEXPR void lex(FILE &&file, LEXER &&lexer, C &&consumer = default_value) {
		using traits_type = typename std::remove_reference_t<FILE>::traits_type;

		// Konstruktorius nenustato eofbit jei failas tuščias todėl reikia šio tikrinimo.
		if (file.peek() == traits_type::eof())
			return;

		// Lexing comments
		enum struct lexing_state : size_t {
			NONE,
			CHECK,
			COMMENT,
			MULTILINE,
			CHECKMULTI
		} state = lexing_state::NONE;

		do {
			const typename traits_type::int_type character = file.get();

			switch (state) {
				case lexing_state::NONE:
					switch (character) {
						case '/':
							state = lexing_state::CHECK;
							continue;
						default:
							lexer(character, consumer);
							continue;
					}

				case lexing_state::CHECK:
					switch (character) {
						case '/':
							state = lexing_state::COMMENT;
							continue;
						case '*':
							state = lexing_state::MULTILINE;
							continue;
						default:
							state = lexing_state::NONE;
							lexer('/', consumer);
							lexer(character, consumer);
							continue;
					}

				case lexing_state::COMMENT:
					switch (character) {
						case '\n':
							state = lexing_state::NONE;
							// Pasibaigus paprastam komentarui vis tiek į lekserį turime nusiųsti '\n' simbolį,
							// nes kitaip gali lekseris nepastebėti, kad pasibaigė parametro reikšmės leksema.
							lexer('\n', consumer);
							continue;
						default:
							continue;
					}

				case lexing_state::MULTILINE:
					switch (character) {
						case '*':
							state = lexing_state::CHECKMULTI;
							continue;
						default:
							continue;
					}

				case lexing_state::CHECKMULTI:
					switch (character) {
						case '/':
							state = lexing_state::NONE;
							continue;
						default:
							state = lexing_state::MULTILINE;
							continue;
					}

				default:
					std::unreachable();
			}
		} while (file.peek() != traits_type::eof());
		// Parametro apibrėžimo forma: PARAMETRO_VARDAS =PARAMETRO_REIKŠMĖ'\n'
		// Svarbu nepamiršti, kad jei parametrą bandoma apibrėžti failo pabaigoje,
		// tai failo paskutinis simbolis turės būti '\n', kitaip parametras nebus apibrėžtas.
	}

}
