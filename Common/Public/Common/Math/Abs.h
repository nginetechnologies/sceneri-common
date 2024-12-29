#pragma once

#include <Common/Math/Select.h>
#include <Common/Platform/Pure.h>

#include <math.h>

namespace ngine::Math
{
	namespace Internal
	{
		template<typename T>
		[[nodiscard]] FORCE_INLINE PURE_NOSTATICS T IntegerAbs(T value) noexcept
		{
			return (value < 0) ? -value : value;
		}
	}

	template<typename T>
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS T Abs(const T value) noexcept
	{
		return Select(value >= 0, value, -value);
	}

	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS double Abs(const double value) noexcept
	{
		return ::fabs(value);
	}

	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS float Abs(const float value) noexcept
	{
		return ::fabsf(value);
	}

	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS int8 Abs(int8 value) noexcept
	{
		return Internal::IntegerAbs(value);
	}
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS int16 Abs(int16 value) noexcept
	{
		return Internal::IntegerAbs(value);
	}
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS int32 Abs(int32 value) noexcept
	{
		return Internal::IntegerAbs(value);
	}
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS int64 Abs(int64 value) noexcept
	{
		return Internal::IntegerAbs(value);
	}

	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS uint8 Abs(const uint8 value) noexcept
	{
		return value;
	}
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS uint16 Abs(const uint16 value) noexcept
	{
		return value;
	}
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS uint32 Abs(const uint32 value) noexcept
	{
		return value;
	}
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS uint64 Abs(const uint64 value) noexcept
	{
		return value;
	}
}
