#include <Common/Memory/New.h>

#include <Common/Tests/UnitTest.h>

#include <Common/Serialization/Deserialize.h>
#include <Common/Serialization/Serialize.h>
#include <Common/Memory/Containers/Vector.h>

#include <Common/Memory/Containers/Serialization/Vector.h>
#include <Common/Serialization/Version.h>
#include <Common/Serialization/Guid.h>
#include <Common/Math/Vector3.h>
#include <Common/Math/Angle.h>
#include <Common/Memory/Containers/Serialization/UnorderedMap.h>

namespace ngine::Tests
{
	struct SplitReadWrite
	{
		bool Serialize(const Serialization::Reader serializer)
		{
			serializer.Serialize("test", m_test);
			return true;
		}

		bool Serialize(Serialization::Writer serializer) const
		{
			serializer.Serialize("test", m_test);
			return true;
		}

		int m_test;
	};

	struct SimpleReadWrite
	{
		bool Serialize(const Serialization::Reader serializer)
		{
			serializer.Serialize("test", m_test);
			return true;
		}

		bool Serialize(Serialization::Writer serializer) const
		{
			serializer.Serialize("test", m_test);
			return true;
		}

		int m_test;
	};

	UNIT_TEST(Serialization, ReadSplitReadWriteStructure)
	{
		constexpr ConstStringView jsonContents = R"(
{
	"test": 1337
})";

