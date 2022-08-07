#pragma once
#
#include "../io/source_logger.hpp"
#include "general.hpp"
#
#
#
// ASSERT'ai turi būti macros, o ne funkcijos, nes esmė, kad neturi būti programos vykdymo metu jokio
// sulėtėjimo dėl ASSERT'ų jei yra NDEBUG defined, o funkcijos atveju jos iškvietimo neišeitų išvengti.
// ASSERT'ai gali būti implementuoti kaip expression'ai, bet nemanau, kad tai būtų naudinga.
#define AA_TRACE_ASSERT(cond, ...) { if (!(cond)) { ::aa::source_logger{}.abort(\
	AA_EXPAND_FIRST(__VA_OPT__((__VA_ARGS__) ,) ("Assertion '" #cond "' failed."))); } } AA_DISCARD()
#define AA_DEBUG_ASSERT(cond, ...) AA_IF_DEBUG(AA_TRACE_ASSERT(cond, __VA_ARGS__))
