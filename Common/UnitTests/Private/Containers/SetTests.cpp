#include <Common/Memory/New.h>

#include <Common/Tests/UnitTest.h>

#include <Common/Memory/Containers/UnorderedSet.h>

namespace ngine::Tests
{
	UNIT_TEST(UnorderedSet, DefaultConstruct)
	{
		UnorderedSet<int> set;
		EXPECT_TRUE(set.IsEmpty());
		EXPECT_FALSE(set.HasElements());
		EXPECT_EQ(set.GetSize(), 0u);
		EXPECT_EQ(set.begin(), set.end());
		EXPECT_EQ(set.Find(0), set.end());
		EXPECT_FALSE(set.Contains(0));
	}

	UNIT_TEST(UnorderedSet, BasicEmplace)
	{
		UnorderedSet<int> set;
		set.Emplace(1337);
		EXPECT_EQ(set.GetSize(), 1);
		EXPECT_TRUE(set.HasElements());
		EXPECT_NE(set.begin(), set.end());
		EXPECT_TRUE(set.Contains(1337));
		EXPECT_FALSE(set.Contains(0));
		EXPECT_FALSE(set.Contains(1));
		EXPECT_FALSE(set.Contains(1336));
		EXPECT_FALSE(set.Contains(1338));
		EXPECT_FALSE(set.Contains(9001));
		EXPECT_EQ(set.Find(1337), set.begin());

		set.Emplace(9001);
		EXPECT_EQ(set.GetSize(), 2);
		EXPECT_TRUE(set.HasElements());
		EXPECT_NE(set.begin(), set.end());
		EXPECT_TRUE(set.Contains(1337));
		EXPECT_TRUE(set.Contains(9001));
		EXPECT_FALSE(set.Contains(0));
		EXPECT_FALSE(set.Contains(1));
		EXPECT_FALSE(set.Contains(1336));
		EXPECT_FALSE(set.Contains(1338));
		EXPECT_NE(set.Find(1337), set.end());
		EXPECT_NE(set.Find(9001), set.end());

		set.Remove(set.Find(1337));
		EXPECT_EQ(set.GetSize(), 1);
		EXPECT_TRUE(set.HasElements());
		EXPECT_NE(set.begin(), set.end());
		EXPECT_FALSE(set.Contains(1337));
		EXPECT_TRUE(set.Contains(9001));
		EXPECT_EQ(set.Find(1337), set.end());
		EXPECT_NE(set.Find(9001), set.end());

		set.Remove(set.Find(9001));
		EXPECT_EQ(set.GetSize(), 0);
		EXPECT_TRUE(set.IsEmpty());
		EXPECT_EQ(set.begin(), set.end());
		EXPECT_FALSE(set.Contains(1337));
		EXPECT_FALSE(set.Contains(9001));
		EXPECT_EQ(set.Find(1337), set.end());
		EXPECT_EQ(set.Find(9001), set.end());
	}
}
