#pragma once

#include <Common/Platform/ForceInline.h>
#include <Common/TypeTraits/EnableIf.h>
#include <Common/TypeTraits/IsIntegral.h>
#include <Common/Math/NumericLimits.h>

namespace ngine::Memory
{
	template<typename ToType, typename FromType, typename = EnableIf<TypeTraits::IsIntegral<ToType> && TypeTraits::IsIntegral<FromType>>>
	[[nodiscard]] FORCE_INLINE ToType CheckedCast(const FromType value)
	{
		// return (m_minimum >= other.m_minimum) & (m_maximum <= other.m_maximum);
		// return (value >= m_minimum) & (value <= m_maximum);
		if constexpr (Math::NumericLimits<FromType>::Min >= Math::NumericLimits<ToType>::Min && Math::NumericLimits<FromType>::Max <= Math::NumericLimits<ToType>::Max)
		{
			// Can convert without issues
			return static_cast<ToType>(value);
		}
		else
		{
			Assert(value >= Math::NumericLimits<ToType>::Min && value <= Math::NumericLimits<ToType>::Max);
			return static_cast<ToType>(value);
		}
	}
}