		SplitReadWrite data;
		const bool success = Serialization::DeserializeFromBuffer(jsonContents, data);
		EXPECT_TRUE(success);
		EXPECT_EQ(data.m_test, 1337);
	}

	UNIT_TEST(Serialization, ReadSimpleStructure)
	{
		constexpr ConstStringView jsonContents = R"(
{
	"test": 9001
})";

		SimpleReadWrite data;
		const bool success = Serialization::DeserializeFromBuffer(jsonContents, data);
		EXPECT_TRUE(success);
		EXPECT_EQ(data.m_test, 9001);
	}

	struct NestedSplitReadWrite
	{
		bool Serialize(const Serialization::Reader serializer)
		{
			serializer.Serialize("nested", m_member);
			return true;
		}

		bool Serialize(Serialization::Writer serializer) const
		{
			serializer.Serialize("nested", m_member);
			return true;
		}

		SplitReadWrite m_member;
	};

	struct NestedSimpleReadWrite
	{
		bool Serialize(const Serialization::Reader serializer)
		{
			serializer.Serialize("nested", m_member);
			return true;
		}

		bool Serialize(Serialization::Writer serializer) const
		{
			serializer.Serialize("nested", m_member);
			return true;
		}

		SimpleReadWrite m_member;
	};

	UNIT_TEST(Serialization, ReadNestedSplitReadWriteStructure)
	{
		constexpr ConstStringView jsonContents = R"(
{
	"nested": { "test": 1337 }
})";

		NestedSplitReadWrite data;
		const bool success = Serialization::DeserializeFromBuffer(jsonContents, data);
		EXPECT_TRUE(success);
		EXPECT_EQ(data.m_member.m_test, 1337);
	}

	UNIT_TEST(Serialization, ReadNestedSimpleStructure)
	{
		constexpr ConstStringView jsonContents = R"(
{
	"nested": { "test": 9001 }
})";

		NestedSimpleReadWrite data;
		const bool success = Serialization::DeserializeFromBuffer(jsonContents, data);
		EXPECT_TRUE(success);
		EXPECT_EQ(data.m_member.m_test, 9001);
	}

	UNIT_TEST(Serialization, WriteSplitReadWriteStructure)
	{
		Optional<String> jsonContents;

		{
			SplitReadWrite data;
			data.m_test = 5003;
			jsonContents = Serialization::SerializeToBuffer(data, Serialization::SavingFlags{});
			EXPECT_TRUE(jsonContents.IsValid());
		}

		if (jsonContents.IsValid())
		{
			SplitReadWrite newData;
			const bool readSuccess =
				Serialization::DeserializeFromBuffer(ConstStringView(jsonContents->GetData(), jsonContents->GetSize()), newData);
			EXPECT_TRUE(readSuccess);
			EXPECT_EQ(newData.m_test, 5003);
		}
	}

	UNIT_TEST(Serialization, WriteSimpleStructure)
	{
		Optional<String> jsonContents;

		{
			SplitReadWrite data;
			data.m_test = 901;
			jsonContents = Serialization::SerializeToBuffer(data, Serialization::SavingFlags{});
			EXPECT_TRUE(jsonContents.IsValid());
		}

		if (jsonContents.IsValid())
		{
			SplitReadWrite newData;
			const bool readSuccess =
				Serialization::DeserializeFromBuffer(ConstStringView(jsonContents->GetData(), jsonContents->GetSize()), newData);
			EXPECT_TRUE(readSuccess);
			EXPECT_EQ(newData.m_test, 901);
		}
	}

	struct VectorStructure
	{
		bool Serialize(const Serialization::Reader serializer)
		{
			serializer.Serialize("array", m_array);
			return true;
		}

		bool Serialize(Serialization::Writer serializer) const
		{
			serializer.Serialize("array", m_array);
			return true;
		}

		Vector<int> m_array;
	};

	UNIT_TEST(Serialization, ReadVector)
	{
		constexpr ConstStringView jsonContents = R"(
{
	"array": [ 0, 1, 2, 3, 4, 5 ]
})";

		VectorStructure data;
		const bool success = Serialization::DeserializeFromBuffer(jsonContents, data);
		EXPECT_TRUE(success);
		EXPECT_EQ(data.m_array.GetSize(), 6u);
		EXPECT_EQ(data.m_array[0], 0);
		EXPECT_EQ(data.m_array[1], 1);
		EXPECT_EQ(data.m_array[2], 2);
		EXPECT_EQ(data.m_array[3], 3);
		EXPECT_EQ(data.m_array[4], 4);
		EXPECT_EQ(data.m_array[5], 5);
	}

	UNIT_TEST(Serialization, WriteVector)
	{
		Optional<String> jsonContents;

		{
			VectorStructure data;
			data.m_array = Vector<int>{1, 2, 3, 4, 5};
			jsonContents = Serialization::SerializeToBuffer(data, Serialization::SavingFlags{});
			EXPECT_TRUE(jsonContents.IsValid());
		}

		if (jsonContents.IsValid())
		{
			VectorStructure newData;
			const bool readSuccess =
				Serialization::DeserializeFromBuffer(ConstStringView(jsonContents->GetData(), jsonContents->GetSize()), newData);
			EXPECT_TRUE(readSuccess);
			EXPECT_EQ(newData.m_array.GetSize(), 5u);
			EXPECT_EQ(newData.m_array[0], 1);
			EXPECT_EQ(newData.m_array[1], 2);
			EXPECT_EQ(newData.m_array[2], 3);
			EXPECT_EQ(newData.m_array[3], 4);
			EXPECT_EQ(newData.m_array[4], 5);
		}
	}

	struct StringStructure
	{
		bool Serialize(const Serialization::Reader serializer)
		{
			serializer.Serialize("string", m_string);
			return true;
		}

		bool Serialize(Serialization::Writer serializer) const
		{
			serializer.Serialize("string", m_string);
			return true;
		}

		String m_string;
	};

	UNIT_TEST(Serialization, ReadString)
	{
		constexpr ConstStringView jsonContents = R"(
{
	"string": "MyString1#!"
})";

		StringStructure data;
		const bool success = Serialization::DeserializeFromBuffer(jsonContents, data);
		EXPECT_TRUE(success);
		EXPECT_EQ(data.m_string, "MyString1#!");
	}

	UNIT_TEST(Serialization, WriteString)
	{
		Optional<String> jsonContents;

		{
			StringStructure data;
			data.m_string = "_MyString1#!";
			jsonContents = Serialization::SerializeToBuffer(data, Serialization::SavingFlags{});
			EXPECT_TRUE(jsonContents.IsValid());
		}

		if (jsonContents.IsValid())
		{
			StringStructure newData;
			const bool readSuccess =
				Serialization::DeserializeFromBuffer(ConstStringView(jsonContents->GetData(), jsonContents->GetSize()), newData);
			EXPECT_TRUE(readSuccess);
			EXPECT_EQ(newData.m_string, "_MyString1#!");
		}
	}

	struct VersionStructure
	{
		bool Serialize(const Serialization::Reader serializer)
		{
			serializer.Serialize("version", m_version);
			return true;
		}

		bool Serialize(Serialization::Writer serializer) const
		{
			serializer.Serialize("version", m_version);
			return true;
		}

		Version m_version = Version(0, 0, 0);
	};

	UNIT_TEST(Serialization, ReadVersion)
	{
		constexpr ConstStringView jsonContents = R"(
{
	"version": "1.2.3"
})";

		VersionStructure data;
		const bool success = Serialization::DeserializeFromBuffer(jsonContents, data);
		EXPECT_TRUE(success);
		EXPECT_EQ(data.m_version.GetMajor(), 1);
		EXPECT_EQ(data.m_version.GetMinor(), 2);
		EXPECT_EQ(data.m_version.GetPatch(), 3);
	}

	UNIT_TEST(Serialization, WriteVersion)
	{
		Optional<String> jsonContents;

		{
			VersionStructure data;
			data.m_version = Version(1, 2, 3);
			jsonContents = Serialization::SerializeToBuffer(data, Serialization::SavingFlags{});
			EXPECT_TRUE(jsonContents.IsValid());
		}

		if (jsonContents.IsValid())
		{
			VersionStructure newData;
			const bool readSuccess =
				Serialization::DeserializeFromBuffer(ConstStringView(jsonContents->GetData(), jsonContents->GetSize()), newData);
			EXPECT_TRUE(readSuccess);
			EXPECT_EQ(newData.m_version.GetMajor(), 1);
			EXPECT_EQ(newData.m_version.GetMinor(), 2);
			EXPECT_EQ(newData.m_version.GetPatch(), 3);
		}
	}

	struct GuidStructure
	{
		bool Serialize(const Serialization::Reader serializer)
		{
			serializer.Serialize("guid", m_guid);
			return true;
		}

		bool Serialize(Serialization::Writer serializer) const
		{
			serializer.Serialize("guid", m_guid);
			return true;
		}

		Guid m_guid;
	};

	UNIT_TEST(Serialization, ReadGuid)
	{
		constexpr ConstStringView jsonContents = R"(
{
	"guid": "BAA2E7B0-C0B8-4BB2-94D8-B26B350C3834"
})";

		GuidStructure data;
		const bool success = Serialization::DeserializeFromBuffer(jsonContents, data);
		EXPECT_TRUE(success);
		EXPECT_EQ(data.m_guid, "BAA2E7B0-C0B8-4BB2-94D8-B26B350C3834"_guid);
	}

	UNIT_TEST(Serialization, WriteGuid)
	{
		Optional<String> jsonContents;

		{
			GuidStructure data;
			data.m_guid = "{A8446DCA-A163-4652-B8B8-671579E70F20}"_guid;
			jsonContents = Serialization::SerializeToBuffer(data, Serialization::SavingFlags{});
			EXPECT_TRUE(jsonContents.IsValid());
		}

		if (jsonContents.IsValid())
		{
			GuidStructure newData;
			const bool readSuccess =
				Serialization::DeserializeFromBuffer(ConstStringView(jsonContents->GetData(), jsonContents->GetSize()), newData);
			EXPECT_TRUE(readSuccess);
			EXPECT_EQ(newData.m_guid, "{A8446DCA-A163-4652-B8B8-671579E70F20}"_guid);
		}
	}

	struct Vector3Structure
	{
		bool Serialize(const Serialization::Reader serializer)
		{
			serializer.Serialize("vector", m_vector);
			return true;
		}

		bool Serialize(Serialization::Writer serializer) const
		{
			serializer.Serialize("vector", m_vector);
			return true;
		}

		Math::Vector3i m_vector;
	};

	UNIT_TEST(Serialization, ReadVector3)
	{
		constexpr ConstStringView jsonContents = R"(
{
	"vector": [1, 2, 3]
})";

		Vector3Structure data;
		const bool success = Serialization::DeserializeFromBuffer(jsonContents, data);
		EXPECT_TRUE(success);
		EXPECT_EQ(data.m_vector.x, 1);
		EXPECT_EQ(data.m_vector.y, 2);
		EXPECT_EQ(data.m_vector.z, 3);
	}

	UNIT_TEST(Serialization, WriteVector3)
	{
		Optional<String> jsonContents;

		{
			Vector3Structure data;
			data.m_vector = Math::Vector3i(2, 3, 4);
			jsonContents = Serialization::SerializeToBuffer(data, Serialization::SavingFlags{});
			EXPECT_TRUE(jsonContents.IsValid());
		}

		if (jsonContents.IsValid())
		{
			Vector3Structure newData;
			const bool readSuccess =
				Serialization::DeserializeFromBuffer(ConstStringView(jsonContents->GetData(), jsonContents->GetSize()), newData);
			EXPECT_TRUE(readSuccess);
			EXPECT_EQ(newData.m_vector.x, 2);
			EXPECT_EQ(newData.m_vector.y, 3);
			EXPECT_EQ(newData.m_vector.z, 4);
		}
	}

	struct AngleStructure
	{
		bool Serialize(const Serialization::Reader serializer)
		{
			serializer.Serialize("angle", m_angle);
			return true;
		}

		bool Serialize(Serialization::Writer serializer) const
		{
			serializer.Serialize("angle", m_angle);
			return true;
		}

		Math::Anglef m_angle;
	};

	UNIT_TEST(Serialization, ReadAngle)
	{
		constexpr ConstStringView jsonContents = R"(
{
	"angle": 85
})";

		AngleStructure data;
		const bool success = Serialization::DeserializeFromBuffer(jsonContents, data);
		EXPECT_TRUE(success);
		EXPECT_LE(Math::Abs(data.m_angle.GetDegrees() - 85.f), 0.01f);
	}

	UNIT_TEST(Serialization, WriteAngle)
	{
		using namespace ngine;

		Optional<String> jsonContents;

		{
			AngleStructure data;
			data.m_angle = 37_degrees;
			jsonContents = Serialization::SerializeToBuffer(data, Serialization::SavingFlags{});
			EXPECT_TRUE(jsonContents.IsValid());
		}

		if (jsonContents.IsValid())
		{
			AngleStructure newData;
			const bool readSuccess =
				Serialization::DeserializeFromBuffer(ConstStringView(jsonContents->GetData(), jsonContents->GetSize()), newData);
			EXPECT_TRUE(readSuccess);
			EXPECT_LE(Math::Abs(newData.m_angle.GetDegrees() - 37.f), 0.01f);
		}
	}

	struct UnorderedMapStructure
	{
		bool Serialize(const Serialization::Reader serializer)
		{
			serializer.Serialize("map", m_map);
			return true;
		}

		bool Serialize(Serialization::Writer serializer) const
		{
			serializer.Serialize("map", m_map);
			return true;
		}

		UnorderedMap<String, int, String::Hash> m_map;
	};

	UNIT_TEST(Serialization, ReadUnorderedMap)
	{
		constexpr ConstStringView jsonContents = R"(
{
	"map": { "first": 1, "second": 2, "third": 3 }
})";

		UnorderedMapStructure data;
		const bool success = Serialization::DeserializeFromBuffer(jsonContents, data);
		EXPECT_TRUE(success);
		{
			auto it = data.m_map.Find(String("first"));
			EXPECT_TRUE(it != data.m_map.end());
			EXPECT_TRUE(it != data.m_map.end() ? it->second == 1 : false);
		}
		{
			auto it = data.m_map.Find(String("second"));
			EXPECT_TRUE(it != data.m_map.end());
			EXPECT_TRUE(it != data.m_map.end() ? it->second == 2 : false);
		}
		{
			auto it = data.m_map.Find(String("third"));
			EXPECT_TRUE(it != data.m_map.end());
			EXPECT_TRUE(it != data.m_map.end() ? it->second == 3 : false);
		}
	}

	UNIT_TEST(Serialization, WriteUnorderedMap)
	{
		Optional<String> jsonContents;

		{
			UnorderedMapStructure data;
			data.m_map.Emplace(String("one"), 1);
			data.m_map.Emplace(String("two"), 2);
			jsonContents = Serialization::SerializeToBuffer(data, Serialization::SavingFlags{});
			EXPECT_TRUE(jsonContents.IsValid());
		}

		if (jsonContents.IsValid())
		{
			UnorderedMapStructure newData;
			const bool readSuccess =
				Serialization::DeserializeFromBuffer(ConstStringView(jsonContents->GetData(), jsonContents->GetSize()), newData);
			EXPECT_TRUE(readSuccess);
			{
				auto it = newData.m_map.Find(String("one"));
				EXPECT_TRUE(it != newData.m_map.end());
				EXPECT_TRUE(it != newData.m_map.end() ? it->second == 1 : false);
			}
			{
				auto it = newData.m_map.Find(String("two"));
				EXPECT_TRUE(it != newData.m_map.end());
				EXPECT_TRUE(it != newData.m_map.end() ? it->second == 2 : false);
			}
		}
	}

	struct UnorderedMapGuidKeyStructure
	{
		bool Serialize(const Serialization::Reader serializer)
		{
			serializer.Serialize("map", m_map);
			return true;
		}

		bool Serialize(Serialization::Writer serializer) const
		{
			serializer.Serialize("map", m_map);
			return true;
		}

		UnorderedMap<Guid, int, Guid::Hash> m_map;
	};

	UNIT_TEST(Serialization, ReadUnorderedMapWithGuidKey)
	{
		constexpr ConstStringView jsonContents = R"(
{
	"map": { "00F64A06-0C3A-49B9-BA0A-A875180375D4": 1, "3032B611-661D-481F-9B83-53FA1E510AB8": 2}
})";

		UnorderedMapGuidKeyStructure data;
		const bool success = Serialization::DeserializeFromBuffer(jsonContents, data);
		EXPECT_TRUE(success);
		{
			auto it = data.m_map.Find("00F64A06-0C3A-49B9-BA0A-A875180375D4"_guid);
			EXPECT_TRUE(it != data.m_map.end());
			EXPECT_TRUE(it != data.m_map.end() ? it->second == 1 : false);
		}
		{
			auto it = data.m_map.Find("3032B611-661D-481F-9B83-53FA1E510AB8"_guid);
			EXPECT_TRUE(it != data.m_map.end());
			EXPECT_TRUE(it != data.m_map.end() ? it->second == 2 : false);
		}
	}

	UNIT_TEST(Serialization, WriteUnorderedMapWithGuidKey)
	{
		Optional<String> jsonContents;

		{
			UnorderedMapGuidKeyStructure data;
			data.m_map.Emplace("0EB8ADE9-36F7-428E-8B86-52A2FB0F2A3A"_guid, 1);
			data.m_map.Emplace("A1A92778-F4AC-4FBC-A19C-67379B7E2F77"_guid, 2);
			jsonContents = Serialization::SerializeToBuffer(data, Serialization::SavingFlags{});
			EXPECT_TRUE(jsonContents.IsValid());
		}

		if (jsonContents.IsValid())
		{
			UnorderedMapGuidKeyStructure newData;
			const bool readSuccess =
				Serialization::DeserializeFromBuffer(ConstStringView(jsonContents->GetData(), jsonContents->GetSize()), newData);
			EXPECT_TRUE(readSuccess);
			{
				auto it = newData.m_map.Find("0EB8ADE9-36F7-428E-8B86-52A2FB0F2A3A"_guid);
				EXPECT_TRUE(it != newData.m_map.end());
				EXPECT_TRUE(it != newData.m_map.end() ? it->second == 1 : false);
			}
			{
				auto it = newData.m_map.Find("A1A92778-F4AC-4FBC-A19C-67379B7E2F77"_guid);
				EXPECT_TRUE(it != newData.m_map.end());
				EXPECT_TRUE(it != newData.m_map.end() ? it->second == 2 : false);
			}
		}
	}
}
