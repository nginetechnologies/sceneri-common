#include <Common/Memory/New.h>

#include <Common/Tests/UnitTest.h>

#include <Common/Memory/Containers/Array.h>

namespace ngine::Tests
{
	UNIT_TEST(Array, InitializeAll)
	{
		const Array<int, 2> view = {Memory::InitializeAll, 2};
		EXPECT_EQ(view[0], 2);
		EXPECT_EQ(view[1], 2);
	}

	UNIT_TEST(Array, BracedInitialization)
	{
		const Array<int, 3> view = {1, 2, 3};
		EXPECT_EQ(view[0], 1);
		EXPECT_EQ(view[1], 2);
		EXPECT_EQ(view[2], 3);
	}

	UNIT_TEST(Array, CopyConstruct)
	{
		const Array<int, 3> firstView = {1, 2, 3};
		const Array<int, 3> copiedView(firstView);
		EXPECT_EQ(copiedView[0], 1);
		EXPECT_EQ(copiedView[1], 2);
		EXPECT_EQ(copiedView[2], 3);
	}

	UNIT_TEST(Array, MoveConstruct)
	{
		Array<int, 3> firstView = {1, 2, 3};
		const Array<int, 3> moveView(Move(firstView));
		EXPECT_EQ(moveView[0], 1);
		EXPECT_EQ(moveView[1], 2);
		EXPECT_EQ(moveView[2], 3);
	}

	/*UNIT_TEST(Array, CopyConstructIntoConst)
	{
	  const Array<int, 3> firstView = { 1, 2, 3 };
	  const Array<const int, 3> copiedView(firstView);
	  EXPECT_EQ(copiedView[0], 1);
	  EXPECT_EQ(copiedView[1], 2);
	  EXPECT_EQ(copiedView[2], 3);
	}

	UNIT_TEST(Array, MoveConstructIntoConst)
	{
	  Array<int, 3> firstView = { 1, 2, 3 };
	  const Array<const int, 3> moveView(Move(firstView));
	  EXPECT_EQ(moveView[0], 1);
	  EXPECT_EQ(moveView[1], 2);
	  EXPECT_EQ(moveView[2], 3);
	}*/
}
