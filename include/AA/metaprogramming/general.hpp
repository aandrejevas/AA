#pragma once

// AA bibliotekos failuose elgiamasi lyg būtų įterpti toliau išdėstyti failai, nes tie failai suteikia galimybę
// sklandžiai programuoti naudojantis esminėmis C++ kalbos savybėmis. Tie failai yra ir jų įterpimo priežastys:
// • <cstddef> ir <cstdint>, failai įterpti, kad nereiktų naudoti daug raktažodžių, kad aprašyti pamatinius tipus.
// • <array> (<compare>, <initializer_list>), failas įterptas, kad nereiktų naudoti C stiliaus masyvų.
// • <type_traits> ir <concepts>, failai įterpti, kad išeitų lengvai protauti apie tipus.
// • <utility> (<compare>, <initializer_list>), failas įterptas, kad išeitų lengvai protauti apie išraiškas.
// • <limits>, failas įterptas, kad išeitų lengvai nautotis pamatinių tipų savybėmis.
// • <bit>, failas įterptas, kad išeitų lengvai manipuliuoti pamatinių tipų bitus.
// • <functional>, failas įterptas, kad išeitų lengvai protauti apie funkcijų objektus.
// • <variant> (<compare>), failas įterptas, kad turėti alternatyvą nesaugiems union tipams.
// • <string_view> ir <string> (<compare>, <initializer_list>), failai įterpti, kad nereiktų naudoti C stiliaus teksto eilučių.
// • <streambuf>, failas įterptas, kad būtų suteiktas būtinas funkcionalumas kontroliuoti įvestį ir išvestį į simbolių sekas.
// Failai paminėti skliausteliose prie įterpiamo failo nurodo kokie failai yra įterpiami pačio įterpiamo failo.

#include <cstddef> // byte, size_t
#include <cstdint> // uint8_t, uint16_t, uint32_t, uint64_t, int8_t, int16_t, int32_t, int64_t
#include <type_traits> // remove_reference_t, is_lvalue_reference_v, is_rvalue_reference_v, type_identity, integral_constant, is_void_v, has_unique_object_representations_v, is_trivial_v, is_trivially_copyable_v, is_trivially_default_constructible_v, is_const_v, is_arithmetic_v, invoke_result_t, underlying_type_t, remove_cvref_t, is_pointer_v, remove_pointer_t, make_unsigned_t, is_invocable_r_v, make_signed_t, is_empty_v
#include <concepts> // same_as, semiregular, regular, relation, invocable, totally_ordered_with, equality_comparable_with, constructible_from, assignable_from, integral, signed_integral, unsigned_integral
#include <limits> // numeric_limits
#include <array> // array
#include <bit> // countl_zero, has_single_bit, bit_cast
#include <utility> // forward, declval, as_const, tuple_size, tuple_size_v, tuple_element, tuple_element_t, index_sequence, make_index_sequence, index_sequence_for
#include <functional> // function, invoke, _1, is_placeholder_v
#include <variant> // monostate
#include <string_view> // basic_string_view
#include <streambuf> // basic_streambuf
#include <string> // char_traits



namespace aa {

	template<class T>
	concept uses_type = requires { typename std::remove_reference_t<T>::type; };

	template<uses_type T>
	using type_in_use_t = typename std::remove_reference_t<T>::type;

	template<class T>
	concept uses_value_type = requires { typename std::remove_reference_t<T>::value_type; };

	template<uses_value_type T>
	using value_type_in_use_t = typename std::remove_reference_t<T>::value_type;

	template<class T>
	concept uses_reference = requires { typename std::remove_reference_t<T>::reference; };

	template<uses_reference T>
	using reference_in_use_t = typename std::remove_reference_t<T>::reference;

	template<class T>
	concept uses_const_reference = requires { typename std::remove_reference_t<T>::const_reference; };

	template<uses_const_reference T>
	using const_reference_in_use_t = typename std::remove_reference_t<T>::const_reference;

	template<class T>
	concept complete = requires { sizeof(T); };

	template<class T>
	concept incomplete = !complete<T>;

	template<class T>
	concept empty_type = std::is_empty_v<T>;



	using byte_t = std::underlying_type_t<std::byte>;

	template<auto V>
	using constant = std::integral_constant<decltype(V), V>;

	template<size_t V>
	using size_constant = constant<V>;

	// Nieko tokio, kad kopijuojame konstantas, kadangi viskas vyksta kompiliavimo metu.
	//
	// Negalime konstantų pakeisti funkcijomis, nes neišeina gauti adreso funkcijos
	// rezultato. Tai reiškia, kad šitoks sprendimas yra universalesnis.
	//
	// constant_t alias neturėtų prasmės, nes, kad juo naudotis jau reiktų nurodyti ką norime gauti.
	template<auto V>
	using const_t = value_type_in_use_t<constant<V>>;

	template<auto V>
	constexpr const_t<V> const_v = V;



	template<class T, class... A>
	concept semiregular_constructible_from = std::semiregular<T> && std::constructible_from<T, A...>;

	template<class T, auto... A>
	concept semiregular_constexpr_constructible = semiregular_constructible_from<T, const_t<A>...>
		&& (requires { const_v<T(A...)>; });

