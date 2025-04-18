#include <Common/Memory/New.h>

#include <Common/Tests/UnitTest.h>
#include <Common/Time/Timestamp.h>

#include <Common/Serialization/Deserialize.h>
#include <Common/Serialization/Serialize.h>

namespace ngine::Tests
{
	UNIT_TEST(Timestamp, Construct)
	{
		EXPECT_FALSE(Time::Timestamp().IsValid());

		const Time::Timestamp referenceTime = Time::Timestamp::FromSeconds(1714669911ull);
		EXPECT_TRUE(referenceTime.IsValid());
		EXPECT_TRUE(referenceTime.ToString().GetView().StartsWith("2024-05-02"));
	}

	UNIT_TEST(Timestamp, GetCurrent)
	{
		const Time::Timestamp currentTime = Time::Timestamp::GetCurrent();
		EXPECT_TRUE(currentTime.IsValid());
		const Time::Timestamp currentTimeStandard = Time::Timestamp::FromSeconds(time(nullptr));
		EXPECT_TRUE(currentTimeStandard.IsValid());
		EXPECT_LT((Math::Max(currentTimeStandard, currentTime) - Math::Min(currentTimeStandard, currentTime)).GetSeconds(), 1);
	}

	struct TimestampStructure
	{
		bool Serialize(const Serialization::Reader serializer)
		{
			serializer.Serialize("timestamp", m_timestamp);
			return true;
		}

		bool Serialize(Serialization::Writer serializer) const
		{
			serializer.Serialize("timestamp", m_timestamp);
			return true;
		}

		Time::Timestamp m_timestamp;
	};

	UNIT_TEST(Timestamp, Read)
	{
		constexpr ConstStringView jsonContents = R"(
{
	"timestamp": "2023-10-27T12:22:18.436695Z"
})";

		TimestampStructure data;
		const bool success = Serialization::DeserializeFromBuffer(jsonContents, data);
		EXPECT_TRUE(success);
		EXPECT_TRUE(data.m_timestamp.IsValid());
	}

	UNIT_TEST(Timestamp, Write)
	{
		Optional<String> jsonContents;

		const Time::Timestamp referenceTime = Time::Timestamp::FromSeconds(1714669911ull);
		{
			TimestampStructure data;
			data.m_timestamp = referenceTime;
			jsonContents = Serialization::SerializeToBuffer(data, Serialization::SavingFlags{});
			EXPECT_TRUE(jsonContents.IsValid());
		}

		if (jsonContents.IsValid())
		{
			TimestampStructure newData;
			const bool readSuccess =
				Serialization::DeserializeFromBuffer(ConstStringView(jsonContents->GetData(), jsonContents->GetSize()), newData);
			EXPECT_TRUE(readSuccess);
			EXPECT_EQ(newData.m_timestamp, referenceTime);
		}
	}
}
