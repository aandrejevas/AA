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
// Failai paminėti skliausteliose prie įterpiamo failo nurodo kokie failai yra įterpiami pačio įterpiamo failo.

#include "../preprocessor/general.hpp"
#include <cstddef> // byte, size_t
#include <cstdint> // uint8_t, uint16_t, uint32_t, uint64_t, int8_t, int16_t, int32_t, int64_t
#include <type_traits> // remove_reference_t, is_lvalue_reference_v, is_rvalue_reference_v, type_identity, integral_constant, conditional, conditional_t, is_void_v, has_unique_object_representations_v, is_trivial_v, is_trivially_copyable_v, is_trivially_default_constructible_v, add_const_t, is_const_v, is_arithmetic_v, invoke_result_t, underlying_type_t, remove_cvref, remove_cvref_t, is_pointer_v, remove_pointer_t, is_function_v, make_unsigned_t, is_invocable_r_v
#include <concepts> // convertible_to, same_as, default_initializable, relation, invocable, derived_from, totally_ordered_with, equality_comparable, equality_comparable_with, constructible_from, assignable_from, signed_integral, unsigned_integral
#include <limits> // numeric_limits
#include <array> // array
#include <bit> // has_single_bit, bit_cast
#include <utility> // declval, as_const, tuple_size, tuple_size_v, tuple_element, tuple_element_t, index_sequence, make_index_sequence, index_sequence_for
#include <functional> // function, invoke, _1



namespace aa {

	// Neturime tiesiog auto parametro, nes tada reiktų kreipti dėmėsį į expression tipą, o tai nepatogu.
	template<class T, T V>
	struct constant_identity : std::integral_constant<T, V> {
		using type = typename constant_identity::value_type;
	};

	template<size_t V>
	using size_t_identity = constant_identity<size_t, V>;
	using uz2_identity = size_t_identity<2uz>;

	template<bool V>
	using bool_identity = constant_identity<bool, V>;
	using true_identity = bool_identity<true>;
	using false_identity = bool_identity<false>;



	template<class T, std::convertible_to<T> X>
	AA_CONSTEXPR T cast(X &&x) {
		return static_cast<T>(std::forward<X>(x));
	}

	template<std::signed_integral X>
	AA_CONSTEXPR std::make_unsigned_t<X> unsign(const X x) {
		return std::bit_cast<std::make_unsigned_t<X>>(x);
	}

	template<std::unsigned_integral T, std::signed_integral X>
	AA_CONSTEXPR T unsign(const X x) {
		return static_cast<T>(unsign(x));
	}

	template<class T, std::convertible_to<T> X>
	AA_CONSTEXPR T unsign_cast(X &&x) {
		if constexpr (std::unsigned_integral<T> && std::signed_integral<X>) {
			return unsign<T>(x);
		} else {
			return cast<T>(std::forward<X>(x));
		}
	}



	template<class T>
	concept char_traits_like = requires {
		typename T::char_type; typename T::int_type; typename T::off_type; typename T::pos_type; typename T::state_type; };

	// Netikriname ar tipo reikšmės using yra toks pat kaip jo traits char_type, nes skirtingi tipai gali skirtingų
	// pavadinimų reikšmės using'us turėti ar gali išviso tokio using neturėti. Patiems tipams paliekama tikrintis ar
	// jų reikšmės using'ai sutampa su jų traits char_type, o dėl šio concept tiesiog galima naudoti traits char_type.
	template<class T>
	concept uses_traits_type = (requires { typename std::remove_reference_t<T>::traits_type; })
		&& char_traits_like<typename std::remove_reference_t<T>::traits_type>;

	template<uses_traits_type T>
	struct traits_type_in_use : std::type_identity<typename std::remove_reference_t<T>::traits_type> {};

	template<uses_traits_type T>
	using traits_type_in_use_t = typename traits_type_in_use<T>::type;



	template<template<class...> class F, uses_traits_type T>
	struct apply_traits : std::type_identity<F<typename traits_type_in_use_t<T>::char_type, traits_type_in_use_t<T>>> {};

	template<template<class...> class F, class T>
	using apply_traits_t = typename apply_traits<F, T>::type;



	// Klasė galėtų būti pakeista į concept, bet nėra logiška, kad concept'as galėtų būti naudojamas su 0 template parametrų.
	template<class T = void, class... A>
	struct are_same : bool_identity<(... && std::same_as<T, A>)> {};