	namespace detail {
		// T tipas nesiskiria kai esame const T & contekste ar T kontekste. Tai reiškia, kad
		// privalome pasirinkti gražinti const T & arba T visiems atvejams. Yra pasirinkta gražinti T,
		// nes buvo nuspręsta, kad šias klases turi būti įmanoma naudoti ir ne su constexpr konstruktorius
		// turinčiomis klasėmis ir joms neišeitų gražinti const T &. Jei vis dėlto reikia const T &
		// tipo, galima dirbti su atitinkamais constexpr kintamaisiais, kurie yra apibrėžti po šių klasių.
		template<auto... A>
		struct value_getter {
			template<semiregular_constexpr_constructible<A...> T>
			consteval operator T() const { return T(A...); }

			template<semiregular_constructible_from<const_t<A>...> T>
			constexpr operator T() const { return T(A...); }
		};

		struct numeric_max_getter {
			template<std::semiregular T>
			consteval operator T() const { return std::numeric_limits<T>::max(); }
		};

		struct numeric_min_getter {
			template<std::semiregular T>
			consteval operator T() const { return std::numeric_limits<T>::min(); }
		};
	}

	template<auto... A>
	constexpr detail::value_getter<A...> value;

	constexpr detail::value_getter<> default_value;

	constexpr detail::numeric_max_getter numeric_max;

	constexpr detail::numeric_min_getter numeric_min;

	template<std::semiregular T, auto... A>
	constexpr T value_v = value<A...>;

	template<std::semiregular T>
	constexpr T default_value_v = default_value;

	template<std::semiregular T>
	constexpr T numeric_max_v = numeric_max;

	template<std::semiregular T>
	constexpr T numeric_min_v = numeric_min;

	template<std::semiregular T>
	constexpr size_t numeric_digits_v = std::numeric_limits<T>::digits;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
	template<auto... A, std::regular T>
	constexpr bool is_value(const T &x) {
		return x == value_v<T, A...>;
	}

	template<std::regular T>
	constexpr bool is_default_value(const T &x) {
		return x == default_value_v<T>;
	}

	template<std::regular T>
	constexpr bool is_numeric_max(const T &x) {
		return x == numeric_max_v<T>;
	}

	template<std::regular T>
	constexpr bool is_numeric_min(const T &x) {
		return x == numeric_min_v<T>;
	}
#pragma GCC diagnostic pop



	namespace detail {
		template<size_t, class T>
		struct tuple_unit {
			// Member types
			using value_type = T;
			using reference = value_type &;
			using const_reference = const value_type &;

			// Element access
			constexpr reference get() { return value; }
			constexpr const_reference get() const { return value; }

			// Member objects
			// Nenaudojame atributo no_unique_address, nes jis čia nieko nekeistų.
			value_type value;
		};

		template<size_t I, empty_type T>
		struct tuple_unit<I, T> {
			// Member types
			using value_type = T;
			using reference = const value_type &;
			using const_reference = reference;

			// Element access
			static consteval const_reference get() { return default_value; }
		};

		template<class, class...>
		struct tuple_base;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpadded"
		template<size_t... I, class... T>
		struct tuple_base<std::index_sequence<I...>, T...> : tuple_unit<I, T>... {};
#pragma GCC diagnostic pop
	}

	// https://ldionne.com/2015/11/29/efficient-parameter-pack-indexing/
	// GCC bug: kai apibrėžiame tik using rejects valid code
	template<size_t I, class... T>
	constexpr std::type_identity type_pack_element_v = ([]<class U>(const detail::tuple_unit<I, U>) static -> U
	{ return default_value; })(default_value_v<detail::tuple_base<std::index_sequence_for<T...>, std::type_identity<T>...>>);

	template<size_t I, class... T>
	using type_pack_element_t = type_in_use_t<const_t<type_pack_element_v<I, T...>>>;

	template<class U, class... T>
	constexpr size_t type_pack_index_v = ([]<size_t I>(const detail::tuple_unit<I, std::type_identity<U>>) static -> size_t
	{ return I; })(default_value_v<detail::tuple_base<std::index_sequence_for<T...>, std::type_identity<T>...>>);

	// https://danlark.org/2020/04/13/why-is-stdpair-broken/
	// https://en.wikipedia.org/wiki/Tuple
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpadded"
	template<class... T>
	struct tuple : detail::tuple_base<std::index_sequence_for<T...>, T...> {
#pragma GCC diagnostic pop
		// Member types
		using tuple_type = tuple;
		using size_type = size_t;

		template<size_type I>
		using unit_type = detail::tuple_unit<I, type_pack_element_t<I, T...>>;

		template<size_type I>
		using value_type = value_type_in_use_t<unit_type<I>>;

		template<size_type I>
		using reference = reference_in_use_t<unit_type<I>>;

		template<size_type I>
		using const_reference = const_reference_in_use_t<unit_type<I>>;

		// Member constants
		template<class U>
		static constexpr size_type index = type_pack_index_v<U, T...>;

		// Capacity
		static consteval size_type tuple_size() { return sizeof...(T); }

		// Element access
		template<size_type I>
		constexpr reference<I> get() { return unit_type<I>::get(); }

		template<size_type I>
		constexpr const_reference<I> get() const { return unit_type<I>::get(); }

