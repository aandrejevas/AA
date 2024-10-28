#pragma once

// Nerealizuojame algoritmų, kurie netikrintų ar masyvas tuščias, nes tą patį galime pasiekti su įprastais algoritmais ir atributo assume naudojimu. Nerealizuojame fixed_string, type_name, log, nes tokį funkcionalumą suteikia žurnalavimo (spdlog) ir tokios kaip nameof bibliotekos. Nerealizuojame savo lexer, nes galime naudoti tiesiog populiarų formatą kaip json. Nerealizuojame print ir read, nes galime naudoti bibliotekas fmt ir scn. Nerealizuojame AA_IF_DEBUG, AA_TRACE_ASSERT, timekeeper, nes jie realizuoti tokiose bibliotekose kaip folly.

// Filosofija bibliotekos tokia, visos funkcijos žymimos constexpr ir tiek. Nesvarbu gali ar negali būti funkcija
// naudojama constexpr kontekste, ji bus pažymėta constexpr. Gal naudotojams kiek neaišku gali būti ar jie gali
// funkciją naudoti constexpr kontekste, bet aš nenoriu mąstyti apie tai ar funkcijos realizacija gali būti constexpr.

// constexpr ir consteval funkcijos specifikatoriai implikuoja inline, todėl nereikia naudoti jų kartu.
// Atitinkamai constexpr kintamojo specifikatorius implikuoja inline ir const specifikatorius.
// Reikia stengtis turėti ir naudoti kuo mažiau macros.

// Nenaudojame atributo "always_inline", nes standartinėje bibliotekoje atributas nenaudojamas, trukdo derintuvei
// atributas, gali būti, kad tik kenkiu greitaveikai naudodamas tą atributą, geriau už kompiliatorių nenuspręsiu,
// kur jį reikia naudoti, o kur ne, galiausiai išprotėčiau jei dar reiktų prižiūrėti ar visur tvarkingai sudėtas jis.

// AA bibliotekos failuose elgiamasi lyg būtų įterpti toliau išdėstyti failai, nes tie failai suteikia galimybę
// sklandžiai programuoti naudojantis esminėmis C++ kalbos savybėmis. Tie failai yra ir jų įterpimo priežastys:
// • <cstddef> ir <cstdint>, failai įterpti, kad nereiktų naudoti daug raktažodžių, kad aprašyti pamatinius tipus.
// • <span> ir <array> (<compare>, <initializer_list>), failai įterpti, kad nereiktų naudoti C stiliaus masyvų.
// • <type_traits> ir <concepts>, failai įterpti, kad išeitų lengvai protauti apie tipus.
// • <utility> (<compare>, <initializer_list>), failas įterptas, kad išeitų lengvai protauti apie išraiškas.
// • <limits>, failas įterptas, kad išeitų lengvai naudotis pamatinių tipų savybėmis.
// • <bit>, failas įterptas, kad išeitų lengvai manipuliuoti pamatinių tipų bitus.
// • <functional>, failas įterptas, kad išeitų lengvai protauti apie funkcijų objektus.
// • <variant> (<compare>), failas įterptas, kad turėti alternatyvą nesaugiems union tipams.
// • <string_view> ir <string> (<compare>, <initializer_list>), failai įterpti, kad nereiktų naudoti C stiliaus teksto eilučių.
// Failai paminėti skliausteliuose prie įterpiamo failo nurodo kokie failai yra įterpiami pačio įterpiamo failo.

