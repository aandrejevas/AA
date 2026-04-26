#pragma once

#include "../metaprogramming/general.hpp"
#include <expected>



// https://github.com/boostorg/leaf/blob/bd58aa238cd389e23a456f0d7ad1422834818496/include/boost/leaf/error.hpp#L64
// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2561r2.html
#define AA_TRY(operand, ...) ({\
	auto && o = (operand);\
	using O = decltype(o);\
	using T = aa::try_traits<std::remove_cvref_t<O>>;\
	if (T::should_break(o)) { __VA_ARGS__; }\
	T::extract_continue(std::forward<O>(o));\
})

#define AA_OR_EXTRACT(...) AA_TRY((__VA_ARGS__), return T::extract_break(std::forward<O>(o)))
#define AA_OR_RETURN(...) AA_TRY((__VA_ARGS__), return auto{std::forward<O>(o)})
#define AA_OR_BREAK(...) AA_TRY((__VA_ARGS__), break)
#define AA_OR_CONTINUE(...) AA_TRY((__VA_ARGS__), continue)
#define AA_OR_GOTO(label, ...) AA_TRY((__VA_ARGS__), goto label)

namespace aa {

	template<std::movable>
	struct try_traits;

	template<class T>
	struct try_traits<std::optional<T>> {
		using continue_type = T;
		using break_type = std::nullopt_t;
		using try_type = std::optional<continue_type>;

		static constexpr bool should_break(const try_type & o) { return !o.has_value(); }

		// extractors
		template<wo_cvref_same_as<try_type> O>
		static constexpr auto && extract_continue(O && o) { return *std::forward<O>(o); }

		template<wo_cvref_same_as<try_type> O>
		static consteval break_type extract_break(O &&) { return std::nullopt; }

		// factories
		template<constructible_to<continue_type> V = continue_type>
		static constexpr try_type from_continue(V && v) {
			return try_type{std::in_place, std::forward<V>(v)};
		}

		static consteval try_type from_break(break_type) { return value<std::nullopt>; }
	};

	template<class T, class E>
	struct try_traits<std::expected<T, E>> {
		using continue_type = T;
		using break_type = E;
		using try_type = std::expected<continue_type, break_type>;

		static constexpr bool should_break(const try_type & o) { return !o.has_value(); }

		// extractors
		template<wo_cvref_same_as<try_type> O>
		static constexpr auto && extract_continue(O && o) { return *std::forward<O>(o); }

		template<wo_cvref_same_as<try_type> O>
		static constexpr auto && extract_break(O && o) { return std::forward<O>(o).error(); }

		// factories
		template<constructible_to<continue_type> V = continue_type>
		static constexpr try_type from_continue(V && v) {
			return try_type{std::in_place, std::forward<V>(v)};
		}
		template<constructible_to<break_type> V = break_type>
		static constexpr try_type from_break(V && v) {
			return try_type{std::unexpect, std::forward<V>(v)};
		}
	};

	template<class E>
	struct try_traits<std::expected<void, E>> {
		using continue_type = void;
		using break_type = E;
		using try_type = std::expected<continue_type, break_type>;

		static constexpr bool should_break(const try_type & o) { return !o.has_value(); }

		// extractors
		template<wo_cvref_same_as<try_type> O>
		static consteval void extract_continue(O &&) {}

		template<wo_cvref_same_as<try_type> O>
		static constexpr auto && extract_break(O && o) { return std::forward<O>(o).error(); }

		// factories
		static consteval try_type from_continue() { return value<std::in_place>; }

		template<constructible_to<break_type> V = break_type>
		static constexpr try_type from_break(V && v) {
			return try_type{std::unexpect, std::forward<V>(v)};
		}
	};

}
