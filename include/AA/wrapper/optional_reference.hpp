#pragma once

#include "../metaprogramming/general.hpp"
#include <type_traits> // invoke_result_t
#include <concepts> // invocable
#include <functional> // invoke
#include <utility> // forward



namespace aa {

	// Prireikė šitos klasės, nes reference_wrapper negali būti default constructed.
	template<class T>
	struct optional_reference {
		// Member types
		using type = T;

		// Member objects
		type *ptr;

		// Observers
		AA_CONSTEXPR operator type &() const { return *ptr; }
		AA_CONSTEXPR type &operator*() const { return *ptr; }
		AA_CONSTEXPR type *operator->() const { return ptr; }

		AA_CONSTEXPR type &get() const { return *ptr; }
		AA_CONSTEXPR type *get_pointer() const { return ptr; }

		explicit AA_CONSTEXPR operator bool() const { return ptr; }

		template<class... A>
			requires (std::invocable<type &, A...>)
		AA_CONSTEXPR std::invoke_result_t<type &, A...> operator()(A&&... args) const {
			return std::invoke(*ptr, std::forward<A>(args)...);
		}

		// Modifiers
		AA_CONSTEXPR void release() { ptr = nullptr; }

		AA_CONSTEXPR void reset(type *const p) { ptr = p; }
	};

}