		template<class U>
		constexpr reference<index<U>> get() { return get<index<U>>(); }

		template<class U>
		constexpr const_reference<index<U>> get() const { return get<index<U>>(); }
	};

	template<class... T>
	tuple(T&&...) -> tuple<T...>;

	template<class T1>
	using unit = tuple<T1>;

	template<class T1, class T2 = T1>
	using pair = tuple<T1, T2>;

	template<class T1, class T2 = T1, class T3 = T2>
	using triplet = tuple<T1, T2, T3>;

	template<class T1, class T2 = T1, class T3 = T2, class T4 = T3>
	using quartet = tuple<T1, T2, T3, T4>;

	template<class T1, class T2 = T1, class T3 = T2, class T4 = T3, class T5 = T4>
	using quintet = tuple<T1, T2, T3, T4, T5>;

	template<class T1, class T2 = T1, class T3 = T2, class T4 = T3, class T5 = T4, class T6 = T5>
	using sextet = tuple<T1, T2, T3, T4, T5, T6>;



	namespace detail {
		template<size_t, auto V>
		struct pack_unit {};

		template<class, auto...>
		struct pack_base;

		template<size_t... I, auto... V>
		struct pack_base<std::index_sequence<I...>, V...> : pack_unit<I, V>... {};
	}

	template<size_t I, auto... V>
	constexpr auto pack_element_v = ([]<auto A>(const detail::pack_unit<I, A>) static -> const_t<A>
	{ return A; })(default_value_v<detail::pack_base<std::index_sequence_for<const_t<V>...>, V...>>);

	template<size_t I, auto... V>
	using pack_element_t = const_t<pack_element_v<I, V...>>;

	template<auto A, auto... V>
	constexpr size_t pack_index_v = ([]<size_t I>(const detail::pack_unit<I, A>) static -> size_t
	{ return I; })(default_value_v<detail::pack_base<std::index_sequence_for<const_t<V>...>, V...>>);

	template<auto... V>
	struct pack {
		// Member types
		using pack_type = pack;
		using size_type = size_t;

		template<size_type I>
		using value_type = pack_element_t<I, V...>;

		// Member constants
		template<auto A>
		static constexpr size_type index = pack_index_v<A, V...>;

		// Capacity
		static consteval size_type tuple_size() { return sizeof...(V); }

		// Element access
		template<size_type I>
		static consteval value_type<I> get() { return pack_element_v<I, V...>; }
	};



	template<class T>
	concept uses_char_type = requires { typename std::remove_reference_t<T>::char_type; };

	template<class T>
	concept uses_int_type = requires { typename std::remove_reference_t<T>::int_type; };

	template<class T>
	concept uses_off_type = requires { typename std::remove_reference_t<T>::off_type; };

	template<class T>
	concept uses_pos_type = requires { typename std::remove_reference_t<T>::pos_type; };

	template<class T>
	concept uses_state_type = requires { typename std::remove_reference_t<T>::state_type; };

	template<class T>
	concept char_traits_like = uses_char_type<T> && uses_int_type<T> && uses_off_type<T> && uses_pos_type<T> && uses_state_type<T>;

	// Netikriname ar tipo reikšmės using yra toks pat kaip jo traits char_type, nes skirtingi tipai gali skirtingų
	// pavadinimų reikšmės using'us turėti ar gali išviso tokio using neturėti. Patiems tipams paliekama tikrintis ar
	// jų reikšmės using'ai sutampa su jų traits char_type, o dėl šio concept tiesiog galima naudoti traits char_type.
	template<class T>
	concept uses_traits_type = (requires { typename std::remove_reference_t<T>::traits_type; })
		&& char_traits_like<typename std::remove_reference_t<T>::traits_type>;

	template<uses_traits_type T>
	using traits_type_in_use_t = typename std::remove_reference_t<T>::traits_type;

	template<uses_char_type T>
	using char_type_in_use_t = typename std::remove_reference_t<T>::char_type;

	template<uses_int_type T>
	using int_type_in_use_t = typename std::remove_reference_t<T>::int_type;

	template<uses_off_type T>
	using off_type_in_use_t = typename std::remove_reference_t<T>::off_type;

	template<uses_pos_type T>
	using pos_type_in_use_t = typename std::remove_reference_t<T>::pos_type;

	template<uses_state_type T>
	using state_type_in_use_t = typename std::remove_reference_t<T>::state_type;

	template<class T>
	concept uses_view_type = requires { typename std::remove_reference_t<T>::view_type; };

	template<uses_view_type T>
	using view_type_in_use_t = typename std::remove_reference_t<T>::view_type;

	template<class T>
	concept uses_allocator_type = requires { typename std::remove_reference_t<T>::allocator_type; };

	template<uses_allocator_type T>
	using allocator_type_in_use_t = typename std::remove_reference_t<T>::allocator_type;

	template<class T>
	concept uses_iterator = requires { typename std::remove_reference_t<T>::iterator; };

	template<uses_iterator T>
	using iterator_in_use_t = typename std::remove_reference_t<T>::iterator;

