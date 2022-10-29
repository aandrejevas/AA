#pragma once

#include "../metaprogramming/general.hpp"
#include <cstring> // memmove
#include <cstddef> // size_t, ptrdiff_t, byte
#include <memory> // construct_at
#include <utility> // forward
#include <concepts> // constructible_from
#include <bit> // bit_cast



namespace aa {

	// https://en.wikipedia.org/wiki/Dynamic_array
	template<trivially_copyable T, size_t N>
	struct fixed_vector {
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



		// Element access
		AA_CONSTEXPR reference operator[](const size_type pos) { return elem(pos); }
		AA_CONSTEXPR const_reference operator[](const size_type pos) const { return elem(pos); }

		AA_CONSTEXPR reference elem(const size_type pos) { return *data(pos); }
		AA_CONSTEXPR const_reference elem(const size_type pos) const { return *data(pos); }
		AA_CONSTEXPR const_reference celem(const size_type pos) const { return elem(pos); }

		AA_CONSTEXPR reference relem(const size_type pos) { return *rdata(pos); }
		AA_CONSTEXPR const_reference relem(const size_type pos) const { return *rdata(pos); }
		AA_CONSTEXPR const_reference crelem(const size_type pos) const { return relem(pos); }

		AA_CONSTEXPR pointer data(const size_type pos) { return data() + pos; }
		AA_CONSTEXPR const_pointer data(const size_type pos) const { return data() + pos; }
		AA_CONSTEXPR const_pointer cdata(const size_type pos) const { return data(pos); }

		AA_CONSTEXPR pointer rdata(const size_type pos) { return rdata() - pos; }
		AA_CONSTEXPR const_pointer rdata(const size_type pos) const { return rdata() - pos; }
		AA_CONSTEXPR const_pointer crdata(const size_type pos) const { return rdata(pos); }

		AA_CONSTEXPR pointer data() { return elements.data(); }
		AA_CONSTEXPR const_pointer data() const { return elements.data(); }
		AA_CONSTEXPR const_pointer cdata() const { return data(); }

		AA_CONSTEXPR pointer rdata() { return r_begin; }
		AA_CONSTEXPR const_pointer rdata() const { return r_begin; }
		AA_CONSTEXPR const_pointer crdata() const { return rdata(); }

		AA_CONSTEXPR reference front() { return *data(); }
		AA_CONSTEXPR const_reference front() const { return *data(); }
		AA_CONSTEXPR const_reference cfront() const { return front(); }

		AA_CONSTEXPR reference back() { return *rdata(); }
		AA_CONSTEXPR const_reference back() const { return *rdata(); }
		AA_CONSTEXPR const_reference cback() const { return back(); }



		// Iterators
		AA_CONSTEXPR iterator begin() { return data(); }
		AA_CONSTEXPR const_iterator begin() const { return data(); }
		AA_CONSTEXPR const_iterator cbegin() const { return begin(); }

		AA_CONSTEXPR iterator end() { return rdata() + 1; }
		AA_CONSTEXPR const_iterator end() const { return rdata() + 1; }
		AA_CONSTEXPR const_iterator cend() const { return end(); }

		AA_CONSTEXPR iterator rbegin() { return rdata(); }
		AA_CONSTEXPR const_iterator rbegin() const { return rdata(); }
		AA_CONSTEXPR const_iterator crbegin() const { return rbegin(); }

		AA_CONSTEXPR iterator rend() { return r_end; }
		AA_CONSTEXPR const_iterator rend() const { return r_end; }
		AA_CONSTEXPR const_iterator crend() const { return rend(); }



		// Capacity
		AA_CONSTEXPR bool empty() const { return r_begin == r_end; }
		AA_CONSTEXPR bool full() const { return size() == max_size(); }

		AA_CONSTEXPR difference_type ssize() const { return r_begin - r_end; }
		AA_CONSTEXPR size_type size() const { return std::bit_cast<size_type>(ssize()); }

