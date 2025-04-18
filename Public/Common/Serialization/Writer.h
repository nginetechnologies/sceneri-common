#pragma once

#include <Common/Serialization/SerializedData.h>
#include <Common/TypeTraits/IsEnum.h>
#include <Common/Memory/Containers/StringView.h>
#include <Common/TypeTraits/EnableIf.h>
#include <Common/TypeTraits/IsPrimitive.h>
#include <Common/Serialization/CanWrite.h>
#include <Common/Serialization/Object.h>
#include <Common/Serialization/Array.h>
#include <Common/Platform/TrivialABI.h>

namespace ngine::Serialization
{
	struct TRIVIAL_ABI Writer
	{
		Writer(Value& __restrict value, Data& serializedData)
			: m_value(TValue::GetFromReference(value))
			, m_serializedData(serializedData)
		{
		}
		Writer(Data& serializedData)
			: m_value(TValue::GetFromReference(serializedData.GetDocument()))
			, m_serializedData(serializedData)
		{
		}

		[[nodiscard]] Document& GetDocument()
		{
			return m_serializedData.GetDocument();
		}

		template<typename T, typename... Args>
		inline static constexpr bool CanWrite = Internal::CanWrite<T, Args...> || TypeTraits::IsPrimitive<T> || TypeTraits::IsEnum<T>;

		template<typename T, typename... Args>
		EnableIf<CanWrite<T, Args...>, bool> Serialize(const ConstStringView memberName, const T& element, Args&... args)
		{
			Value value;
			if (SerializeInternal(value, element, args...))
			{
				Object& object = Object::GetFromReference<Value>(m_value);
				if (Optional<TValue*> memberValue = object.FindMember(memberName))
				{
					*memberValue = Move(value);
				}
				else
				{
					object.AddMember(memberName, Move(value), GetDocument());
				}

				return true;
			}
			else
			{
				// Remove the object if it already existed
				Object& object = Object::GetFromReference<Value>(m_value);
				object.RemoveMember(memberName);
			}

			return false;
		}

		template<typename T, typename... Args>
		EnableIf<CanWrite<T, Args...>, bool>
		SerializeWithDefaultValue(const ConstStringView memberName, const T& element, const T& defaultValue, Args&... args)
		{
			if (element != defaultValue)
			{
				Value value;
				if (SerializeInternal(value, element, args...))
				{
					Object& object = Object::GetFromReference<Value>(m_value);
					if (Optional<TValue*> memberValue = object.FindMember(memberName))
					{
						*memberValue = Move(value);
					}
					else
					{
						object.AddMember(memberName, Move(value), GetDocument());
					}

					return true;
				}
			}
			else
			{
				// Remove the object if it already existed
				Object& object = Object::GetFromReference<Value>(m_value);
				object.RemoveMember(memberName);
			}

			return false;
		}

		template<typename Callback, typename IndexType, typename... Args>
		bool SerializeArrayWithCallback(const ConstStringView memberName, Callback&& callback, const IndexType count, Args&... args)
		{
			Value value;
			if (Writer(value, m_serializedData).SerializeArrayCallbackInPlace(callback, count, args...))
			{
				Object& object = Object::GetFromReference<Value>(m_value);
				if (Optional<TValue*> memberValue = object.FindMember(memberName))
				{
					*memberValue = Move(value);
				}
				else
				{
					object.AddMember(memberName, Move(value), GetDocument());
				}
				return true;
			}
			else
			{
				// Remove the object if it already existed
				Object& object = Object::GetFromReference<Value>(m_value);
				object.RemoveMember(memberName);
				return false;
			}
		}

		template<typename Callback, typename... Args>
		bool SerializeObjectWithCallback(const ConstStringView memberName, Callback&& callback, Args&... args)
		{
			Object& object = Object::GetFromReference<Value>(m_value);

			if (Optional<TValue*> memberValue = object.FindMember(memberName))
			{
				Value& value = memberValue->GetValue();
				return callback(Writer(value, m_serializedData), args...);
			}
			else
			{
				Value value(rapidjson::Type::kObjectType);
				const bool result = callback(Writer(value, m_serializedData), args...);
				if (result)
				{
					object.AddMember(memberName, Move(value), GetDocument());
				}
				else
				{
					// Remove the object if it already existed
					object.RemoveMember(memberName);
				}
				return result;
			}
		}

