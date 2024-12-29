#include <Common/Memory/New.h>

#include <Common/Tests/UnitTest.h>

#include <Common/Memory/Containers/UnorderedMap.h>

namespace ngine::Tests
{
	UNIT_TEST(UnorderedMap, DefaultConstruct)
	{
		UnorderedMap<int, float> map;
		EXPECT_TRUE(map.IsEmpty());
		EXPECT_FALSE(map.HasElements());
		EXPECT_EQ(map.GetSize(), 0u);
		EXPECT_EQ(map.begin(), map.end());
		EXPECT_EQ(map.Find(0), map.end());
		EXPECT_FALSE(map.Contains(0));
	}

	UNIT_TEST(UnorderedMap, BasicEmplace)
	{
		UnorderedMap<int, float> map;
		map.Emplace(1337, 90.01f);
		EXPECT_EQ(map.GetSize(), 1);
		EXPECT_TRUE(map.HasElements());
		EXPECT_NE(map.begin(), map.end());
		EXPECT_TRUE(map.Contains(1337));
		EXPECT_FALSE(map.Contains(0));
		EXPECT_FALSE(map.Contains(1));
		EXPECT_FALSE(map.Contains(1336));
		EXPECT_FALSE(map.Contains(1338));
		EXPECT_FALSE(map.Contains(9001));
		EXPECT_EQ(map.Find(1337), map.begin());
		EXPECT_NEAR(map.Find(1337)->second, 90.01f, 0.01f);

		map.Emplace(9001, 13.37f);
		EXPECT_EQ(map.GetSize(), 2);
		EXPECT_TRUE(map.HasElements());
		EXPECT_NE(map.begin(), map.end());
		EXPECT_TRUE(map.Contains(1337));
		EXPECT_TRUE(map.Contains(9001));
		EXPECT_FALSE(map.Contains(0));
		EXPECT_FALSE(map.Contains(1));
		EXPECT_FALSE(map.Contains(1336));
		EXPECT_FALSE(map.Contains(1338));
		EXPECT_NE(map.Find(1337), map.end());
		EXPECT_NE(map.Find(9001), map.end());
		EXPECT_NEAR(map.Find(1337)->second, 90.01f, 0.01f);
		EXPECT_NEAR(map.Find(9001)->second, 13.37f, 0.01f);

		map.Remove(map.Find(1337));
		EXPECT_EQ(map.GetSize(), 1);
		EXPECT_TRUE(map.HasElements());
		EXPECT_NE(map.begin(), map.end());
		EXPECT_FALSE(map.Contains(1337));
		EXPECT_TRUE(map.Contains(9001));
		EXPECT_EQ(map.Find(1337), map.end());
		EXPECT_NE(map.Find(9001), map.end());
		EXPECT_NEAR(map.Find(9001)->second, 13.37f, 0.01f);

		map.Remove(map.Find(9001));
		EXPECT_EQ(map.GetSize(), 0);
		EXPECT_TRUE(map.IsEmpty());
		EXPECT_EQ(map.begin(), map.end());
		EXPECT_FALSE(map.Contains(1337));
		EXPECT_FALSE(map.Contains(9001));
		EXPECT_EQ(map.Find(1337), map.end());
		EXPECT_EQ(map.Find(9001), map.end());
	}
}
