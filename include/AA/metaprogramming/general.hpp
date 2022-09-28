#pragma once

// AA bibliotekos failuose elgiamasi lyg būtų įterpti toliau išdėstyti failai, nes tie failai suteikia galimybę
// sklandžiai programuoti naudojantis esminėmis c++ kalbos savybėmis. Tie failai yra ir jų įterpimo priežastys:
// • <cstddef> ir <cstdint>, failai įterpti, kad nereiktų naudoti daug raktažodžių, kad aprašyti pamatinius tipus.
// • <array> (<compare>, <initializer_list>), failas įterptas, kad nereiktų naudoti C stiliaus masyvų.
// • <string_view> (<compare>), failas įterptas, kad nereiktų naudoti C stiliaus teksto eilučių.
// • <type_traits> ir <concepts>, failai įterpti, kad išeitų lengvai protauti apie tipus.
// • <utility> (<compare>, <initializer_list>), failas įterptas, kad išeitų lengvai protauti apie išraiškas.
// • <limits>, failas įterptas, kad išeitų lengvai nautotis pamatinių tipų savybėmis.
// Failai paminėti skliausteliose prie įterpiamo failo nurodo kokie failai yra įterpiami pačio įterpiamo failo.

#include "../preprocessor/general.hpp"
#include <cstddef> // byte, size_t
#include <cstdint> // uint8_t, uint16_t, uint32_t, uint64_t, int8_t, int16_t, int32_t, int64_t
#include <type_traits> // remove_reference_t, is_lvalue_reference_v, is_rvalue_reference_v, type_identity, bool_constant, true_type, integral_constant, conditional, conditional_t, is_void_v, has_unique_object_representations_v, is_trivial_v, is_trivially_copyable_v, is_trivially_default_constructible_v, add_const_t, is_const_v, is_arithmetic_v, invoke_result_t, underlying_type_t, extent_v, remove_cvref, remove_cvref_t, remove_const_t, is_pointer_v, remove_pointer_t, is_function_v
#include <concepts> // convertible_to, same_as, default_initializable, copy_constructible, relation, invocable, derived_from, semiregular, totally_ordered_with, equality_comparable_with, constructible_from
#include <limits> // numeric_limits
#include <string_view> // string_view
#include <array> // array
#include <utility> // forward, declval, as_const, tuple_size, tuple_size_v, tuple_element, tuple_element_t, index_sequence, make_index_sequence



namespace aa {

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
	using traits_type_in_use_t = traits_type_in_use<T>::type;



	template<template<class...> class F, class... A1>
	struct bind_types {
		template<class... A2>
		struct front : std::type_identity<F<A1..., A2...>> {};

		template<class... A2>
		using front_t = front<A2...>::type;
	};

	template<template<size_t...> class F, size_t N>
	struct apply_indices : decltype(([]<size_t... I>(const std::index_sequence<I...>) consteval ->
		std::type_identity<F<I...>> { return {}; })(std::make_index_sequence<N>{})) {};

	template<template<size_t...> class F, size_t N>
	using apply_indices_t = apply_indices<F, N>::type;

	template<template<size_t...> class F, size_t N>
	AA_CONSTEXPR const auto apply_indices_t_v = apply_indices_t<F, N>::value;

	template<template<class...> class F, uses_traits_type T>
	struct apply_traits : std::type_identity<F<typename traits_type_in_use_t<T>::char_type, traits_type_in_use_t<T>>> {};

	template<template<class...> class F, class T>
	using apply_traits_t = apply_traits<F, T>::type;



	// Klasė negali būti pakeista į concept, nes concepts negali būti recursive.
	template<class T = void, class... A>
	struct are_unique : std::bool_constant<!(... || std::same_as<T, A>) && are_unique<A...>::value> {};

	template<class T>
	struct are_unique<T> : std::true_type {};

	template<class... A>
	AA_CONSTEXPR const bool are_unique_v = are_unique<A...>::value;

	// Klasė galėtų būti pakeista į concept, bet nėra logiška, kad concept'as galėtų būti naudojamas su 0 template parametrų.
	template<class T = void, class... A>
	struct are_same : std::bool_constant<!sizeof...(A) || (... && std::same_as<T, A>)> {};

	template<class... A>
	AA_CONSTEXPR const bool are_same_v = are_same<A...>::value;



	template<class T>
	concept arithmetic = std::is_arithmetic_v<T>;

	template<class T>
	concept uniquely_representable = std::has_unique_object_representations_v<T>;

