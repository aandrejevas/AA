#pragma once

#include "../metaprogramming/general.hpp"
#include <cstddef> // size_t, ptrdiff_t
#include <iterator> // reverse_iterator
#include <string_view> // basic_string_view, hash
#include <ostream> // basic_ostream
#include <string> // basic_string, char_traits
#include <functional> // hash
#include <type_traits> // type_identity, basic_common_reference
#include <utility> // tuple_size, tuple_element



namespace aa {

	// Nors tai labai nepatogu, traits tipas turi būti antras, nes standarte visos tokio tipo klasės apibrėžtos panašiai.
	// https://en.wikipedia.org/wiki/Null-terminated_string
	template<trivially_copyable C, char_traits_for<C> T, size_t N>
	struct basic_fixed_string {
		// Member types
		using value_type = C;
		using traits_type = T;
		using size_type = size_t;
		using difference_type = ptrdiff_t;
		using reference = value_type &;
		using const_reference = const value_type &;
		using pointer = value_type *;
		using const_pointer = const value_type *;
		using iterator = pointer;
		using const_iterator = const_pointer;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		using view_type = std::basic_string_view<value_type, traits_type>;
		using ostream_type = std::basic_ostream<value_type, traits_type>;



		// Element access
		template<size_type I>
		AA_CONSTEXPR reference get() requires (I < N) { return get(I); }
		template<size_type I>
		AA_CONSTEXPR const_reference get() const requires (I < N) { return get(I); }

		AA_CONSTEXPR reference operator[](const size_type pos) { return get(pos); }
		AA_CONSTEXPR const_reference operator[](const size_type pos) const { return get(pos); }

		AA_CONSTEXPR reference get(const size_type pos) { return *data(pos); }
		AA_CONSTEXPR const_reference get(const size_type pos) const { return *data(pos); }
		AA_CONSTEXPR const_reference cget(const size_type pos) const { return get(pos); }

		AA_CONSTEXPR pointer data(const size_type pos) { return data() + pos; }
		AA_CONSTEXPR const_pointer data(const size_type pos) const { return data() + pos; }
		AA_CONSTEXPR const_pointer cdata(const size_type pos) const { return data(pos); }

		AA_CONSTEXPR pointer data() { return elements.data(); }
		AA_CONSTEXPR const_pointer data() const { return elements.data(); }
		AA_CONSTEXPR const_pointer cdata() const { return data(); }

		AA_CONSTEXPR reference front() { return elements.front(); }
		AA_CONSTEXPR const_reference front() const { return elements.front(); }
		AA_CONSTEXPR const_reference cfront() const { return front(); }

		AA_CONSTEXPR reference back() { return elements.back(); }
		AA_CONSTEXPR const_reference back() const { return elements.back(); }
		AA_CONSTEXPR const_reference cback() const { return back(); }

		AA_CONSTEXPR operator view_type() const { return view_type{data(), N}; }



		// Iterators
		AA_CONSTEXPR iterator begin() { return elements.begin(); }
		AA_CONSTEXPR const_iterator begin() const { return elements.begin(); }
		AA_CONSTEXPR const_iterator cbegin() const { return begin(); }

		AA_CONSTEXPR iterator end() { return elements.end(); }
		AA_CONSTEXPR const_iterator end() const { return elements.end(); }
		AA_CONSTEXPR const_iterator cend() const { return end(); }

		AA_CONSTEXPR reverse_iterator rbegin() { return elements.rbegin(); }
		AA_CONSTEXPR const_reverse_iterator rbegin() const { return elements.rbegin(); }
		AA_CONSTEXPR const_reverse_iterator crbegin() const { return rbegin(); }

		AA_CONSTEXPR reverse_iterator rend() { return elements.rend(); }
		AA_CONSTEXPR const_reverse_iterator rend() const { return elements.rend(); }
		AA_CONSTEXPR const_reverse_iterator crend() const { return rend(); }



		// Capacity
		static AA_CONSTEVAL bool empty() { return !N; }
		static AA_CONSTEVAL bool single() { return N == 1; }
		static AA_CONSTEVAL difference_type ssize() { return N; }
		static AA_CONSTEVAL size_type size() { return N; }
		static AA_CONSTEVAL size_type max_size() { return N; }
		static AA_CONSTEVAL size_type last_index() { return N - 1; }
		static AA_CONSTEVAL size_type max_index() { return N - 1; }



