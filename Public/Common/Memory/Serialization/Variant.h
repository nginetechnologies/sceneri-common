#pragma once

#include "../Variant.h"

#include <Common/Serialization/Reader.h>
#include <Common/Serialization/Writer.h>

#include <Common/Reflection/Registry.h>

namespace ngine
{
	namespace Internal
	{
		template<typename VariantType, typename... ArgumentTypes>
		struct TryAssignAnyToVariant;

		template<typename VariantType>
		struct TryAssignAnyToVariant<VariantType>
		{
			static bool TryAssign(VariantType&, Any&&)
			{
				return false;
			}
		};

		template<typename VariantType, typename FirstArgumentType, typename... ArgumentTypes>
		struct TryAssignAnyToVariant<VariantType, FirstArgumentType, ArgumentTypes...>
		{
			static bool TryAssign(VariantType& variant, Any&& value)
			{
				if (const Optional<FirstArgumentType*> pValue = value.Get<FirstArgumentType>())
				{
					variant = Move(*pValue);
					return true;
				}
				else
				{
					return TryAssignAnyToVariant<VariantType, ArgumentTypes...>::TryAssign(variant, Forward<Any>(value));
				}
			}
		};
	}

	template<typename... ArgumentTypes>
	inline bool Serialize(Variant<ArgumentTypes...>& value, const Serialization::Reader reader)
	{
		if (Optional<Any> anyValue = reader.ReadInPlace<Any>())
		{
			return Internal::TryAssignAnyToVariant<Variant<ArgumentTypes...>, ArgumentTypes...>::TryAssign(value, Move(*anyValue));
		}
		return false;
	}

	template<typename... ArgumentTypes>
	inline bool Serialize(const Variant<ArgumentTypes...>& value, Serialization::Writer writer)
	{
		return writer.SerializeInPlace(value.Get());
	}
}
