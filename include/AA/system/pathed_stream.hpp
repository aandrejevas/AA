#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/io.hpp"
#include "../preprocessor/assert.hpp"
#include <filesystem> // path
#include <concepts> // constructible_from
#include <utility> // forward
#include <fstream> // ofstream, ifstream



namespace aa {

	template<stream_constructible_from<const std::filesystem::path &> S>
	struct pathed_stream {
		// Member types
		using stream_type = S;
		using char_type = stream_type::char_type;
		using traits_type = stream_type::traits_type;
		using int_type = stream_type::int_type;
		using pos_type = stream_type::pos_type;
		using off_type = stream_type::off_type;



		// Observers
		AA_CONSTEXPR operator stream_type &() { return stream; }
		AA_CONSTEXPR operator const stream_type &() const { return stream; }

		AA_CONSTEXPR stream_type &operator*() { return stream; }
		AA_CONSTEXPR const stream_type &operator*() const { return stream; }

		AA_CONSTEXPR stream_type *operator->() { return &stream; }
		AA_CONSTEXPR const stream_type *operator->() const { return &stream; }

		AA_CONSTEXPR stream_type &get() { return stream; }
		AA_CONSTEXPR const stream_type &get() const { return stream; }



		// Special member functions
		template<class T, class... U>
			requires (std::constructible_from<stream_type, const std::filesystem::path &, U...>)
		AA_CONSTEXPR pathed_stream(T &&t, U&&... args) : path{std::forward<T>(t)}, stream{path, std::forward<U>(args)...} {
			AA_TRACE_ASSERT(!stream.fail(), type_name<stream_type>(), " (", path, ") in fail state after construction.");
		}

		AA_CONSTEXPR ~pathed_stream() {
			AA_TRACE_ASSERT(!stream.fail(), type_name<stream_type>(), " (", path, ") in fail state before destruction.");
		}



		// Member objects
		const std::filesystem::path path;
		stream_type stream;
	};



	template<output_stream S>
	using pathed_ostream = pathed_stream<S>;

	template<input_stream S>
	using pathed_istream = pathed_stream<S>;

	using pathed_ofstream = pathed_ostream<std::ofstream>;

	using pathed_ifstream = pathed_istream<std::ifstream>;

}