	template<class T>
	concept trivial = std::is_trivial_v<T>;

	template<class T>
	concept trivially_copyable = std::is_trivially_copyable_v<T>;

	template<class T>
	concept trivially_default_constructible = std::is_trivially_default_constructible_v<T>;

	template<class L, class R>
	concept remove_ref_derived_from = std::derived_from<std::remove_reference_t<L>, R>;

	template<class L, class R>
	concept remove_cvref_same_as = std::same_as<std::remove_cvref_t<L>, R>;

	template<class L, class R>
	concept remove_ref_same_as = std::same_as<std::remove_reference_t<L>, R>;

	template<class L, template<class...> class R, template<class> class... A>
	concept same_as_template = remove_ref_same_as<L, R<typename A<std::remove_reference_t<L>>::type...>>;



	// https://en.wikipedia.org/wiki/Function_object
	template<class T>
	concept functor = requires { T::operator(); };

	template<class F, auto... A>
	concept nttp_accepting_functor = std::invocable<decltype(&(std::remove_reference_t<F>::template AA_CALL_OPERATOR<A...>)), F>;

	// Vietoje requires parametrų galima naudoti declval, bet atrodo tai negražiai, nėra reikalo to daryti.
	// https://mathworld.wolfram.com/Multiplier.html
	template<class L, class R = L>
	concept multiplier = requires(L && l, R && r) { (std::forward<L>(l) * std::forward<R>(r)); };

	// https://mathworld.wolfram.com/Multiplicand.html
	template<class R, class L = R>
	concept multiplicand = multiplier<L, R>;

	// AA_MUL uždeda skliaustelius apie išraišką, bet šiuo atveju tai nesvarbu, nes decltype ir su
	// skliausteliais ir be jų nustatytų tą patį tipą, nes jam paduodama išraiška, o ne kintamojo vardas.
	template<class L, multiplicand<L> R = L>
	struct product_result
		: std::type_identity<decltype(AA_MUL(std::declval<L>(), std::declval<R>()))> {};

	template<class L, class R = L>
	using product_result_t = product_result<L, R>::type;



	// T čia nebūtinai turi būti tuple_like, nes tiesiog sakome, kad šias deklaracijas galime naudoti su visais tipais.
	template<class T, size_t I>
	concept member_get = (I < std::tuple_size_v<std::remove_reference_t<T>>) && requires(T & t) {
		{ t.template get<I>() } -> remove_ref_same_as<std::tuple_element_t<I, std::remove_reference_t<T>>>;
	};

	template<class T, size_t I>
	concept adl_get = (I < std::tuple_size_v<std::remove_reference_t<T>>) && requires(T & t) {
		{ get<I>(t) } -> remove_ref_same_as<std::tuple_element_t<I, std::remove_reference_t<T>>>;
	};

	template<class T, size_t I>
	concept gettable = member_get<T, I> || adl_get<T, I>;

	template<size_t I>
	struct getter {
		template<gettable<I> T>
		[[gnu::always_inline]] AA_CONSTEXPR decltype(auto) operator()(T &&t) const {
			if constexpr (member_get<T, I>) {
				return t.template get<I>();
			} else {
				return get<I>(t);
			}
		}
	};

	AA_CONSTEXPR const getter<0> get_0, get_x;
	AA_CONSTEXPR const getter<1> get_1, get_y;

	template<class T, size_t I>
	concept regular_gettable = requires(std::remove_const_t<T> &t) {
		{ getter<I>{}(t) } -> std::same_as<std::tuple_element_t<I, std::remove_cvref_t<T>> &>;
		{ getter<I>{}(std::as_const(t)) } -> std::same_as<std::tuple_element_t<I, const std::remove_cvref_t<T>> &>;
	};

	template<class T>
	struct is_tuple {
		template<size_t... I>
		struct regular : std::bool_constant<(... && regular_gettable<T, I>)> {};

		template<size_t... I>
		static AA_CONSTEXPR const bool regular_v = regular<I...>::value;

		template<size_t... I>
		struct valid : std::bool_constant<(... && gettable<T, I>)> {};

		template<size_t... I>
		static AA_CONSTEXPR const bool valid_v = valid<I...>::value;

		template<size_t... I>
		struct uniform : are_same<std::tuple_element_t<I, std::remove_reference_t<T>>...> {};

		template<size_t... I>
		static AA_CONSTEXPR const bool uniform_v = uniform<I...>::value;
	};

