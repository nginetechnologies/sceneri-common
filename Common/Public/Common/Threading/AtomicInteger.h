#pragma once

#include <Common/Threading/AtomicBase.h>
#include <Common/Math/CoreNumericTypes.h>
#include <Common/TypeTraits/IsIntegral.h>
#include <Common/TypeTraits/IsEnum.h>
#include <Common/TypeTraits/Select.h>

namespace ngine::Threading
{
	template<typename Type>
	struct Atomic<Type, EnableIf<TypeTraits::IsIntegral<Type> && !TypeTraits::IsEnum<Type>>>
		: public Internal::LockfreeAtomicWithArithmetic<Type, EnableIf<TypeTraits::IsIntegral<Type> && !TypeTraits::IsEnum<Type>>>
	{
		using BaseType = Internal::LockfreeAtomicWithArithmetic<Type, EnableIf<TypeTraits::IsIntegral<Type> && !TypeTraits::IsEnum<Type>>>;

		using BaseType::BaseType;
		using BaseType::operator Type;
		using BaseType::CompareExchangeStrong;
		using BaseType::CompareExchangeWeak;
		using BaseType::Exchange;
		using BaseType::AssignMax;
		using BaseType::operator=;
	};

	extern template struct Atomic<uint8>;
	extern template struct Atomic<uint16>;
	extern template struct Atomic<uint32>;
	extern template struct Atomic<uint64>;
	extern template struct Atomic<int8>;
	extern template struct Atomic<int16>;
	extern template struct Atomic<int32>;
	extern template struct Atomic<int64>;

	extern template struct Internal::LockfreeAtomicWithArithmetic<uint8>;
	extern template struct Internal::LockfreeAtomicWithArithmetic<uint16>;
	extern template struct Internal::LockfreeAtomicWithArithmetic<uint32>;
	extern template struct Internal::LockfreeAtomicWithArithmetic<uint64>;
	extern template struct Internal::LockfreeAtomicWithArithmetic<int8>;
	extern template struct Internal::LockfreeAtomicWithArithmetic<int16>;
	extern template struct Internal::LockfreeAtomicWithArithmetic<int32>;
	extern template struct Internal::LockfreeAtomicWithArithmetic<int64>;

	extern template struct Internal::LockfreeAtomic<uint8>;
	extern template struct Internal::LockfreeAtomic<uint16>;
	extern template struct Internal::LockfreeAtomic<uint32>;
	extern template struct Internal::LockfreeAtomic<uint64>;
	extern template struct Internal::LockfreeAtomic<int8>;
	extern template struct Internal::LockfreeAtomic<int16>;
	extern template struct Internal::LockfreeAtomic<int32>;
	extern template struct Internal::LockfreeAtomic<int64>;

#if IS_SIZE_UNIQUE_TYPE
	extern template struct Atomic<size>;
	extern template struct Internal::LockfreeAtomicWithArithmetic<int64>;
	extern template struct Internal::LockfreeAtomicWithArithmetic<size>;
#endif
}
