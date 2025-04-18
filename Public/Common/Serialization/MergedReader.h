#pragma once

#include <Common/Serialization/Deserialize.h>
#include <Common/Memory/Variant.h>

namespace ngine::Serialization
{
	void MergeObjects(Serialization::Value& target, const Serialization::Value& source, Serialization::Value::AllocatorType& allocator);
	void MergeArrays(Serialization::Value& target, const Serialization::Value& source, Serialization::Value::AllocatorType& allocator);

	[[nodiscard]] inline bool
	MergeValues(Serialization::Value& target, const Serialization::Value& source, Serialization::Value::AllocatorType& allocator)
	{
		if (target.GetType() == source.GetType())
		{
			switch (source.GetType())
			{
				case rapidjson::kNullType:
					target.SetNull();
					break;
				case rapidjson::kFalseType:
					target.SetBool(false);
					break;
				case rapidjson::kTrueType:
					target.SetBool(true);
					break;
				case rapidjson::kObjectType:
					MergeObjects(target, source, allocator);
					break;
				case rapidjson::kArrayType:
					MergeArrays(target, source, allocator);
					break;
				case rapidjson::kStringType:
					target.SetString(source.GetString(), source.GetStringLength(), allocator);
					break;
				case rapidjson::kNumberType:
					target.CopyFrom(source, allocator);
					break;
			}
			return true;
		}
		else
		{
			target.CopyFrom(source, allocator);
			return false;
		}
	}

	inline void MergeArrays(Serialization::Value& target, const Serialization::Value& source, Serialization::Value::AllocatorType& allocator)
	{
		target.Reserve(target.Size() + source.Size(), allocator);
		for (Serialization::Value::ConstValueIterator it = source.Begin(), end = source.End(); it != end; ++it)
		{
			const uint32 sourceIndex = static_cast<uint32>(it - source.Begin());
			if (sourceIndex < target.Size())
			{
				[[maybe_unused]] const bool wasMerged = MergeValues(target[sourceIndex], *it, allocator);
			}
			else
			{
				target.PushBack(Serialization::Value(*it, allocator), allocator);
			}
		}
	}

	inline void MergeObjects(Serialization::Value& target, const Serialization::Value& source, Serialization::Value::AllocatorType& allocator)
	{
		Assert(target.IsObject());
		Assert(source.IsObject());
		target.ReserveMembers(target.MemberCount() + source.MemberCount(), allocator);

		for (Serialization::Value::ConstMemberIterator it = source.GetObject().MemberBegin(), end = source.GetObject().MemberEnd(); it != end;
		     ++it)
		{
			Serialization::Value::MemberIterator targetIt = target.FindMember(it->name);
			if (targetIt != target.MemberEnd())
			{
				[[maybe_unused]] const bool wasMerged = MergeValues(targetIt->value, it->value, allocator);
			}
			else
			{
				target.AddMember(
					Serialization::Value(it->name.GetString(), it->name.GetStringLength(), allocator),
					Serialization::Value(it->value, allocator),
					allocator
				);
			}
		}
	}

	struct MergedReader
	{
		MergedReader() = default;
		MergedReader(Serialization::RootReader&& root)
			: m_reader(Forward<Serialization::RootReader>(root))
		{
		}
		MergedReader(const Serialization::Reader& reader)
			: m_reader(reader)
		{
		}
		MergedReader(const MergedReader&) = delete;
		MergedReader& operator=(const MergedReader&) = delete;
		MergedReader(MergedReader&& other) = default;
		MergedReader& operator=(MergedReader&&) = default;

		[[nodiscard]] Optional<Serialization::Reader> GetReader() const
		{
			switch (static_cast<Type>(m_reader.GetActiveIndex()))
			{
				case Type::Invalid:
					return Invalid;
				case Type::Root:
					return m_reader.GetExpected<Serialization::RootReader>();
				case Type::Reader:
					return m_reader.GetExpected<Serialization::Reader>();
				default:
					ExpectUnreachable();
			}
		}

		[[nodiscard]] Optional<Serialization::RootReader*> GetRootReader() LIFETIME_BOUND
		{
			switch (static_cast<Type>(m_reader.GetActiveIndex()))
			{
				case Type::Invalid:
					return Invalid;
				case Type::Root:
					return &m_reader.GetExpected<Serialization::RootReader>();
				case Type::Reader:
					return Invalid;
				default:
					ExpectUnreachable();
			}
		}
	private:
		Variant<Serialization::RootReader, Serialization::Reader> m_reader;

		enum class Type : uint8
		{
			Root,
			Reader,
			Invalid = decltype(m_reader)::InvalidIndex
		};
	};

	[[nodiscard]] inline MergedReader
	MergeSerializers(const Serialization::Reader serializer, Optional<Serialization::RootReader>&& sceneSerializer)
	{
		if (sceneSerializer && sceneSerializer->GetData().IsValid())
		{
			const Serialization::Value& value = serializer.GetValue();
			Assert(value.GetType() == sceneSerializer->GetData().GetDocument().GetType());
			MergeObjects(sceneSerializer->GetData().GetDocument(), value, sceneSerializer->GetData().GetDocument().GetAllocator());
			return MergedReader(Move(*sceneSerializer));
		}
		else
		{
			return serializer;
		}
	}
}
