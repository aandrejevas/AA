#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/io.hpp"
#include "../preprocessor/assert.hpp"
#include "source.hpp"
#include <fstream> // ofstream, ifstream
#include <ios> // openmode, out, in



namespace aa {

	template<stream_insertable P, stream_like S>
	struct pathed_stream {
		// Member types
		using path_type = P;
		using stream_type = S;
		using base_type = stream_t<stream_type> &;
		using char_type = char_type_in_use_t<stream_type>;
		using traits_type = traits_type_in_use_t<stream_type>;
		using int_type = int_type_in_use_t<stream_type>;
		using pos_type = pos_type_in_use_t<stream_type>;
		using off_type = off_type_in_use_t<stream_type>;



		// Observers
		constexpr operator stream_type &() { return stream; }
		constexpr operator const stream_type &() const { return stream; }

		constexpr stream_type &operator*() { return stream; }
		constexpr const stream_type &operator*() const { return stream; }

		constexpr stream_type *operator->() { return &stream; }
		constexpr const stream_type *operator->() const { return &stream; }

		constexpr stream_type &get() { return stream; }
		constexpr const stream_type &get() const { return stream; }



		// Special member functions
		template<constructible_to<path_type> T = path_type, invoke_result_constructible_to<stream_type, const path_type &> F>
		constexpr pathed_stream(T &&t, F &&f)
			: path{std::forward<T>(t)}, stream{std::invoke(std::forward<F>(f), std::as_const(path))}
		{
			AA_TRACE_ASSERT(!cast<base_type>(stream).fail(), "Stream ",
				type_name_v<stream_type>, '(', path, ") in fail state after construction.");
		}

		constexpr ~pathed_stream() {
			AA_TRACE_ASSERT(!cast<base_type>(stream).fail(), "Stream ",
				type_name_v<stream_type>, '(', path, ") in fail state before destruction.");
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
	constexpr pathed_stream<T, std::ofstream> make_p_ofstream(T &&t) {
		return {std::forward<T>(t), [](const T &p) { return std::ofstream{p, MODE}; }};
	}

	template<std::ios_base::openmode MODE = std::ios_base::in, class T>
	constexpr pathed_stream<T, std::ifstream> make_p_ifstream(T &&t) {
		return {std::forward<T>(t), [](const T &p) { return std::ifstream{p, MODE}; }};
	}

}
