#pragma once
#
#
#
#define AA_STRINGIFY_I(...) #__VA_ARGS__
#define AA_STRINGIFY(...) AA_STRINGIFY_I(__VA_ARGS__)
#
#
#
#define AA_CONCATENATE_I(a, b) a ## b
#define AA_CONCATENATE(a, b) AA_CONCATENATE_I(a, b)
#
#
#
#define AA_EXPAND(...) __VA_ARGS__
#define AA_EXPAND_FIRST(a, ...) AA_EXPAND a
#
#
#
#define AA_VARIADIC_SIZE_I(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, size, ...) size
#define AA_VARIADIC_SIZE(...) AA_VARIADIC_SIZE_I(__VA_ARGS__ __VA_OPT__(,) 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#
#
#
#define AA_OVERLOAD(prefix, ...) AA_CONCATENATE(prefix, AA_VARIADIC_SIZE(__VA_ARGS__))
#
#
#
#ifdef NDEBUG
#define AA_IF_DEBUG(...) static_cast<void>(false)
#else
#define AA_IF_DEBUG(...) AA_EXPAND(__VA_ARGS__)
#endif
#
#
#
# // Filosofija bibliotekos tokia, visos funkcijos žymimos constexpr ir tiek. Nesvarbu gali ar negali būti funkcija
# // naudojama constexpr kontekste, ji bus pažymėta constexpr. Gal naudotojams kiek neaišku gali būti ar jie gali
# // funkciją naudoti constexpr kontekste, bet aš nenoriu mąstyti apie tai ar funkcijos realizacija gali būti constexpr.
#define AA_CONSTEXPR inline constexpr
#define AA_CONSTEVAL inline consteval
#
# // Reikia stengtis turėti kuo mažiau macros. DISCARD macro nereikia, nes tiesiog galime cast'inti į void. Taip pat
# // nėra reikalo naudoti std::ignore, kad discard'inti, nes reiktų visur dėti papildomą include, taip pat nėra apibrėžta
# // gerai kaip veikia std::ignore. NAMEOF macro nereikia, nes mažai panaudojamas toks macro ir neteisingai jis veikia.
#
# // Nenaudojame atributo "always_inline", nes standartinėje bibliotekoje atributas nenaudojamas, trukdo derintuvei
# // atributas, gali būti, kad tik kenkiu greitaveikai naudodamas tą atributą, geriau už kompiliatorių nenuspręsiu,
# // kur jį reikia naudoti, o kur ne, galiausiai išprotėčiau jei dar reiktų prižiūrėti ar visur tvarkingai sudėtas jis.
