#pragma once

#include <Common/TypeTraits/WithoutReference.h>
#include <Common/Platform/ForceInline.h>
#include <Common/Platform/NoDebug.h>

namespace ngine
{
	template<class Type>
	[[nodiscard]] FORCE_INLINE NO_DEBUG constexpr Type&& Forward(typename TypeTraits::WithoutReference<Type>& argument) noexcept
	{
		return static_cast<Type&&>(argument);
	}

	template<class Type>
	[[nodiscard]] FORCE_INLINE NO_DEBUG constexpr Type&& Forward(typename TypeTraits::WithoutReference<Type>&& argument) noexcept
	{
		return static_cast<Type&&>(argument);
	}
}
