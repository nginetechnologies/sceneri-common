#pragma once

#include <Common/Threading/AtomicBase.h>

namespace ngine::Threading
{
	template<typename Type>
	struct Atomic<Type*> : private Internal::LockfreeAtomicWithArithmetic<void*>
	{
		using BaseType = Internal::LockfreeAtomicWithArithmetic<void*>;

		FORCE_INLINE constexpr Atomic()
			: BaseType(nullptr)
		{
		}
		FORCE_INLINE constexpr Atomic(Type* const value)
			: BaseType(value)
		{
		}

		[[nodiscard]] operator Type*() const
		{
			return static_cast<Type*>(BaseType::operator void*());
		}
		[[nodiscard]] FORCE_INLINE Type* operator->() const noexcept
		{
			Assert(Load() != nullptr);
			return static_cast<Type*>(BaseType::operator void*());
		}
		[[nodiscard]] FORCE_INLINE Type& operator*() const noexcept
		{
			Assert(Load() != nullptr);
			return *static_cast<Type*>(BaseType::operator void*());
		}
		[[nodiscard]] bool operator==(Type* const other) const
		{
			return Load() == other;
		}
		[[nodiscard]] bool operator!=(Type* const other) const
		{
			return Load() != other;
		}
		void operator=(Type* const value)
		{
			Store(value);
		}

		bool CompareExchangeStrong(Type*& expected, Type* const desired)
		{
			return BaseType::CompareExchangeStrong(reinterpret_cast<void*&>(expected), desired);
		}

		[[nodiscard]] bool CompareExchangeWeak(Type*& expected, Type* const desired)
		{
			return BaseType::CompareExchangeWeak(reinterpret_cast<void*&>(expected), desired);
		}

		[[nodiscard]] Type* Exchange(Type* const other)
		{
			return reinterpret_cast<Type*>(BaseType::Exchange(reinterpret_cast<void*>(other)));
		}

		[[nodiscard]] Type* Load() const
		{
			return reinterpret_cast<Type*>(BaseType::Load());
		}

		void AssignMax(Type* value)
		{
			BaseType::AssignMax(reinterpret_cast<void*>(value));
		}
	};
}
