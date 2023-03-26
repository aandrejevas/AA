#pragma once

#include "../metaprogramming/general.hpp"



namespace aa {

	// Prireikė šitos klasės, nes reference_wrapper negali būti default constructed.
	// https://vector-of-bool.github.io/2018/02/27/opt-ref.html
	// https://en.wikipedia.org/wiki/Smart_pointer
	template<class T>
	struct optional_reference {
		// Member types
		using value_type = T;



		// Observers
		AA_CONSTEXPR operator value_type &() const { return *ptr; }
		AA_CONSTEXPR value_type &operator*() const { return *ptr; }
		AA_CONSTEXPR value_type *operator->() const { return ptr; }

		AA_CONSTEXPR value_type &get() const { return *ptr; }
		AA_CONSTEXPR value_type *get_pointer() const { return ptr; }

		explicit AA_CONSTEXPR operator bool() const { return ptr; }

		template<class... A>
			requires (std::invocable<value_type &, A...>)
		AA_CONSTEXPR std::invoke_result_t<value_type &, A...> operator()(A&&... args) const {
			return std::invoke(*ptr, std::forward<A>(args)...);
		}



		// Modifiers
		AA_CONSTEXPR void release() { ptr = nullptr; }

		AA_CONSTEXPR void reset(value_type *const p) { ptr = p; }



		// Member objects
		value_type *ptr;
	};

}
