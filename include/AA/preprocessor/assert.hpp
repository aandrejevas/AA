#pragma once
#
#include "../metaprogramming/general.hpp"
#include "../system/source.hpp"
#include "general.hpp"
#
#
#
#define AA_SOURCE_DATA (::aa::source_data<::aa::numeric_max, __LINE__, __FILE__, __PRETTY_FUNCTION__>{})
#
#
#
# // ASSERT'ai turi būti macros, o ne funkcijos, nes esmė, kad neturi būti programos vykdymo metu jokio
# // sulėtėjimo dėl ASSERT'ų jei yra NDEBUG defined, o funkcijos atveju jos iškvietimo neišeitų išvengti.
# //
# // ASSERT negali būti realizuotas kaip if, nes tokiu atveju mažiau kur galėtume naudoti ASSERT,
# // greitaveikos tikrai nelaimėtume. cast'o reikia, kad veiktų short-circuit evaluation.
#define AA_TRACE_ASSERT(cond, ...) static_cast<void>(::aa::cast<bool>(cond) || (::aa::abort<AA_SOURCE_DATA>(\
	AA_EXPAND_FIRST(__VA_OPT__((__VA_ARGS__) ,) ("Assertion '" #cond "' failed."))), false))
#define AA_DEBUG_ASSERT(cond, ...) AA_IF_DEBUG(AA_TRACE_ASSERT(cond, __VA_ARGS__))