	template<class... A>
	AA_CONSTEXPR const bool are_same_v = are_same<A...>::value;

	template<class>
	struct is_template : false_identity {};

	template<template<class...> class T, class... A>
	struct is_template<T<A...>> : true_identity {};

	template<template<auto...> class T, auto... A>
	struct is_template<T<A...>> : true_identity {};

	template<class T>
	AA_CONSTEXPR const bool is_template_v = is_template<T>::value;

	template<class, template<class...> class>
	struct is_instance_of_twtp : false_identity {};

	template<template<class...> class T, class... A>
	struct is_instance_of_twtp<T<A...>, T> : true_identity {};

	template<class T, template<class...> class U>
	AA_CONSTEXPR const bool is_instance_of_twtp_v = is_instance_of_twtp<T, U>::value;

	template<class, template<auto...> class>
	struct is_instance_of_twntp : false_identity {};

	template<template<auto...> class T, auto... A>
	struct is_instance_of_twntp<T<A...>, T> : true_identity {};

	template<class T, template<auto...> class U>
	AA_CONSTEXPR const bool is_instance_of_twntp_v = is_instance_of_twntp<T, U>::value;

	// Constraint negali būti pakeistas į concept, nes pirmas parametras concept'o nebūtų tipas.
	template<template<class...> class T, class... A>
		requires (requires { T(std::declval<A>()...); })
	struct deduced_template : std::type_identity<decltype(T(std::declval<A>()...))> {};

	template<template<class...> class T, class... A>
	using deduced_template_t = typename deduced_template<T, A...>::type;



	template<class T>
	concept not_const = !std::is_const_v<T>;

	template<class T, template<auto...> class U>
	concept instance_of_twntp = is_instance_of_twntp_v<T, U>;

	template<class T, template<class...> class U>
	concept instance_of_twtp = is_instance_of_twtp_v<std::remove_cvref_t<T>, U>;

	template<class T, template<class...> class U>
	concept not_const_instance_of_twtp = not_const<T> && instance_of_twtp<T, U>;

	template<class T, template<class...> class U>
	concept not_instance_of_twtp = !instance_of_twtp<T, U>;

	template<class T, class... A>
	concept same_as_any = (... || std::same_as<T, A>);

	template<class T>
	concept same_as_void = std::is_void_v<T>;

	template<class T>
	concept pointer = std::is_pointer_v<T>;

	template<class T>
	concept lvalue_reference = std::is_lvalue_reference_v<T>;

	template<class T>
	concept rvalue_reference = std::is_rvalue_reference_v<T>;

	template<class T>
	concept arithmetic = std::is_arithmetic_v<T>;

	template<class T>
	concept not_const_arithmetic = not_const<T> && arithmetic<T>;

	template<class T>
	concept uniquely_representable = std::has_unique_object_representations_v<T>;

	template<class T>
	concept trivial = std::is_trivial_v<T>;

	template<class T>
	concept trivially_copyable = std::is_trivially_copyable_v<T>;

	template<class T>
	concept trivially_default_constructible = std::is_trivially_default_constructible_v<T>;

	template<class T>
	concept wo_cvref_default_initializable = std::default_initializable<std::remove_cvref_t<T>>;

	template<class L, class R>
	concept wo_ref_derived_from = std::derived_from<std::remove_reference_t<L>, R>;

	template<class L, class R>
	concept wo_ref_same_as = std::same_as<std::remove_reference_t<L>, R>;

	template<class T>
	concept regular_unsigned_integral = std::unsigned_integral<T> && std::has_single_bit(unsign(std::numeric_limits<T>::digits));

	// https://en.wikipedia.org/wiki/Function_object
	template<class T>
	concept functor = requires { &T::operator(); };

	template<class F, class... A>
	concept function_pointer = std::invocable<F, A...> && pointer<F> && std::is_function_v<std::remove_pointer_t<F>>;

	template<class F, class... A>
	concept invocable_ref = std::invocable<F &, A...>;

	template<class F, class R, class... A>
	concept invocable_r = std::is_invocable_r_v<R, F, A...>;

	template<class F, class T, class... A>
	concept invoke_result_constructible_to = std::invocable<F, A...> && std::constructible_from<T, std::invoke_result_t<F, A...>>;

