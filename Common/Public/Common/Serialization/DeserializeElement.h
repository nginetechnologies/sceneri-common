#pragma once

#include <Common/Serialization/ForwardDeclarations/Writer.h>
#include <Common/Serialization/CanRead.h>

namespace ngine::Serialization::Internal
{
	template<typename T, typename... Args>
	inline EnableIf<HasGlobalRead<T, Args...> && !TypeTraits::IsPointer<T>, bool>
	DeserializeElement(T& element, const Reader reader, Args&... args)
	{
		return Serialize(element, reader, args...);
	}

	template<typename T, typename... Args>
	inline EnableIf<HasGlobalRead<TypeTraits::WithoutPointer<T>, Args...> && TypeTraits::IsPointer<T>, bool>
	DeserializeElement(T pElement, const Reader reader, Args&... args)
	{
		if (pElement == nullptr)
		{
			return false;
		}

		return Serialize(*pElement, reader, args...);
	}

	template<typename T, typename... Args>
	inline EnableIf<HasMemberRead<T, Args...> && !TypeTraits::IsPointer<T>, bool>
	DeserializeElement(T& element, const Reader reader, Args&... args)
	{
		return element.Serialize(reader, args...);
	}

	template<typename T, typename... Args>
	inline EnableIf<HasMemberRead<TypeTraits::WithoutPointer<T>, Args...> && TypeTraits::IsPointer<T>, bool>
	DeserializeElement(T pElement, const Reader reader, Args&... args)
	{
		if (pElement == nullptr)
		{
			return false;
		}

		return pElement->Serialize(reader, args...);
	}

	template<typename T>
	inline EnableIf<TypeTraits::IsPrimitive<T> && !TypeTraits::IsPointer<T> && !TypeTraits::IsSame<T, nullptr_type>, bool>
	DeserializeElement(T& element, const Reader reader)
	{
		element = reader.GetValue().GetPrimitiveValue<T>();
		return true;
	}

	template<typename T>
	inline EnableIf<TypeTraits::IsEnum<T>, bool> DeserializeElement(T& element, const Reader reader)
	{
		using UnderlyingType = UNDERLYING_TYPE(T);
		UnderlyingType& elementValue = reinterpret_cast<UnderlyingType&>(element);
		elementValue = reader.GetValue().GetPrimitiveValue<UnderlyingType>();
		return true;
	}
}