	// Constraint negali būti pakeistas į concept, nes pirmas parametras concept'o nebūtų tipas.
	template<template<class...> class T, class... A>
		requires (requires { T(std::declval<A>()...); })
	using deduced_template_t = decltype(T(std::declval<A>()...));

	template<class... A>
	using first_t = type_pack_element_t<0, A...>;

	// Nors galėtume paveldėti tiesiog iš first, bet to nedarome, kad nekurti nereikalingų paveldėjimo ryšių.
	template<class... A>
	using first_or_void_t = first_t<A..., void>;

	template<class T>
	concept not_const = !std::is_const_v<std::remove_reference_t<T>>;

	template<class... A>
	concept same_as_every = (... && std::same_as<first_t<A...>, A>);

	template<class T, class... A>
	concept same_as_any = (... || std::same_as<T, A>);

	template<class T>
	concept same_as_void = std::is_void_v<T>;

	template<class T, size_t I = numeric_max>
	concept placeholder = (is_numeric_max(I) ? (!!std::is_placeholder_v<T>) : (std::is_placeholder_v<T> == I));

	template<class T>
	concept pointer = std::is_pointer_v<T>;

	template<class T>
	concept lvalue_reference = std::is_lvalue_reference_v<T>;

	template<class T>
	concept rvalue_reference = std::is_rvalue_reference_v<T>;

	template<class T>
	concept arithmetic = std::is_arithmetic_v<T>;

	template<class T, class F>
	concept arithmetic_or_assignable_from = arithmetic<T> || std::assignable_from<T, F>;

	template<class T>
	concept regular_scalar = arithmetic<T> || pointer<T>;

	template<class T>
	concept uniquely_representable = std::has_unique_object_representations_v<T>;

	template<class T>
	concept trivial = std::is_trivial_v<T>;

	template<class T>
	concept trivially_copyable = std::is_trivially_copyable_v<T>;

	template<class T>
	concept trivially_default_constructible = std::is_trivially_default_constructible_v<T>;

	template<class L, class R>
	concept wo_ref_same_as = std::same_as<std::remove_reference_t<L>, R>;

	template<class T>
	concept regular_unsigned_integral = std::unsigned_integral<T> && std::has_single_bit(numeric_digits_v<T>);

	template<class T, class U>
	concept unsigned_integral_or_same_as = std::unsigned_integral<T> || std::same_as<T, U>;

	// https://en.wikipedia.org/wiki/Function_object
	template<class T>
	concept functor = requires { &T::operator(); };

	template<class F, class... A>
	concept ref_invocable = std::invocable<F &, A...>;

	template<class F, class R, class... A>
	concept invocable_r = std::is_invocable_r_v<R, F, A...>;

	template<class F, class T, class... A>
	concept invoke_result_constructible_to = (std::invocable<F, A...> && std::constructible_from<T, std::invoke_result_t<F, A...>>);

	template<class F, class T>
	concept constructible_to = std::constructible_from<T, F>;

	template<class R, class L>
	concept assignable_to = std::assignable_from<L, R>;

	template<class T>
	concept constructible_from_floating_point =
		(std::constructible_from<T, float> && std::constructible_from<T, double> && std::constructible_from<T, long double>);

	template<class T>
	using string_view_t = std::basic_string_view<char_type_in_use_t<traits_type_in_use_t<T>>, traits_type_in_use_t<T>>;

	template<class T>
	using streambuf_t = std::basic_streambuf<char_type_in_use_t<traits_type_in_use_t<T>>, traits_type_in_use_t<T>>;

	// Sakyčiau standartinės bibliotekos klaida, kad ji nenurodo numatyto char_traits tipo.
	using char_traits = std::char_traits<char>;



	template<class T, constructible_to<T> X>
	constexpr T cast(X &&x) {
		return static_cast<T>(std::forward<X>(x));
	}

	template<std::unsigned_integral X>
	constexpr std::make_signed_t<X> sign(const X x) {
		return std::bit_cast<std::make_signed_t<X>>(x);
	}

	template<std::signed_integral X>
	constexpr std::make_unsigned_t<X> unsign(const X x) {
		return std::bit_cast<std::make_unsigned_t<X>>(x);
	}

	template<std::unsigned_integral T, std::signed_integral X>
	constexpr T unsign(const X x) {
		return cast<T>(unsign(x));
	}

	template<class T, constructible_to<T> X>
	constexpr T unsign_cast(X &&x) {
		if constexpr (std::unsigned_integral<T> && std::signed_integral<std::remove_reference_t<X>>) {
			return unsign<T>(x);
		} else {
			return cast<T>(std::forward<X>(x));
		}
	}



	template<class, class>
	constexpr bool is_instance_of_v = false;

	template<template<class...> class F, class... A1, class... A2>
	constexpr bool is_instance_of_v<F<A1...>, F<A2...>> = true;

	template<template<auto...> class F, auto... A1, auto... A2>
	constexpr bool is_instance_of_v<F<A1...>, F<A2...>> = true;

	template<template<class, auto...> class F, class T1, auto... A1, class T2, auto... A2>
	constexpr bool is_instance_of_v<F<T1, A1...>, F<T2, A2...>> = true;

	template<class T, class U>
	concept instance_of = is_instance_of_v<std::remove_reference_t<T>, U>;



