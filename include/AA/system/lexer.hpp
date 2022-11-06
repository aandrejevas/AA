#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/io.hpp"
#include "../algorithm/init.hpp"
#include "../container/fixed_immutable_set.hpp"
#include <cstddef> // size_t
#include <string> // string
#include <functional> // invoke, equal_to
#include <utility> // unreachable, forward
#include <type_traits> // remove_reference_t



namespace aa {

	// Būvo idėja realizuoti escape sequences, bet faile galima tiesiog įterpti pavyzdžiui naują eilutę todėl jų neprireikia.
	// Teksto eilutės reikšmėse pirminiame kode to padaryti negalima todėl tokiame kontekste yra reikalingos escape sequences.
	// https://en.wikipedia.org/wiki/Lexical_analysis
	template<fixed_immutable_set S, class E = std::ranges::equal_to, size_t R = 50>
		requires (S.valid())
	struct lexer {
		// Member types
		using container_type = array_t<std::string, S.size()>;
		using comparer_type = E;

		struct lexer_type {
			// Special member functions
			template<class U = comparer_type>
			AA_CONSTEXPR lexer_type(container_type &c, U &&e) : values{c}, comparer{std::forward<U>(e)} {
				if constexpr (R) {
					token.reserve(R);
					whitespace.reserve(R);
				}
			}



		protected:
			// Member objects
			container_type &values;
			[[no_unique_address]] const comparer_type comparer;

			enum struct lexing_state : size_t {
				BEFORE_KEY, KEY, KEY_SPACE,
				VALUE, SKIP_VALUE
			} state = lexing_state::BEFORE_KEY;
			size_t index;
			std::string token, whitespace;



			// Member functions
			AA_CONSTEXPR void init_key() {
				index = S.index(token);
				S.get(index, [&]<in_relation_with<std::string, comparer_type> T>(const T & s) -> void {
					if (std::invoke(comparer, s, token)) {
						state = lexing_state::VALUE;
					} else {
						state = lexing_state::SKIP_VALUE;
					}
				});
			}

		public:
			AA_CONSTEXPR void operator()(const int character) {
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
								// Nedarome move, nes tada reiktų vėl išskirti atmintį token kintamajam.
								values[index] = token;
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



		// Special member functions
		// Nereikalaujame, kad file kintamasis su savimi neštųsi failo kelią, nes šioje funkcijoje kelio mums nereikia.
		// Patariama pačiam naudoti naudotojui pathed_stream klasę, nes ji automatiškai taip pat patikrina failed state.
		template<input_stream T, class U = comparer_type>
		AA_CONSTEXPR lexer(T &&file, U &&e = {}) : values{
				create_with_invocable<container_type>([&](container_type &c) -> void
		{
			using traits_type = typename std::remove_reference_t<T>::traits_type;

			// Konstruktorius nenustato eofbit jei failas tuščias todėl reikia šio tikrinimo.
			if (file.peek() == traits_type::eof())
				return;

			// Lexing parameters
			lexer_type p_lexer = {c, std::forward<U>(e)};

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
								p_lexer(character);
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
								p_lexer('/');
								p_lexer(character);
								continue;
						}

					case lexing_state::COMMENT:
						switch (character) {
							case '\n':
								state = lexing_state::NONE;
								// Pasibaigus paprastam komentarui vis tiek į lekserį turime nusiųsti '\n' simbolį,
								// nes kitaip gali lekseris nepastebėti, kad pasibaigė parametro reikšmės leksema.
								p_lexer('\n');
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
		})} {}



		// Member objects
		const container_type values;
	};

}
