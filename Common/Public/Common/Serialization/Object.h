#pragma once

#include "Value.h"
#include <Common/Memory/Containers/StringView.h>
#include <Common/Memory/Optional.h>
#include <Common/Algorithms/Sort.h>

namespace ngine::Serialization
{
	struct Object : public TValue
	{
		using BaseType = TValue;

		Object(Value&& value)
			: BaseType(Forward<Value>(value))
		{
			Assert(value.IsObject());
		}
		Object(const Value& value, Document& document)
			: BaseType(value, document)
		{
			Assert(value.IsObject());
		}
		Object()
			: BaseType(rapidjson::kObjectType)
		{
		}
		/*Object(Memory::ReserveType, const uint32 capacity, Document& document)
		  : Object()
		{
		  //m_value.Reserve(capacity, document.GetAllocator());
		}*/

		template<typename MemberIteratorType, typename ValueType, typename MemberType>
		struct TIterator
		{
			TIterator() = default;
			TIterator(const MemberIteratorType pValue)
				: m_pValue(pValue)
			{
			}

			[[nodiscard]] bool operator==(const TIterator other) const
			{
				return m_pValue == other.m_pValue;
			}
			[[nodiscard]] bool operator!=(const TIterator other) const
			{
				return m_pValue != other.m_pValue;
			}
			[[nodiscard]] MemberType operator*() const
			{
				auto& member = *m_pValue;
				return MemberType{
					ConstStringView(member.name.GetString(), static_cast<uint32>(member.name.GetStringLength())),
					TValue::GetFromReference<ValueType>(member.value)
				};
			}
			TIterator operator++()
			{
				return TIterator{m_pValue++};
			}
			TIterator operator++(int)
			{
				return TIterator{++m_pValue};
			}

			MemberIteratorType m_pValue;
		};

		template<typename BaseValueType>
		struct TMember
		{
			ConstStringView name;
			BaseValueType& value;
		};
		using Member = TMember<TValue>;
		using ConstMember = TMember<const TValue>;

		using Iterator = TIterator<Value::MemberIterator, Value, TMember<TValue>>;
		using ConstIterator = TIterator<Value::ConstMemberIterator, const Value, TMember<const TValue>>;

		[[nodiscard]] Iterator begin()
		{
			return Iterator{m_value.MemberBegin()};
		}
		[[nodiscard]] ConstIterator begin() const
		{
			return ConstIterator{m_value.MemberBegin()};
		}
		[[nodiscard]] Iterator end()
		{
			return Iterator{m_value.MemberEnd()};
		}
		[[nodiscard]] ConstIterator end() const
		{
			return ConstIterator{m_value.MemberEnd()};
		}

		TValue& AddMember(const ConstStringView memberName, Value&& value, Document& document)
		{
			Serialization::Value::Member& newValue = BaseType::m_value.AddMember(
				Value(memberName.GetData(), static_cast<rapidjson::SizeType>(memberName.GetSize()), document.GetAllocator()),
				Forward<Value>(value),
				document.GetAllocator()
			);
			return TValue::GetFromReference<Value>(newValue.value);
		}

		TValue& AddMember(const ConstStringView memberName, TValue&& value, Document& document)
		{
			Serialization::Value::Member& newValue = BaseType::m_value.AddMember(
				Value(memberName.GetData(), static_cast<rapidjson::SizeType>(memberName.GetSize()), document.GetAllocator()),
				Move(value.GetValue()),
				document.GetAllocator()
			);
			return TValue::GetFromReference<Value>(newValue.value);
		}

		void Reserve(const uint32 count, Document& document)
		{
			BaseType::m_value.ReserveMembers(count, document.GetAllocator());
		}

		template<typename Type>
		[[nodiscard]] static auto& GetFromReference(Type& value)
		{
			Assert(value.IsObject());
			if constexpr (TypeTraits::IsConst<Type>)
			{
				return *reinterpret_cast<const Object*>(&value);
			}
			else
			{
				return *reinterpret_cast<Object*>(&value);
			}
		}

		[[nodiscard]] Optional<TValue*> FindMember(const ConstStringView memberName)
		{
			Value::MemberIterator memberIterator =
				BaseType::m_value.FindMember(Value(memberName.GetData(), static_cast<rapidjson::SizeType>(memberName.GetSize())));
			return Optional<TValue*>(&TValue::GetFromReference<Value>(memberIterator->value), memberIterator != BaseType::m_value.MemberEnd());
		}

		bool RemoveMember(const ConstStringView memberName)
		{
			return BaseType::m_value.RemoveMember(Value(memberName.GetData(), static_cast<rapidjson::SizeType>(memberName.GetSize())));
		}

		[[nodiscard]] TValue& FindOrCreateMember(const ConstStringView memberName, TValue&& object, Document& document)
		{
			if (Optional<TValue*> memberObject = FindMember(memberName))
			{
				return *memberObject;
			}

			return AddMember(memberName, Forward<TValue>(object), document);
		}

		[[nodiscard]] bool HasMember(const ConstStringView memberName) const
		{
			return BaseType::m_value.HasMember(Value(memberName.GetData(), static_cast<rapidjson::SizeType>(memberName.GetSize())));
		}

		[[nodiscard]] Optional<const TValue*> FindMember(const ConstStringView memberName) const
		{
			Value::ConstMemberIterator memberIterator =
				BaseType::m_value.FindMember(Value(memberName.GetData(), static_cast<rapidjson::SizeType>(memberName.GetSize())));
			return Optional<const TValue*>(
				&TValue::GetFromReference<const Value>(memberIterator->value),
				memberIterator != BaseType::m_value.MemberEnd()
			);
		}

		[[nodiscard]] size GetMemberCount() const
		{
			return BaseType::m_value.MemberCount();
		}

		template<typename Comparator>
		void Sort()
		{
			Algorithms::Sort(BaseType::m_value.MemberBegin(), BaseType::m_value.MemberEnd(), Comparator());
		}

		void Sort()
		{
			struct NameComparator
			{
				bool operator()(const Value::Member& __restrict left, const Value::Member& __restrict right) const
				{
					return ConstStringView(left.name.GetString(), left.name.GetStringLength()) <
					       ConstStringView(right.name.GetString(), right.name.GetStringLength());
				}
			};
			Sort<NameComparator>();
		}

		template<typename Type>
		Type GetPrimitiveValue() const = delete;
	};

	[[nodiscard]] inline Optional<Object*> TValue::AsObject()
	{
		return {static_cast<Object&>(*this), IsObject()};
	}
	[[nodiscard]] inline Optional<const Object*> TValue::AsObject() const
	{
		return {static_cast<const Object&>(*this), IsObject()};
	}
}
