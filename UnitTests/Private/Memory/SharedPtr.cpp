#include <Common/Memory/SharedPtr.h>
#include <Common/Memory/Move.h>

#include <Common/Tests/UnitTest.h>

namespace ngine::Tests
{
	class Foo
	{
	public:
		static int32 sCount;
	public:
		Foo()
		{
			++sCount;
		}
		~Foo()
		{
			--sCount;
		}
	};
	int32 Foo::sCount = 0;

	UNIT_TEST(Memory, SharedPtrCopy)
	{
		Foo::sCount = 0;
		{
			SharedPtr<Foo> pBar = SharedPtr<Foo>::Make();
			EXPECT_EQ(true, pBar.IsValid());
			EXPECT_EQ(1, Foo::sCount);
			{
				SharedPtr<Foo> pBarTmp = SharedPtr<Foo>::Make();
				EXPECT_EQ(true, pBarTmp.IsValid());
				EXPECT_EQ(2, Foo::sCount);

				SharedPtr<Foo> pBar2 = pBar;
				EXPECT_EQ(true, pBar2.IsValid());
				EXPECT_EQ(2, Foo::sCount);
			}
			EXPECT_EQ(1, Foo::sCount);
		}
		EXPECT_EQ(0, Foo::sCount);
	}

	UNIT_TEST(Memory, SharedPtrMove)
	{
		Foo::sCount = 0;
		{
			SharedPtr<Foo> pBar = SharedPtr<Foo>::Make();
			EXPECT_EQ(true, pBar.IsValid());
			EXPECT_EQ(1, Foo::sCount);
			{
				SharedPtr<Foo> pBarTmp = SharedPtr<Foo>::Make();
				EXPECT_EQ(true, pBarTmp.IsValid());
				EXPECT_EQ(2, Foo::sCount);

				SharedPtr<Foo> pBar2 = Move(pBar);
				EXPECT_EQ(true, pBar2.IsValid());
				EXPECT_EQ(2, Foo::sCount);
			}
			EXPECT_EQ(0, Foo::sCount);
		}
		EXPECT_EQ(0, Foo::sCount);
	}
}