	template<class T>
	concept tuple_like = std::derived_from<std::tuple_size<std::remove_reference_t<T>>,
		std::integral_constant<size_t, std::tuple_size_v<std::remove_reference_t<T>>>>
		&& apply_indices_t_v<is_tuple<T>::template valid, std::tuple_size_v<std::remove_reference_t<T>>>;

	template<class T>
	concept array_like = tuple_like<T> && !!std::tuple_size_v<std::remove_reference_t<T>>
		&& apply_indices_t_v<is_tuple<T>::template uniform, std::tuple_size_v<std::remove_reference_t<T>>>;

	template<array_like T>
	struct array_element : std::tuple_element<0, std::remove_reference_t<T>> {};

	template<class T>
	using array_element_t = array_element<T>::type;

	// Netikriname ar tipas gali būti sukonstruotas su elementų tipais, nes vis tiek nežinotume kokius
	// elementus inicializuos nurodyti argumentai konstruktoriuje. Tai prašome užtat, kad tipas būtų
	// default initializable, tada kode galime tiesiog rankomis inicializuoti elementus.
	template<class T>
	concept vector_like = array_like<T> && arithmetic<array_element_t<T>> && std::semiregular<std::remove_reference_t<T>>
		&& apply_indices_t_v<is_tuple<T>::template regular, std::tuple_size_v<std::remove_reference_t<T>>>;

	template<size_t N, class T>
	concept tupleN_like = tuple_like<T> && (std::tuple_size_v<T> == N);

	template<class T>
	concept pair_like = tupleN_like<2, T>;

	template<size_t N, class T>
	concept arrayN_like = array_like<T> && (std::tuple_size_v<T> == N);

	template<class T>
	concept array2_like = arrayN_like<2, T>;

	template<size_t N, class T>
	concept vectorN_like = vector_like<T> && (std::tuple_size_v<T> == N);

	template<class T>
	concept vector2_like = vectorN_like<2, T>;



	template<class F, class... A>
	concept function_pointer = std::invocable<F, A...> && std::is_pointer_v<F> && std::is_function_v<std::remove_pointer_t<F>>;

	template<class F, class... A>
	concept invocable_ref = std::invocable<F &, A...>;

	template<class F, class T>
	concept constructible_to = std::constructible_from<T, F>;

	template<class T, class F>
	concept convertible_from = std::convertible_to<F, T>;

	template<class T, class F>
	concept void_or_convertible_from = std::is_void_v<T> || std::convertible_to<F, T>;

	template<class T>
	concept void_or_convertible_from_floating_point = std::is_void_v<T>
		|| (std::convertible_to<float, T> && std::convertible_to<double, T> && std::convertible_to<long double, T>);



	template<class T>
	struct propagate_const : std::type_identity<const T> {};

	template<class T>
		requires (std::is_pointer_v<T>)
	struct propagate_const<T> : std::type_identity<typename propagate_const<std::remove_pointer_t<T>>::type *const> {};

	template<class T>
		requires (std::is_lvalue_reference_v<T>)
	struct propagate_const<T> : std::type_identity<typename propagate_const<std::remove_reference_t<T>>::type &> {};

	template<class T>
		requires (std::is_rvalue_reference_v<T>)
	struct propagate_const<T> : std::type_identity<typename propagate_const<std::remove_reference_t<T>>::type &&> {};

	template<class T>
	using propagate_const_t = propagate_const<T>::type;



	template<class T>
	concept storable = std::default_initializable<std::remove_reference_t<T>> && std::copy_constructible<T>;

	template<class U, class V, class T>
	concept in_relation_with = std::relation<const T &, const U &, const V &>;

	template<class T, class U, class V = U>
	concept storable_relation_for = in_relation_with<U, V, T> && storable<T>;

	template<class U, class T>
	concept hashable_by = std::invocable<const T &, const U &> && std::same_as<std::invoke_result_t<const T &, const U &>, size_t>;

	template<class T, class U>
	concept storable_hasher_for = hashable_by<U, T> && storable<T>;

	template<class T, class U>
	concept char_traits_for = char_traits_like<T> && std::same_as<typename T::char_type, U>;

	// evaluator'ius gali nebūtinai gražinti be kvalifikatorių tipą, bet copy_constructible turėtų užtikrinti,
	// kad kad ir ką gražintų evaluator'ius, kad tai būtų paverčiama į tipą be kvalifikatorių.
	template<class T>
	concept storable_evaluator = std::invocable<const T &, const std::string_view &> && storable<T>
		&& std::copy_constructible<std::remove_cvref_t<std::invoke_result_t<const T &, const std::string_view &>>>;