	// Su funkcija std::to_array neišeina sukurti tuščio masyvo.
	template<class T, T... A>
	consteval std::array<T, sizeof...(A)> to_array() {
		return {A...};
	}

	template<auto A1, const_t<A1>... A>
	consteval std::array<const_t<A1>, 1 + sizeof...(A)> to_array() {
		return {A1, A...};
	}



	// T čia neturi būti tuple_like, nes tuple_like tipo visi get validūs, o čia tikrinamas tik vienas get.
	template<class T, size_t I>
	concept member_get_exists = requires(std::remove_cvref_t<T> &t) {
		t.template get<I>();
		std::as_const(t).template get<I>();
	};

	template<class T, size_t I>
	concept adl_get_exists = requires(std::remove_cvref_t<T> &t) {
		get<I>(t);
		get<I>(std::as_const(t));
	};

	template<class T, size_t I>
	concept gettable = member_get_exists<T, I> || adl_get_exists<T, I>;

	// GCC bug: jei bandome alias realizuoti be struct tai compiler rejects valid code, nes nepalaiko lambda type gavimo.
	template<size_t I, gettable<I> T>
	constexpr auto get_result_v = [] static {
		// declval viduje reference, nes getter taip veikia, o getter taip veikia, nes iš standarto imiau pavyzdį.
		if constexpr (member_get_exists<T, I>)
			return default_value_v<std::type_identity<decltype(std::declval<T &>().template get<I>())>>;
		else
			return default_value_v<std::type_identity<decltype(get<I>(std::declval<T &>()))>>;
	};

	template<size_t I, gettable<I> T>
	using get_result_t = type_in_use_t<const_t<get_result_v<I, T>()>>;

	// Negalime turėti funkcijos, nes neišeina turėti function aliases patogių.
	template<size_t I>
	constexpr auto getter_v = []<gettable<I> T>(T &&t) static -> get_result_t<I, T> {
		if constexpr (member_get_exists<T, I>)	return t.template get<I>();
		else									return get<I>(t);
	};

	constexpr const_t<getter_v<0>> get_0, get_x, get_w;
	constexpr const_t<getter_v<1>> get_1, get_y, get_h;
	constexpr const_t<getter_v<2>> get_2, get_z;
	constexpr const_t<getter_v<3>> get_3;



	// Neįmanoma requires į concept paversti.
	template<auto... A, class F, class... T>
		requires (requires(F &&f, T&&... t) { std::forward<F>(f).template operator()<A...>(std::forward<T>(t)...); })
	constexpr decltype(auto) invoke(F &&f, T&&... t) {
		return std::forward<F>(f).template operator()<A...>(std::forward<T>(t)...);
	}

	// Negalime turėti funkcijos, nes ant funkcijų negalima atlikti partial specialization.
	template<instance_of<std::index_sequence<>>>
	constexpr std::monostate applier_v;

	template<size_t... I>
	constexpr auto applier_v<std::index_sequence<I...>> = []<class F, class... A>(F &&f, A&&... args) static
		-> decltype(auto) { return invoke<I...>(std::forward<F>(f), std::forward<A>(args)...); };

	template<size_t N, class F, class... A>
	constexpr decltype(auto) apply(F &&f, A&&... args) {
		return applier_v<std::make_index_sequence<N>>(std::forward<F>(f), std::forward<A>(args)...);
	}

	template<class T, size_t N = numeric_max>
	concept tuple_like = complete<std::tuple_size<T>> && (is_numeric_max(N) || std::tuple_size_v<T> == N) && apply<std::tuple_size_v<T>>(
		[]<size_t... I> -> bool { return (... && wo_ref_same_as<get_result_t<I, T>, std::tuple_element_t<I, T>>); });

	template<class T, size_t N = numeric_max, class F, class... A>
		requires (tuple_like<T, N>)
	constexpr decltype(auto) apply(F &&f, A&&... args) {
		return aa::apply<std::tuple_size_v<T>>(std::forward<F>(f), std::forward<A>(args)...);
	}

	template<class F, class T, class... A>
		requires (std::invocable<F, T &, A...>)
	constexpr T &extend(T &t, F &&f, A&&... args) {
		std::invoke(std::forward<F>(f), t, std::forward<A>(args)...);
		return t;
	}



	template<bool B, class T>
	using add_const_if_t = std::conditional_t<B, const T, T>;

	// Nenaudojame dangling, nes ta klasė naudojama ranges kontekste.
	template<class T, class U = void>
	using borrowed_t = std::conditional_t<lvalue_reference<T>, std::conditional_t<same_as_void<U>, T, U>, void>;

	// Reikia using šio, nes testavimui reikėjo sukurti tuple su 100 elementų ir nėra variantas turėti 100 using'ų.
	template<template<class...> class T, auto F, size_t N>
	using filled_t = type_in_use_t<const_t<apply<N>([]<size_t... I> ->
		std::type_identity<T<type_in_use_t<const_t<invoke<I>(F)>>...>> { return default_value; })>>;

	template<class T>
	concept new_tuple_like = apply<std::remove_cvref_t<T>::tuple_size()>(
		[]<size_t... I> -> bool { return (... && gettable<T, I>); });

	template<class T, size_t N = numeric_max>
	concept array_like = apply<T, N>([]<size_t... I> -> bool { return same_as_every<get_result_t<I, T>...>; });

