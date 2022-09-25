#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/io.hpp"
#include "../preprocessor/assert.hpp"
#include <filesystem> // path
#include <concepts> // constructible_from
#include <utility> // forward



namespace aa {

	template<std::constructible_from<const std::filesystem::path &> S>
		requires (output_stream<S> || input_stream<S>)
	struct pathed_stream {
		// Member types
		using stream_type = S;



		// Observers
		AA_CONSTEXPR operator stream_type &() { return stream; }
		AA_CONSTEXPR operator const stream_type &() const { return stream; }



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

}