		TValue& AddMember(const ConstStringView memberName, Value&& value)
		{
			Object& object = Object::GetFromReference<Value>(m_value);
			return object.AddMember(memberName, Forward<Value>(value), GetDocument());
		}

		template<typename T, typename... Args>
		EnableIf<CanWrite<T, Args...>, bool> SerializeInPlace(const T& element, Args&... args)
		{
			return SerializeInternal(m_value, element, args...);
		}

		template<typename Callback, typename IndexType, typename... Args>
		bool SerializeArrayCallbackInPlace(Callback&& callback, const IndexType count, Args&... args)
		{
			ReserveArray(count);
			Array& object = Array::GetFromReference<Value>(m_value);

			Value elementValue;
			bool pushedBackElements = false;

			for (IndexType i = 0; i < count; ++i)
			{
				if (callback(Writer(elementValue, m_serializedData), i, args...))
				{
					object.PushBack(Move(elementValue), GetDocument());
					pushedBackElements = true;
				}
			}

			return pushedBackElements;
		}

		[[nodiscard]] Object& GetAsObject()
		{
			return Object::GetFromReference(m_value);
		}

		[[nodiscard]] Array& GetAsArray()
		{
			return Array::GetFromReference(m_value);
		}

		template<typename T, typename... Args>
		EnableIf<CanWrite<T, Args...>, bool> SerializeArrayElementToBack(const T& element, Args&... args)
		{
			Value elementValue;
			if (SerializeInternal(elementValue, element, args...))
			{
				Array& object = Array::GetFromReference<Value>(m_value);
				object.PushBack(Move(elementValue), GetDocument());
				return true;
			}

			return false;
		}

		void ReserveArray(const uint32 capacity)
		{
			if (m_value.IsNull())
			{
				m_value = (rapidjson::Type::kArrayType);
			}
			Array& object = Array::GetFromReference<Value>(m_value);
			object.Reserve(capacity, GetDocument());
		}

		void ReserveMembers(const uint32 capacity)
		{
			if (m_value.IsNull())
			{
				m_value = (rapidjson::Type::kObjectType);
			}
			Object& object = Object::GetFromReference<Value>(m_value);
			object.Reserve(capacity, GetDocument());
		}

		[[nodiscard]] Writer EmplaceArrayElement(TValue&& elementValue = {})
		{
			Array& object = Array::GetFromReference<Value>(m_value);
			Value& emplacedValue = object.PushBack(Move(elementValue.GetValue()), GetDocument());
			return Writer(emplacedValue, m_serializedData);
		}

		[[nodiscard]] Writer EmplaceObjectElement(const ConstStringView name, TValue&& elementValue = {})
		{
			Object& object = Object::GetFromReference<Value>(m_value);
			Value& emplacedValue = object.AddMember(name, Move(elementValue.GetValue()), GetDocument());
			return Writer(emplacedValue, m_serializedData);
		}

		[[nodiscard]] Value& GetValue()
		{
			return m_value;
		}
		[[nodiscard]] const Value& GetValue() const
		{
			return m_value;
		}
		[[nodiscard]] Data& GetData() const
		{
			return m_serializedData;
		}

		template<typename T, typename... Args>
		[[nodiscard]] inline EnableIf<CanWrite<T, Args...>, bool> SerializeInternal(Value& valueOut, const T& value, Args&... args)
		{
			if (valueOut.IsNull())
			{
				valueOut = Value(rapidjson::Type::kObjectType);
			}

			Writer writer(valueOut, m_serializedData);
			return Internal::SerializeElement(value, writer, args...);
		}

		template<typename Type, typename... Args>
		[[nodiscard]] EnableIf<!CanWrite<Type, Args...>, bool> SerializeInternal(Value&, const Type&) const
		{
			static_unreachable("Not implemented");
		}

		template<typename StringType>
		StringType SaveToBuffer() const
		{
			return m_value.SaveToBuffer<StringType>();
		}

		template<typename StringType>
		StringType SaveToReadableBuffer() const
		{
			return m_value.SaveToReadableBuffer<StringType>();
		}
	protected:
		TValue& __restrict m_value;
		Data& m_serializedData;
	};
}

#include <Common/Serialization/SerializeElement.h>
