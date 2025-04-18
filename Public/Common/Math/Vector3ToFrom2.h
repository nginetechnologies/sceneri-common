#pragma once

#include "Vector2.h"
#include "Vector3.h"

namespace ngine::Math
{
	template<typename T>
	FORCE_INLINE PURE_NOSTATICS Math::TVector2<T> ToVector2(const Math::TVector3<T> vec3)
	{
		return {vec3.x, vec3.y};
	}

	template<typename T>
	FORCE_INLINE PURE_NOSTATICS Math::TVector3<T> ToVector3(const Math::TVector2<T> vec2)
	{
		return {vec2.x, vec2.y, T(0)};
	}
}
