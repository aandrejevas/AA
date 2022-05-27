#pragma once

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
		inline constexpr operator type &() const { return *ptr; }
		inline constexpr type &operator*() const { return *ptr; }
		inline constexpr type *operator->() const { return ptr; }

		inline constexpr type &get() const { return *ptr; }
		inline constexpr type *get_pointer() const { return ptr; }

		explicit inline constexpr operator bool() const { return ptr; }

		template<class... A>
			requires (std::invocable<type &, A...>)
		inline constexpr std::invoke_result_t<type &, A...> operator()(A&&... args) const {
			return std::invoke(*ptr, std::forward<A>(args)...);
		}

		// Modifiers
		inline constexpr void release() { ptr = nullptr; }

		inline constexpr void reset(type *const p) { ptr = p; }
	};

}
