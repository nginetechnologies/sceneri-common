#pragma once

#include <Common/Math/CoreNumericTypes.h>
#include <Common/Threading/Atomics/CompareExchangeStrong.h>
#include <Common/Threading/Atomics/CompareExchangeWeak.h>
#include <Common/Threading/Atomics/Load.h>

namespace ngine::Threading
{
	template<>
	struct Atomic<uint128>
	{
		Atomic() = default;
		FORCE_INLINE constexpr Atomic(const uint128 value)
			: m_value(value)
		{
		}
		Atomic(const Atomic&) = default;
		Atomic& operator=(const Atomic&) = default;
		Atomic(Atomic&&) = default;
		Atomic& operator=(Atomic&&) = default;
		~Atomic() = default;

		FORCE_INLINE bool CompareExchangeStrong(uint128& expected, const uint128 desired)
		{
			return Atomics::CompareExchangeStrong(m_value, expected, desired);
		}

		[[nodiscard]] FORCE_INLINE bool CompareExchangeWeak(uint128& expected, const uint128 desired)
		{
			return Atomics::CompareExchangeWeak(m_value, expected, desired);
		}

		[[nodiscard]] FORCE_INLINE uint128 Load() const
		{
			return Atomics::Load(m_value);
		}
	protected:
		uint128 m_value;
	};
}
