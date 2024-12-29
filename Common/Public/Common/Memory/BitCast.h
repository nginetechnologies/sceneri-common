#pragma once

#include "Common/TypeTraits/EnableIf.h"
#include "Common/Platform/Pure.h"
#include "Common/Platform/NoDebug.h"
#include "Common/TypeTraits/IsTriviallyCopyable.h"
#include "Common/TypeTraits/IsTriviallyConstructible.h"
#include <Common/Memory/Copy.h>

namespace ngine::Memory
{
	// std::bit_cast C++20
#if __has_builtin(__builtin_bit_cast)
#define MEMORY_CONSTEXPR_BITCAST

	template<class To, class From>
	[[nodiscard]] PURE_STATICS NO_DEBUG constexpr EnableIf<
		sizeof(To) == sizeof(From) && TypeTraits::IsTriviallyCopyable<From> && TypeTraits::IsTriviallyCopyable<To>,
		To>
	BitCast(const From& src) noexcept
	{
		return __builtin_bit_cast(To, src);
	}
#else
	template<class To, class From>
	[[nodiscard]] PURE_STATICS
		NO_DEBUG EnableIf<sizeof(To) == sizeof(From) && TypeTraits::IsTriviallyCopyable<From> && TypeTraits::IsTriviallyCopyable<To>, To>
		BitCast(const From& src) noexcept
	{
		static_assert(TypeTraits::IsTriviallyConstructible<To>, "This compiler requires destination to be trivially constructible");

		To dst;
		CopyWithoutOverlap(&dst, &src, sizeof(dst));
		return dst;
	}
#endif
}
