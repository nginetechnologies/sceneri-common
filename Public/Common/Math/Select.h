#pragma once

#include <Common/Math/CoreNumericTypes.h>
#include <Common/Platform/ForceInline.h>
#include <Common/Platform/Pure.h>
#include <Common/TypeTraits/EnableIf.h>
#include <Common/TypeTraits/IsConvertibleTo.h>

namespace ngine::Math
{
	template<typename ConditionType, typename T1, typename T2, typename = EnableIf<TypeTraits::IsConvertibleTo<ConditionType, bool>>>
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS constexpr auto
	Select(const ConditionType condition, const T1 trueValue, const T2 falseValue) noexcept
	{
		return condition != ConditionType(0) ? trueValue : falseValue;
	}

	template<typename ConditionType, typename T>
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS T Select(const ConditionType condition, const T trueValue, const ZeroType) noexcept
	{
		return Select(condition, trueValue, T(Zero));
	}
	template<typename ConditionType, typename T>
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS T Select(const ConditionType condition, const ZeroType, const T falseValue) noexcept
	{
		return Select(condition, T(Zero), falseValue);
	}
	template<typename ConditionType, typename T>
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS T Select(const ConditionType condition, const T trueValue, const IdentityType) noexcept
	{
		return Select(condition, trueValue, T(Identity));
	}
	template<typename ConditionType, typename T>
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS T Select(const ConditionType condition, const IdentityType, const T falseValue) noexcept
	{
		return Select(condition, T(Identity), falseValue);
	}

	template<typename ConditionType, typename T>
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS T* Select(const ConditionType condition, T* const trueValue, T* const falseValue) noexcept
	{
		return reinterpret_cast<T*>(Select(condition, reinterpret_cast<uintptr>(trueValue), reinterpret_cast<uintptr>(falseValue)));
	}

	template<typename ConditionType, typename T>
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS T* Select(const ConditionType condition, T* const trueValue, const nullptr_type) noexcept
	{
		return reinterpret_cast<T*>(Select(condition, reinterpret_cast<uintptr>(trueValue), (uintptr) nullptr));
	}

	template<typename ConditionType, typename T>
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS T* Select(const ConditionType condition, const nullptr_type, T* const falseValue) noexcept
	{
		return reinterpret_cast<T*>(Select(condition, (uintptr)0, reinterpret_cast<uintptr>(falseValue)));
	}
}
