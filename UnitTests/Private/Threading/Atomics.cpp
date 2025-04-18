#include <Common/Memory/New.h>

#include <Common/Tests/UnitTest.h>

#include <Common/Threading/AtomicInteger.h>

namespace ngine::Tests
{
	template<typename Type>
	void BasicArithmeticGeneric()
	{
		Threading::Atomic<Type> value = 1;
		EXPECT_EQ(value, (Type)1);
		value -= 1;
		EXPECT_EQ(value, (Type)0);
		const Type valueBeforeAddition = value.FetchAdd(2);
		EXPECT_EQ(valueBeforeAddition, (Type)0);
		EXPECT_EQ(value, (Type)2);
		const Type valueBeforeSubtract = value.FetchSubtract(2);
		EXPECT_EQ(valueBeforeSubtract, (Type)2);
		EXPECT_EQ(value, (Type)0);

		const Type valueBeforePostIncrement = value++;
		EXPECT_EQ(valueBeforePostIncrement, (Type)0);
		EXPECT_EQ(value, (Type)1);

		const Type valueBeforePostDecrement = value--;
		EXPECT_EQ(valueBeforePostDecrement, (Type)1);
		EXPECT_EQ(value, (Type)0);

		const Type valueAfterPreIncrement = ++value;
		EXPECT_EQ(valueAfterPreIncrement, (Type)1);
		EXPECT_EQ(value, (Type)1);

		const Type valueAfterPreDecrement = --value;
		EXPECT_EQ(valueAfterPreDecrement, (Type)0);
		EXPECT_EQ(value, (Type)0);

		value.AssignMax(10);
		EXPECT_EQ(value, (Type)10);
	}

	UNIT_TEST(Atomics, BasicArithmetic)
	{
		BasicArithmeticGeneric<uint8>();
		BasicArithmeticGeneric<uint16>();
		BasicArithmeticGeneric<uint32>();
		BasicArithmeticGeneric<uint64>();
		BasicArithmeticGeneric<int8>();
		BasicArithmeticGeneric<int16>();
		BasicArithmeticGeneric<int32>();
		BasicArithmeticGeneric<int64>();
	}

	template<typename Type>
	void BasicExchangeGeneric()
	{
		Threading::Atomic<Type> value = 1;
		{
			Type expected = 2;
			EXPECT_FALSE(value.CompareExchangeStrong(expected, (Type)2));
			EXPECT_EQ(expected, (Type)1);

			expected = 1;
			EXPECT_TRUE(value.CompareExchangeStrong(expected, (Type)2));
			EXPECT_EQ(expected, (Type)1);
			EXPECT_EQ(value, (Type)2);

			expected = 2;
			while (!value.CompareExchangeWeak(expected, (Type)1))
				;
			EXPECT_EQ(expected, (Type)2);
			EXPECT_EQ(value, (Type)1);
		}

		const Type previousValue = value.Exchange(2);
		EXPECT_EQ(value, (Type)2);
		EXPECT_EQ(previousValue, (Type)1);
	}

	UNIT_TEST(Atomics, BasicExchange)
	{
		BasicExchangeGeneric<uint8>();
		BasicExchangeGeneric<uint16>();
		BasicExchangeGeneric<uint32>();
		BasicExchangeGeneric<uint64>();
		BasicExchangeGeneric<int8>();
		BasicExchangeGeneric<int16>();
		BasicExchangeGeneric<int32>();
		BasicExchangeGeneric<int64>();
	}

	template<typename Type>
	void BasicBitwiseGeneric()
	{
		enum class Flags : Type
		{
			First = 1 << 0,
			Second = 1 << 1,
			Third = 1 << 2,
			Fourth = 1 << 3,
			Fifth = 1 << 4,
			Sixth = 1 << 5,
			Seventh = 1 << 6,
			All = First | Second | Third | Fourth | Fifth | Sixth | Seventh
		};

		Threading::Atomic<Type> value = (Type)Flags::All;
		EXPECT_EQ(value, (Type)Flags::All);

		const Type valueBeforeAnd = value.FetchAnd((Type) ~(Type)Flags::Second);
		EXPECT_EQ(valueBeforeAnd, (Type)Flags::All);
		EXPECT_EQ(value, (Type)((Type)Flags::All & ~(Type)Flags::Second));

		const Type valueBeforeOr = value.FetchOr((Type)Flags::Second);
		EXPECT_EQ(valueBeforeOr, ((Type)Flags::All & ~(Type)Flags::Second));
		EXPECT_EQ(value, (Type)Flags::All);

		const Type valueBeforeXor = value.FetchXor((Type)Flags::All);
		EXPECT_EQ(valueBeforeXor, (Type)Flags::All);
		EXPECT_EQ(value, (Type)0);
	}

	UNIT_TEST(Atomics, BasicBitwise)
	{
		BasicBitwiseGeneric<uint8>();
		BasicBitwiseGeneric<uint16>();
		BasicBitwiseGeneric<uint32>();
		BasicBitwiseGeneric<uint64>();
		BasicBitwiseGeneric<int8>();
		BasicBitwiseGeneric<int16>();
		BasicBitwiseGeneric<int32>();
		BasicBitwiseGeneric<int64>();
	}
}