	template<class F, class T>
	concept constructible_to = std::constructible_from<T, F>;

	template<class R, class L>
	concept assignable_to = std::assignable_from<L, R>;

	template<class R, class L>
	concept cref_assignable_to = std::assignable_from<L, const R &>;

	template<class T, class F>
	concept convertible_from = std::convertible_to<F, T>;

	template<class T>
	concept convertible_from_floating_point =
		std::convertible_to<float, T> && std::convertible_to<double, T> && std::convertible_to<long double, T>;



	// C yra objektas, galėtume jį padavinėti per const&, o ne per value, bet nusprendžiau, kad kompiliavimo
	// metu nėra skirtumo kaip tas objektas bus padavinėjamas, net jei jis būtų labai didelis.
	//
	// Negali šitos funkcijos būti paverstos į konstantas, nes neegzistuoja konstantų užklojimas ir dalinė specializacija nesikompiliuoja.
	// Taip pat funkcijos geriau atspindi kas vyksta, tai yra tą faktą, kad reikšmės gaunamos ne iš kažkokios klasės (pvz. integral_constant).
	template<class T, T C>
	AA_CONSTEVAL T constant() { return C; }

	template<std::default_initializable T>
	AA_CONSTEVAL T constant() { return T{}; }

	template<auto C>
	AA_CONSTEVAL decltype(C) constant() { return C; }



	// Galėtų šios klasės turėti get funkciją, bet sakome, kad naudotojas tegul naudoja numeric_limits, nereikia
	// turėti dviejų kelių, kad padaryti visiškai tą patį. Kad gauti pakeistą konstantą naudoti constant funkciją.
	struct numeric_max_t {
		template<class T>
		AA_CONSTEVAL operator T() const { return std::numeric_limits<T>::max(); }
	};

	AA_CONSTEXPR const numeric_max_t numeric_max;

	struct numeric_min_t {
		template<class T>
		AA_CONSTEVAL operator T() const { return std::numeric_limits<T>::min(); }
	};

	AA_CONSTEXPR const numeric_min_t numeric_min;

	template<std::equality_comparable T>
	AA_CONSTEXPR bool is_numeric_max(const T &x) {
		return x == std::numeric_limits<T>::max();
	}

	template<std::equality_comparable T>
	AA_CONSTEXPR bool is_numeric_min(const T &x) {
		return x == std::numeric_limits<T>::min();
	}



	// T čia neturi būti tuple_like, nes tuple_like tipo visi get validūs, o čia tikrinamas tik vienas get.
	template<class T, size_t I>
	concept member_get = (I < std::tuple_size_v<std::remove_reference_t<T>>) && requires(std::remove_cvref_t<T> &t) {
		{ t.template get<I>() } -> wo_ref_same_as<std::tuple_element_t<I, std::remove_cvref_t<T>>>;
		{ std::as_const(t).template get<I>() } -> wo_ref_same_as<std::tuple_element_t<I, const std::remove_cvref_t<T>>>;
	};

	template<class T, size_t I>
	concept adl_get = (I < std::tuple_size_v<std::remove_reference_t<T>>) && requires(std::remove_cvref_t<T> &t) {
		{ get<I>(t) } -> wo_ref_same_as<std::tuple_element_t<I, std::remove_cvref_t<T>>>;
		{ get<I>(std::as_const(t)) } -> wo_ref_same_as<std::tuple_element_t<I, const std::remove_cvref_t<T>>>;
	};

	template<class T, size_t I>
	concept gettable = member_get<T, I> || adl_get<T, I>;

	// declval viduje reference, nes getter taip veikia, o getter taip veikia, nes iš standarto imiau pavyzdį.
	template<size_t I, gettable<I>>
	struct get_result;

	template<size_t I, member_get<I> T>
	struct get_result<I, T> : std::type_identity<decltype(std::declval<T &>().template get<I>())> {};

	template<size_t I, adl_get<I> T>
	struct get_result<I, T> : std::type_identity<decltype(get<I>(std::declval<T &>()))> {};

	template<size_t I, class T>
	using get_result_t = typename get_result<I, T>::type;

	template<size_t I>
	struct getter {
		template<gettable<I> T>
		AA_CONSTEXPR get_result_t<I, T> operator()(T &&t) const {
			if constexpr (member_get<T, I>)		return t.template get<I>();
			else								return get<I>(t);
		}
	};

