#pragma once

#include "Value.h"
#include <Common/Memory/Move.h>
#include <Common/Algorithms/Sort.h>

namespace ngine::Serialization
{
	struct Array : public TValue
	{
		using BaseType = TValue;

		Array(Value&& value)
			: BaseType(Forward<Value>(value))
		{
			Assert(value.IsArray());
		}
		Array(const Value& value, Document& document)
			: BaseType(value, document)
		{
			Assert(value.IsArray());
		}

		Array()
			: BaseType(rapidjson::kArrayType)
		{
		}

		Array(Memory::ReserveType, const uint32 capacity, Document& document)
			: BaseType(rapidjson::kArrayType)
		{
			BaseType::m_value.Reserve(capacity, document.GetAllocator());
		}

		void Reserve(const uint32 capacity, Document& document)
		{
			BaseType::m_value.Reserve(capacity, document.GetAllocator());
		}

		Serialization::Value& PushBack(Value&& value, Document& document)
		{
			return BaseType::m_value.PushBack(Forward<Value>(value), document.GetAllocator());
		}

		Serialization::Value& PushBack(TValue&& value, Document& document)
		{
			Value& rawValue = value;
			return BaseType::m_value.PushBack(Move(rawValue), document.GetAllocator());
		}

		template<typename Comparator>
		void Sort()
		{
			Algorithms::Sort(BaseType::m_value.Begin(), BaseType::m_value.End(), Comparator());
		}

		void Sort()
		{
			struct Comparator
			{
				bool operator()(const Value& __restrict left, const Value& __restrict right) const
				{
					Assert(left.GetType() == right.GetType());
					if (left.IsString())
					{
						return ConstStringView(left.GetString(), left.GetStringLength()) < ConstStringView(right.GetString(), right.GetStringLength());
					}
					else if (left.IsDouble())
					{
						return left.GetDouble() < right.GetDouble();
					}
					else if (left.IsNumber())
					{
						return left.GetUint64() < right.GetUint64();
					}
					else
					{
						Assert(false, "Not supported!");
						return false;
					}
				}
			};
			Sort<Comparator>();
		}

		[[nodiscard]] uint32 GetSize() const
		{
			return (uint32)BaseType::m_value.Size();
		}

		[[nodiscard]] bool IsEmpty()
		{
			return BaseType::m_value.Empty();
		}
		[[nodiscard]] bool HasElements()
		{
			return !BaseType::m_value.Empty();
		}

		template<typename ValueType, typename BaseValueType>
		struct TIterator
		{
			[[nodiscard]] bool operator==(const TIterator other) const
			{
				return m_pValue == other.m_pValue;
			}
			[[nodiscard]] bool operator!=(const TIterator other) const
			{
				return m_pValue != other.m_pValue;
			}
			[[nodiscard]] BaseValueType& operator*() const
			{
				return BaseValueType::GetFromReference(*m_pValue);
			}
			TIterator operator++()
			{
				return TIterator{m_pValue++};
			}
			TIterator operator++(int)
			{
				return TIterator{++m_pValue};
			}

			ValueType* m_pValue;
		};
		using Iterator = TIterator<Value, TValue>;
		using ConstIterator = TIterator<const Value, const TValue>;

		[[nodiscard]] Iterator begin()
		{
			return Iterator{m_value.Begin()};
		}
		[[nodiscard]] ConstIterator begin() const
		{
			return ConstIterator{m_value.Begin()};
		}
		[[nodiscard]] Iterator end()
		{
			return Iterator{m_value.End()};
		}
		[[nodiscard]] ConstIterator end() const
		{
			return ConstIterator{m_value.End()};
		}

		template<typename Callback>
		void RemoveAllOccurrencesPredicate(Callback callback)
		{
			for (Value *it = BaseType::m_value.Begin(), *end = BaseType::m_value.End(); it != end;)
			{
				if (callback(BaseType::GetFromReference(*it)))
				{
					end--;
					BaseType::m_value.Erase(it);
				}
				else
				{
					++it;
				}
			}
		}

		template<typename Callback>
		bool RemoveFirstOccurrencePredicate(Callback callback)
		{
			for (Value *it = BaseType::m_value.Begin(), *end = BaseType::m_value.End(); it != end; ++it)
			{
				if (callback(BaseType::GetFromReference(*it)))
				{
					BaseType::m_value.Erase(it);
					return true;
				}
			}
			return false;
		}

		template<typename Type>
		[[nodiscard]] static auto& GetFromReference(Type& value)
		{
			Assert(value.IsArray());
			if constexpr (TypeTraits::IsConst<Type>)
			{
				return *reinterpret_cast<const Array*>(&value);
			}
			else
			{
				return *reinterpret_cast<Array*>(&value);
			}
		}

		template<typename Type>
		Type GetPrimitiveValue() const = delete;
	};

	[[nodiscard]] inline Optional<Array*> TValue::AsArray()
	{
		return {static_cast<Array&>(*this), IsArray()};
	}
	[[nodiscard]] inline Optional<const Array*> TValue::AsArray() const
	{
		return {static_cast<const Array&>(*this), IsArray()};
	}
}
