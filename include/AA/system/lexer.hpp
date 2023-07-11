#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/io.hpp"
#include "../algorithm/hash.hpp"
#include <string> // string



namespace aa {

	enum struct lexing_state : size_t {
		BEFORE_KEY, KEY, KEY_SPACE,
		VALUE, SKIP_VALUE
	};

	template<instance_of_twnttp<string_perfect_hash> H>
	struct lexer_config : H::pack_type {
		// Special member functions
		AA_CONSTEXPR lexer_config(const size_t r1 = 64, const size_t r2 = 64) {
			token.reserve(r1);
			whitespace.reserve(r2);
		}



		// Member types
		using hasher_type = H;

	protected:
		template<not_const_and_instance_of_twttp<lexer_config>, class>
		friend struct lexer;



		// Member objects
		lexing_state state = lexing_state::BEFORE_KEY;
		std::string token, whitespace;
	};

	// Lexing parameters
	// Būvo idėja realizuoti escape sequences, bet faile galima tiesiog įterpti pavyzdžiui naują eilutę todėl jų neprireikia.
	// Teksto eilutės reikšmėse pirminiame kode to padaryti negalima todėl tokiame kontekste yra reikalingos escape sequences.
	template<not_const_and_instance_of_twttp<lexer_config> CONFIG, class CONSUMER>
	struct lexer {
		// Member types
		using config_type = std::remove_reference_t<CONFIG>;
		using consumer_type = std::remove_cvref_t<CONSUMER>;
		using hasher_type = typename config_type::hasher_type;



		// Special member functions
		template<constructible_to<CONFIG> C1 = CONFIG, constructible_to<CONSUMER> C2 = CONSUMER>
		AA_CONSTEXPR lexer(C1 &&c1, C2 &&c2 = default_value) : config{std::forward<C1>(c1)}, consumer{std::forward<C2>(c2)} {}



		// Member objects
	protected:
		CONFIG config;
		[[no_unique_address]] const CONSUMER consumer;
		void(consumer_type:: *target)(const std::string &) const;



		// Member functions
		AA_CONSTEXPR void init_key() {
			constant_v<hasher_type>(config.token, [&]<size_t I> -> void {
				if constexpr (I != std::tuple_size_v<hasher_type>) {
					// Metodų rodyklių dydis (16 baitai) yra didesnis negu size_t tipo (8 baitai).
					target = &consumer_type::template operator()<I>;
					config.state = lexing_state::VALUE;
				} else {
					config.state = lexing_state::SKIP_VALUE;
				}
			});
		}

	public:
		AA_CONSTEXPR void operator()(const int character) {
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
		}
	};

	template<class C1, class C2>
	lexer(C1 &&, C2 &&) -> lexer<C1, C2>;



	// https://en.wikipedia.org/wiki/Lexical_analysis
	// Nereikalaujame, kad file kintamasis su savimi neštųsi failo kelią, nes šioje funkcijoje kelio mums nereikia.
	// Patariama pačiam naudoti naudotojui pathed_stream klasę, nes ji automatiškai taip pat patikrina failed state.
	template<not_const_and_instance_of_twttp<lexer> LEXER, char_input_stream FILE>
	AA_CONSTEXPR void lex(FILE &&file, LEXER &&lexer) {
		// Konstruktorius nenustato eofbit jei failas tuščias todėl reikia šio tikrinimo.
		if (file.peek() == traits_type_in_use_t<FILE>::eof())
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
			const int_type_in_use_t<FILE> character = file.get();

			switch (state) {
				case lexing_state::NONE:
					switch (character) {
						case '/':
							state = lexing_state::CHECK;
							continue;
						default:
							lexer(character);
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
							lexer('/');
							lexer(character);
							continue;
					}

				case lexing_state::COMMENT:
					switch (character) {
						case '\n':
							state = lexing_state::NONE;
							// Pasibaigus paprastam komentarui vis tiek į lekserį turime nusiųsti '\n' simbolį,
							// nes kitaip gali lekseris nepastebėti, kad pasibaigė parametro reikšmės leksema.
							lexer('\n');
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
		} while (file.peek() != traits_type_in_use_t<FILE>::eof());
		// Parametro apibrėžimo forma: PARAMETRO_VARDAS =PARAMETRO_REIKŠMĖ'\n'
		// Svarbu nepamiršti, kad jei parametrą bandoma apibrėžti failo pabaigoje,
		// tai failo paskutinis simbolis turės būti '\n', kitaip parametras nebus apibrėžtas.
	}

}
