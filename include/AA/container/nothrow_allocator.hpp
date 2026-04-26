#pragma once

#include "../metaprogramming/general.hpp"
#include "../algorithm/arithmetic.hpp"



namespace aa {

	template<class T>
	struct nothrow_allocator {
		// Member types
		using value_type = T;
		using size_type = size_t;
		using difference_type = ptrdiff_t;
		using reference = value_type &;
		using const_reference = const value_type &;
		using pointer = value_type *;
		using const_pointer = const value_type *;



		// Member functions
		// https://gcc.gnu.org/onlinedocs/libstdc++/manual/dynamic_memory.html
		static constexpr pointer allocate(const size_type n) {
			// This creates value_type[n] because it is an implicit-lifetime type and this allocating function can create such types. GCC does the same.
			// https://en.cppreference.com/w/cpp/language/objects.html#Object_creation
			return std::bit_cast<pointer>(__builtin_operator_new(
				product<sizeof(value_type)>(n), std::align_val_t{alignof(value_type)}, std::nothrow));
		}

		static constexpr void deallocate(const pointer p, const size_type n) {
			__builtin_operator_delete(
				p, product<sizeof(value_type)>(n), std::align_val_t{alignof(value_type)});
		}

		static constexpr void deallocate(const pointer p) {
			__builtin_operator_delete(
				p, std::align_val_t{alignof(value_type)});
		}
	};

}
