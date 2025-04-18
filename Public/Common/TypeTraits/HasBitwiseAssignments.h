#pragma once

#include <Common/TypeTraits/TypeConstant.h>
#include <Common/TypeTraits/Void.h>
#include <Common/TypeTraits/DeclareValue.h>

namespace ngine::TypeTraits
{
	namespace Internal
	{
		template<typename T, typename = void>
		struct HasBitwiseOrEqual : FalseType
		{
		};
		template<typename T>
		struct HasBitwiseOrEqual<T, Void<decltype(DeclareValue<T&>() |= DeclareValue<T>())>> : TrueType
		{
		};

		template<typename T, typename = void>
		struct HasBitwiseAndEqual : FalseType
		{
		};
		template<typename T>
		struct HasBitwiseAndEqual<T, Void<decltype(DeclareValue<T&>() &= DeclareValue<T>())>> : TrueType
		{
		};

		template<typename T, typename = void>
		struct HasBitwiseXorEqual : FalseType
		{
		};
		template<typename T>
		struct HasBitwiseXorEqual<T, Void<decltype(DeclareValue<T&>() ^= DeclareValue<T>())>> : TrueType
		{
		};
	}
	template<typename T>
	inline static constexpr bool HasBitwiseOrEqual = Internal::HasBitwiseOrEqual<T>::Value;
	template<typename T>
	inline static constexpr bool HasBitwiseAndEqual = Internal::HasBitwiseAndEqual<T>::Value;
	template<typename T>
	inline static constexpr bool HasBitwiseXorEqual = Internal::HasBitwiseXorEqual<T>::Value;
}
