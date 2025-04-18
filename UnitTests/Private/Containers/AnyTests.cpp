#include <Common/Memory/New.h>

#include <Common/Tests/UnitTest.h>

#include <Common/Memory/Any.h>
#include <Common/Memory/AnyView.h>
#include <Common/Memory/Containers/String.h>
#include <Common/Reflection/GenericType.h>

#include <Common/Serialization/Reader.h>
#include <Common/Serialization/Writer.h>

namespace ngine::Tests
{
	UNIT_TEST(Any, DefaultConstruct)
	{
		Any any;
		EXPECT_TRUE(any.IsInvalid());
		EXPECT_FALSE(any.IsValid());
		EXPECT_FALSE(any.Get<bool>().IsValid());
	}

	UNIT_TEST(Any, MoveConstruct)
	{
		Any any(String("test"));
		EXPECT_TRUE(any.IsValid());
		const Optional<const String*> value = any.Get<String>();
		EXPECT_TRUE(value.IsValid());
		EXPECT_EQ(*value, "test");
	}

	UNIT_TEST(Any, CopyConstruct)
	{
		const String initialValue{"test"};
		Any any(initialValue);
		EXPECT_TRUE(any.IsValid());
		EXPECT_TRUE(any.Is<String>());
		const Optional<const String*> value = any.Get<String>();
		EXPECT_TRUE(value.IsValid());
		EXPECT_EQ(*value, "test");
	}

	UNIT_TEST(Any, Move)
	{
		Any any(String("test"));
		Any movedAny = Move(any);
		EXPECT_FALSE(any.IsValid());
		EXPECT_FALSE(any.Is<String>());
		EXPECT_TRUE(movedAny.IsValid());
		EXPECT_TRUE(movedAny.Is<String>());
		EXPECT_FALSE(any.Get<String>().IsValid());
		EXPECT_TRUE(movedAny.Get<String>().IsValid());
		EXPECT_EQ(*movedAny.Get<String>(), "test");
	}

	UNIT_TEST(Any, Copy)
	{
		Any any(String("test"));
		Any movedAny = any;
		EXPECT_TRUE(any.IsValid());
		EXPECT_TRUE(any.Is<String>());
		EXPECT_TRUE(movedAny.IsValid());
		EXPECT_TRUE(movedAny.Is<String>());
		EXPECT_TRUE(any.Get<String>().IsValid());
		EXPECT_EQ(*any.Get<String>(), "test");
		EXPECT_TRUE(movedAny.Get<String>().IsValid());
		EXPECT_EQ(*movedAny.Get<String>(), "test");
	}

	UNIT_TEST(Any, Comparison)
	{
		const Any value = String("test");
		EXPECT_EQ(value, Any(String("test")));
		EXPECT_NE(value, Any(String("nada")));
	}

	UNIT_TEST(AnyView, Construct)
	{
		int value = 1337;
		AnyView view = value;
		EXPECT_EQ(view.GetExpected<int>(), 1337);
		value = 9001;
		EXPECT_EQ(view.GetExpected<int>(), 9001);
		int otherValue = 5;
		view = otherValue;
		EXPECT_EQ(view.GetExpected<int>(), 5);
		EXPECT_EQ(value, 9001);
	}

	UNIT_TEST(ConstAnyView, Construct)
	{
		int value = 1337;
		ConstAnyView view = value;
		EXPECT_EQ(view.GetExpected<int>(), 1337);
		value = 9001;
		EXPECT_EQ(view.GetExpected<int>(), 9001);
		int otherValue = 5;
		view = otherValue;
		EXPECT_EQ(view.GetExpected<int>(), 5);
		EXPECT_EQ(value, 9001);
	}

	UNIT_TEST(AnyView, FromAny)
	{
		Any value = 5;
		AnyView valueView = value;
		EXPECT_TRUE(valueView.IsValid());
		EXPECT_TRUE(valueView.Is<int>());
		EXPECT_EQ(*valueView.Get<int>(), 5);
	}

	UNIT_TEST(AnyView, FromAnyConst)
	{
		Any value = 5;
		ConstAnyView valueView = value;
		EXPECT_TRUE(valueView.IsValid());
		EXPECT_TRUE(valueView.Is<int>());
		EXPECT_EQ(*valueView.Get<int>(), 5);
	}
}