		static AA_CONSTEVAL size_type max_size() { return N; }

		AA_CONSTEXPR size_type max_index() const { return std::bit_cast<size_type>(r_begin - data()); }



		// Anksčiau buvo, bet dabar nebėra elemento įdėjimo metodų overload'ų, kurie priimtų
		// value_type&&, nes move semantics neturi prasmės trivially copyable tipams.
		// Modifiers
		AA_CONSTEXPR void clear() { r_begin = r_end; }

		AA_CONSTEXPR iterator resize(const size_type count) { return r_begin = r_end + count; }
		AA_CONSTEXPR iterator resize(const const_iterator pos) { return r_begin = const_cast<iterator>(pos); }

		AA_CONSTEXPR iterator pop_back() { return --r_begin; }
		AA_CONSTEXPR iterator push_back() { return ++r_begin; }

		AA_CONSTEXPR iterator pop_back(const size_type count) { return r_begin -= count; }
		AA_CONSTEXPR iterator push_back(const size_type count) { return r_begin += count; }

		template<class... A>
			requires (std::constructible_from<value_type, A...>)
		AA_CONSTEXPR iterator emplace_back(A&&... args) {
			return std::ranges::construct_at(++r_begin, std::forward<A>(args)...);
		}

		AA_CONSTEXPR void insert_back(const value_type &value) { *++r_begin = value; }

		template<class... A>
			requires (std::constructible_from<value_type, A...>)
		AA_CONSTEXPR iterator emplace(const const_iterator pos, A&&... args) {
			std::memmove(const_cast<iterator>(pos + 1), pos,
				std::bit_cast<size_type>(std::bit_cast<const std::byte *>(++r_begin) - std::bit_cast<const std::byte *>(pos)));
			return std::ranges::construct_at(const_cast<iterator>(pos), std::forward<A>(args)...);
		}

		AA_CONSTEXPR void insert(const const_iterator pos, const value_type &value) {
			std::memmove(const_cast<iterator>(pos + 1), pos,
				std::bit_cast<size_type>(std::bit_cast<const std::byte *>(++r_begin) - std::bit_cast<const std::byte *>(pos)));
			*const_cast<iterator>(pos) = value;
		}

		AA_CONSTEXPR void erase(const const_iterator pos) {
			std::memmove(const_cast<iterator>(pos), pos + 1,
				std::bit_cast<size_type>(std::bit_cast<const std::byte *>(r_begin--) - std::bit_cast<const std::byte *>(pos)));
		}

		template<class... A>
			requires (std::constructible_from<value_type, A...>)
		AA_CONSTEXPR iterator fast_emplace(const const_iterator pos, A&&... args) {
			insert_back(*pos);
			return std::ranges::construct_at(const_cast<iterator>(pos), std::forward<A>(args)...);
		}

		AA_CONSTEXPR void fast_insert(const const_iterator pos, const value_type &value) {
			insert_back(*pos);
			*const_cast<iterator>(pos) = value;
		}

		AA_CONSTEXPR void fast_erase(const const_iterator pos) {
			*const_cast<iterator>(pos) = *r_begin--;
		}



		// Special member functions
		// Kompiliatorius meta klaidą dėl to kaip kintamasis r_end yra inicializuojamas.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
		// Nedarome = default, nes konstrukrotius vis tiek nebus trivial,
		// nes klasė turi kintamųjų su numatytais inicializatoriais.
		AA_CONSTEXPR fixed_vector() {}
		AA_CONSTEXPR fixed_vector(const const_iterator pos) : r_begin{const_cast<iterator>(pos)} {}
		AA_CONSTEXPR fixed_vector(const size_type count) : r_begin{r_end + count} {}
#pragma GCC diagnostic pop



		// Member objects
		array_t<value_type, N> elements;

	protected:
		value_type *const r_end = elements.data() - 1, *r_begin = r_end;
	};

}
