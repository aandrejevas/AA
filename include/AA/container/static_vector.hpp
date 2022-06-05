#pragma once

#include "../metaprogramming/general.hpp"
#include <cstring> // memmove
#include <cstddef> // size_t, ptrdiff_t, byte
#include <memory> // construct_at
#include <utility> // forward
#include <concepts> // constructible_from
#include <type_traits> // conditional_t



namespace aa {

	// https://en.wikipedia.org/wiki/Dynamic_array
	template<trivially_copyable T, size_t N>
	struct static_vector {
		// Member types
		using value_type = T;
		using size_type = size_t;
		using difference_type = ptrdiff_t;
		using reference = value_type &;
		using const_reference = const value_type &;
		using pointer = value_type *;
		using const_pointer = const value_type *;
		using iterator = pointer;
		using const_iterator = const_pointer;
		using container_type = static_vector<T, N>;



		// Element access
		inline constexpr reference operator[](const size_type pos) { return at(pos); }
		inline constexpr const_reference operator[](const size_type pos) const { return at(pos); }

		inline constexpr reference at(const size_type pos) { return *pointer_at(pos); }
		inline constexpr const_reference at(const size_type pos) const { return *pointer_at(pos); }
		inline constexpr const_reference c_at(const size_type pos) const { return at(pos); }

		inline constexpr reference r_at(const size_type pos) { return *rpointer_at(pos); }
		inline constexpr const_reference r_at(const size_type pos) const { return *rpointer_at(pos); }
		inline constexpr const_reference cr_at(const size_type pos) const { return r_at(pos); }

		inline constexpr pointer pointer_at(const size_type pos) { return data() + pos; }
		inline constexpr const_pointer pointer_at(const size_type pos) const { return data() + pos; }
		inline constexpr const_pointer cpointer_at(const size_type pos) const { return pointer_at(pos); }

		inline constexpr pointer rpointer_at(const size_type pos) { return r_begin - pos; }
		inline constexpr const_pointer rpointer_at(const size_type pos) const { return r_begin - pos; }
		inline constexpr const_pointer crpointer_at(const size_type pos) const { return rpointer_at(pos); }

		inline constexpr pointer data() { return elements.data(); }
		inline constexpr const_pointer data() const { return elements.data(); }
		inline constexpr const_pointer cdata() const { return data(); }

		inline constexpr pointer rdata() { return r_begin; }
		inline constexpr const_pointer rdata() const { return r_begin; }
		inline constexpr const_pointer crdata() const { return rdata(); }

		inline constexpr reference front() { return *data(); }
		inline constexpr const_reference front() const { return *data(); }
		inline constexpr const_reference cfront() const { return front(); }

		inline constexpr reference back() { return *r_begin; }
		inline constexpr const_reference back() const { return *r_begin; }
		inline constexpr const_reference cback() const { return back(); }



		// Iterators
		inline constexpr iterator begin() { return data(); }
		inline constexpr const_iterator begin() const { return data(); }
		inline constexpr const_iterator cbegin() const { return begin(); }

		inline constexpr iterator end() { return r_begin + 1; }
		inline constexpr const_iterator end() const { return r_begin + 1; }
		inline constexpr const_iterator cend() const { return end(); }

		inline constexpr iterator rbegin() { return r_begin; }
		inline constexpr const_iterator rbegin() const { return r_begin; }
		inline constexpr const_iterator crbegin() const { return rbegin(); }

		inline constexpr iterator rend() { return r_end; }
		inline constexpr const_iterator rend() const { return r_end; }
		inline constexpr const_iterator crend() const { return rend(); }



		// Capacity
		inline constexpr bool empty() const { return r_begin == r_end; }
		inline constexpr bool full() const { return size() == N; }

		inline constexpr difference_type ssize() const { return r_begin - r_end; }
		inline constexpr size_type size() const { return static_cast<size_type>(ssize()); }

		static inline consteval size_type max_size() { return N; }



		// Anksčiau buvo, bet dabar nebėra elemento įdėjimo metodų overload'ų, kurie priimtų
		// value_type&&, nes move semantics neturi prasmės trivially copyable tipams.
		// Modifiers
		inline constexpr void clear() { r_begin = r_end; }

		inline constexpr void resize(const size_type count) { r_begin = r_end + count; }
		inline constexpr void resize(const const_iterator pos) { r_begin = const_cast<iterator>(pos); }

		inline constexpr iterator pop_back() { return --r_begin; }
		inline constexpr iterator push_back() { return ++r_begin; }

		inline constexpr iterator pop_back(const size_type count) { return r_begin -= count; }
		inline constexpr iterator push_back(const size_type count) { return r_begin += count; }

		template<class... A>
			requires (std::constructible_from<value_type, A...>)
		inline constexpr iterator emplace_back(A&&... args) {
			return std::ranges::construct_at(++r_begin, std::forward<A>(args)...);
		}

		inline constexpr void insert_back(const value_type &value) { *++r_begin = value; }

		template<class... A>
			requires (std::constructible_from<value_type, A...>)
		inline iterator emplace(const const_iterator pos, A&&... args) {
			std::memmove(const_cast<iterator>(pos + 1), pos,
				static_cast<size_type>(reinterpret_cast<const std::byte *>(++r_begin) - reinterpret_cast<const std::byte *>(pos)));
			return std::ranges::construct_at(const_cast<iterator>(pos), std::forward<A>(args)...);
		}

		inline void insert(const const_iterator pos, const value_type &value) {
			std::memmove(const_cast<iterator>(pos + 1), pos,
				static_cast<size_type>(reinterpret_cast<const std::byte *>(++r_begin) - reinterpret_cast<const std::byte *>(pos)));
			*const_cast<iterator>(pos) = value;
		}

		inline void erase(const const_iterator pos) {
			std::memmove(const_cast<iterator>(pos), pos + 1,
				static_cast<size_type>(reinterpret_cast<const std::byte *>(r_begin--) - reinterpret_cast<const std::byte *>(pos)));
		}

		template<class... A>
			requires (std::constructible_from<value_type, A...>)
		inline constexpr iterator fast_emplace(const const_iterator pos, A&&... args) {
			insert_back(*pos);
			return std::ranges::construct_at(const_cast<iterator>(pos), std::forward<A>(args)...);
		}

		inline constexpr void fast_insert(const const_iterator pos, const value_type &value) {
			insert_back(*pos);
			*const_cast<iterator>(pos) = value;
		}

		inline constexpr void fast_erase(const const_iterator pos) {
			*const_cast<iterator>(pos) = *r_begin--;
		}



		// Special member functions
		// GCC bug https://www.mail-archive.com/gcc-bugs@gcc.gnu.org/msg685363.html,
		// Matą netikrą klaidą kompiliuojant su -Ofast.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
		// Nedarome = default, nes konstrukrotius vis tiek nebus trivial,
		// nes klasė turi kintamųjų su numatytais inicializatoriais.
		inline constexpr static_vector() {}
#pragma GCC diagnostic pop
		inline constexpr static_vector(const const_iterator pos) : r_begin{const_cast<iterator>(pos)} {}
		inline constexpr static_vector(const size_type count) : r_begin{r_end + count} {}



		// Member objects
	protected: // protected, kad r_end nebūtų galima sugadinti
		array_t<value_type, N> elements;
		value_type *const r_end = elements.data() - 1, *r_begin = r_end;
	};

}