	AA_CONSTEXPR const getter<0> get_0, get_x, get_w;
	AA_CONSTEXPR const getter<1> get_1, get_y, get_h;
	AA_CONSTEXPR const getter<2> get_2, get_z;
	AA_CONSTEXPR const getter<3> get_3;



	// Neįmanoma requires į concept paversti.
	template<auto... A, class F, class... T>
		requires (requires(F &&f, T&&... t) { std::forward<F>(f).template AA_CALL_OPERATOR<A...>(std::forward<T>(t)...); })
	AA_CONSTEXPR decltype(auto) invoke(F &&f, T&&... t) {
		return std::forward<F>(f).template AA_CALL_OPERATOR<A...>(std::forward<T>(t)...);
	}

	template<class... A, class F, class... T>
		requires (requires(F &&f, T&&... t) { std::forward<F>(f).template AA_CALL_OPERATOR<A...>(std::forward<T>(t)...); })
	AA_CONSTEXPR decltype(auto) invoke(F &&f, T&&... t) {
		return std::forward<F>(f).template AA_CALL_OPERATOR<A...>(std::forward<T>(t)...);
	}

	template<class F, size_t... I>
	AA_CONSTEXPR decltype(auto) apply(F &&f, const std::index_sequence<I...> &&) {
		return invoke<I...>(std::forward<F>(f));
	}

	template<size_t N, class F>
	AA_CONSTEXPR decltype(auto) apply(F &&f) {
		return aa::apply(std::forward<F>(f), constant<std::make_index_sequence<N>>());
	}

	template<class T, class F>
	AA_CONSTEXPR decltype(auto) apply(F &&f) {
		return aa::apply<std::tuple_size_v<std::remove_reference_t<T>>>(std::forward<F>(f));
	}



	template<size_t N>
	using make_reverse_index_sequence = decltype(apply<N>([]<size_t... I>() -> std::index_sequence<(N - 1uz - I)...> { return {}; }));

	template<class T>
	using index_sequence_for_tuple = std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<T>>>;

	template<auto... A>
	using index_sequence_for_pack = std::make_index_sequence<sizeof...(A)>;



	template<class T, size_t N = numeric_max>
	concept tuple_like = apply<T>([]<size_t... I>() -> bool { return (... && gettable<T, I>); })
		&& (is_numeric_max(N) || std::tuple_size_v<std::remove_reference_t<T>> == N);

	template<class T, size_t N = numeric_max>
	concept array_like = tuple_like<T, N> && !!std::tuple_size_v<std::remove_reference_t<T>>
		&& apply<T>([]<size_t... I>() -> bool { return are_same_v<get_result_t<I, T>...>; });

	template<array_like T>
	struct array_element : std::tuple_element<0, std::remove_reference_t<T>> {};

	template<array_like T>
	using array_element_t = typename array_element<T>::type;

	template<class T, size_t N = numeric_max>
	concept arithmetic_array_like = array_like<T, N> && arithmetic<array_element_t<T>>;

	template<class T, class U>
	concept same_tuple_size_as = std::tuple_size_v<std::remove_reference_t<T>> == std::tuple_size_v<std::remove_reference_t<U>>;

	template<class T, class U>
	concept array_similar_to = std::same_as<array_element_t<T>, array_element_t<U>> && same_tuple_size_as<T, U>;



	template<class T, class F, class R = void>
	AA_CONSTEXPR const auto tuple_getter_table = apply<T>([]<size_t... I>() ->
			std::array<R(*)(F &&, T &&), std::tuple_size_v<std::remove_reference_t<T>>>
	{ return {([](F &&f, T &&t) -> R { return invoke<I>(std::forward<F>(f), constant<getter<I>>()(t)); })...}; });

	// visit funkcijų reikia, nes visoms tuple klasėms tiesiogiai nepatogu dirbti su funkcijų masyvais.
	template<class R = void, class T, class F>
	AA_CONSTEXPR R visit(const size_t i, F &&f, T &&t) {
		return tuple_getter_table<T &, F, R>[i](std::forward<F>(f), t);
	}

