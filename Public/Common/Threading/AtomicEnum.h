#pragma once

#include <Common/Threading/AtomicBase.h>
#include <Common/Platform/UnderlyingType.h>
#include <Common/TypeTraits/IsEnum.h>
#include <Common/TypeTraits/EnableIf.h>

namespace ngine::Threading
{
	template<typename Type>
	struct Atomic<Type, EnableIf<TypeTraits::IsEnum<Type>>> : private Internal::LockfreeAtomicWithArithmetic<UNDERLYING_TYPE(Type)>
	{
		using UnderlyingType = UNDERLYING_TYPE(Type);
		using BaseType = Internal::LockfreeAtomicWithArithmetic<UnderlyingType>;

		Atomic()
		{
		}

		FORCE_INLINE constexpr Atomic(const Type value)
			: BaseType(static_cast<UnderlyingType>(value))
		{
		}

		[[nodiscard]] operator Type() const
		{
			return static_cast<Type>(BaseType::operator UnderlyingType());
		}
		[[nodiscard]] Type Load() const
		{
			return static_cast<Type>(BaseType::operator UnderlyingType());
		}

		void operator=(const Type value)
		{
			BaseType::operator=(static_cast<UnderlyingType>(value));
		}

		bool CompareExchangeStrong(Type& expected, const Type desired)
		{
			return BaseType::CompareExchangeStrong(reinterpret_cast<UnderlyingType&>(expected), static_cast<UnderlyingType>(desired));
		}

		bool CompareExchangeWeak(Type& expected, const Type desired)
		{
			return BaseType::CompareExchangeWeak(reinterpret_cast<UnderlyingType&>(expected), static_cast<UnderlyingType>(desired));
		}

		[[nodiscard]] Type Exchange(const Type other)
		{
			return static_cast<Type>(BaseType::Exchange(static_cast<UnderlyingType>(other)));
		}

		[[nodiscard]] Type FetchAnd(const Type value)
		{
			return static_cast<Type>(BaseType::FetchAnd(static_cast<UnderlyingType>(value)));
		}

		[[nodiscard]] Type FetchOr(const Type value)
		{
			return static_cast<Type>(BaseType::FetchOr(static_cast<UnderlyingType>(value)));
		}

		[[nodiscard]] Type FetchXor(const Type value)
		{
			return static_cast<Type>(BaseType::FetchXor(static_cast<UnderlyingType>(value)));
		}
	};
}
