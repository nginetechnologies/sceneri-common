#pragma once

#include <Common/Threading/ForwardDeclarations/Atomic.h>

#include <Common/Platform/ForceInline.h>

#include <Common/TypeTraits/IsSame.h>
#include <Common/Math/Max.h>

#include <Common/Threading/Atomics/Load.h>
#include <Common/Threading/Atomics/Exchange.h>
#include <Common/Threading/Atomics/Store.h>
#include <Common/Threading/Atomics/CompareExchangeWeak.h>
#include <Common/Threading/Atomics/CompareExchangeStrong.h>
#include <Common/Threading/Atomics/FetchIncrement.h>
#include <Common/Threading/Atomics/FetchDecrement.h>
#include <Common/Threading/Atomics/FetchAdd.h>
#include <Common/Threading/Atomics/FetchSubtract.h>
#include <Common/Threading/Atomics/FetchAnd.h>
#include <Common/Threading/Atomics/FetchOr.h>
#include <Common/Threading/Atomics/FetchXor.h>
#include <Common/TypeTraits/TypeConstant.h>

namespace ngine::Threading
{
	namespace Internal
	{
		template<typename Type>
		struct LockfreeAtomic
		{
			LockfreeAtomic() = default;
			FORCE_INLINE constexpr LockfreeAtomic(const Type value)
				: m_value(value)
			{
			}
			LockfreeAtomic(const LockfreeAtomic&) = default;
			LockfreeAtomic& operator=(const LockfreeAtomic&) = default;
			LockfreeAtomic(LockfreeAtomic&&) = default;
			LockfreeAtomic& operator=(LockfreeAtomic&&) = default;
			~LockfreeAtomic() = default;

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
				Atomics::Store(m_value, value);
			}
		protected:
			Type m_value;
		};

		template<typename Type, typename = TypeTraits::TrueType>
		struct LockfreeAtomicWithArithmetic : public LockfreeAtomic<Type>
		{
			using BaseType = LockfreeAtomic<Type>;
			using BaseType::BaseType;

			[[nodiscard]] FORCE_INLINE Type FetchAdd(const Type value)
			{
				return Atomics::FetchAdd(BaseType::m_value, value);
			}
			[[nodiscard]] FORCE_INLINE Type FetchSubtract(const Type value)
			{
				return Atomics::FetchSubtract(BaseType::m_value, value);
			}
			[[nodiscard]] FORCE_INLINE Type FetchAnd(const Type value)
			{
				return Atomics::FetchAnd(BaseType::m_value, value);
			}
			[[nodiscard]] FORCE_INLINE Type FetchOr(const Type value)
			{
				return Atomics::FetchOr(BaseType::m_value, value);
			}
			[[nodiscard]] FORCE_INLINE Type FetchXor(const Type value)
			{
				return Atomics::FetchXor(BaseType::m_value, value);
			}

			void AssignMax(const Type value)
			{
				Type currentValue = BaseType::Load();
				Type max = Math::Max(currentValue, value);
				while (!BaseType::CompareExchangeWeak(currentValue, max))
				{
					max = Math::Max(currentValue, value);
				}
			}

			FORCE_INLINE void operator=(const Type value)
			{
				BaseType::Store(value);
			}

			FORCE_INLINE Type operator++()
			{
				return Atomics::FetchIncrement(BaseType::m_value) + 1;
			}
			FORCE_INLINE Type operator++(int)
			{
				return Atomics::FetchIncrement(BaseType::m_value);
			}
			FORCE_INLINE Type operator--()
			{
				return Atomics::FetchDecrement(BaseType::m_value) - 1;
			}
			FORCE_INLINE Type operator--(int)
			{
				return Atomics::FetchDecrement(BaseType::m_value);
			}

			FORCE_INLINE Type operator+=(const Type value)
			{
				return FetchAdd(value);
			}

			FORCE_INLINE Type operator-=(const Type value)
			{
				return FetchSubtract(value);
			}
			FORCE_INLINE Type operator&=(const Type value)
			{
				return FetchAnd(value);
			}

			FORCE_INLINE Type operator|=(const Type value)
			{
				return FetchOr(value);
			}

			FORCE_INLINE Type operator^=(const Type value)
			{
				return FetchXor(value);
			}
		};

		extern template struct Internal::LockfreeAtomic<bool>;
		extern template struct Internal::LockfreeAtomic<uint8>;
		extern template struct Internal::LockfreeAtomicWithArithmetic<uint8>;
		extern template struct Internal::LockfreeAtomic<uint16>;
		extern template struct Internal::LockfreeAtomicWithArithmetic<uint16>;
		extern template struct Internal::LockfreeAtomic<uint32>;
		extern template struct Internal::LockfreeAtomicWithArithmetic<uint32>;
		extern template struct Internal::LockfreeAtomic<uint64>;
		extern template struct Internal::LockfreeAtomicWithArithmetic<uint64>;
		extern template struct Internal::LockfreeAtomic<int8>;
		extern template struct Internal::LockfreeAtomicWithArithmetic<int8>;
		extern template struct Internal::LockfreeAtomic<int16>;
		extern template struct Internal::LockfreeAtomicWithArithmetic<int16>;
		extern template struct Internal::LockfreeAtomic<int32>;
		extern template struct Internal::LockfreeAtomicWithArithmetic<int32>;
		extern template struct Internal::LockfreeAtomic<int64>;
		extern template struct Internal::LockfreeAtomicWithArithmetic<int64>;

#if IS_SIZE_UNIQUE_TYPE
		extern template struct Internal::LockfreeAtomic<size>;
		extern template struct Internal::LockfreeAtomicWithArithmetic<size>;
#endif

		extern template struct Internal::LockfreeAtomic<void*>;
	}
}
