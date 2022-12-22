#pragma once

#include "../metaprogramming/general.hpp"
#include "../metaprogramming/io.hpp"
#include "../preprocessor/assert.hpp"
#include "../system/source.hpp"
#include <concepts> // constructible_from
#include <utility> // forward
#include <fstream> // ofstream, ifstream
#include <ostream> // basic_ostream, ostream



namespace aa {

	template<class P, stream_like S>
	struct pathed_stream {
		// Member types
		using path_type = propagate_const_t<P>;
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
		// Kad sukonstruoti path, nesuteikiame galimybės paduoti parameter pack, nes įmanoma tiesiog paduoti path_type
		// argumentą, kuris bus nukopijuotas. To negalime padaryti su stream. Tokia realizacija kai kuriais atvejais gal lėta.
		template<constructible_to<path_type> T, class... U>
			requires (std::constructible_from<stream_type, path_type &, U...>)
		AA_CONSTEXPR pathed_stream(T &&t, U&&... args) : path{std::forward<T>(t)}, stream{path, std::forward<U>(args)...} {
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
		path_type path;
		stream_type stream;
	};



	// Turime sukurti papildomas klases, nes ant originalios klasės neišeina nurodyti gero deduction guide.
	// Kartojasi deduction guides kodas, bet net jei leistų ant alias daryti guides, vis tiek kodas turėtų kartotis.
	template<class P>
	struct pathed_ofstream : pathed_stream<P, std::ofstream> {
		using pathed_stream<P, std::ofstream>::pathed_stream;
	};

	template<class P, class... U>
	pathed_ofstream(P &&, U&&...) -> pathed_ofstream<P>;

	template<class P>
	struct pathed_ifstream : pathed_stream<P, std::ifstream> {
		using pathed_stream<P, std::ifstream>::pathed_stream;
	};

	template<class P, class... U>
	pathed_ifstream(P &&, U&&...) -> pathed_ifstream<P>;

}
