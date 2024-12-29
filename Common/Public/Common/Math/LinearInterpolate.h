#pragma once

namespace ngine::Math
{
	template<typename TypeA, typename TypeB, typename TAlpha>
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS constexpr TypeA LinearInterpolate(const TypeA a, const TypeB b, const TAlpha alpha) noexcept
	{
		return a + (TypeA(b) - a) * alpha;
	}
}
