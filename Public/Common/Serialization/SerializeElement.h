#pragma once

#include <Common/Serialization/ForwardDeclarations/Writer.h>
#include <Common/Serialization/CanWrite.h>

namespace ngine::Serialization
{
	namespace Internal
	{
		template<typename T, typename... Args>
		inline EnableIf<HasGlobalWrite<T, Args...> && !TypeTraits::IsPointer<T>, bool>
		SerializeElement(const T& element, Writer writer, Args&... args)
		{
			return Serialize(element, writer, args...);
		}

		template<typename T, typename... Args>
		inline EnableIf<HasGlobalWrite<TypeTraits::WithoutPointer<T>, Args...> && TypeTraits::IsPointer<T>, bool>
		SerializeElement(const T pElement, Writer writer, Args&... args)
		{
			if (pElement == nullptr)
			{
				return false;
			}

			return Serialize(*pElement, writer, args...);
		}

		template<typename T, typename... Args>
		inline EnableIf<HasMemberWrite<T, Args...> && !TypeTraits::IsPointer<T>, bool>
		SerializeElement(const T& element, Writer writer, Args&... args)
		{
			return element.Serialize(writer, args...);
		}

		template<typename T, typename... Args>
		inline EnableIf<HasMemberWrite<TypeTraits::WithoutPointer<T>, Args...> && TypeTraits::IsPointer<T>, bool>
		SerializeElement(const T pElement, Writer writer, Args&... args)
		{
			if (pElement == nullptr)
			{
				return false;
			}

			return pElement->Serialize(writer, args...);
		}

		namespace Internal
		{
			inline void SerializePrimitive(Value& valueOut, const float& value)
			{
				valueOut = Value(static_cast<double>(value));
			}

			inline void SerializePrimitive(Value& valueOut, const double& value)
			{
				valueOut = Value(value);
			}

			inline void SerializePrimitive(Value& valueOut, const int8& value)
			{
				valueOut = Value(value);
			}

			inline void SerializePrimitive(Value& valueOut, const uint8& value)
			{
				valueOut = Value(value);
			}

			inline void SerializePrimitive(Value& valueOut, const int16& value)
			{
				valueOut = Value(value);
			}

			inline void SerializePrimitive(Value& valueOut, const uint16& value)
			{
				valueOut = Value(value);
			}

			inline void SerializePrimitive(Value& valueOut, const int32& value)
			{
				valueOut = Value(value);
			}

			inline void SerializePrimitive(Value& valueOut, const uint32& value)
			{
				valueOut = Value(value);
			}

			inline void SerializePrimitive(Value& valueOut, const signed long& value)
			{
				valueOut = Value((int64_t)value);
			}

			inline void SerializePrimitive(Value& valueOut, const unsigned long& value)
			{
				valueOut = Value((uint64_t)value);
			}

			inline void SerializePrimitive(Value& valueOut, const signed long long& value)
			{
				valueOut = Value((int64_t)value);
			}

			inline void SerializePrimitive(Value& valueOut, const unsigned long long& value)
			{
				valueOut = Value((uint64_t)value);
			}

			inline void SerializePrimitive(Value& valueOut, const bool& value)
			{
				valueOut = Value(value);
			}

			inline void SerializePrimitive(Value& valueOut, const nullptr_type&)
			{
				valueOut = Value(rapidjson::Type::kNullType);
			}
		}

		template<typename T>
		inline EnableIf<TypeTraits::IsPrimitive<T> && !TypeTraits::IsPointer<T>, bool> SerializeElement(const T& element, Writer writer)
		{
			Internal::SerializePrimitive(writer.GetValue(), element);
			return true;
		}

		template<typename T>
		inline EnableIf<TypeTraits::IsEnum<T>, bool> SerializeElement(const T& element, Writer writer)
		{
			writer.GetValue() = Value(static_cast<UNDERLYING_TYPE(T)>(element));
			return true;
		}
	}
}
