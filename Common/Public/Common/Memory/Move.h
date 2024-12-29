#pragma once

#include <Common/TypeTraits/WithoutReference.h>
#include <Common/TypeTraits/IsConst.h>
#include <Common/TypeTraits/EnableIf.h>
#include <Common/TypeTraits/WithoutReference.h>
#include <Common/Platform/ForceInline.h>
#include <Common/Platform/NoDebug.h>

namespace ngine
{
	template<typename Type, typename = EnableIf<!TypeTraits::IsConst<TypeTraits::WithoutReference<Type>>>>
	[[nodiscard]] FORCE_INLINE NO_DEBUG constexpr typename TypeTraits::WithoutReference<Type>&& Move(Type&& argument) noexcept
	{ // forward _Arg as movable
		return (static_cast<typename TypeTraits::WithoutReference<Type>&&>(argument));
	}
}