	// Nereikia N parametro, nes jei naudotojas naudotų N=0, tai tada jis žinotų, kad concept bus false ir atvirkščiai.
	template<class T>
	concept not_empty_array_like = !!std::tuple_size_v<T> && array_like<T>;

	template<not_empty_array_like T>
	using array_element_t = std::tuple_element_t<0, T>;

	template<class T, size_t N = numeric_max>
	concept arithmetic_array_like = (array_like<T, N> && arithmetic<array_element_t<T>>);

	template<class T, class U>
	concept array_similar_to = (std::same_as<array_element_t<T>, array_element_t<U>> && std::tuple_size_v<T> == std::tuple_size_v<U>);

	template<class F, size_t I = 0>
	using constifier_func_t = const_t<&std::remove_cvref_t<F>::template operator()<I>>;

	template<class F, size_t I>
	constexpr constifier_func_t<F, I> constifier_func_v = &std::remove_cvref_t<F>::template operator()<I>;

	template<class F, size_t N>
	concept constifier_like = apply<N>([]<size_t... I> -> bool { return same_as_every<constifier_func_t<F, I>...>; });

	template<class F, size_t N, class... A>
	concept invocable_constifier = (constifier_like<F, N> && std::invocable<constifier_func_t<F>, F, A...>);

	template<class F, size_t N, class R, class... A>
	concept invocable_r_constifier = (constifier_like<F, N> && invocable_r<constifier_func_t<F>, R, F, A...>);



	template<size_t N, constifier_like<N> F>
	constexpr std::array constifier_table = apply<N>([]<size_t... I> ->
		std::array<constifier_func_t<F>, N> { return {constifier_func_v<F, I>...}; });

	template<size_t N, class F, class... A>
		requires (invocable_constifier<F, N, A...>)
	constexpr decltype(auto) constify(const size_t i, F &&f, A&&... args) {
		return (std::forward<F>(f).*constifier_table<N, F>[i])(std::forward<A>(args)...);
	}

	template<tuple_like T, class F, class... A>
	constexpr decltype(auto) constify(const size_t i, F &&f, A&&... args) {
		return constify<std::tuple_size_v<T>>(i, std::forward<F>(f), std::forward<A>(args)...);
	}



	namespace detail {
		template<class T>
		struct propagate_const : std::type_identity<const T> {};

		template<pointer T>
		struct propagate_const<T> : std::type_identity<type_in_use_t<propagate_const<std::remove_pointer_t<T>>> *const> {};

		template<lvalue_reference T>
		struct propagate_const<T> : std::type_identity<type_in_use_t<propagate_const<std::remove_reference_t<T>>> &> {};

		template<rvalue_reference T>
		struct propagate_const<T> : std::type_identity<type_in_use_t<propagate_const<std::remove_reference_t<T>>> &&> {};
	}

	template<class T>
	using propagate_const_t = type_in_use_t<detail::propagate_const<T>>;



	template<class U, class V, class T>
	concept in_relation_with = std::relation<T, const U &, const V &>;

	template<class U, class V, class T>
	concept in_relation_with_and_assignable_to = (in_relation_with<U, V, T> && assignable_to<const U &, V &>);

	template<class T, class U, class V = U>
	concept relation_for = in_relation_with<U, V, const T &>;

	template<class U, class T>
	concept hashable_by = invocable_r<T, size_t, const U &>;

	template<class U, template<class> class T>
	concept hashable_by_template = (hashable_by<U, T<U>> && trivially_default_constructible<T<U>>);

	template<class T, class U>
	concept hasher_for = hashable_by<U, const T &>;

	template<class U, class T>
	concept evaluable_by = invocable_r<T, bool, int, U &>;

	template<class T, class U>
	concept evaluator_for = evaluable_by<U, T &>;

	template<class U, class T, size_t N = numeric_max>
	concept tuple_evaluable_by = apply<U, N>([]<size_t... I> -> bool { return (... && evaluable_by<std::tuple_element_t<I, U>, T>); });

	template<class T, class U, size_t N = numeric_max>
	concept evaluator_for_tuple = tuple_evaluable_by<U, T &, N>;

	template<class T, class U, size_t N = numeric_max>
	concept arithmetic_array_getter_like = (std::invocable<const T &, const U &>
		&& arithmetic_array_like<std::remove_cvref_t<std::invoke_result_t<const T &, const U &>>, N>);

	template<class U, arithmetic_array_getter_like<U> T>
	using arithmetic_array_getter_result_t = std::remove_cvref_t<std::invoke_result_t<const T &, const U &>>;



	namespace detail {
		// https://mathworld.wolfram.com/Hypermatrix.html
		// Gal reiktų vadinti struktūrą ne array, o hypermatrix, bet pasiliekama prie array vardo,
		// nes galime interpretuoti stuktūros pavadinimą kaip masyvą masyvų. Taip pat struktūrą vadinti
		// matrix būtų netikslinga, nes tai implikuotų, kad struktūra palaiko matricos operacijas.
		template<class T, size_t N1, size_t... N>
		struct array : std::type_identity<type_in_use_t<array<std::array<T, N1>, N...>>> {};

