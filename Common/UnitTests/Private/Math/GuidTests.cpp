#include <Common/Memory/New.h>

#include <Common/Tests/UnitTest.h>

#include <Common/Guid.h>
#include <Common/Memory/Containers/FlatString.h>

namespace ngine::Tests
{
	UNIT_TEST(Math, Guid)
	{
		EXPECT_FALSE(Guid{}.IsValid());
		EXPECT_FALSE(Guid::TryParse("guid").IsValid());

		EXPECT_EQ("00000000-0000-0000-0000-000000000000"_guid, "00000000-0000-0000-0000-000000000000"_guid);
		EXPECT_NE("00000000-0000-0000-0000-000000000000"_guid, "2b1d900c-8be9-4d03-a877-9ca4c6864447"_guid);
		EXPECT_EQ("2b1d900c-8be9-4d03-a877-9ca4c6864447"_guid, "2b1d900c-8be9-4d03-a877-9ca4c6864447"_guid);
		EXPECT_NE("2b1d900c-8be9-4d03-a877-9ca4c6864447"_guid, "00000000-0000-0000-0000-000000000000"_guid);
		EXPECT_NE("2b1d900c-8be9-4d03-a877-9ca4c6864447"_guid, "0b1d900c-8be9-4d03-a877-9ca4c6864447"_guid);
		EXPECT_NE("2b1d900c-8be9-4d03-a877-9ca4c6864447"_guid, "2c1d900c-8be9-4d03-a877-9ca4c6864447"_guid);
		EXPECT_NE("2b1d900c-8be9-4d03-a877-9ca4c6864447"_guid, "2b1d900c-9be9-4d03-a877-9ca4c6864447"_guid);
		EXPECT_NE("2b1d900c-8be9-4d03-a877-9ca4c6864447"_guid, "2b1d900c-8be9-8d03-a877-9ca4c6864447"_guid);
		EXPECT_NE("2b1d900c-8be9-4d03-a877-9ca4c6864447"_guid, "2b1d900c-8be9-4d03-b877-9ca4c6864447"_guid);
		EXPECT_NE("2b1d900c-8be9-4d03-a877-9ca4c6864447"_guid, "2b1d900c-8be9-4d03-a877-1ca4c6864447"_guid);

		EXPECT_FALSE(("00000000-0000-0000-0000-000000000000"_guid).IsValid());
		EXPECT_TRUE(("2b1d900c-8be9-4d03-a877-9ca4c6864447"_guid).IsValid());

		{
			const FlatString<37> data = ("2b1d900c-8be9-4d03-a877-9ca4c6864447"_guid).ToString();
			EXPECT_EQ(data.GetView(), "2b1d900c-8be9-4d03-a877-9ca4c6864447");
		}
		{
			const FlatString<37> data = ("00000000-0000-0000-0000-000000000000"_guid).ToString();
			EXPECT_EQ(data.GetView(), "00000000-0000-0000-0000-000000000000");
		}

		{
			const FlatString<37> data = ("7b5c3927-1373-4526-baaf-d7a77cf80dad"_guid).ToString();
			EXPECT_EQ(data.GetView(), "7b5c3927-1373-4526-baaf-d7a77cf80dad");
			EXPECT_EQ("7b5c3927-1373-4526-baaf-d7a77cf80dad"_guid, "7b5c3927-1373-4526-baaf-d7a77cf80dad"_guid);
			EXPECT_EQ(Guid::TryParse(data.GetView()), "7b5c3927-1373-4526-baaf-d7a77cf80dad"_guid);
		}
	}
}
