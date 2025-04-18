#pragma once

#include <Common/Serialization/ForwardDeclarations/Reader.h>
#include <Common/Serialization/SerializedData.h>
#include <Common/IO/ForwardDeclarations/ZeroTerminatedPathView.h>
#include <Common/TypeTraits/EnableIf.h>
#include <Common/TypeTraits/IsPrimitive.h>
#include <Common/Memory/ReferenceWrapper.h>
#include <Common/Memory/Optional.h>
#include <Common/Memory/Pair.h>
#include <Common/Serialization/CanRead.h>
#include <Common/Serialization/Object.h>
#include <Common/Serialization/Array.h>
#include <Common/Platform/TrivialABI.h>

namespace ngine::Serialization
{
	template<typename Type>
	using Member = Pair<ConstStringView, Type>;

	template<typename Type>
	struct ArrayView;

	template<>
	struct ArrayView<Reader>
	{
		ArrayView(const TValue& value, const Data& data)
			: m_value(value)
			, m_serializedData(data)
		{
		}

		struct Iterator : public Array::ConstIterator
		{
			using BaseType = Array::ConstIterator;

			Iterator(Array::ConstIterator iterator, const Data& serializedData)
				: BaseType(iterator)
				, m_serializedData(serializedData)
			{
			}
			using BaseType::BaseType;

			[[nodiscard]] Reader operator*() const;

			using BaseType::operator==;
			using BaseType::operator!=;
			using BaseType::operator++;
		protected:
			ReferenceWrapper<const Data> m_serializedData;
		};

		[[nodiscard]] Iterator begin() const
		{
			Assert(m_value->IsArray());
			if (LIKELY(m_value->IsArray()))
			{
				return {Array::GetFromReference<const TValue>(*m_value).begin(), m_serializedData};
			}
			else
			{
				return {Array::ConstIterator{nullptr}, m_serializedData};
			}
		}
		[[nodiscard]] Iterator end() const
		{
			Assert(m_value->IsArray());
			if (LIKELY(m_value->IsArray()))
			{
				return {Array::GetFromReference(*m_value).end(), m_serializedData};
			}
			else
			{
				return {Array::ConstIterator{nullptr}, m_serializedData};
			}
		}
	protected:
		ReferenceWrapper<const TValue> m_value;
		ReferenceWrapper<const Data> m_serializedData;
	};

	template<typename Type>
	struct ArrayView : public ArrayView<Reader>
	{
		using BaseType = ArrayView<Reader>;
		using BaseType::BaseType;

		struct Iterator : public BaseType::Iterator
		{
			using BaseType = ArrayView<Reader>::Iterator;
			using BaseType::BaseType;

			[[nodiscard]] Optional<Type> operator*() const;
		};

		[[nodiscard]] Iterator begin() const
		{
			return Iterator{BaseType::begin(), m_serializedData};
		}
		[[nodiscard]] Iterator end() const
		{
			return Iterator{BaseType::end(), m_serializedData};
		}
	};

	template<typename Type>
	struct ObjectView;

	template<>
	struct ObjectView<Reader>
	{
		ObjectView(const TValue& value, const Data& data)
			: m_value(value)
			, m_serializedData(data)
		{
		}

		using Pair = Member<Reader>;

		struct Iterator : public Object::ConstIterator
		{
			using BaseType = Object::ConstIterator;

			Iterator(Object::ConstIterator iterator, const Data& serializedData)
				: BaseType(iterator)
				, m_serializedData(serializedData)
			{
			}

			[[nodiscard]] Pair operator*() const;

			using BaseType::operator==;
			using BaseType::operator!=;
			using BaseType::operator++;
		protected:
			ReferenceWrapper<const Data> m_serializedData;
		};

		[[nodiscard]] Iterator begin() const
		{
			Assert(m_value->IsObject());
			if (LIKELY(m_value->IsObject()))
			{
				const Object& object = Object::GetFromReference(*m_value);
				return {object.begin(), m_serializedData};
			}
			else
			{
				return {Object::ConstIterator{}, m_serializedData};
			}
		}
		[[nodiscard]] Iterator end() const
		{
			Assert(m_value->IsObject());
			if (LIKELY(m_value->IsObject()))
			{
				const Object& object = Object::GetFromReference(*m_value);
				return {object.end(), m_serializedData};
			}
			else
			{
				return {Object::ConstIterator{}, m_serializedData};
			}
		}
	protected:
		ReferenceWrapper<const TValue> m_value;
		ReferenceWrapper<const Data> m_serializedData;
	};