		template<class T, size_t N1>
		struct array<T, N1> : std::type_identity<std::array<T, N1>> {};
	}

	template<class T, size_t... N>
		requires (!!sizeof...(N))
	using array_t = type_in_use_t<detail::array<T, N...>>;

	namespace detail {
		// https://mathworld.wolfram.com/Hypercube.html
		template<class T, size_t D, size_t N>
		struct hypercube_array : std::type_identity<type_in_use_t<hypercube_array<std::array<T, N>, D - 1, N>>> {};

		template<class T, size_t N>
		struct hypercube_array<T, 1, N> : std::type_identity<std::array<T, N>> {};
	}

	template<class T, size_t D, size_t N>
	using hypercube_array_t = type_in_use_t<detail::hypercube_array<T, D, N>>;

	// https://mathworld.wolfram.com/SquareArray.html
	template<class T, size_t N>
	using square_array_t = hypercube_array_t<T, 2, N>;

	template<class T, size_t N>
	using cube_array_t = hypercube_array_t<T, 3, N>;



	template<std::integral U = size_t, std::unsigned_integral T>
	constexpr U int_exp2(const T x) {
		return value_v<U, 1> << x;
	}

	// T yra tipas, kurio bitus skaičiuosime, U nurodo kokiu tipu pateikti rezutatus.
	// [0, digits<T>) ∪ {0b(1)_digits<T>}
	template<std::integral U = size_t, std::unsigned_integral T>
	constexpr U int_log2(const T x) {
		return (value_v<U, numeric_digits_v<std::make_signed_t<T>>>) - unsign_cast<U>(std::countl_zero(x));
	}



	// Naudojamas size_t tipas kaip konstantos tipas, o ne sekančio dydžio integer tipas, nes konstanta
	// turėtų būti naudojama malloc ir panašiuose kontekstuose, o ten reikalaujama size_t tipo išraiška.
	// T turi turėti unique object representations, nes kitaip neišeis patikimai apskaičiuoti konstantos.
	//
	// Vietoje byte negalime naudoti uint8_t, nes jei sistemoje baitas būtų ne 8 bitų, tas tipas nebus apibrėžtas.
	template<uniquely_representable T>
	constexpr size_t representable_values_v = int_exp2(sizeof(T[numeric_digits_v<byte_t>]));



	// Galėtume vietoje _1 naudoti std::ignore. Bet minėtos konstantos tipas nėra tuščias.
	// Taip pat jei reiktų negalėtume to pačio parametro su skirtingomis konstantomis specializuoti.
	//
	// Comparators do not declare is_transparent, nes jų ir taip neišeitų panaudoti su c++ standarto konteineriais.
	//
	// Mes nurodome dešinės pusės operandą su template parametru, nes realizuojame šią gražią elgseną: sakykime turime
	// objektą tipo less<3>, šio objekto operator() grąžins true tik tada kai paduotas kintamasis bus mažesnis už 3.
	template<auto R = std::placeholders::_1>
	struct less {
		template<std::totally_ordered_with<const_t<R>> L>
		static constexpr bool operator()(const L &l) { return l < R; }
	};

	template<>
	struct less<std::placeholders::_1> {
		template<auto R, std::totally_ordered_with<const_t<R>> L>
		static constexpr bool operator()(const L &l) { return l < R; }
	};

	template<auto R = std::placeholders::_1>
	struct less_equal {
		template<std::totally_ordered_with<const_t<R>> L>
		static constexpr bool operator()(const L &l) { return l <= R; }
	};

	template<>
	struct less_equal<std::placeholders::_1> {
		template<auto R, std::totally_ordered_with<const_t<R>> L>
		static constexpr bool operator()(const L &l) { return l <= R; }
	};

	template<auto R = std::placeholders::_1>
	struct greater {
		template<std::totally_ordered_with<const_t<R>> L>
		static constexpr bool operator()(const L &l) { return l > R; }
	};

	template<>
	struct greater<std::placeholders::_1> {
		template<auto R, std::totally_ordered_with<const_t<R>> L>
		static constexpr bool operator()(const L &l) { return l > R; }
	};

	template<auto R = std::placeholders::_1>
	struct greater_equal {
		template<std::totally_ordered_with<const_t<R>> L>
		static constexpr bool operator()(const L &l) { return l >= R; }
	};

	template<>
	struct greater_equal<std::placeholders::_1> {
		template<auto R, std::totally_ordered_with<const_t<R>> L>
		static constexpr bool operator()(const L &l) { return l >= R; }
	};

	template<auto R = std::placeholders::_1>
	struct equal_to {
		template<std::equality_comparable_with<const_t<R>> L>
		static constexpr bool operator()(const L &l) { return l == R; }
	};

	template<>
	struct equal_to<std::placeholders::_1> {
		template<auto R, std::equality_comparable_with<const_t<R>> L>
		static constexpr bool operator()(const L &l) { return l == R; }
	};

	template<auto R = std::placeholders::_1>
	struct not_equal_to {
		template<std::equality_comparable_with<const_t<R>> L>
		static constexpr bool operator()(const L &l) { return l != R; }
	};

	template<>
	struct not_equal_to<std::placeholders::_1> {
		template<auto R, std::equality_comparable_with<const_t<R>> L>
		static constexpr bool operator()(const L &l) { return l != R; }
	};



