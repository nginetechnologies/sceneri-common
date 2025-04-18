#pragma once

#include "Common.h"
#include <Common/TypeTraits/IsConst.h>
#include <Common/TypeTraits/IsSigned.h>
#include <Common/TypeTraits/IsIntegral.h>
#include <Common/Memory/Forward.h>
#include <Common/Memory/Optional.h>
#include <Common/Math/NumericLimits.h>
#include <Common/Platform/StaticUnreachable.h>
#include <Common/Platform/LifetimeBound.h>

namespace ngine::Serialization
{
	struct Object;
	struct Array;

	// TODO: Rename to Value
	struct TValue
	{
		TValue() = default;
		TValue(const Value& value, Document& document)
			: m_value(value, document.GetAllocator())
		{
		}
		TValue(Value&& value)
			: m_value(Forward<Value>(value))
		{
		}
		TValue(const rapidjson::Type type)
			: m_value(type)
		{
		}
		TValue& operator=(Value&& value)
		{
			m_value = Forward<Value>(value);
			return *this;
		}

		template<typename Type>
		[[nodiscard]] static auto& GetFromReference(Type& value LIFETIME_BOUND)
		{
			if constexpr (TypeTraits::IsConst<Type>)
			{
				return *reinterpret_cast<const TValue*>(&value);
			}
			else
			{
				return *reinterpret_cast<TValue*>(&value);
			}
		}

		[[nodiscard]] operator Value&() LIFETIME_BOUND
		{
			return m_value;
		}

		[[nodiscard]] operator const Value &() const LIFETIME_BOUND
		{
			return m_value;
		}

		[[nodiscard]] Value& GetValue() LIFETIME_BOUND
		{
			return m_value;
		}

		[[nodiscard]] const Value& GetValue() const LIFETIME_BOUND
		{
			return m_value;
		}

		[[nodiscard]] bool IsObject() const
		{
			return m_value.IsObject();
		}
		[[nodiscard]] bool IsString() const
		{
			return m_value.IsString();
		}
		[[nodiscard]] bool IsArray() const
		{
			return m_value.IsArray();
		}
		[[nodiscard]] bool IsNumber() const
		{
			return m_value.IsNumber();
		}
		[[nodiscard]] bool IsDouble() const
		{
			return m_value.IsDouble();
		}
		[[nodiscard]] bool IsBool() const
		{
			return m_value.IsBool();
		}
		[[nodiscard]] bool IsNull() const
		{
			return m_value.IsNull();
		}
		[[nodiscard]] bool IsValid() const
		{
			return !m_value.IsNull();
		}

		[[nodiscard]] Optional<Object*> AsObject();
		[[nodiscard]] Optional<const Object*> AsObject() const;
		[[nodiscard]] Optional<Array*> AsArray();
		[[nodiscard]] Optional<const Array*> AsArray() const;

		template<typename StringType, typename WriterType>
		StringType SaveToBufferGeneric() const
		{
			using StringBufferType = rapidjson::GenericStringBuffer<rapidjson::UTF8<>, Internal::RapidJsonAllocator>;
			StringBufferType buffer;
			WriterType jsonWriter(buffer);
			if (UNLIKELY(!m_value.Accept(jsonWriter)))
			{
				return {};
			}

			return StringType{buffer.GetString(), (uint32)buffer.GetSize()};
		}

		template<typename StringType>
		StringType SaveToBuffer() const
		{
			using StringBufferType = rapidjson::GenericStringBuffer<rapidjson::UTF8<>, Internal::RapidJsonAllocator>;
			using WriterType = rapidjson::Writer<StringBufferType, rapidjson::UTF8<>, rapidjson::UTF8<>, Internal::RapidJsonAllocator>;
			return SaveToBufferGeneric<StringType, WriterType>();
		}

		template<typename StringType>
		StringType SaveToReadableBuffer() const
		{
			using StringBufferType = rapidjson::GenericStringBuffer<rapidjson::UTF8<>, Internal::RapidJsonAllocator>;
			using PrettyWriterType =
				rapidjson::PrettyWriter<StringBufferType, rapidjson::UTF8<>, rapidjson::UTF8<>, Internal::RapidJsonAllocator>;
			return SaveToBufferGeneric<StringType, PrettyWriterType>();
		}

		template<typename Type>
		[[nodiscard]] Type GetPrimitiveValue() const
		{
			if constexpr (TypeTraits::IsSame<Type, float>)
			{
				Assert(m_value.IsNumber());
				return m_value.GetFloat();
			}
			else if constexpr (TypeTraits::IsSame<Type, double>)
			{
				Assert(m_value.IsNumber());
				return m_value.GetDouble();
			}
			else if constexpr (TypeTraits::IsSame<Type, bool>)
			{
				Assert(m_value.IsBool());
				return m_value.GetBool();
			}
			else if constexpr (TypeTraits::IsIntegral<Type>)
			{
				if constexpr (TypeTraits::IsSigned<Type>)
				{
					if constexpr (TypeTraits::IsSame<Type, signed long>)
					{
						Assert(m_value.IsNumber());
						return (signed long)m_value.GetInt64();
					}
					else if constexpr (TypeTraits::IsSame<Type, signed long long>)
					{
						Assert(m_value.IsNumber());
						return (signed long long)m_value.GetInt64();
					}
					else
					{
						Assert(m_value.GetInt() >= Math::NumericLimits<Type>::Min);
						Assert(m_value.GetInt() <= Math::NumericLimits<Type>::Max);
						return static_cast<Type>(m_value.GetInt());
					}
				}
				else
				{
					if constexpr (TypeTraits::IsSame<Type, unsigned long>)
					{
						Assert(m_value.IsNumber());
						return (unsigned long)m_value.GetUint64();
					}
					else if constexpr (TypeTraits::IsSame<Type, unsigned long long>)
					{
						Assert(m_value.IsNumber());
						return (unsigned long long)m_value.GetUint64();
					}
					else
					{
						Assert(m_value.IsNumber());
						Assert(m_value.GetUint() <= Math::NumericLimits<Type>::Max);
						return static_cast<Type>(m_value.GetUint());
					}
				}
			}
			else
			{
				static_unreachable("Not implemented for type");
			}
		}
	protected:
		Value m_value;
	};
}