// Nenurodome ką iš failų naudojame, nes net jei niekas nebūtų naudojama iš failo, failas turėtų būti įterptas.
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <concepts>
#include <limits>
#include <span>
#include <array>
#include <bit>
#include <utility>
#include <functional>
#include <variant>
#include <string_view>
#include <string>
#include <iterator>
#include <ranges>
#include <algorithm>



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

	template<class T>
	concept not_cv = std::same_as<std::remove_reference_t<T>, std::remove_cvref_t<T>>;

	template<class T>
	concept not_cvref = std::same_as<T, std::remove_cvref_t<T>>;

	template<class T = void, class... A>
	constexpr bool is_same_as_every_v = (... && std::same_as<T, A>);

	template<class T, class... A>
	constexpr bool is_same_as_any_v = (... || std::same_as<T, A>);

	template<class T>
	concept scoped_enum_like = std::regular<T> && std::is_scoped_enum_v<T>;

	template<class T>
	concept enum_like = std::regular<T> && std::is_enum_v<T>;

	template<class T>
	concept unscoped_enum_like = enum_like<T> && !scoped_enum_like<T>;

	template<class T>
	concept pointer_like = std::is_pointer_v<T>;

	template<class T>
	concept object_pointer_like = pointer_like<T> && std::is_object_v<std::remove_pointer_t<T>>;

	template<class T>
	concept lvalue_reference_like = std::is_lvalue_reference_v<T>;

	template<class T>
	concept not_lvalue_reference_like = !lvalue_reference_like<T>;

	template<class T>
	concept rvalue_reference_like = std::is_rvalue_reference_v<T>;

	template<class T>
	concept arithmetic = std::is_arithmetic_v<T>;

	template<class T, class F>
	concept arithmetic_or_assignable_from = arithmetic<T> || std::assignable_from<T &, F>;

	template<class T>
	concept regular_scalar = arithmetic<T> || object_pointer_like<T>;

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

	template<class T, class U>
	concept unsigned_integral_or_same_as = (std::unsigned_integral<T> && !std::unsigned_integral<U>) || std::same_as<T, U>;

	template<class T, class U>
	concept signed_integral_or_same_as = (std::signed_integral<T> && !std::signed_integral<U>) || std::same_as<T, U>;

	// https://en.wikipedia.org/wiki/Function_object
	template<class T>
	concept functor = requires { &T::operator(); };

	template<class F, class... A>
	concept ref_invocable = std::invocable<F &, A...>;

	template<class F, class... A>
	concept cref_invocable = std::invocable<const F &, A...>;

	template<class F, class R, class... A>
	concept invocable_r = std::is_invocable_r_v<R, F, A...>;

	template<class F, class T, class... A>
	concept invoke_result_constructible_to = (std::invocable<F, A...> && std::constructible_from<T, std::invoke_result_t<F, A...>>);

	template<class F, class T>
	concept constructible_to = std::constructible_from<T, F>;

	template<class F, class T>
	concept constructible_between = (std::constructible_from<T, F> && std::constructible_from<F, T>);

	// https://en.cppreference.com/w/cpp/concepts/boolean-testable.
	template<class B>
	concept bool_testable = (std::constructible_from<bool, B>
		&& requires(B &&b) { { !std::forward<B>(b) } -> constructible_to<bool>; });

	template<class R, class L>
	concept assignable_to = std::assignable_from<L, R>;

	template<class T, class... F>
	concept constructible_from_every = (... && std::constructible_from<T, F>);

	template<class T>
	concept constructible_from_floating = constructible_from_every<T, float, double, long double>;

	template<class T, class U>
	concept constructible_from_floating_or_same_as = constructible_from_floating<T> || std::same_as<T, U>;

	template<class T>
	using string_view_for_t = std::basic_string_view<char_type_in_use_t<traits_type_in_use_t<T>>, traits_type_in_use_t<T>>;

	template<class T, size_t N = std::dynamic_extent>
	using span_for_t = std::span<char_type_in_use_t<traits_type_in_use_t<T>>, N>;

	template<class T>
	concept iterator_tag_like = is_same_as_any_v<T, std::input_iterator_tag, std::output_iterator_tag, std::forward_iterator_tag,
		std::bidirectional_iterator_tag, std::random_access_iterator_tag, std::contiguous_iterator_tag>;

	template<class T>
	concept uses_iterator_category = (requires { typename std::remove_reference_t<T>::iterator_category; })
		&& iterator_tag_like<typename std::remove_reference_t<T>::iterator_category>;

	template<uses_iterator_category T>
	using iterator_category_in_use_t = typename std::remove_reference_t<T>::iterator_category;

	template<class T>
	concept uses_iterator_concept = (requires { typename std::remove_reference_t<T>::iterator_concept; })
		&& iterator_tag_like<typename std::remove_reference_t<T>::iterator_concept>;

	template<uses_iterator_concept T>
	using iterator_concept_in_use_t = typename std::remove_reference_t<T>::iterator_concept;

	template<class T>
	concept uses_pointer = requires { typename std::remove_reference_t<T>::pointer; };

	template<uses_pointer T>
	using pointer_in_use_t = typename std::remove_reference_t<T>::pointer;

	template<class T>
	concept uses_difference_type = requires { typename std::remove_reference_t<T>::difference_type; };

	template<uses_difference_type T>
	using difference_type_in_use_t = typename std::remove_reference_t<T>::difference_type;

	// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2445r1.pdf
	template<class T, class U>
	using override_ref_t = std::conditional_t<std::is_rvalue_reference_v<T>, std::remove_reference_t<U> &&, U &>;

	template<class T, class U>
	using copy_const_t = std::conditional_t<std::is_const_v<std::remove_reference_t<T>>, U const, U>;

	template<class T, class U>
	using forward_like_t = override_ref_t<T &&, copy_const_t<T, std::remove_reference_t<U>>>;

	// Nieko tokio, kad kopijuojame konstantas, kadangi viskas vyksta kompiliavimo metu.
	//
	// Negalime konstantų pakeisti funkcijomis, nes neišeina gauti adreso funkcijos
	// rezultato. Tai reiškia, kad šitoks sprendimas yra universalesnis.
	//
	// constant_t alias neturėtų prasmės, nes, kad juo naudotis jau reiktų nurodyti ką norime gauti.
	template<auto V>
	using const_t = decltype(V);

	// Ne movable, nes jei tipas būtų tik movable, negalėtume daryti move, nes V tipas yra const.
	template<std::copyable auto V>
	constexpr const_t<V> const_v = V;

	template<std::copyable auto V>
	using constant = std::integral_constant<const_t<V>, V>;

	template<auto V>
	using type_in_const_t = type_in_use_t<const_t<V>>;

	// <><><><><><><><><><><><><><><><><><><><><><><><> STR RANGES <><><><><><><><><><><><><><><><><><><><><><><><>

	template<class B, class I>
	concept bool_auxiliary_range_for = (std::ranges::random_access_range<B> && std::ranges::output_range<B, bool>
		&& bool_testable<std::ranges::range_reference_t<B>> && constructible_between<I, std::ranges::range_difference_t<B>>);

	template<class R>
	concept sized_random_access_range = std::ranges::random_access_range<R> && std::ranges::sized_range<R>;

	template<class T>
	concept sized_contiguous_range = std::ranges::contiguous_range<T> && std::ranges::sized_range<T>;

	template<class T>
	concept sized_input_range = std::ranges::input_range<T> && std::ranges::sized_range<T>;

	template<class R>
	concept nonrange = !std::ranges::range<R>;

	template<class R>
	concept permutable_range = (sized_random_access_range<R> && std::permutable<std::ranges::iterator_t<R>>);

	template<class I>
	concept char_output_iterator = std::output_iterator<I, const char &>;

	template<class I>
	concept int_input_iterator = std::input_iterator<I> && std::assignable_from<int &, std::iter_reference_t<I>>;

	template<class T>
	concept range_using_traits_type = sized_contiguous_range<T> &&
		std::same_as<char_type_in_use_t<traits_type_in_use_t<T>>, std::ranges::range_value_t<T>>;

	template<class T, class I, class F>
	concept ptr_in_indirect_predicate_with = std::indirect_binary_predicate<F, I, const T *>;

	template<class I>
	using iter_size_t = std::make_unsigned_t<std::iter_difference_t<I>>;

	template<std::contiguous_iterator I>
	using iter_pointer_t = std::add_pointer_t<std::iter_reference_t<I>>;

	template<std::ranges::contiguous_range R>
	using range_pointer_t = std::add_pointer_t<std::ranges::range_reference_t<R>>;

	template<class T>
	concept reverse_range = requires(T& t) {
		std::ranges::rbegin(t);
		std::ranges::rend(t);
	};

	template<class T>
	using reverse_iterator_t = std::invoke_result_t<const_t<std::ranges::rbegin>, T &>;

	template<reverse_range R>
	using reverse_sentinel_t = std::invoke_result_t<const_t<std::ranges::rend>, R &>;

	template<class R>
	concept rbegin_accessible_by_const_cast_range = std::ranges::contiguous_range<R>
		&& requires(const std::ranges::sentinel_t<R> s) { const_cast<std::ranges::iterator_t<R>>(s); };

	template<class R>
	concept rbegin_accessible_by_constructor_range = std::ranges::bidirectional_range<R>
		&& std::constructible_from<std::ranges::iterator_t<R>, std::ranges::sentinel_t<R>>;

	// common_range negalime naudoti, nes mano ranges nėra common.
	template<class R>
	concept rbegin_accessible_range = rbegin_accessible_by_const_cast_range<R>
		|| rbegin_accessible_by_constructor_range<R> || sized_random_access_range<R>;

	template<class R>
	concept unusual_range = rbegin_accessible_range<R> && std::same_as<reverse_iterator_t<R>, std::ranges::iterator_t<R>>;

	// Nėra atitinkamos funkcijos rend iteratoriui, nes jis nėra svarbus.
	// get_rdata nėra funkcijos, nes iš šios funkcijos galima gauti rodyklę.
	// Čia turėtų būti O(1) sudėtingumo funkcija, todėl netinka naudoti distance.
	// Algoritmai priima ranges kur tik įmanoma. Jei turime tik iteratorius tai galima paduoti iš jų sukurtą range.
	template<rbegin_accessible_range R>
	constexpr std::ranges::iterator_t<R> get_rbegin(R &&r) {
		if constexpr (unusual_range<R>) {
			// std::reverse_iterator nėra constructible į iterator.
			return std::ranges::rbegin(r);

		} else if constexpr (rbegin_accessible_by_const_cast_range<R>) {
			return std::ranges::prev(const_cast<std::ranges::iterator_t<R>>(std::ranges::end(r)));

		} else if constexpr (rbegin_accessible_by_constructor_range<R>) {
			return std::ranges::prev(cast<std::ranges::iterator_t<R>>(std::ranges::end(r)));

		} else {
			return std::ranges::begin(r) + (std::ranges::ssize(r) - 1);
		}
	}

	// <><><><><><><><><><><><><><><><><><><><><><><><> END RANGES <><><><><><><><><><><><><><><><><><><><><><><><>

	template<class T, class... A>
	concept movable_constructible_from = std::movable<T> && std::constructible_from<T, A...>;

	// T turi būti movable, nes mums reikia žinoti tik ar galime iš gražinamos reikšmės (rvalue) sukonstruoti T.
	// Negalime naudoti NTTP, kad patikrinti ar išraiška yra constexpr, nes T gali būti ne structural tipas.
	template<class T, auto... A>
	concept movable_immediately_constructible_from = movable_constructible_from<T, const_t<A>...>
		&& (requires { ([] static consteval -> T { return T(A...); })(); });

	namespace detail {
		// T tipas nesiskiria kai esame const T & kontekste ar T kontekste. Tai reiškia, kad
		// privalome pasirinkti grąžinti const T & arba T visiems atvejams. Yra pasirinkta grąžinti T,
		// nes buvo nuspręsta, kad šias klases turi būti įmanoma naudoti ir ne su constexpr konstruktorius
		// turinčiomis klasėmis ir joms neišeitų grąžinti const T &. Jei vis dėlto reikia const T &
		// tipo, galima dirbti su atitinkamais constexpr kintamaisiais, kurie yra apibrėžti po šių klasių.
		template<auto... A>
		struct value_getter {
			template<movable_immediately_constructible_from<A...> T>
			consteval operator T() const { return T(A...); }

			template<movable_constructible_from<const_t<A>...> T>
			constexpr operator T() const { return T(A...); }
		};

		struct numeric_max_getter {
			template<std::movable T>
			consteval operator T() const { return std::numeric_limits<T>::max(); }
		};

		struct numeric_min_getter {
			template<std::movable T>
			consteval operator T() const { return std::numeric_limits<T>::min(); }
		};
	}

	template<auto... A>
	constexpr detail::value_getter<A...> value;

	constexpr detail::value_getter<> default_value;

	constexpr detail::numeric_max_getter numeric_max;

	constexpr detail::numeric_min_getter numeric_min;

	template<std::movable T, auto... A>
	constexpr T value_v = value<A...>;

	template<std::movable T>
	constexpr T default_v = default_value;

	template<class T>
	constexpr std::type_identity<T> type_v = default_v<std::type_identity<T>>;

	template<std::movable T>
	constexpr T numeric_max_v = numeric_max;

	template<enum_like T>
	constexpr std::underlying_type_t<T> numeric_max_v<T> = numeric_max;

	template<std::movable T>
	constexpr T numeric_min_v = numeric_min;

	template<enum_like T>
	constexpr std::underlying_type_t<T> numeric_min_v<T> = numeric_min;

	template<std::movable T>
	constexpr size_t numeric_digits_v = std::numeric_limits<T>::digits;

	template<enum_like T>
	constexpr size_t numeric_digits_v<T> = std::numeric_limits<std::underlying_type_t<T>>::digits;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
	template<auto... A, std::regular T>
	constexpr bool is_value(const T &x) {
		return x == value_v<T, A...>;
	}

	template<std::regular T>
	constexpr bool is_default_value(const T &x) {
		return x == default_v<T>;
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

	template<class T, constructible_to<T> X>
	constexpr T cast(X &&x) {
		return static_cast<T>(std::forward<X>(x));
	}

	// https://docs.libreoffice.org/o3tl/html/temporary_8hxx_source.html
	template<not_lvalue_reference_like T>
	constexpr T &stay(T &&x) {
		return cast<T &>(x);
	}



	template<class T>
	concept empty_unit_param_like = std::is_empty_v<T> && movable_constructible_from<std::remove_const_t<T>>;

	template<class T>
	concept nonempty_unit_param_like = !empty_unit_param_like<T>;

	template<class T>
	concept const_unit_param_like = std::is_const_v<T> && movable_constructible_from<std::remove_const_t<T>>;

	namespace detail {
		template<size_t, class T>
		struct tuple_unit {
			// Member types
			using value_type = T;
			using reference = value_type &;
			using const_reference = const value_type &;

			// Element access
			template<class S>
			constexpr forward_like_t<S, value_type> get(this S &&self) {
				return std::forward<S>(self).value;
			}

			// Special member functions
			constexpr tuple_unit() = default;
			constexpr tuple_unit() requires (const_unit_param_like<value_type>) : value{default_value} {}
			template<constructible_to<value_type> U>
			constexpr tuple_unit(U &&u) : value{std::forward<U>(u)} {}

			// Member objects
			// Nenaudojame atributo no_unique_address, nes jis čia nieko nekeistų.
			value_type value;
		};

		// Negalime paveldėti iš T, nes kažkodėl tada tuple dydis padidėja.
		template<size_t I, empty_unit_param_like T>
		struct tuple_unit<I, T> {
			// Member types
			using value_type = std::remove_const_t<T>;
			using reference = value_type;
			using const_reference = value_type;

			// Element access
			static consteval value_type get() { return default_value; }
		};

		template<size_t, auto>
		struct pack_unit {};

		template<class, auto...>
		struct pack_base;

		template<size_t... I, auto... V>
		struct pack_base<std::index_sequence<I...>, V...> : pack_unit<I, V>... {};
	}

	template<size_t I, auto... V>
	constexpr auto pack_element_v = ([]<auto A>(const detail::pack_unit<I, A>) static -> const_t<A>
	{ return A; })(default_v<detail::pack_base<std::index_sequence_for<const_t<V>...>, V...>>);



	// Jeigu reikia kitokių template template parametrų tai tokiu atveju reikia apibrėžti atitinkamus alias.
	template<template<class...> class F, class... A>
	using deduced_t = decltype(F(std::declval<A>()...));

	// GCC ICE: kai apibrėžiame tik using rejects valid code, pvz.: `aa::make_with_invocable([](std::string &) {});`
	namespace detail {
		template<class F, size_t I>
		constexpr std::type_identity function_argument_t = ([]<class R, class... A>(const std::type_identity<std::function<R(A...)>>) static -> auto {
			if constexpr (is_numeric_max(I)) {
				return type_v<R>;
			} else {
				return type_v<__type_pack_element<I, A...>>;
			}
		})(type_v<deduced_t<std::function, F>>);
	}

	template<class F, size_t I = 0>
	using function_argument_t = type_in_const_t<detail::function_argument_t<F, I>>;

	template<class F>
	using function_result_t = function_argument_t<F, numeric_max>;

	template<class F>
	constexpr size_t function_arity_v = ([]<class R, class... A>(const std::type_identity<std::function<R(A...)>>) static ->
		size_t { return sizeof...(A); })(type_v<deduced_t<std::function, F>>);

	template<class F>
	concept unary_invocable = std::invocable<F, function_argument_t<F>>;

	template<class F, class... A>
	concept out_unary_invocable = unary_invocable<F> && lvalue_reference_like<function_argument_t<F>>
		&& movable_constructible_from<std::remove_reference_t<function_argument_t<F>>, A...>;

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

	// Kitaip alias galėtų būti realizuotas per specializacijas, bet tokiu atveju reiktų papildomo
	// apibrėžimo klasės ar variable (klasės nenaudotume), nes alias apibrėžimų negalima specializuoti.
	template<size_t I, gettable<I> T>
	using get_result_t = type_in_const_t<([] static -> auto {
		if constexpr (member_get_exists<T, I>) {
			// Negalime naudoti `type_v<std::invoke_result_t<const_t<&std::remove_cvref_t<T>::template get<I>>, T>>`,
			// nes jei get deklaruotas su deducing this (tuple get taip deklaruotas), nerasime get adreso.
			return type_v<decltype(std::declval<T>().template get<I>())>;
		} else {
			// Negalime gauti funkcijos rodyklės su ADL.
			return type_v<decltype(get<I>(std::declval<T>()))>;
		}
	})()>;

	// Negalime turėti funkcijos, nes neišeina turėti function aliases patogių.
	// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2769r1.html
	template<size_t I>
	constexpr auto get_element = []<gettable<I> T>(T &&t) static -> get_result_t<I, T> {
		if constexpr (member_get_exists<T, I>)	return std::forward<T>(t).template get<I>();
		else									return get<I>(std::forward<T>(t));
	};

	constexpr const_t<get_element<0>> get_0, get_x, get_key;
	constexpr const_t<get_element<1>> get_1, get_y, get_val;
	constexpr const_t<get_element<2>> get_2, get_z;
	constexpr const_t<get_element<3>> get_3;

	template<class F, auto... A>
	using call_template_t = const_t<&std::remove_cvref_t<F>::template operator()<A...>>;

	template<class M, class F, class... T>
	concept member_invocable = (std::is_member_function_pointer_v<M> ? std::invocable<M, F, T...> : std::invocable<M, T...>);

	template<class M, class F, class... T>
		requires (member_invocable<M, F, T...>)
	using member_result_t = type_in_const_t<([] static -> auto {
		if constexpr (std::is_member_function_pointer_v<M>) {
			return type_v<std::invoke_result_t<M, F, T...>>;
		} else {
			return type_v<std::invoke_result_t<M, T...>>;
		}
	})()>;

	// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p2989r2.pdf
	template<auto... A, class F, class... T>
	constexpr member_result_t<call_template_t<F, A...>, F, T...> invoke(F &&f, T&&... t) {
		return std::forward<F>(f).template operator()<A...>(std::forward<T>(t)...);
	}

	template<size_t N, class F, class... A>
	using apply_result_t = type_in_const_t<([]<size_t... I>(const std::index_sequence<I...>) static
		-> auto { return type_v<member_result_t<call_template_t<F, I...>, F, A...>>; })(default_v<std::make_index_sequence<N>>)>;

	// Nesijaudinant galima naudoti tag tipus kaip funkcijų parametrus.
	// https://www.fluentcpp.com/2021/03/05/stdindex_sequence-and-its-improvement-in-c20/
	// Jei lambdą iškeltume į funkciją, tai ji jokio funkcionalumo nesuteiktų, nes patogiau kiekvienu atveju būtų ne ją naudoti, o atitinkamą invoke.
	template<size_t N, class F, class... A>
	constexpr apply_result_t<N, F, A...> apply(F &&f, A&&... args) {
		return const_v<[]<size_t... I>(const std::index_sequence<I...>, F &&_f, A&&... _args) static
			-> member_result_t<call_template_t<F, I...>, F, A...> {
			return invoke<I...>(std::forward<F>(_f), std::forward<A>(_args)...);
		}>(default_v<std::make_index_sequence<N>>, std::forward<F>(f), std::forward<A>(args)...);
	}

	template<class T, size_t N = 0>
	concept tuple_like = complete<std::tuple_size<T>> && (std::tuple_size_v<T> >= N) && apply<std::tuple_size_v<T>>(
		[]<size_t... I> static -> bool { return (... && wo_ref_same_as<get_result_t<I, T>, std::tuple_element_t<I, T>>); });

	template<class F, class T, class... A>
		requires (std::invocable<F, T &, A...>)
	constexpr T &extend(T &t, F &&f, A&&... args) {
		std::invoke(std::forward<F>(f), t, std::forward<A>(args)...);
		return t;
	}

	template<class F, class... T>
	concept tuple_constructible_to = (tuple_like<F, sizeof...(T)> && apply<sizeof...(T)>(
		[]<size_t... I> static -> bool { return (... && std::constructible_from<T, get_result_t<I, F>>); }));



	// Naudojame constexpr algoritmą, kad surasti indeksą, nes lygiai taip pat ir GCC daro.
	template<auto A, std::equality_comparable_with<const_t<A>> auto... V>
	constexpr size_t pack_index_v = pack_index_v<true, (A == V)...>;

	template<bool A, bool... V>
	constexpr size_t pack_index_v<A, V...> = get_key(*std::ranges::find(std::views::enumerate(stay(std::array{V...})), A, get_val));

	template<class U, class... T>
	constexpr size_t type_pack_index_v = pack_index_v<true, std::same_as<U, T>...>;

	template<class, class...>
	struct basic_tuple;

	// https://danlark.org/2020/04/13/why-is-stdpair-broken/
	// https://en.wikipedia.org/wiki/Tuple
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpadded"
	template<size_t... S, class... T>
	struct basic_tuple<std::index_sequence<S...>, T...> : detail::tuple_unit<S, T>... {
#pragma GCC diagnostic pop
		// Member types
		using tuple_type = basic_tuple;

		template<size_t I>
		using unit_type = detail::tuple_unit<I, __type_pack_element<I, T...>>;

		template<size_t I>
		using value_type = value_type_in_use_t<unit_type<I>>;

		template<size_t I>
		using reference = reference_in_use_t<unit_type<I>>;

		template<size_t I>
		using const_reference = const_reference_in_use_t<unit_type<I>>;

		// Member constants
		template<class U>
		static constexpr size_t index = type_pack_index_v<U, T...>;

		// Capacity
		static consteval size_t tuple_size() { return sizeof...(T); }

		// Element access
		template<size_t I, class H>
			requires (nonempty_unit_param_like<value_type<I>>)
		constexpr forward_like_t<H, value_type<I>> get(this H &&self) {
			return std::forward<H>(self).unit_type<I>::value;
		}

		template<size_t I>
			requires (empty_unit_param_like<value_type<I>>)
		static consteval value_type<I> get() { return unit_type<I>::get(); }

		template<class U, class H>
			requires (nonempty_unit_param_like<value_type<index<U>>>)
		constexpr forward_like_t<H, value_type<index<U>>> get(this H &&self) {
			return std::forward<H>(self).unit_type<index<U>>::value;
		}

		template<class U>
			requires (empty_unit_param_like<value_type<index<U>>>)
		static consteval value_type<index<U>> get() { return unit_type<index<U>>::get(); }

		// Special member functions
		template<constructible_to<value_type<S>>... U>
			requires (!!sizeof...(S))
		constexpr basic_tuple(U&&... u) : unit_type<S>{std::forward<U>(u)}... {}

		template<size_t... I, constructible_to<value_type<I>>... U>
			requires (!!sizeof...(I))
		constexpr basic_tuple(const std::index_sequence<I...>, U&&... u) : unit_type<I>{std::forward<U>(u)}... {}

		template<tuple_constructible_to<value_type<S>...> U>
		constexpr basic_tuple(U &&u) : unit_type<S>{get_element<S>(std::forward<U>(u))}... {}

		constexpr basic_tuple() = default;
	};

	template<class... T>
	using tuple = basic_tuple<std::index_sequence_for<T...>, T...>;

	template<class... T>
	basic_tuple(T&&...) -> tuple<T...>;

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

	template<auto... V>
	struct pack {
		// Member types
		using pack_type = pack;

		template<size_t I>
		using value_type = const_t<pack_element_v<I, V...>>;

		// Member constants
		template<auto A>
		static constexpr size_t index = pack_index_v<A, V...>;

		// Capacity
		static consteval size_t tuple_size() { return sizeof...(V); }

		// Element access
		template<size_t I>
		static consteval value_type<I> get() { return pack_element_v<I, V...>; }
	};



	// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p2098r1.pdf
	template<class T, template<class...> class F>
	concept specialization_of = ([]<class... A>(const std::type_identity<F<A...>>) static -> bool { return true; })(type_v<std::remove_cvref_t<T>>);

	// Su funkcija std::to_array neišeina sukurti tuščio masyvo.
	template<class T, T... A>
	consteval std::array<T, sizeof...(A)> to_array() {
		return {A...};
	}

	template<auto A1, const_t<A1>... A>
	consteval std::array<const_t<A1>, 1 + sizeof...(A)> to_array() {
		return {A1, A...};
	}

	template<sized_contiguous_range T>
	using range_char_traits_t = type_in_const_t<([] static -> auto {
		if constexpr (range_using_traits_type<T>) {
			return type_v<traits_type_in_use_t<T>>;
		} else {
			// Darome prielaidą, kad char_traits yra apibrėžtas su visais tipais.
			return type_v<std::char_traits<std::ranges::range_value_t<T>>>;
		}
	})()>;

	template<class T, class U>
	concept same_range_char_traits_as = std::same_as<range_char_traits_t<T>, U>;

	template<class T>
	concept regular_unsigned_integral = std::unsigned_integral<T> && std::has_single_bit(numeric_digits_v<T>);

	// Nors galėtume paveldėti tiesiog iš first, bet to nedarome, kad nekurti nereikalingų paveldėjimo ryšių.
	template<class T, class... A>
	using first_not_t = __type_pack_element<pack_index_v<false, std::same_as<T, A>...>, A...>;

	template<class N, class T, class U>
	using coalesce_t = std::conditional_t<std::same_as<N, T>, U, T>;

	template<bool B, class T>
	using add_const_if_t = std::conditional_t<B, const T, T>;

	// Reikia using šio, nes testavimui reikėjo sukurti tuple su 100 elementų ir nėra variantas turėti 100 using'ų.
	template<template<class...> class T, auto F, size_t N>
	using filled_t = type_in_const_t<apply<N>([]<size_t... I> static ->
		auto { return type_v<T<type_in_const_t<invoke<I>(F)>...>>; })>;

	template<class T>
	concept new_tuple_like = ((requires { { std::remove_cvref_t<T>::tuple_size() } -> std::same_as<size_t>; })
		&& apply<std::remove_cvref_t<T>::tuple_size()>([]<size_t... I> static -> bool { return (... && gettable<T, I>); }));

	template<class T, size_t N = 0>
	concept uniform_tuple_like = (tuple_like<T, N>
		&& apply<std::tuple_size_v<T>>([]<size_t... I> static -> bool { return is_same_as_every_v<get_result_t<I, T>...>; }));

	template<class T, size_t N = 0>
	concept fixed_string_like = range_using_traits_type<T> && uniform_tuple_like<T, N>;

	template<class F, size_t N>
	concept constifier_like = !!N && apply<N>([]<size_t... I> static -> bool { return is_same_as_every_v<call_template_t<F, I>...>; });

	template<size_t N, constifier_like<N> F>
	constexpr std::array constifier_table_v = apply<N>([]<size_t... I> static ->
		std::array<call_template_t<F, 0uz>, N> { return {(&std::remove_cvref_t<F>::template operator()<I>)...}; });

	template<size_t N, constifier_like<N> F, class... A>
	constexpr member_result_t<call_template_t<F, 0uz>, F, A...> constify(const size_t i, F &&f, A&&... args) {
		if constexpr (std::is_member_function_pointer_v<call_template_t<F, 0uz>>) {
			return (std::forward<F>(f).*constifier_table_v<N, F>[i])(std::forward<A>(args)...);
		} else {
			return constifier_table_v<N, F>[i](std::forward<A>(args)...);
		}
	}



	template<class T>
	using propagate_const_t = type_in_const_t<([]<class U>(this const auto lambda, const std::type_identity<U>) -> auto {
		if constexpr (pointer_like<U>) {
			return type_v<type_in_const_t<lambda(type_v<std::remove_pointer_t<U>>)> *const>;
		} else if constexpr (lvalue_reference_like<U>) {
			return type_v<type_in_const_t<lambda(type_v<std::remove_reference_t<U>>)> &>;
		} else if constexpr (rvalue_reference_like<U>) {
			return type_v<type_in_const_t<lambda(type_v<std::remove_reference_t<U>>)> &&>;
		} else {
			return type_v<const U>;
		}
	})(type_v<T>)>;

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

	// https://mathworld.wolfram.com/Hypermatrix.html
	template<class T, size_t... N>
		requires (!!sizeof...(N))
	using hypermatrix_t = type_in_const_t<([]<size_t I1, size_t... I>(this const auto lambda, const constant<I1>, const constant<I>... args) -> auto {
		if constexpr (sizeof...(I)) {
			return type_v<std::array<type_in_const_t<lambda(args...)>, I1>>;
		} else {
			return type_v<std::array<T, I1>>;
		}
	})(default_v<constant<N>>...)>;

	template<unsigned_integral_or_same_as<void> U = void, std::integral X>
	constexpr coalesce_t<void, U, std::make_unsigned_t<X>> unsign(const X x) {
		return cast<coalesce_t<void, U, std::make_unsigned_t<X>>>(
			std::bit_cast<std::make_unsigned_t<X>>(x));
	}

	template<signed_integral_or_same_as<void> U = void, std::integral X>
	constexpr coalesce_t<void, U, std::make_signed_t<X>> sign(const X x) {
		using T = coalesce_t<void, U, std::make_signed_t<X>>;
		return std::bit_cast<T>(unsign<std::make_unsigned_t<T>>(x));
	}

	template<std::integral T, std::integral X>
	constexpr T sign_cast(const X x) {
		if constexpr (std::unsigned_integral<T>)	return unsign<T>(x);
		else										return sign<T>(x);
	}

	template<class T, constructible_to<T> X>
	constexpr T sign_or_cast(X &&x) {
		if constexpr (std::integral<T> && std::integral<std::remove_reference_t<X>>) {
			return sign_cast<T>(x);
		} else {
			return cast<T>(std::forward<X>(x));
		}
	}

	template<std::integral U = size_t, std::unsigned_integral T>
	constexpr U int_exp2(const T x) {
		return value_v<U, 1> << x;
	}

	// T yra tipas, kurio bitus skaičiuosime, U nurodo kokiu tipu pateikti rezutatus.
	// [0, digits<T>) ∪ {0b(1)_digits<T>}
	template<std::integral U = size_t, std::unsigned_integral T>
	constexpr U int_log2(const T x) {
		return (value_v<U, numeric_digits_v<std::make_signed_t<T>>>) - sign_cast<U>(std::countl_zero(x));
	}

	// Naudojamas size_t tipas kaip konstantos tipas, o ne sekančio dydžio integer tipas, nes konstanta
	// turėtų būti naudojama malloc ir panašiuose kontekstuose, o ten reikalaujama size_t tipo išraiška.
	// T turi turėti unique object representations, nes kitaip neišeis patikimai apskaičiuoti konstantos.
	//
	// Vietoje byte negalime naudoti uint8_t, nes jei sistemoje baitas būtų ne 8 bitų, tas tipas nebus apibrėžtas.
	template<uniquely_representable T>
	constexpr size_t representable_values_v = int_exp2(sizeof(T[numeric_digits_v<std::byte>]));

	template<out_unary_invocable F>
	constexpr std::remove_reference_t<function_argument_t<F>> make_with_invocable(F &&f = default_value) {
		std::remove_reference_t<function_argument_t<F>> d;
		std::invoke(std::forward<F>(f), d);
		return d;
	}

	template<class F, class... A>
		requires (!!sizeof...(A) && out_unary_invocable<F, A...>)
	constexpr std::remove_reference_t<function_argument_t<F>> make_with_invocable(F &&f, A&&... args) {
		std::remove_reference_t<function_argument_t<F>> d = {std::forward<A>(args)...};
		std::invoke(std::forward<F>(f), d);
		return d;
	}

	// Klasė naudinga naudoti su klasėmis, kurių move konstruktorius yra ištrintas, nes tokios klasės negali dalyvauti NRVO.
	// https://devblogs.microsoft.com/oldnewthing/20230612-00/?p=108329
	template<nonempty_unit_param_like T>
	struct make : unit<T> {
		// Member types
		using typename unit<T>::tuple_type;
		using unit_type = typename tuple_type::unit_type<0>;
		using typename unit_type::value_type, unit_type::get,
			typename unit_type::reference, typename unit_type::const_reference;

		// Observers
		template<class S>
		constexpr std::add_pointer_t<forward_like_t<S, value_type>> operator->(this S &&self) {
			return &std::forward<S>(self).value;
		}

		template<class S>
		constexpr std::add_pointer_t<forward_like_t<S, value_type>> operator&(this S &&self) {
			return &std::forward<S>(self).value;
		}

		template<class S>
		constexpr operator forward_like_t<S, value_type>(this S &&self) {
			return std::forward<S>(self).value;
		}

		template<class S>
		constexpr forward_like_t<S, value_type> operator*(this S &&self) {
			return std::forward<S>(self).value;
		}

		// Special member functions
		template<std::invocable<reference> F>
		constexpr make(F &&f) {
			std::invoke(std::forward<F>(f), unit_type::value);
		}

		template<std::invocable<reference> F, constructible_to<value_type> U>
		constexpr make(F &&f, U &&u) : tuple_type{std::forward<U>(u)} {
			std::invoke(std::forward<F>(f), unit_type::value);
		}
	};



	template<scoped_enum_like E>
	constexpr E to_enum(const std::underlying_type_t<E> u) {
		return std::bit_cast<E>(u);
	}

	namespace operators {
		template<scoped_enum_like E>
		constexpr bool operator!(const E rhs) {
			return !std::to_underlying(rhs);
		}

		template<scoped_enum_like E>
		constexpr std::underlying_type_t<E> operator+(const E rhs) {
			return std::to_underlying(rhs);
		}

		template<scoped_enum_like E>
		constexpr std::underlying_type_t<E> operator~(const E rhs) {
			return ~std::to_underlying(rhs);
		}

		template<scoped_enum_like E>
		constexpr std::underlying_type_t<E> operator|(const E lhs, const E rhs) {
			return std::to_underlying(lhs) | std::to_underlying(rhs);
		}

		template<scoped_enum_like E>
		constexpr std::underlying_type_t<E> operator|(const std::underlying_type_t<E> lhs, const E rhs) {
			return lhs | std::to_underlying(rhs);
		}

		template<scoped_enum_like E>
		constexpr std::underlying_type_t<E> operator|(const E lhs, const std::underlying_type_t<E> rhs) {
			return std::to_underlying(lhs) | rhs;
		}

		template<scoped_enum_like E>
		constexpr std::underlying_type_t<E> operator&(const E lhs, const E rhs) {
			return std::to_underlying(lhs) & std::to_underlying(rhs);
		}

		template<scoped_enum_like E>
		constexpr std::underlying_type_t<E> operator&(const std::underlying_type_t<E> lhs, const E rhs) {
			return lhs & std::to_underlying(rhs);
		}

		template<scoped_enum_like E>
		constexpr std::underlying_type_t<E> operator&(const E lhs, const std::underlying_type_t<E> rhs) {
			return std::to_underlying(lhs) & rhs;
		}

		template<scoped_enum_like E>
		constexpr std::underlying_type_t<E> operator^(const E lhs, const E rhs) {
			return std::to_underlying(lhs) ^ std::to_underlying(rhs);
		}

		template<scoped_enum_like E>
		constexpr std::underlying_type_t<E> operator^(const std::underlying_type_t<E> lhs, const E rhs) {
			return lhs ^ std::to_underlying(rhs);
		}

		template<scoped_enum_like E>
		constexpr std::underlying_type_t<E> operator^(const E lhs, const std::underlying_type_t<E> rhs) {
			return std::to_underlying(lhs) ^ rhs;
		}
	}



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



	// Parameter pack negali susidėti iš daug elementų todėl šitą funkciją reiktų naudoti tik kai reikia mažai
	// iteracijų. Tačiau toks iteravimas greitesnis, nes nereikia prižiūrėti papildomo ciklo kintamojo.
	//
	// Greitaveika nenukenčia padavinėjant template parametrus todėl neturime funkcijos užklojimo kito.
	// P yra value tipas, nes funktoriai, kurie bus kviečiami daug kartų yra taip padavinėjami. Kitaip P tipas būtų rvalue reference.
	template<size_t N, class F>
	constexpr void repeat(F f) {
		apply<N>([&]<size_t... I> -> void { (aa::invoke<I>(f), ...); });
	}



	// Atrodo galima būtų turėti tipą, kuris generalizuotų šią klasę, tai yra tiesiog jis susidėtų iš
	// paduotų tipų. Bet toks tipas nebūtų naudingas, nes užklojimai neveiktų gerai, todėl būtent ir
	// reikia šio overload tipo, kad bent jau veiktų operatoriaus () užklojimai gerai. Tačiau realizacija
	// reikalauja, kad visi paduodami tipai turėtų būtinai tik vieną operatorių (), gal būtų galima realizuoti
	// taip tipą, kad tokio reikalavimo neliktų, bet tokios realizacijos savybės dabar nereikalingos.
	template<functor... T>
	struct overload : T... {
		using T::operator()...;
		using is_transparent = void;
	};

	template<class T, class... A>
	using next_type_t = __type_pack_element<(type_pack_index_v<T, A...>) + 1, A...>;

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
struct std::tuple_size<T &> : aa::constant<std::tuple_size_v<T>> {};

template<size_t I, class T>
struct std::tuple_element<I, T &> : std::type_identity<std::tuple_element_t<I, T>> {};

// Negalime tikrinti ar prieš šį momentą tuple_size<T> buvo deklaruotas tipas ar ne, nes įeitume į begalinį
// ciklą. Reiškia turi mums pats tipas pranešti ar jis nori būti laikomas kaip tuple like tipas.
template<aa::new_tuple_like T>
struct std::tuple_size<T> : aa::constant<T::tuple_size()> {};

template<size_t I, aa::new_tuple_like T>
struct std::tuple_element<I, T> : std::type_identity<std::remove_reference_t<aa::get_result_t<I, T>>> {};
