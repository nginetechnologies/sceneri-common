#pragma once

#include <Common/Threading/Atomics/Load.h>
#include <Common/Threading/Atomics/Exchange.h>
#include <Common/Threading/Atomics/Store.h>
#include <Common/Threading/Atomics/CompareExchangeWeak.h>
#include <Common/Threading/Atomics/CompareExchangeStrong.h>
#include <Common/Platform/TrivialABI.h>

namespace ngine::Threading
{
	template<typename Type>
	struct TRIVIAL_ABI AtomicView
	{
		FORCE_INLINE constexpr AtomicView(Type& value)
			: m_value(value)
		{
		}

		[[nodiscard]] FORCE_INLINE operator Type() const
		{
			return Load();
		}
		[[nodiscard]] FORCE_INLINE bool operator==(const Type other) const
		{
			return Load() == other;
		}
		[[nodiscard]] FORCE_INLINE bool operator!=(const Type other) const
		{
			return Load() != other;
		}

		FORCE_INLINE bool CompareExchangeStrong(Type& expected, const Type desired)
		{
			return Atomics::CompareExchangeStrong(m_value, expected, desired);
		}

		[[nodiscard]] FORCE_INLINE bool CompareExchangeWeak(Type& expected, const Type desired)
		{
			return Atomics::CompareExchangeWeak(m_value, expected, desired);
		}

		[[nodiscard]] FORCE_INLINE Type Exchange(const Type other)
		{
			return Atomics::Exchange(m_value, other);
		}

		FORCE_INLINE void operator=(const Type value)
		{
			Store(value);
		}

		[[nodiscard]] FORCE_INLINE Type Load() const
		{
			return Atomics::Load(m_value);
		}
	protected:
		FORCE_INLINE void Store(const Type value)
		{
			Atomics::Store(value);
		}
	private:
		Type& m_value;
	};
}