		// Operations
		AA_CONSTEXPR bool starts_with(const value_type &c) const { return traits_type::eq(front(), c); }
		AA_CONSTEXPR bool ends_with(const value_type &c) const { return traits_type::eq(back(), c); }

		friend AA_CONSTEXPR bool operator==(const basic_fixed_string &l, const basic_fixed_string &r) {
			return !traits_type::compare(l.data(), r.data(), N);
		}

		// Nors argumentai gali būti sukonstruoti programos veikimo metu, kadangi jų tipai yra literal types,
		// consteval funkciją vis tiek bus galima iškviesti tik žinoma joje jų negalima reikšmingai naudoti.
		friend AA_CONSTEVAL bool operator==(const basic_fixed_string &, const basic_fixed_string &) requires (!N) {
			return true;
		}

		template<size_type N2>
		friend AA_CONSTEVAL bool operator==(const basic_fixed_string &, const basic_fixed_string<value_type, traits_type, N2> &) {
			return false;
		}

		friend AA_CONSTEXPR bool operator==(const basic_fixed_string &l, const view_type &r) {
			if constexpr (N)	return (N == r.size()) && !traits_type::compare(l.data(), r.data(), N);
			else				return (N == r.size());
		}



		// Input/output
		friend AA_CONSTEXPR ostream_type &operator<<(ostream_type &os, const basic_fixed_string &str) {
			return os << str.operator view_type();
		}



		// Special member functions
		AA_CONSTEXPR basic_fixed_string(const value_type *const cstring) {
			if constexpr (N)	traits_type::copy(elements.data(), cstring, N);
		}



		// Member objects
		array_t<value_type, N> elements;
	};



	template<trivially_copyable C, size_t N>
	using semibasic_fixed_string = basic_fixed_string<C, std::char_traits<C>, N>;

	template<size_t N>
	using fixed_string = semibasic_fixed_string<char, N>;

	// Neuždedame suvaržymų and deduction guide, kurie yra ant klasės, nes šie suvaržymai tiesiog pašalintų
	// iš svarstymų guide, o tai bus tiesiog automatiškai padaryta todėl nėra reikalo kartotis.
	//
	// Nekopijuojame į fixed_string null character, nes tik užimtų bereikalingai vietą simbolis.
	// Netikriname ar (N != 0), nes C++ standartas draudžia deklaruoti tokius masyvus.
	template<class T, size_t N>
	basic_fixed_string(const T(&)[N]) -> basic_fixed_string<T, std::char_traits<T>, N - 1>;



	// https://stackoverflow.com/questions/62853609/understanding-user-defined-string-literals-addition-for-c20
	inline namespace literals {
		template<basic_fixed_string S>
		AA_CONSTEVAL decltype(S) operator""_fs() { return S; }
	}

}



namespace std {

	template<class C, class T, size_t N>
	struct hash<aa::basic_fixed_string<C, T, N>> {
		AA_CONSTEXPR size_t operator()(const aa::basic_fixed_string<C, T, N> &string) const {
			return aa::constant<std::hash<std::basic_string_view<C, T>>>()(string);
		}
	};



	template<class C, class T, size_t N>
	struct tuple_size<aa::basic_fixed_string<C, T, N>> : aa::size_identity<N> {};

	template<size_t I, class C, class T, size_t N>
		requires (I < N)
	struct tuple_element<I, aa::basic_fixed_string<C, T, N>> : std::type_identity<C> {};



	template<class C, class T, class A, size_t N, template<class> class TQUAL, template<class> class QQUAL>
	struct basic_common_reference<aa::basic_fixed_string<C, T, N>, std::basic_string<C, T, A>, TQUAL, QQUAL>
		: std::type_identity<std::basic_string_view<C, T>> {};

	template<class C, class T, class A, size_t N, template<class> class TQUAL, template<class> class QQUAL>
	struct basic_common_reference<std::basic_string<C, T, A>, aa::basic_fixed_string<C, T, N>, TQUAL, QQUAL>
		: std::type_identity<std::basic_string_view<C, T>> {};

}