	template<storable_evaluator T>
	struct evaluator_result : std::remove_cvref<std::invoke_result_t<const T &, const std::string_view &>> {};

	template<class T>
	using evaluator_result_t = evaluator_result<T>::type;

	template<class T, class U>
	concept storable_vector2_getter = storable<T>
		&& std::invocable<const T &, const U &> && vector2_like<std::remove_cvref_t<std::invoke_result_t<const T &, const U &>>>;

	template<class U, storable_vector2_getter<U> T>
	struct vector2_getter_result : std::remove_cvref<std::invoke_result_t<const T &, const U &>> {};

	template<class U, class T>
	using vector2_getter_result_t = vector2_getter_result<U, T>::type;



	// https://mathworld.wolfram.com/Hypermatrix.html
	// Gal reiktų vadinti struktūrą ne array, o hypermatrix, bet pasiliekama prie array vardo,
	// nes galime interpretuoti stuktūros pavadinimą kaip masyvą masyvų. Taip pat struktūrą vadinti
	// matrix būtų netikslinga, nes tai implikuotų, kad struktūra palaiko matricos operacijas.
	template<class T, size_t N1, size_t... N>
	struct array : array<std::array<T, N1>, N...> {};

	template<class T, size_t N1>
	struct array<T, N1> : std::type_identity<std::array<T, N1>> {};

	template<class T, size_t N1, size_t... N>
	using array_t = array<T, N1, N...>::type;



	// https://mathworld.wolfram.com/Hypercube.html
	template<class T, size_t D, size_t N>
	struct hypercube_array : hypercube_array<std::array<T, N>, D - 1, N> {};

	template<class T, size_t N>
	struct hypercube_array<T, 1, N> : std::type_identity<std::array<T, N>> {};

	template<class T, size_t D, size_t N>
	using hypercube_array_t = hypercube_array<T, D, N>::type;

	// https://mathworld.wolfram.com/SquareArray.html
	template<class T, size_t N>
	using square_array_t = hypercube_array_t<T, 2, N>;

	template<class T, size_t N>
	using cube_array_t = hypercube_array_t<T, 3, N>;



	template<class...>
	struct first_not_void;

	template<class A1, class... A>
	struct first_not_void<A1, A...> : std::conditional_t<std::is_void_v<A1>, first_not_void<A...>, std::type_identity<A1>> {};

	template<class... A>
	using first_not_void_t = first_not_void<A...>::type;



	template<class...>
	struct first_or_void;

	template<class A1, class... A>
	struct first_or_void<A1, A...> : std::type_identity<A1> {};

	template<>
	struct first_or_void<> : std::type_identity<void> {};

	template<class... A>
	using first_or_void_t = first_or_void<A...>::type;



	// C yra objektas, galėtume jį padavinėti per const&, o ne per value, bet nusprendžiau, kad kompiliavimo
	// metu nėra skirtumo kaip tas objektas bus padavinėjamas, net jei jis būtų labai didelis.
	//
	// Negali šitos funkcijos būti paverstos į konstantas, nes neegzistuoja konstantų užklojimas ir dalinė specializacija nesikompiliuoja.
	// Taip pat funkcijos geriau atspindi kas vyksta, tai yra tą faktą, kad reikšmės gaunamos ne iš kažkokios klasės (pvz. integral_constant).
	template<class T, T C>
	AA_CONSTEVAL T constant() { return C; }

	template<auto C>
	AA_CONSTEVAL decltype(C) constant() { return C; }



	template<convertible_from<size_t> T>
	struct zero : std::integral_constant<T, static_cast<T>(0uz)> {};

	template<class T>
	AA_CONSTEXPR const T zero_v = zero<T>::value;

	template<convertible_from<size_t> T>
	struct one : std::integral_constant<T, static_cast<T>(1uz)> {};

	template<class T>
	AA_CONSTEXPR const T one_v = one<T>::value;

	template<convertible_from<size_t> T>
	struct two : std::integral_constant<T, static_cast<T>(2uz)> {};

	template<class T>
	AA_CONSTEXPR const T two_v = two<T>::value;



	template<class, class...>
	struct next_type;

	template<class T, class A1, class A2, class... A>
	struct next_type<T, A1, A2, A...> : std::conditional_t<std::same_as<T, A1>, std::type_identity<A2>, next_type<T, A2, A...>> {};

	template<class T, class... A>
	using next_type_t = next_type<T, A...>::type;