	template<class T, class F, class R = void>
	AA_CONSTEXPR const auto pack_getter_table = apply<T>([]<size_t... I>() ->
			std::array<R(*)(F &&), std::tuple_size_v<T>>
	{ return {([](F &&f) -> R { return invoke<I, T::template get<I>()>(std::forward<F>(f)); })...}; });

	template<class R = void, class T, class F>
	AA_CONSTEXPR R visit(const size_t i, F &&f) {
		return pack_getter_table<T, F, R>[i](std::forward<F>(f));
	}



	template<class T>
	struct propagate_const : std::type_identity<const T> {};

	template<pointer T>
	struct propagate_const<T> : std::type_identity<typename propagate_const<std::remove_pointer_t<T>>::type *const> {};

	template<lvalue_reference T>
	struct propagate_const<T> : std::type_identity<typename propagate_const<std::remove_reference_t<T>>::type &> {};

	template<rvalue_reference T>
	struct propagate_const<T> : std::type_identity<typename propagate_const<std::remove_reference_t<T>>::type &&> {};

	template<class T>
	using propagate_const_t = typename propagate_const<T>::type;



	template<class U, class V, class T>
	concept in_relation_with = std::relation<T, const U &, const V &>;

	template<class T, class U, class V = U>
	concept relation_for = in_relation_with<U, V, const T &>;

	template<class U, template<class> class T>
	concept argument_for_tdc_template = trivially_default_constructible<T<U>>;

	template<class U, class T>
	concept hashable_by = invocable_r<T, size_t, const U &>;

	template<class U, template<class> class T>
	concept hashable_by_template = hashable_by<U, T<U>> && trivially_default_constructible<T<U>>;

	template<class T, class... U>
	concept hasher_for = (... && hashable_by<U, const T &>);

	template<class T, class U>
	concept char_traits_for = char_traits_like<T> && std::same_as<typename T::char_type, U>;

	template<class T, class U, size_t N = numeric_max>
	concept arithmetic_array_getter = std::invocable<const T &, const U &>
		&& arithmetic_array_like<std::remove_cvref_t<std::invoke_result_t<const T &, const U &>>, N>;

	template<class U, arithmetic_array_getter<U> T>
	struct arithmetic_array_getter_result : std::remove_cvref<std::invoke_result_t<const T &, const U &>> {};

	template<class U, class T>
	using arithmetic_array_getter_result_t = typename arithmetic_array_getter_result<U, T>::type;



	// https://mathworld.wolfram.com/Hypermatrix.html
	// Gal reiktų vadinti struktūrą ne array, o hypermatrix, bet pasiliekama prie array vardo,
	// nes galime interpretuoti stuktūros pavadinimą kaip masyvą masyvų. Taip pat struktūrą vadinti
	// matrix būtų netikslinga, nes tai implikuotų, kad struktūra palaiko matricos operacijas.
	template<class T, size_t N1, size_t... N>
	struct array : array<std::array<T, N1>, N...> {};

	template<class T, size_t N1>
	struct array<T, N1> : std::type_identity<std::array<T, N1>> {};

	template<class T, size_t N1, size_t... N>
	using array_t = typename array<T, N1, N...>::type;



	// https://mathworld.wolfram.com/Hypercube.html
	template<class T, size_t D, size_t N>
	struct hypercube_array : hypercube_array<std::array<T, N>, D - 1, N> {};

	template<class T, size_t N>
	struct hypercube_array<T, 1, N> : std::type_identity<std::array<T, N>> {};

	template<class T, size_t D, size_t N>
	using hypercube_array_t = typename hypercube_array<T, D, N>::type;

	// https://mathworld.wolfram.com/SquareArray.html
	template<class T, size_t N>
	using square_array_t = hypercube_array_t<T, 2, N>;

	template<class T, size_t N>
	using cube_array_t = hypercube_array_t<T, 3, N>;



	template<class A1, class...>
	struct first : std::type_identity<A1> {};

	template<class... A>
	using first_t = typename first<A...>::type;

	template<class... A>
	struct first_or_void : first<A..., void> {};

	template<class... A>
	using first_or_void_t = typename first_or_void<A...>::type;



	template<convertible_from<size_t> T>
	struct zero : constant_identity<T, static_cast<T>(0uz)> {};

	template<class T>
	AA_CONSTEXPR const T zero_v = zero<T>::value;

	template<convertible_from<size_t> T>
	struct one : constant_identity<T, static_cast<T>(1uz)> {};