	template<typename Type>
	struct ObjectView : public ObjectView<Reader>
	{
		using BaseType = ObjectView<Reader>;
		using BaseType::BaseType;

		using Pair = Member<Optional<Type>>;

		struct Iterator : public BaseType::Iterator
		{
			using BaseType = ObjectView<Reader>::Iterator;
			using BaseType::BaseType;

			[[nodiscard]] Pair operator*() const;
		};

		[[nodiscard]] Iterator begin() const
		{
			return Iterator{BaseType::begin(), m_serializedData};
		}
		[[nodiscard]] Iterator end() const
		{
			return Iterator{BaseType::end(), m_serializedData};
		}
	};

	struct TRIVIAL_ABI Reader
	{
		Reader(const Value& __restrict value, const Data& serializedData)
			: m_value(TValue::GetFromReference(value))
			, m_serializedData(serializedData)
		{
		}
		Reader(const Value&) = delete;
		Reader(const Data& serializedData LIFETIME_BOUND)
			: m_value(TValue::GetFromReference(serializedData.GetDocument()))
			, m_serializedData(serializedData)
		{
		}

		[[nodiscard]] const Document& GetDocument() const
		{
			return m_serializedData->GetDocument();
		}

		template<typename T, typename... Args>
		inline static constexpr bool CanRead = Internal::CanRead<T, Args&...> || TypeTraits::IsPrimitive<T> || TypeTraits::IsEnum<T>;

		template<typename T, typename... Args>
		[[nodiscard]] EnableIf<CanRead<T, Args&...>, Optional<T>> Read(const ConstStringView memberName, Args&... args) const
		{
			Optional<T> value;
			[[maybe_unused]] const bool read = Serialize(memberName, value, args...);
			return value;
		}

		template<typename T, typename... Args>
		[[nodiscard]] EnableIf<CanRead<T, Args&...>, T>
		ReadWithDefaultValue(const ConstStringView memberName, T&& defaultValue, Args&... args) const
		{
			T value = Forward<T>(defaultValue);
			Serialize(memberName, value, args...);
			return value;
		}

		template<typename T, typename... Args>
		EnableIf<CanRead<T, Args&...>, Optional<T>> ReadInPlace(Args&... args) const
		{
			Optional<T> value;
			[[maybe_unused]] const bool read = SerializeInternal<Optional<T>, Args...>(m_value, value, args...);
			return value;
		}

		template<typename T, typename... Args>
		EnableIf<CanRead<T, Args&...>, T> ReadInPlaceWithDefaultValue(T&& defaultValue, Args&... args) const
		{
			T value = Forward<T>(defaultValue);
			[[maybe_unused]] const bool read = SerializeInternal<T, Args...>(m_value, value, args...);
			return value;
		}

		template<typename T, typename... Args>
		inline EnableIf<CanRead<T, Args&...>, bool> Serialize(const ConstStringView memberName, T& element, Args&... args) const
		{
			Assert(m_value->IsObject());
			if (LIKELY(m_value->IsObject()))
			{
				const Object& object = Object::GetFromReference(*m_value);
				if (const Optional<const TValue*> memberValue = object.FindMember(memberName))
				{
					return SerializeInternal<T, Args...>(*memberValue, element, args...);
				}
			}

			return false;
		}

		inline Reader GetSerializer(const ConstStringView memberName) const
		{
			Assert(m_value->IsObject());
			const Object& object = Object::GetFromReference(*m_value);
			const TValue& memberValue = *object.FindMember(memberName);
			return Reader(memberValue, m_serializedData);
		}

		[[nodiscard]] inline bool HasSerializer(const ConstStringView memberName) const
		{
			Assert(m_value->IsObject());
			if (LIKELY(m_value->IsObject()))
			{
				const Object& object = Object::GetFromReference(*m_value);
				return object.HasMember(memberName);
			}
			else
			{
				return false;
			}
		}

