#pragma once

#include "../preprocessor/assert.hpp"
#include "../metaprogramming/general.hpp"
#include "../wrapper/general.hpp"
#include "../wrapper/range.hpp"
#include <cstddef> // size_t
#include <string> // string, hash
#include <variant> // variant, monostate, get_if
#include <fstream> // ifstream
#include <unordered_map> // unordered_map
#include <initializer_list> // initializer_list
#include <functional> // invoke, hash
#include <string_view> // string_view, hash
#include <tuple> // tuple, get
#include <utility> // unreachable, forward
#include <bit> // bit_cast



namespace aa {

	// Būvo idėja realizuoti escape sequences, bet faile galima	tiesiog įterpti pavyzdžiui naują eilutę todėl jų neprireikia.
	// Teksto eilutės reikšmėse pirminiame kode to padaryti negalima todėl tokiame kontekste yra reikalingos escape sequences.
	// https://en.wikipedia.org/wiki/Lexical_analysis
	template<storable_evaluator... A>
		requires (are_unique_v<std::monostate, evaluator_result_t<A>...>)
	struct lexer {
		using params_map = std::unordered_map<std::string, std::variant<std::monostate, evaluator_result_t<A>...>,
			overload<std::hash<std::string>, std::hash<std::string_view>>, char_equal_to>;

	protected:
		params_map params = {};

		struct params_lexer {
			params_map &params;

			template<class T>
			AA_CONSTEXPR void evaluate() const {
				params_iter->second.template emplace<evaluator_result_t<T>>(
					std::invoke(std::get<T>(evaluators), token.operator std::string_view()));
			}

			using types_map = std::unordered_map<std::string_view, void(params_lexer:: *)() const,
				typename params_map::hasher, typename params_map::key_equal>;

			const std::tuple<A...> evaluators;
			const types_map types = {std::initializer_list<typename types_map::value_type>{
				typename types_map::value_type{type_name<evaluator_result_t<A>>(), &evaluate<A>}...
			}};

			enum struct lexing_state : size_t {
				BEFORE_TYPE, TYPE, TYPE_SPACE,
				BEFORE_KEY, KEY, KEY_SPACE,
				VALUE
			} state = lexing_state::BEFORE_TYPE;
			// Jeigu deklaruotume savo konstruktorius, galėtume inicializuoti iteratorius su default
			// initialization, bet to nedarome, nes niekas nepasikeistų, nes naudojamų stl konteinerių
			// default konstruktoriai nėra trivial. Todėl params_lexer tipas gali likti aggregate.
			types_map::const_iterator types_iter = {};
			params_map::iterator params_iter = {};
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
				}
				std::unreachable();
			}
		};

		AA_CONSTEXPR lexer(const std::string_view &filename, params_lexer &&p_lexer) {
			std::ifstream file = std::ifstream{filename.data()};
			AA_TRACE_ASSERT(file.is_open(), "Error while openning file `", filename, "`.");

			if (file.peek() == std::ifstream::traits_type::eof())
				return;

			// Lexing comments
			enum struct lexing_state : size_t {
				NONE,
				CHECK,
				COMMENT,
				MULTILINE,
				CHECKMULTI
			} state = lexing_state::NONE;
			int character = file.get();

			do {
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
				}
			} while ((character = file.get()) != std::ifstream::traits_type::eof());

			AA_TRACE_ASSERT(!file.bad(), "Error while reading file `", filename, "`.");
		}

	public:
		AA_CONSTEXPR lexer(const std::string_view &filename) : lexer{filename, {params, {}}} {}
		template<class... U>
		AA_CONSTEXPR lexer(const std::string_view &filename, U&&... args) : lexer{filename, {params, {std::forward<U>(args)...}}} {}

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
	};

	template<class... A>
	AA_CONSTEXPR lexer<A...> make_lexer(const std::string_view &filename, A&&... args) {
		return {filename, std::forward<A>(args)...};
	}

}
