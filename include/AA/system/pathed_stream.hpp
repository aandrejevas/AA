#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/io.hpp"
#include "../preprocessor/assert.hpp"
#include "source.hpp"
#include <fstream> // ofstream, ifstream
#include <ostream> // basic_ostream, ostream
#include <ios> // openmode, out, in



namespace aa {

	template<class P, stream_like S>
	struct pathed_stream {
		// Member types
		using path_type = P;
		using stream_type = S;
		using char_type = typename stream_type::char_type;
		using traits_type = typename stream_type::traits_type;
		using int_type = typename stream_type::int_type;
		using pos_type = typename stream_type::pos_type;
		using off_type = typename stream_type::off_type;



		// Observers
		AA_CONSTEXPR operator stream_type &() { return stream; }
		AA_CONSTEXPR operator const stream_type &() const { return stream; }

		AA_CONSTEXPR stream_type &operator*() { return stream; }
		AA_CONSTEXPR const stream_type &operator*() const { return stream; }

		AA_CONSTEXPR stream_type *operator->() { return &stream; }
		AA_CONSTEXPR const stream_type *operator->() const { return &stream; }

		AA_CONSTEXPR stream_type &get() { return stream; }
		AA_CONSTEXPR const stream_type &get() const { return stream; }



		// Input/output
		template<insertable_into<stream_type> U>
		AA_CONSTEXPR apply_traits_t<std::basic_ostream, stream_type> &operator<<(const U &u) {
			return stream << u;
		}



		// Special member functions
		template<constructible_to<path_type> T = path_type, invoke_result_constructible_to<stream_type, const path_type &> F>
		AA_CONSTEXPR pathed_stream(T &&t, F &&f) : path{std::forward<T>(t)}, stream{std::invoke(std::forward<F>(f), path)} {
			if constexpr (insertable_into<path_type &, std::ostream>) {
				AA_TRACE_ASSERT(!stream.fail(), type_name<stream_type>(), " (", path, ") in fail state after construction.");
			} else {
				AA_TRACE_ASSERT(!stream.fail(), type_name<stream_type>(), " in fail state after construction.");
			}
		}

		AA_CONSTEXPR ~pathed_stream() {
			if constexpr (insertable_into<path_type &, std::ostream>) {
				AA_TRACE_ASSERT(!stream.fail(), type_name<stream_type>(), " (", path, ") in fail state before destruction.");
			} else {
				AA_TRACE_ASSERT(!stream.fail(), type_name<stream_type>(), " in fail state before destruction.");
			}
		}



		// Member objects
		// path kintamasis laiko kelią, su kuriuo stream buvo sukonstruotas.
		// Todėl path gali būti const nors ir stream vėliau gali savo kelią pakeisti.
		[[no_unique_address]] const path_type path;
		[[no_unique_address]] stream_type stream;
	};

	template<class T, std::invocable<const T &> F>
	pathed_stream(T &&, F &&) -> pathed_stream<T, std::invoke_result_t<F, const T &>>;



	template<std::ios_base::openmode MODE = std::ios_base::out, class T>
	AA_CONSTEXPR pathed_stream<T, std::ofstream> make_p_ofstream(T &&t) {
		return {std::forward<T>(t), [](const T &p) { return std::ofstream{p, MODE}; }};
	}

	template<std::ios_base::openmode MODE = std::ios_base::in, class T>
	AA_CONSTEXPR pathed_stream<T, std::ifstream> make_p_ifstream(T &&t) {
		return {std::forward<T>(t), [](const T &p) { return std::ifstream{p, MODE}; }};
	}

}
