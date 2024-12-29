#pragma once

#include <Common/Platform/ForceInline.h>
#include <Common/TypeTraits/IsMoveConstructible.h>
#include <Common/TypeTraits/IsMoveAssignable.h>
#include <Common/Memory/Move.h>

namespace ngine
{
	template<typename Type>
	FORCE_INLINE constexpr void Swap(Type& left, Type& right) noexcept
	{
		Type tmp = Move(left);
		left = Move(right);
		right = Move(tmp);
	}
}