	template<class T>
	AA_CONSTEXPR const T one_v = one<T>::value;

	template<convertible_from<size_t> T>
	struct two : constant_identity<T, static_cast<T>(2uz)> {};

	template<class T>
	AA_CONSTEXPR const T two_v = two<T>::value;



	template<class, class...>
	struct next_type;

	template<class T, class A1, class A2, class... A>
	struct next_type<T, A1, A2, A...> : std::conditional_t<std::same_as<T, A1>, std::type_identity<A2>, next_type<T, A2, A...>> {};

	template<class T, class... A>
	using next_type_t = typename next_type<T, A...>::type;

	template<class T>
	struct next_unsigned : next_type<T, uint8_t, uint16_t, uint32_t, uint64_t> {};
	template<class T>
	using next_unsigned_t = typename next_unsigned<T>::type;

	template<class T>
	struct prev_unsigned : next_type<T, uint64_t, uint32_t, uint16_t, uint8_t> {};
	template<class T>
	using prev_unsigned_t = typename prev_unsigned<T>::type;

	template<class T>
	struct next_signed : next_type<T, int8_t, int16_t, int32_t, int64_t> {};
	template<class T>
	using next_signed_t = typename next_signed<T>::type;

	template<class T>
	struct prev_signed : next_type<T, int64_t, int32_t, int16_t, int8_t> {};
	template<class T>
	using prev_signed_t = typename prev_signed<T>::type;



	template<template<class> class F, bool C, class T>
	struct apply_if : std::conditional<C, F<T>, T> {};

	template<template<class> class F, bool C, class T>
	using apply_if_t = typename apply_if<F, C, T>::type;



	template<bool C, class T>
	struct add_const_if : apply_if<std::add_const_t, C, T> {};

	template<bool C, class T>
	using add_const_if_t = typename add_const_if<C, T>::type;



	template<class F, class T>
	struct copy_const : add_const_if<std::is_const_v<F>, T> {};

	template<class F, class T>
	using copy_const_t = typename copy_const<F, T>::type;



	// Naudojamas size_t tipas kaip konstantos tipas, o ne sekančio dydžio integer tipas, nes konstanta
	// turėtų būti naudojama malloc ir panašiuose kontekstuose, o ten reikalaujama size_t tipo išraiška.
	// T turi turėti unique object representations, nes kitaip neišeis patikimai apskaičiuoti konstantos.
	//
	// Vietoje byte negalime naudoti uint8_t, nes jei sistemoje baitas būtų ne 8 bitų, tas tipas nebus apibrėžtas.
	template<uniquely_representable T>
	struct representable_values
		: size_t_identity<AA_SHL(1uz, sizeof(T[std::numeric_limits<std::underlying_type_t<std::byte>>::digits]))> {};

	template<class T>
	AA_CONSTEXPR const size_t representable_values_v = representable_values<T>::value;



	// Galėtume vietoje _1 naudoti std::ignore. Bet minėtos konstantos tipas nėra tuščias.
	// Taip pat jei reiktų negalėtume to pačio parametro su skirtingomis konstantomis specializuoti.
	//
	// Comparators do not declare is_transparent, nes jų ir taip neišeitų panaudoti su c++ standarto konteineriais.
	//
	// Mes nurodome dešinės pusės operandą su template parametru, nes realizuojame šią gražią elgseną: sakykime turime
	// objektą tipo less<3>, šio objekto operator() gražins true tik tada kai paduotas kintamasis bus mažesnis už 3.
	template<auto R = std::placeholders::_1>
	struct less {
		template<std::totally_ordered_with<decltype(R)> L>
		AA_CONSTEXPR bool operator()(const L &l) const { return l < R; }
	};

	template<>
	struct less<std::placeholders::_1> {
		template<auto R, std::totally_ordered_with<decltype(R)> L>
		AA_CONSTEXPR bool operator()(const L &l) const { return l < R; }
	};

	template<auto R = std::placeholders::_1>
	struct less_equal {
		template<std::totally_ordered_with<decltype(R)> L>
		AA_CONSTEXPR bool operator()(const L &l) const { return l <= R; }
	};

	template<>
	struct less_equal<std::placeholders::_1> {
		template<auto R, std::totally_ordered_with<decltype(R)> L>
		AA_CONSTEXPR bool operator()(const L &l) const { return l <= R; }
	};

