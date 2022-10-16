#pragma once

#include "../preprocessor/assert.hpp"
#include "../metaprogramming/general.hpp"
#include "../metaprogramming/range.hpp"
#include <cstddef> // size_t
#include <string> // string, hash
#include <variant> // variant, monostate, get_if
#include <unordered_map> // unordered_map
#include <initializer_list> // initializer_list
#include <functional> // invoke, hash
#include <string_view> // string_view, hash
#include <utility> // unreachable, forward
#include <bit> // bit_cast
#include <istream> // basic_istream



namespace aa {

	// Būvo idėja realizuoti escape sequences, bet faile galima	tiesiog įterpti pavyzdžiui naują eilutę todėl jų neprireikia.
	// Teksto eilutės reikšmėse pirminiame kode to padaryti negalima todėl tokiame kontekste yra reikalingos escape sequences.
	// https://en.wikipedia.org/wiki/Lexical_analysis
	template<storable_evaluator... A>
		requires (are_unique_v<std::monostate, evaluator_result_t<A>...>)
	struct lexer {
		// Member types
		using params_map = std::unordered_map<std::string, std::variant<std::monostate, evaluator_result_t<A>...>,
			overload<std::hash<std::string>, std::hash<std::string_view>>, char_equal_to>;

	protected:
		struct params_lexer {
			params_map &params;

			template<class T>
			AA_CONSTEXPR void evaluate() const {
				params_iter->second.template emplace<evaluator_result_t<T>>(
					std::invoke(evaluators.template get<T>(), token.operator std::string_view()));
			}

			using types_map = std::unordered_map<std::string_view, void(params_lexer:: *)() const,
				typename params_map::hasher, typename params_map::key_equal>;

			[[no_unique_address]] const tuple<A...> evaluators;
			const types_map types = {std::initializer_list<typename types_map::value_type>{
				typename types_map::value_type{type_name<evaluator_result_t<A>>(), &params_lexer::evaluate<A>}...
			}};

			enum struct lexing_state : size_t {
				BEFORE_TYPE, TYPE, TYPE_SPACE,
				BEFORE_KEY, KEY, KEY_SPACE,
				VALUE
			} state = lexing_state::BEFORE_TYPE;
			// Jeigu deklaruotume savo konstruktorius, galėtume inicializuoti iteratorius su default
			// initialization, bet to nedarome, nes niekas nepasikeistų, nes naudojamų stl konteinerių
			// default konstruktoriai nėra trivial. Todėl params_lexer tipas gali likti aggregate.
			typename types_map::const_iterator types_iter = {};
			typename params_map::iterator params_iter = {};
			std::string token = {}, whitespace = {};



			AA_CONSTEXPR void init_type() {
				state = lexing_state::BEFORE_KEY;
				types_iter = types.template find<std::string>(token);
				token.clear();
			}

			AA_CONSTEXPR void init_key() {
				state = lexing_state::VALUE;
				params_iter = params.try_emplace(token).first;
				token.clear();
			}

			AA_CONSTEXPR void operator()(const int character) {
				switch (state) {
					case lexing_state::BEFORE_TYPE:
						switch (character) {
							case '-': abort<AA_SOURCE_DATA>();
							case ' ': case '\t': case '\n': case '\r': return;
							default:
								state = lexing_state::TYPE;
								token.push_back(*std::bit_cast<const char *>(&character));
								return;
						}

					case lexing_state::TYPE:
						switch (character) {
							default:
								token.push_back(*std::bit_cast<const char *>(&character));
								return;
							case ' ': case '\t':
								state = lexing_state::TYPE_SPACE;
								whitespace.push_back(*std::bit_cast<const char *>(&character));
								return;
							case '-':
								init_type();
								return;
						}

					case lexing_state::TYPE_SPACE:
						switch (character) {
							default:
								state = lexing_state::TYPE;
								token.append(whitespace).push_back(*std::bit_cast<const char *>(&character));
								whitespace.clear();
								return;
							case ' ': case '\t':
								whitespace.push_back(*std::bit_cast<const char *>(&character));
								return;
							case '-':
								init_type();
								whitespace.clear();
								return;
						}

					case lexing_state::BEFORE_KEY:
						switch (character) {
							case '=': abort<AA_SOURCE_DATA>();
							case ' ': case '\t': case '\n': case '\r': return;
							default:
								state = lexing_state::KEY;
								token.push_back(*std::bit_cast<const char *>(&character));
								return;
						}

					case lexing_state::KEY:
						switch (character) {
							default:
								token.push_back(*std::bit_cast<const char *>(&character));
								return;
							case ' ': case '\t':
								state = lexing_state::KEY_SPACE;
								whitespace.push_back(*std::bit_cast<const char *>(&character));
								return;
							case '=':
								init_key();
								return;
						}

					case lexing_state::KEY_SPACE:
						switch (character) {
							default:
								state = lexing_state::KEY;
								token.append(whitespace).push_back(*std::bit_cast<const char *>(&character));
								whitespace.clear();
								return;
							case ' ': case '\t':
								whitespace.push_back(*std::bit_cast<const char *>(&character));
								return;
							case '=':
								init_key();
								whitespace.clear();
								return;
						}

					case lexing_state::VALUE:
						switch (character) {
							default:
								token.push_back(*std::bit_cast<const char *>(&character));
								return;
							case '\n':
								state = lexing_state::BEFORE_TYPE;
								if (types_iter != types.cend())
									(this->*types_iter->second)();
								token.clear();
								return;
						}

					default:
						std::unreachable();
				}
			}
		};



		// Special member functions
	public:
		// Nereikalaujame, kad file kintamasis su savimi neštųsi failo kelią, nes šioje funkcijoje kelio mums nereikia.
		// Patariama pačiam naudoti naudotojui pathed_stream klasę, kuri automatiškai taip pat patikrina failed state.
		template<class C, char_traits_for<C> T, class... U>
		AA_CONSTEXPR lexer(std::basic_istream<C, T> &file, U&&... args) {
			// Konstruktorius nenustato eofbit jei failas tuščias todėl reikia šio tikrinimo.
			if (file.peek() == T::eof())
				return;

			// Lexing parameters
			params_lexer p_lexer = {params, {std::forward<U>(args)...}};

			// Lexing comments
			enum struct lexing_state : size_t {
				NONE,
				CHECK,
				COMMENT,
				MULTILINE,
				CHECKMULTI
			} state = lexing_state::NONE;

			do {
				const typename T::int_type character = file.get();

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
			} while (file.peek() != T::eof());
		}



		// Observers
		AA_CONSTEXPR const params_map &get_params() const {
			return params;
		}

		template<class T>
		AA_CONSTEXPR const T &get_param(const std::string_view &name) const {
			const typename params_map::const_iterator iter = params.template find<std::string_view>(name);
			AA_TRACE_ASSERT(iter != params.cend(), "Parameter `", type_name<T>(), " - ", name, "` not found.");

			const T *const param = std::get_if<T>(&iter->second);
			AA_TRACE_ASSERT(param, "Parameter `", type_name<T>(), " - ", name, "` not found.");
			return *param;
		}



		// Member objects
	protected:
		params_map params = {};
	};

	template<class C, class T, class... A>
	lexer(std::basic_istream<C, T> &, A&&...)->lexer<A...>;

}
