#pragma once

#include <Common/TypeTraits/TypeConstant.h>
#include <Common/TypeTraits/Void.h>

namespace ngine
{
	namespace Internal
	{
		template<typename, typename = void>
		struct TIsTransparent : TypeTraits::FalseType
		{
		};
		template<typename T>
		struct TIsTransparent<T, TypeTraits::Void<typename T::is_transparent>> : TypeTraits::TrueType
		{
		};
		template<typename T>
		inline static constexpr bool IsTransparent = TIsTransparent<T>::Value;
	}
}
