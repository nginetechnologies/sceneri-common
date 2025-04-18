#pragma once

#include <Common/Platform/ForceInline.h>
#include <Common/Platform/Pure.h>

namespace ngine::Math
{
	template<typename T>
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS T Mod(const T x, const T y) noexcept
	{
		return x % y;
	}

	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS double Mod(const double x, const double y) noexcept
	{
		return ::fmod(x, y);
	}

	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS float Mod(const float x, const float y) noexcept
	{
		return ::fmodf(x, y);
	}
}