	template<auto R = std::placeholders::_1>
	struct greater {
		template<std::totally_ordered_with<decltype(R)> L>
		AA_CONSTEXPR bool operator()(const L &l) const { return l > R; }
	};

	template<>
	struct greater<std::placeholders::_1> {
		template<auto R, std::totally_ordered_with<decltype(R)> L>
		AA_CONSTEXPR bool operator()(const L &l) const { return l > R; }
	};

	template<auto R = std::placeholders::_1>
	struct greater_equal {
		template<std::totally_ordered_with<decltype(R)> L>
		AA_CONSTEXPR bool operator()(const L &l) const { return l >= R; }
	};

	template<>
	struct greater_equal<std::placeholders::_1> {
		template<auto R, std::totally_ordered_with<decltype(R)> L>
		AA_CONSTEXPR bool operator()(const L &l) const { return l >= R; }
	};

	template<auto R = std::placeholders::_1>
	struct equal_to {
		template<std::equality_comparable_with<decltype(R)> L>
		AA_CONSTEXPR bool operator()(const L &l) const { return l == R; }
	};

	template<>
	struct equal_to<std::placeholders::_1> {
		template<auto R, std::equality_comparable_with<decltype(R)> L>
		AA_CONSTEXPR bool operator()(const L &l) const { return l == R; }
	};

	template<auto R = std::placeholders::_1>
	struct not_equal_to {
		template<std::equality_comparable_with<decltype(R)> L>
		AA_CONSTEXPR bool operator()(const L &l) const { return l != R; }
	};

	template<>
	struct not_equal_to<std::placeholders::_1> {
		template<auto R, std::equality_comparable_with<decltype(R)> L>
		AA_CONSTEXPR bool operator()(const L &l) const { return l != R; }
	};



	// Funkcijos nepilnai generic kaip iota_view, nes neegzistuoja weakly_decrementable concept.
	// Jei reikia normalių indeksų, tada jau reikia naudoti nebe šias funkcijas, o pavyzdžiui iota_view su for_each.
	// Galėtume kažkokius argumentus kiekvieną iteraciją paduoti į funkciją, bet niekur taip nėra realizuoti algoritmai.
	template<invocable_ref F>
	AA_CONSTEXPR void repeat(size_t e, F &&f) {
		do { std::invoke(f); if (e != 1) --e; else return; } while (true);
	}

	template<invocable_ref<size_t> F>
	AA_CONSTEXPR void repeat(size_t e, F &&f) {
		do { std::invoke(f, std::as_const(e)); if (e != 1) --e; else return; } while (true);
	}