		inline Optional<Reader> FindSerializer(const ConstStringView memberName) const
		{
			Assert(m_value->IsObject());
			if (LIKELY(m_value->IsObject()))
			{
				const Object& object = Object::GetFromReference(*m_value);
				if (const Optional<const TValue*> memberValue = object.FindMember(memberName))
				{
					return Reader(*memberValue, m_serializedData);
				}
			}

			return Optional<Reader>();
		}

		[[nodiscard]] bool IsString() const
		{
			return m_value->IsString();
		}
		[[nodiscard]] bool IsObject() const
		{
			return m_value->IsObject();
		}
		[[nodiscard]] bool IsArray() const
		{
			return m_value->IsArray();
		}

		[[nodiscard]] size GetArraySize() const
		{
			Assert(m_value->IsArray());
			if (LIKELY(m_value->IsArray()))
			{
				const Array& object = Array::GetFromReference(*m_value);
				return object.GetSize();
			}
			else
			{
				return 0;
			}
		}

		[[nodiscard]] size GetMemberCount() const
		{
			Assert(m_value->IsObject());
			if (LIKELY(m_value->IsObject()))
			{
				const Object& object = Object::GetFromReference(*m_value);
				return object.GetMemberCount();
			}
			else
			{
				return 0;
			}
		}

		[[nodiscard]] ArrayView<Reader> GetArrayView() const
		{
			return ArrayView<Reader>{m_value, m_serializedData};
		}

		template<typename Type>
		[[nodiscard]] ArrayView<Type> GetArrayView() const
		{
			return ArrayView<Type>{m_value, m_serializedData};
		}

		[[nodiscard]] ObjectView<Reader> GetMemberView() const
		{
			return ObjectView<Reader>{m_value, m_serializedData};
		}

		template<typename Type>
		[[nodiscard]] ObjectView<Type> GetMemberView() const
		{
			return ObjectView<Type>{m_value, m_serializedData};
		}

		template<typename T, typename... Args>
		EnableIf<CanRead<T, Args...>, bool> SerializeInPlace(T& element, Args&... args) const
		{
			return SerializeInternal<T, Args...>(m_value, element, args...);
		}

		[[nodiscard]] const TValue& GetValue() const
		{
			return m_value;
		}
		[[nodiscard]] const Data& GetData() const
		{
			return m_serializedData;
		}

		template<typename T, typename... Args>
		[[nodiscard]] EnableIf<CanRead<T, Args...>, bool> SerializeInternal(const TValue& value, T& element, Args&... args) const
		{
			Reader reader(value, m_serializedData);
			return Internal::DeserializeElement<T, Args...>(element, reader, args...);
		}

		template<typename Type>
		[[nodiscard]] EnableIf<!CanRead<Type>, bool> SerializeInternal(const TValue&, Type&) const
		{
			static_unreachable("Not implemented");
		}
	protected:
		ReferenceWrapper<const TValue> m_value;
		ReferenceWrapper<const Data> m_serializedData;
	};

	[[nodiscard]] inline Reader ArrayView<Reader>::Iterator::operator*() const
	{
		auto& value = BaseType::operator*();
		return Reader(value, m_serializedData);
	}

	template<typename Type>
	[[nodiscard]] inline Optional<Type> ArrayView<Type>::Iterator::operator*() const
	{
		auto& value = Array::ConstIterator::operator*();
		Reader reader(value, m_serializedData);
		return reader.ReadInPlace<Type>();
	}

	[[nodiscard]] inline Member<Reader> ObjectView<Reader>::Iterator::operator*() const
	{
		const Object::ConstMember member = Object::ConstIterator::operator*();
		return Pair{member.name, Reader(member.value, m_serializedData)};
	}

	template<typename Type>
	[[nodiscard]] inline Member<Optional<Type>> ObjectView<Type>::Iterator::operator*() const
	{
		const Object::ConstMember member = Object::ConstIterator::operator*();
		const Reader reader(member.value, m_serializedData);
		return Pair{member.name, reader.ReadInPlace<Type>()};
	}
}

#include <Common/Serialization/DeserializeElement.h>
#include <Common/Memory/Serialization/Optional.h>