	template<class T>
	struct next_unsigned : next_type<T, uint8_t, uint16_t, uint32_t, uint64_t> {};
	template<class T>
	using next_unsigned_t = next_unsigned<T>::type;

	template<class T>
	struct prev_unsigned : next_type<T, uint64_t, uint32_t, uint16_t, uint8_t> {};
	template<class T>
	using prev_unsigned_t = prev_unsigned<T>::type;

	template<class T>
	struct next_signed : next_type<T, int8_t, int16_t, int32_t, int64_t> {};
	template<class T>
	using next_signed_t = next_signed<T>::type;

	template<class T>
	struct prev_signed : next_type<T, int64_t, int32_t, int16_t, int8_t> {};
	template<class T>
	using prev_signed_t = prev_signed<T>::type;



	template<template<class> class F, bool C, class T>
	struct apply_if : std::conditional<C, F<T>, T> {};

	template<template<class> class F, bool C, class T>
	using apply_if_t = apply_if<F, C, T>::type;



	template<bool C, class T>
	struct add_const_if : apply_if<std::add_const_t, C, T> {};

	template<bool C, class T>
	using add_const_if_t = add_const_if<C, T>::type;



	template<class F, class T>
	struct copy_const : add_const_if<std::is_const_v<F>, T> {};

	template<class F, class T>
	using copy_const_t = copy_const<F, T>::type;



	// Naudojamas size_t tipas kaip konstantos tipas, o ne sekančio dydžio integer tipas, nes konstanta
	// turėtų būti naudojama malloc ir panašiuose kontekstuose, o ten reikalaujama size_t tipo išraiška.
	// T turi turėti unique object representations, nes kitaip neišeis patikimai apskaičiuoti konstantos.
	//
	// Vietoje byte negalime naudoti uint8_t, nes jei sistemoje baitas būtų ne 8 bitų, tas tipas nebus apibrėžtas.
	template<uniquely_representable T>
	struct representable_values
		: std::integral_constant<size_t, AA_SHL(1uz, sizeof(T[std::numeric_limits<std::underlying_type_t<std::byte>>::digits]))> {};

	template<class T>
	AA_CONSTEXPR const size_t representable_values_v = representable_values<T>::value;



	// Galėtų šios klasės turėti get funkciją, bet sakome, kad naudotojas tegul naudoja numeric_limits, nereikia
	// turėti dviejų kelių, kad padaryti visiškai tą patį. Kad gauti pakeistą konstantą naudoti constant funkciją.
	AA_CONSTEXPR const struct {
		template<class T>
		AA_CONSTEVAL operator T() const { return std::numeric_limits<T>::max(); }
	} numeric_max;

	AA_CONSTEXPR const struct {
		template<class T>
		AA_CONSTEVAL operator T() const { return std::numeric_limits<T>::min(); }
	} numeric_min;



	// Galima būtų naudoti source_location, bet ta klasė surenka daugiau duomenų negu reikia ir taip pat
	// kadangi neišeitų panaudoti strlen tai ir neišeitų užtikrinti lengvai compile time veikimo.
	template<class>
	AA_CONSTEVAL std::string_view type_name() {
		return std::string_view{__PRETTY_FUNCTION__ + 76, (std::extent_v<std::remove_reference_t<decltype(__PRETTY_FUNCTION__)>>) - 127};
	}



	template<auto R>
	struct less {
		template<std::totally_ordered_with<decltype(R)> L>
		AA_CONSTEXPR bool operator()(const L &l) const { return l < R; }
	};

	template<auto R>
	struct less_equal {
		template<std::totally_ordered_with<decltype(R)> L>
		AA_CONSTEXPR bool operator()(const L &l) const { return l <= R; }
	};

	template<auto R>
	struct greater {
		template<std::totally_ordered_with<decltype(R)> L>
		AA_CONSTEXPR bool operator()(const L &l) const { return l > R; }
	};

	template<auto R>
	struct greater_equal {
		template<std::totally_ordered_with<decltype(R)> L>
		AA_CONSTEXPR bool operator()(const L &l) const { return l >= R; }
	};

	template<auto R>
	struct equal_to {
		template<std::equality_comparable_with<decltype(R)> L>
		AA_CONSTEXPR bool operator()(const L &l) const { return l == R; }
	};

	template<auto R>
	struct not_equal_to {
		template<std::equality_comparable_with<decltype(R)> L>
		AA_CONSTEXPR bool operator()(const L &l) const { return l != R; }
	};



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

}