	// Lyginimas su 0 yra greitesnė operacija negu lyginimas su kokia kita konstanta.
	template<ref_invocable F>
	constexpr void unsafe_repeat(size_t e_S1, F &&f) {
		do { std::invoke(f); if (e_S1 != 0) --e_S1; else return; } while (true);
	}

	template<ref_invocable<size_t> F>
	constexpr void unsafe_repeat(size_t e_S1, F &&f) {
		do { std::invoke(f, std::as_const(e_S1)); if (e_S1 != 0) --e_S1; else return; } while (true);
	}

	// Funkcijos nepilnai generic kaip iota_view, nes neegzistuoja weakly_decrementable concept.
	// Jei reikia normalių indeksų, tada jau reikia naudoti nebe šias funkcijas, o pavyzdžiui iota_view su for_each.
	// Galėtume kažkokius argumentus kiekvieną iteraciją paduoti į funkciją, bet niekur taip nėra realizuoti algoritmai.
	template<ref_invocable F>
	constexpr void repeat(const size_t e, F &&f) {
		unsafe_repeat(e - 1, f);
	}

	template<ref_invocable<size_t> F>
	constexpr void repeat(const size_t e, F &&f) {
		unsafe_repeat(e - 1, f);
	}

	// Parameter pack negali susidėti iš daug elementų todėl šitą funkciją reiktų naudoti tik kai reikia mažai
	// iteracijų. Tačiau toks iteravimas greitesnis, nes nereikia prižiūrėti papildomo ciklo kintamojo.
	//
	// Greitaveika nenukenčia padavinėjant template parametrus todėl neturime funkcijos užklojimo kito.
	template<size_t N, class F>
	constexpr void repeat(F &&f) {
		apply<N>([&]<size_t... I> -> void { (aa::invoke<I>(f), ...); });
	}

	template<tuple_like T, class F>
	constexpr void repeat(F &&f) {
		repeat<std::tuple_size_v<T>>(std::forward<F>(f));
	}



	// Atrodo galima būtų turėti tipą, kuris generalizuotu šią klasę, tai yra tiesiog jis susidėtų iš
	// paduotų tipų. Bet toks tipas nebūtų naudingas, nes užklojimai neveiktų gerai, todėl būtent ir
	// reikia šio overload tipo, kad bent jau veiktų operatoriaus () užklojimai gerai. Tačiau realizacija
	// reikalauja, kad visi paduodami tipai turėtų būtinai tik vieną operatorių (), gal būtų galima realizuoti
	// taip tipą, kad tokio reikalavimo neliktų, bet tokios realizacijos savybės dabar nereikalingos.
	template<functor... T>
	struct overload : T... {
		using T::operator()...;
		using is_transparent = void;
	};



	namespace detail {
		template<class F, size_t I>
		struct function_argument : std::type_identity<type_in_use_t<function_argument<deduced_template_t<std::function, F>, I>>> {};

		template<class R, class... A, size_t I>
		struct function_argument<std::function<R(A...)>, I> : std::type_identity<type_pack_element_t<I, A...>> {};

		template<class R, class... A>
		struct function_argument<std::function<R(A...)>, numeric_max> : std::type_identity<R> {};
	}

	template<class F, size_t I = 0>
	using function_argument_t = type_in_use_t<detail::function_argument<F, I>>;

	template<class F>
	using function_result_t = function_argument_t<F, numeric_max>;

	template<class F>
	concept unary_invocable = std::invocable<F, function_argument_t<F>>;

	template<class F>
	concept out_unary_invocable = unary_invocable<F> && lvalue_reference<function_argument_t<F>>
		&& std::semiregular<std::remove_reference_t<function_argument_t<F>>>;



	template<class T, class... A>
	using next_type_t = type_pack_element_t<(type_pack_index_v<T, A...>) + 1, A...>;

	template<class T>
	using next_unsigned_t = next_type_t<T, uint8_t, uint16_t, uint32_t, uint64_t>;

	template<class T>
	using prev_unsigned_t = next_type_t<T, uint64_t, uint32_t, uint16_t, uint8_t>;

	template<class T>
	using next_signed_t = next_type_t<T, int8_t, int16_t, int32_t, int64_t>;

	template<class T>
	using prev_signed_t = next_type_t<T, int64_t, int32_t, int16_t, int8_t>;

}



template<class T>
struct std::tuple_size<T &> : aa::size_constant<std::tuple_size_v<T>> {};

template<size_t I, class T>
struct std::tuple_element<I, T &> : std::type_identity<std::tuple_element_t<I, T>> {};

// Negalime tikrinti ar prieš šį momentą tuple_size<T> buvo deklaruotas tipas ar ne, nes įeitume į begalinį
// ciklą. Reiškia turi mums pats tipas pranešti ar jis nori būti laikomas kaip tuple like tipas.
template<aa::new_tuple_like T>
struct std::tuple_size<T> : aa::size_constant<T::tuple_size()> {};

template<size_t I, aa::new_tuple_like T>
struct std::tuple_element<I, T> : std::type_identity<std::remove_reference_t<aa::get_result_t<I, T>>> {};
