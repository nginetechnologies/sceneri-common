#include <Common/Memory/New.h>

#include <Common/Tests/UnitTest.h>

#include <Common/Memory/Containers/String.h>
#include <Common/Memory/Containers/Format/String.h>

namespace ngine::Tests
{
	UNIT_TEST(Format, Simple)
	{
		String formattedString;
		formattedString.Format("{0}", 1337);
		EXPECT_EQ(formattedString, "1337");
	}
}
