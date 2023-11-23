#pragma once
#
#include "../metaprogramming/general.hpp"
#include "../system/source.hpp"
#include "general.hpp"
#
#
#
#define AA_PRETTY_SOURCE_DATA ::aa::default_value_v<::aa::source_data<__LINE__, __FILE__, __PRETTY_FUNCTION__>>
#define AA_SOURCE_DATA ::aa::default_value_v<::aa::source_data<__LINE__, __FILE__, __FUNCTION__>>
#
#
#
#define AA_LOG(...) ::aa::log<AA_PRETTY_SOURCE_DATA>(__VA_ARGS__)
#define AA_ABORT(...) ::aa::abort<AA_PRETTY_SOURCE_DATA>(__VA_ARGS__)
#
# // ASSERT'ai turi būti macros, o ne funkcijos, nes esmė, kad neturi būti programos vykdymo metu jokio
# // sulėtėjimo dėl ASSERT'ų jei yra NDEBUG defined, o funkcijos atveju jos iškvietimo neišeitų išvengti.
# //
# // ASSERT negali būti realizuotas kaip if, nes tokiu atveju mažiau kur galėtume naudoti ASSERT,
# // greitaveikos tikrai nelaimėtume. cast'o reikia, kad veiktų short-circuit evaluation.
#define AA_TRACE_ASSERT(cond, ...) static_cast<void>(::aa::cast<bool>(cond) || (AA_ABORT(\
	AA_EXPAND_FIRST(__VA_OPT__((__VA_ARGS__) ,) ("Assertion '" #cond "' failed."))), false))
#define AA_DEBUG_ASSERT(cond, ...) AA_IF_DEBUG(AA_TRACE_ASSERT(cond, __VA_ARGS__))
