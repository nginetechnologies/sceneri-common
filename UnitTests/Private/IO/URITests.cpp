#include <Common/Memory/New.h>

#include <Common/Tests/UnitTest.h>

#include <Common/IO/URIView.h>
#include <Common/IO/URI.h>

#include <Common/Memory/Containers/Format/String.h>

namespace ngine::Tests
{
	UNIT_TEST(URI, Queries)
	{
		IO::URIView uri(MAKE_URI("https://www.sceneri.com/assets/my asset/1.tex.nasset?arg1=value1&arg2=value2"));
		EXPECT_EQ(uri.GetRightMostExtension(), MAKE_URI(".nasset"));
		EXPECT_EQ(uri.GetLeftMostExtension(), MAKE_URI(".tex"));
		EXPECT_EQ(uri.GetAllExtensions(), MAKE_URI(".tex.nasset"));
		EXPECT_EQ(uri.GetWithoutExtensions(), MAKE_URI("https://www.sceneri.com/assets/my asset/1"));
		EXPECT_EQ(uri.GetFileNameWithoutExtensions(), MAKE_URI("1"));
		EXPECT_EQ(uri.GetQueryString(), MAKE_URI("?arg1=value1&arg2=value2"));
		EXPECT_EQ(uri.GetWithoutQueryString(), MAKE_URI("https://www.sceneri.com/assets/my asset/1.tex.nasset"));
	}
}