	// Parameter pack negali susidėti iš daug elementų todėl šitą funkciją reiktų naudoti tik kai reikia mažai
	// iteracijų. Tačiau toks iteravimas greitesnis, nes nereikia prižiūrėti papildomo ciklo kintamojo.
	//
	// Greitaveika nenukenčia padavinėjant template parametrus todėl neturime funkcijos užklojimo kito.
	template<size_t N, class F>
	AA_CONSTEXPR void repeat(F &&f) {
		apply<N>([&]<size_t... I>() -> void { (aa::invoke<I>(f), ...); });
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



	template<size_t, class T>
	struct tuple_unit {
		// Member types
		using value_type = T;

		// Member objects
		[[no_unique_address]] value_type value;
	};

	template<class, class...>
	struct tuple_base;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpadded"
	template<size_t... I, class... T>
	struct tuple_base<std::index_sequence<I...>, T...> : tuple_unit<I, T>... {};
#pragma GCC diagnostic pop

	// https://ldionne.com/2015/11/29/efficient-parameter-pack-indexing/
	template<size_t I, class... T>
	struct type_pack_element : decltype(([]<class U>(const tuple_unit<I, U> &&) ->
		std::type_identity<U> { return {}; })(std::declval<tuple_base<std::index_sequence_for<T...>, T...>>())) {};

	template<size_t I, class... T>
	using type_pack_element_t = typename type_pack_element<I, T...>::type;

	template<class U, class... T>
	struct type_pack_index : decltype(([]<size_t I>(const tuple_unit<I, U> &&) ->
		size_t_identity<I> { return {}; })(std::declval<tuple_base<std::index_sequence_for<T...>, T...>>())) {};

	template<class U, class... T>
	AA_CONSTEXPR const size_t type_pack_index_v = type_pack_index<U, T...>::value;

	// https://danlark.org/2020/04/13/why-is-stdpair-broken/
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpadded"
	template<class... T>
	struct tuple : tuple_base<std::index_sequence_for<T...>, T...> {
#pragma GCC diagnostic pop
		// Member types
		template<size_t I>
		using value_type = type_pack_element_t<I, T...>;

		template<size_t I>
		using unit_type = tuple_unit<I, value_type<I>>;

		// Member constants
		template<class U>
		static AA_CONSTEXPR const size_t index = type_pack_index_v<U, T...>;



		// Element access
		template<size_t I>
		AA_CONSTEXPR value_type<I> &get() { return unit_type<I>::value; }

		template<size_t I>
		AA_CONSTEXPR const value_type<I> &get() const { return unit_type<I>::value; }

		template<class U>
		AA_CONSTEXPR value_type<index<U>> &get() { return get<index<U>>(); }

		template<class U>
		AA_CONSTEXPR const value_type<index<U>> &get() const { return get<index<U>>(); }

		template<class R = void, class F>
		AA_CONSTEXPR R get(const size_t i, F &&f) { return visit<R>(i, std::forward<F>(f), *this); }

		template<class R = void, class F>
		AA_CONSTEXPR R get(const size_t i, F &&f) const { return visit<R>(i, std::forward<F>(f), *this); }
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



	template<size_t, auto V>
	struct pack_unit : constant_identity<decltype(V), V> {};

	template<class, auto...>
	struct pack_base;

	template<size_t... I, auto... V>
	struct pack_base<std::index_sequence<I...>, V...> : pack_unit<I, V>... {};

	template<size_t I, auto... V>
	struct pack_element : decltype(([]<auto A>(const pack_unit<I, A> &&) ->
		constant_identity<decltype(A), A> { return {}; })(pack_base<index_sequence_for_pack<V...>, V...>{})) {};

	template<size_t I, auto... V>
	using pack_element_t = typename pack_element<I, V...>::type;

	template<size_t I, auto... V>
	AA_CONSTEXPR const pack_element_t<I, V...> pack_element_v = pack_element<I, V...>::value;

	template<auto A, auto... V>
	struct pack_index : decltype(([]<size_t I>(const pack_unit<I, A> &&) ->
		size_t_identity<I> { return {}; })(pack_base<index_sequence_for_pack<V...>, V...>{})) {};

	template<auto A, auto... V>
	AA_CONSTEXPR const size_t pack_index_v = pack_index<A, V...>::value;

	template<auto... V>
	struct pack : pack_base<index_sequence_for_pack<V...>, V...> {
		// Member types
		template<size_t I>
		using value_type = pack_element_t<I, V...>;

		template<size_t I>
		using unit_type = pack_unit<I, pack_element_v<I, V...>>;

		// Member constants
		template<auto A>
		static AA_CONSTEXPR const size_t index = pack_index_v<A, V...>;



		// Element access
		template<size_t I>
		static AA_CONSTEVAL value_type<I> get() { return unit_type<I>::value; }

		template<class R = void, class F>
		static AA_CONSTEXPR R get(const size_t i, F &&f) { return visit<R, pack>(i, std::forward<F>(f)); }
	};



	template<class F, size_t I = 0>
	struct function_argument : function_argument<deduced_template_t<std::function, F>, I> {};

	template<class R, class... A, size_t I>
	struct function_argument<std::function<R(A...)>, I> : type_pack_element<I, A...> {};

	template<class F, size_t I = 0>
	using function_argument_t = typename function_argument<F, I>::type;

}



namespace std {

	template<class... T>
	struct tuple_size<aa::tuple<T...>> : aa::size_t_identity<sizeof...(T)> {};

	template<size_t I, class... T>
	struct tuple_element<I, aa::tuple<T...>> : aa::type_pack_element<I, T...> {};



	template<auto... V>
	struct tuple_size<aa::pack<V...>> : aa::size_t_identity<sizeof...(V)> {};

	template<size_t I, auto... V>
	struct tuple_element<I, aa::pack<V...>> : aa::pack_element<I, V...> {};

	template<size_t I, auto... V>
	struct tuple_element<I, const aa::pack<V...>> : aa::pack_element<I, V...> {};

}
