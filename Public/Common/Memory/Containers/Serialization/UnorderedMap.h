#pragma once

#include "../UnorderedMap.h"
#include <Common/Memory/Containers/String.h>
#include <Common/Serialization/Reader.h>
#include <Common/Serialization/Writer.h>
#include <Common/Memory/Containers/Format/StringView.h>
#include <Common/TypeTraits/IsIntegral.h>

namespace ngine
{
	template<typename KeyType, typename ValueType, typename HashType, typename EqualityType, typename Callback, typename... Args>
	inline bool SerializeWithCallback(
		UnorderedMap<KeyType, ValueType, HashType, EqualityType>& map,
		const Serialization::Reader serializer,
		Callback&& callback,
		Args&... args
	)
	{
		const Serialization::Value& __restrict currentElement = serializer.GetValue();
		if (LIKELY(currentElement.IsObject()))
		{
			const Serialization::Object& currentObject = Serialization::Object::GetFromReference(currentElement);

			map.Reserve(map.GetSize() + (uint32)currentObject.GetMemberCount());

			uint32 serializedValues = 0;

			for (const Serialization::Object::ConstMember member : currentObject)
			{
				ValueType value;
				if (serializer.SerializeInternal(member.value, value, args...))
				{
					callback(map, member.name, Move(value));
					serializedValues++;
				}
			}

			return serializedValues > 0;
		}
		else
		{
			return false;
		}
	}

	template<typename ContainedKeyType, typename ContainedValueType, typename... Args>
	FORCE_INLINE EnableIf<TypeTraits::IsIntegral<ContainedKeyType>, bool>
	Serialize(UnorderedMap<ContainedKeyType, ContainedValueType>& map, const Serialization::Reader serializer, Args&... args)
	{
		return SerializeWithCallback(
			map,
			serializer,
			[](UnorderedMap<ContainedKeyType, ContainedValueType>& serializedMap, const ConstStringView name, ContainedValueType&& value)
			{
				serializedMap.Emplace(name.ToIntegral<ContainedKeyType>(), Move(value));
			},
			args...
		);
	}

	template<typename ContainedValueType, typename... Args>
	FORCE_INLINE bool
	Serialize(UnorderedMap<String, ContainedValueType, String::Hash>& map, const Serialization::Reader serializer, Args&... args)
	{
		return SerializeWithCallback(
			map,
			serializer,
			[](UnorderedMap<String, ContainedValueType, String::Hash>& serializedMap, const ConstStringView name, ContainedValueType&& value)
			{
				serializedMap.Emplace(String(name), Move(value));
			},
			args...
		);
	}

	template<typename ContainedValueType, typename... Args>
	FORCE_INLINE bool Serialize(
		UnorderedMap<UnicodeString, ContainedValueType, UnicodeString::Hash>& map, const Serialization::Reader serializer, Args&... args
	)
	{
		return SerializeWithCallback(
			map,
			serializer,
			[](
				UnorderedMap<UnicodeString, ContainedValueType, UnicodeString::Hash>& serializedMap,
				const ConstStringView name,
				ContainedValueType&& value
			)
			{
				serializedMap.Emplace(UnicodeString(name), Move(value));
			},
			args...
		);
	}

	template<typename ContainedValueType, typename... Args>
	FORCE_INLINE bool
	Serialize(UnorderedMap<Guid, ContainedValueType, Guid::Hash>& map, const Serialization::Reader serializer, Args&... args)
	{
		return SerializeWithCallback(
			map,
			serializer,
			[](UnorderedMap<Guid, ContainedValueType, Guid::Hash>& serializedMap, const ConstStringView name, ContainedValueType&& value)
			{
				serializedMap.Emplace(Guid(name), Move(value));
			},
			args...
		);
	}

	template<
		typename CallbackResultType,
		typename KeyType,
		typename ValueType,
		typename HashType,
		typename EqualityType,
		typename Callback,
		typename... Args>
	inline bool SerializeWithCallback(
		const UnorderedMap<KeyType, ValueType, HashType, EqualityType>& map,
		Serialization::Writer serializer,
		Callback&& callback,
		Args&... args
	)
	{
		if (map.IsEmpty())
		{
			return false;
		}

		Serialization::Value& __restrict currentElement = serializer.GetValue();
		if (!currentElement.IsObject())
		{
			currentElement = Serialization::Value(rapidjson::Type::kObjectType);
		}

		currentElement.ReserveMembers(currentElement.MemberCount() + map.GetSize(), serializer.GetDocument().GetAllocator());

		uint32 numSerializedElements = 0;

		for (auto it = map.begin(), endIt = map.end(); it != endIt; ++it)
		{
			Serialization::Value elementValue;
			const ValueType& value = it->second;
			if (serializer.SerializeInternal(elementValue, value, args...))
			{
				const CallbackResultType key = callback(it->first);
				currentElement.AddMember(
					Serialization::Value(key.GetData(), key.GetSize(), serializer.GetDocument().GetAllocator()),
					Move(elementValue),
					serializer.GetDocument().GetAllocator()
				);

				numSerializedElements++;
			}
		}

		Serialization::Object::GetFromReference(currentElement).Sort();

		return numSerializedElements > 0;
	}

	template<typename ContainedKeyType, typename ContainedValueType, typename... Args>
	FORCE_INLINE EnableIf<TypeTraits::IsIntegral<ContainedKeyType>, bool>
	Serialize(const UnorderedMap<ContainedKeyType, ContainedValueType>& map, Serialization::Writer serializer, Args&... args)
	{
		return SerializeWithCallback<FlatString<37>>(
			map,
			serializer,
			[](const ContainedKeyType key) -> const FlatString<37>
			{
				FlatString<37> result;
				result.Format("{}", key);
				return result;
			},
			args...
		);
	}

	template<typename ContainedValueType, typename... Args>
	FORCE_INLINE bool
	Serialize(const UnorderedMap<String, ContainedValueType, String::Hash>& map, Serialization::Writer serializer, Args&... args)
	{
		return SerializeWithCallback<const String&>(
			map,
			serializer,
			[](const String& key) -> const String&
			{
				return key;
			},
			args...
		);
	}

	template<typename ContainedValueType, typename... Args>
	FORCE_INLINE bool Serialize(
		const UnorderedMap<UnicodeString, ContainedValueType, UnicodeString::Hash>& map, Serialization::Writer serializer, Args&... args
	)
	{
		return SerializeWithCallback<String>(
			map,
			serializer,
			[](const UnicodeString& key) -> String
			{
				return String(key.GetView());
			},
			args...
		);
	}

	template<typename ContainedValueType, typename... Args>
	FORCE_INLINE bool
	Serialize(const UnorderedMap<Guid, ContainedValueType, Guid::Hash>& map, Serialization::Writer serializer, Args&... args)
	{
		return SerializeWithCallback<FlatString<37>>(
			map,
			serializer,
			[](const Guid& key) -> FlatString<37>
			{
				return key.ToString();
			},
			args...
		);
	}
}
