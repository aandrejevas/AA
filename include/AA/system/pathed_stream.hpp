#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/io.hpp"
#include "../preprocessor/assert.hpp"
#include <concepts> // constructible_from
#include <utility> // forward
#include <fstream> // ofstream, ifstream
#include <ostream> // ostream



namespace aa {

	template<class P, stream_constructible_from<propagate_const_t<P> &> S>
		requires (output_stream<std::ostream, propagate_const_t<P> &>)
	struct pathed_stream {
		// Member types
		using path_type = propagate_const_t<P>;
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
		// Kad sukonstruoti path, nesuteikiame galimybės paduoti parameter pack, nes įmanoma tiesiog paduoti path_type
		// argumentą, kuris bus nukopijuotas. To negalime padaryti su stream. Tokia realizacija kai kuriais atvejais gal lėta.
		template<constructible_to<path_type> T, class... U>
			requires (std::constructible_from<stream_type, path_type &, U...>)
		AA_CONSTEXPR pathed_stream(T &&t, U&&... args) : path{std::forward<T>(t)}, stream{path, std::forward<U>(args)...} {
			AA_TRACE_ASSERT(!stream.fail(), type_name<stream_type>(), " (", path, ") in fail state after construction.");
		}

		AA_CONSTEXPR ~pathed_stream() {
			AA_TRACE_ASSERT(!stream.fail(), type_name<stream_type>(), " (", path, ") in fail state before destruction.");
		}



		// Member objects
		path_type path;
		stream_type stream;
	};



	// Turime sukurti papildomas klases, nes ant originalios klasės neišeina nurodyti gero deduction guide.
	// Kartojasi deduction guides kodas, bet net jei leistų ant alias daryti guides, vis tiek kodas turėtų kartotis.
	template<class P>
	struct pathed_ofstream : pathed_stream<P, std::ofstream> {};

	template<class P, class... U>
	pathed_ofstream(P &&, U&&...)->pathed_ofstream<P>;

	template<class P>
	struct pathed_ifstream : pathed_stream<P, std::ifstream> {};

	template<class P, class... U>
	pathed_ifstream(P &&, U&&...)->pathed_ifstream<P>;

}
