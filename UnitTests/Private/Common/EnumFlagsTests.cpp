#include <Common/Memory/New.h>

#include <Common/Tests/UnitTest.h>

#include <Common/EnumFlags.h>
#include <Common/EnumFlagOperators.h>

namespace ngine::Tests
{
	enum class TestFlags : uint8
	{
		Flag1 = 1 << 0,

		Flag2 = 1 << 1,
		RangeBegin = Flag2,
		Flag3 = 1 << 2,
		RangeEnd = Flag3
	};

	ENUM_FLAG_OPERATORS(TestFlags);

	UNIT_TEST(EnumFlags, GetRange)
	{
		EnumFlags parent = EnumFlags(TestFlags::Flag1 | TestFlags::Flag2 | TestFlags::Flag3);
		EnumFlags range = parent.GetRange(TestFlags::RangeBegin, TestFlags::RangeEnd);

		EXPECT_EQ(range, EnumFlags(TestFlags::Flag2 | TestFlags::Flag3));
	}

	UNIT_TEST(EnumFlags, AreNoneSet)
	{
		EXPECT_TRUE(EnumFlags(TestFlags{}).AreNoneSet());
		EXPECT_TRUE(EnumFlags(TestFlags::Flag1 | TestFlags::Flag3).AreNoneSet(TestFlags::Flag2));
		EXPECT_TRUE(EnumFlags(TestFlags::Flag1).AreNoneSet(TestFlags::Flag2 | TestFlags::Flag3));
		EXPECT_TRUE(EnumFlags(TestFlags{}).AreNoneSet(TestFlags::Flag2 | TestFlags::Flag3));
		EXPECT_FALSE(EnumFlags(TestFlags::Flag2).AreNoneSet());
		EXPECT_FALSE(EnumFlags(TestFlags::Flag2).AreNoneSet(TestFlags::Flag2));
		EXPECT_FALSE(EnumFlags(TestFlags::Flag2 | TestFlags::Flag3).AreNoneSet(TestFlags::Flag2 | TestFlags::Flag3));
	}

	UNIT_TEST(EnumFlags, AreAnySet)
	{
		EXPECT_TRUE(EnumFlags(TestFlags::Flag3).AreAnySet());
		EXPECT_TRUE(EnumFlags(TestFlags::Flag1 | TestFlags::Flag2 | TestFlags::Flag3).AreAnySet(TestFlags::Flag1));
		EXPECT_TRUE(EnumFlags(TestFlags::Flag1 | TestFlags::Flag2 | TestFlags::Flag3).AreAnySet(TestFlags::Flag1 | TestFlags::Flag3));
		EXPECT_TRUE(EnumFlags(TestFlags::Flag1 | TestFlags::Flag2).AreAnySet(TestFlags::Flag1 | TestFlags::Flag3));
		EXPECT_FALSE(EnumFlags(TestFlags{}).AreAnySet());
		EXPECT_FALSE(EnumFlags(TestFlags::Flag2 | TestFlags::Flag3).AreAnySet(TestFlags::Flag1));
		EXPECT_FALSE(EnumFlags(TestFlags::Flag2).AreAnySet(TestFlags::Flag1 | TestFlags::Flag3));
	}

	UNIT_TEST(EnumFlags, AreAnyNotSet)
	{
		EXPECT_TRUE(EnumFlags(TestFlags::Flag3).AreAnyNotSet(TestFlags::Flag1 | TestFlags::Flag3));
		EXPECT_TRUE(EnumFlags(TestFlags::Flag3).AreAnyNotSet(TestFlags::Flag1));
		EXPECT_FALSE(EnumFlags(TestFlags::Flag3).AreAnyNotSet(TestFlags::Flag3));
		EXPECT_FALSE(EnumFlags(TestFlags::Flag2 | TestFlags::Flag3).AreAnyNotSet(TestFlags::Flag2 | TestFlags::Flag3));
	}

	UNIT_TEST(EnumFlags, AreAllSet)
	{
		EXPECT_TRUE(EnumFlags(TestFlags::Flag1 | TestFlags::Flag2 | TestFlags::Flag3).AreAllSet(TestFlags::Flag1));
		EXPECT_TRUE(EnumFlags(TestFlags::Flag1 | TestFlags::Flag2 | TestFlags::Flag3).AreAllSet(TestFlags::Flag1 | TestFlags::Flag3));
		EXPECT_TRUE(
			EnumFlags(TestFlags::Flag1 | TestFlags::Flag2 | TestFlags::Flag3).AreAllSet(TestFlags::Flag1 | TestFlags::Flag2 | TestFlags::Flag3)
		);
		EXPECT_TRUE(EnumFlags(TestFlags{}).AreAllSet({}));
		EXPECT_FALSE(EnumFlags(TestFlags{}).AreAllSet(TestFlags::Flag2));
		EXPECT_FALSE(EnumFlags(TestFlags::Flag2 | TestFlags::Flag3).AreAllSet(TestFlags::Flag1 | TestFlags::Flag3));
	}

	UNIT_TEST(EnumFlags, Iterator)
	{
		const EnumFlags<TestFlags> allFlags = TestFlags::Flag1 | TestFlags::Flag2 | TestFlags::Flag3;
		EnumFlags<TestFlags> resultingFlags;
		for (const TestFlags flag : allFlags)
		{
			resultingFlags |= flag;
		}
		EXPECT_TRUE(allFlags == resultingFlags);

		constexpr auto constexprTest = []() constexpr -> bool
		{
			constexpr EnumFlags<TestFlags> allFlags = TestFlags::Flag1 | TestFlags::Flag2 | TestFlags::Flag3;
			EnumFlags<TestFlags> resultingFlags;
			for (const TestFlags flag : allFlags)
			{
				resultingFlags |= flag;
			}
			return resultingFlags == allFlags;
		};
		static_assert(constexprTest());
	}
}
