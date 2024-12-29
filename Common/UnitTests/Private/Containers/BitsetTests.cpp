#include <Common/Memory/New.h>

#include <Common/Tests/UnitTest.h>

#include <Common/Memory/Bitset.h>
#include <Common/Memory/AtomicBitset.h>
#include <Common/Memory/CompressedBitset.h>
#include <Common/Memory/Containers/FlatVector.h>

namespace ngine::Tests
{
	UNIT_TEST(Bitset, DefaultConstruct)
	{
		constexpr uint16 count = 507;
		Bitset<count> bits;
		EXPECT_TRUE(bits.AreNoneSet());
		EXPECT_FALSE(bits.AreAnySet());
		EXPECT_TRUE(bits.AreAnyNotSet());
		EXPECT_FALSE(bits.AreAllSet());
		for (uint16 i = 0; i < count; ++i)
		{
			EXPECT_FALSE(bits.IsSet(i));
		}
		EXPECT_EQ(bits.GetNumberOfSetBits(), 0u);
		EXPECT_FALSE(bits.GetFirstSetIndex().IsValid());
		EXPECT_FALSE(bits.GetLastSetIndex().IsValid());
		EXPECT_FALSE(bits.GetNextSetIndex(0u));

		for ([[maybe_unused]] const uint16 index : bits.GetSetBitsIterator())
		{
			// Should never be reached
			EXPECT_TRUE(false);
		}
	}

	UNIT_TEST(Bitset, SetAllConstruct)
	{
		constexpr uint16 count = 507;
		Bitset<count> bits{Memory::SetAll};
		EXPECT_FALSE(bits.AreNoneSet());
		EXPECT_TRUE(bits.AreAnySet());
		EXPECT_FALSE(bits.AreAnyNotSet());
		EXPECT_TRUE(bits.AreAllSet());
		for (uint16 i = 0; i < count; ++i)
		{
			EXPECT_TRUE(bits.IsSet(i));
		}
		EXPECT_EQ(bits.GetNumberOfSetBits(), count);
		EXPECT_TRUE(bits.GetFirstSetIndex().IsValid());
		EXPECT_EQ(*bits.GetFirstSetIndex(), 0);
		EXPECT_TRUE(bits.GetLastSetIndex().IsValid());
		EXPECT_EQ(*bits.GetLastSetIndex(), count - 1);
		EXPECT_TRUE(bits.GetNextSetIndex(0u));
	}

	UNIT_TEST(Bitset, GetFirstSetIndex)
	{
		constexpr uint16 count = 507;
		Bitset<count> bits;
		EXPECT_FALSE(bits.GetFirstSetIndex().IsValid());

		bits.Set(506);
		EXPECT_EQ(*bits.GetFirstSetIndex(), 506);

		bits.Set(496);
		EXPECT_EQ(*bits.GetFirstSetIndex(), 496);

		bits.Set(1);
		EXPECT_EQ(*bits.GetFirstSetIndex(), 1);

		bits.Set(0);
		EXPECT_EQ(*bits.GetFirstSetIndex(), 0);
	}

	UNIT_TEST(Bitset, GetLastSetIndex)
	{
		constexpr uint16 count = 507;
		Bitset<count> bits;
		EXPECT_FALSE(bits.GetLastSetIndex().IsValid());

		bits.Set(0);
		EXPECT_EQ(*bits.GetLastSetIndex(), 0);

		bits.Set(1);
		EXPECT_EQ(*bits.GetLastSetIndex(), 1);

		bits.Set(Bitset<count>::BitsPerBlock);
		EXPECT_EQ(*bits.GetLastSetIndex(), Bitset<count>::BitsPerBlock);

		bits.Set(496);
		EXPECT_EQ(*bits.GetLastSetIndex(), 496);

		bits.Set(506);
		EXPECT_EQ(*bits.GetLastSetIndex(), 506);
	}

	UNIT_TEST(Bitset, GetSetBitsIterator)
	{
		{
			constexpr uint16 count = 507;
			Bitset<count> bits;
			bits.Set(0);
			bits.Set(2);
			bits.Set(64);
			bits.Set(496);
			bits.Set(506);

			Bitset<count> iteratedBits;
			FlatVector<uint16, 5> order;
			for (const uint16 bitIndex : bits.GetSetBitsIterator())
			{
				iteratedBits.Set(bitIndex);
				order.EmplaceBack(bitIndex);
			}

			EXPECT_EQ(iteratedBits.GetNumberOfSetBits(), 5);
			EXPECT_TRUE(iteratedBits.IsSet(0));
			EXPECT_TRUE(iteratedBits.IsSet(2));
			EXPECT_TRUE(iteratedBits.IsSet(64));
			EXPECT_TRUE(iteratedBits.IsSet(496));
			EXPECT_TRUE(iteratedBits.IsSet(506));

			const Array<uint16, 5> expectedOrder{(uint16)0u, (uint16)2u, (uint16)64u, (uint16)496, (uint16)506};
			EXPECT_EQ(order.GetView(), expectedOrder.GetDynamicView());
		}
		{
			constexpr uint16 count = 507;
			Bitset<count> bits;
			bits.Set(0);
			bits.Set(2);
			bits.Set(64);
			bits.Set(496);
			bits.Set(506);

			const Bitset<count>::SetBitsIterator iterator = bits.GetSetBitsIterator();
			// EXPECT_EQ(iterator, bits.GetSetBitsIterator(0, count));
			EXPECT_EQ(*bits.GetSetBitsIterator(1, count - 1).begin(), 2);

			Bitset<count>::SetBitsIterator::Iterator it = iterator.begin();
			Bitset<count>::SetBitsIterator::Iterator endIt = iterator.end();
			EXPECT_TRUE(it < endIt);
			EXPECT_TRUE(it != endIt);
			EXPECT_FALSE(it == endIt);
			EXPECT_EQ(*it, 0);
			EXPECT_EQ(it, iterator.begin() + 0);

			++it;
			EXPECT_TRUE(it < endIt);
			EXPECT_TRUE(it != endIt);
			EXPECT_FALSE(it == endIt);
			EXPECT_EQ(*it, 2);
			EXPECT_EQ(it, iterator.begin() + 1);

			++it;
			EXPECT_TRUE(it < endIt);
			EXPECT_TRUE(it != endIt);
			EXPECT_FALSE(it == endIt);
			EXPECT_EQ(*it, 64);
			EXPECT_EQ(it, iterator.begin() + 2);

			++it;
			EXPECT_TRUE(it < endIt);
			EXPECT_TRUE(it != endIt);
			EXPECT_FALSE(it == endIt);
			EXPECT_EQ(*it, 496);
			EXPECT_EQ(it, iterator.begin() + 3);

			++it;
			EXPECT_TRUE(it < endIt);
			EXPECT_TRUE(it != endIt);
			EXPECT_FALSE(it == endIt);
			EXPECT_EQ(*it, 506);
			EXPECT_EQ(it, iterator.begin() + 4);

			++it;
			EXPECT_FALSE(it < endIt);
			EXPECT_FALSE(it != endIt);
			EXPECT_TRUE(it == endIt);
			EXPECT_EQ(it, iterator.begin() + 5);

			++it;
			EXPECT_FALSE(it < endIt);
		}
		{
			constexpr uint8 count = 255;
			Bitset<count> bits;
			bits.Set(0);
			const Bitset<count>::SetBitsIterator iterator = bits.GetSetBitsIterator();
			Bitset<count>::SetBitsIterator::Iterator it = iterator.begin();
			Bitset<count>::SetBitsIterator::Iterator endIt = iterator.end();
			EXPECT_TRUE(it < endIt);
			EXPECT_TRUE(it != endIt);
			EXPECT_FALSE(it == endIt);
			EXPECT_EQ(*it, 0);
			EXPECT_EQ(it, iterator.begin() + 0);

			++it;
			EXPECT_FALSE(it < endIt);
			EXPECT_FALSE(it != endIt);
			EXPECT_TRUE(it == endIt);
			EXPECT_EQ(it, iterator.begin() + 1);
		}
	}

	UNIT_TEST(Bitset, GetSetBitsReverseIterator)
	{
		constexpr uint16 count = 507;
		Bitset<count> bits;
		bits.Set(0);
		bits.Set(1);
		bits.Set(64);
		bits.Set(496);
		bits.Set(506);

		Bitset<count> iteratedBits;
		FlatVector<uint16, 5> order;
		for (const uint16 bitIndex : bits.GetSetBitsReverseIterator())
		{
			iteratedBits.Set(bitIndex);
			order.EmplaceBack(bitIndex);
		}

		EXPECT_EQ(iteratedBits.GetNumberOfSetBits(), 5);
		EXPECT_TRUE(iteratedBits.IsSet(0));
		EXPECT_TRUE(iteratedBits.IsSet(1));
		EXPECT_TRUE(iteratedBits.IsSet(64));
		EXPECT_TRUE(iteratedBits.IsSet(496));
		EXPECT_TRUE(iteratedBits.IsSet(506));

		const Array<uint16, 5> expectedOrder{(uint16)506u, (uint16)496u, (uint16)64u, (uint16)1u, (uint16)0u};
		EXPECT_EQ(order.GetView(), expectedOrder.GetDynamicView());
	}

	UNIT_TEST(Bitset, AreNoneSet)
	{
		{
			constexpr uint16 count = 507;
			Bitset<count> leftBits;
			EXPECT_TRUE(leftBits.AreNoneSet());
			Bitset<count> rightBits;
			EXPECT_TRUE(leftBits.AreNoneSet(rightBits));
			rightBits.Set(0);
			EXPECT_TRUE(leftBits.AreNoneSet(rightBits));
			rightBits.Set(496);
			EXPECT_TRUE(leftBits.AreNoneSet(rightBits));
			rightBits.Set(506);
			EXPECT_TRUE(leftBits.AreNoneSet(rightBits));
		}
		{
			constexpr uint16 count = 507;
			Bitset<count> leftBits;
			leftBits.Set(0);
			EXPECT_FALSE(leftBits.AreNoneSet());
			leftBits.Set(496);
			EXPECT_FALSE(leftBits.AreNoneSet());
			leftBits.Set(506);
			EXPECT_FALSE(leftBits.AreNoneSet());

			Bitset<count> rightBits;
			EXPECT_TRUE(leftBits.AreNoneSet(rightBits));
			rightBits.Set(1);
			EXPECT_TRUE(leftBits.AreNoneSet(rightBits));
			rightBits.Set(2);
			EXPECT_TRUE(leftBits.AreNoneSet(rightBits));
			rightBits.Set(497);
			EXPECT_TRUE(leftBits.AreNoneSet(rightBits));
			rightBits.Set(496);
			EXPECT_FALSE(leftBits.AreNoneSet(rightBits));
			rightBits.Set(506);
			EXPECT_FALSE(leftBits.AreNoneSet(rightBits));
		}
	}

	UNIT_TEST(Bitset, AreAnySet)
	{
		{
			constexpr uint16 count = 507;
			Bitset<count> leftBits;
			EXPECT_FALSE(leftBits.AreAnySet());
			Bitset<count> rightBits;
			EXPECT_FALSE(leftBits.AreAnySet(rightBits));
			rightBits.Set(0);
			EXPECT_FALSE(leftBits.AreAnySet(rightBits));
			rightBits.Set(496);
			EXPECT_FALSE(leftBits.AreAnySet(rightBits));
			rightBits.Set(506);
			EXPECT_FALSE(leftBits.AreAnySet(rightBits));
		}
		{
			constexpr uint16 count = 507;
			Bitset<count> leftBits;
			leftBits.Set(0);
			EXPECT_TRUE(leftBits.AreAnySet());
			leftBits.Set(496);
			EXPECT_TRUE(leftBits.AreAnySet());
			leftBits.Set(506);
			EXPECT_TRUE(leftBits.AreAnySet());

			Bitset<count> rightBits;
			EXPECT_FALSE(leftBits.AreAnySet(rightBits));
			rightBits.Set(1);
			EXPECT_FALSE(leftBits.AreAnySet(rightBits));
			rightBits.Set(2);
			EXPECT_FALSE(leftBits.AreAnySet(rightBits));
			rightBits.Set(497);
			EXPECT_FALSE(leftBits.AreAnySet(rightBits));
			rightBits.Set(496);
			EXPECT_TRUE(leftBits.AreAnySet(rightBits));
			rightBits.Set(506);
			EXPECT_TRUE(leftBits.AreAnySet(rightBits));
		}
	}

	UNIT_TEST(Bitset, AreAnyNotSet)
	{
		{
			constexpr uint16 count = 507;
			Bitset<count> leftBits;
			Bitset<count> rightBits;
			EXPECT_FALSE(leftBits.AreAnyNotSet(rightBits));
			rightBits.Set(0);
			EXPECT_TRUE(leftBits.AreAnyNotSet(rightBits));
			rightBits.Set(496);
			EXPECT_TRUE(leftBits.AreAnyNotSet(rightBits));
			rightBits.Set(506);
			EXPECT_TRUE(leftBits.AreAnyNotSet(rightBits));
		}
		{
			constexpr uint16 count = 507;
			Bitset<count> leftBits;
			leftBits.Set(0);
			leftBits.Set(496);
			leftBits.Set(506);

			Bitset<count> rightBits;
			EXPECT_FALSE(leftBits.AreAnyNotSet(rightBits));
			rightBits.Set(0);
			EXPECT_FALSE(leftBits.AreAnyNotSet(rightBits));
			rightBits.Set(496);
			EXPECT_FALSE(leftBits.AreAnyNotSet(rightBits));
			rightBits.Set(1);
			EXPECT_TRUE(leftBits.AreAnyNotSet(rightBits));
			rightBits.Set(2);
			EXPECT_TRUE(leftBits.AreAnyNotSet(rightBits));
			rightBits.Set(497);
			EXPECT_TRUE(leftBits.AreAnyNotSet(rightBits));
			rightBits.Set(506);
			EXPECT_TRUE(leftBits.AreAnyNotSet(rightBits));
		}
	}

	UNIT_TEST(Bitset, AreAllSet)
	{
		{
			constexpr uint16 count = 507;
			Bitset<count> leftBits;
			Bitset<count> rightBits;
			EXPECT_TRUE(leftBits.AreAllSet(rightBits));
			rightBits.Set(0);
			EXPECT_FALSE(leftBits.AreAllSet(rightBits));
			rightBits.Set(496);
			EXPECT_FALSE(leftBits.AreAllSet(rightBits));
			rightBits.Set(506);
			EXPECT_FALSE(leftBits.AreAllSet(rightBits));
		}
		{
			constexpr uint16 count = 507;
			Bitset<count> leftBits;
			leftBits.Set(0);
			leftBits.Set(496);
			leftBits.Set(506);

			Bitset<count> rightBits;
			EXPECT_TRUE(leftBits.AreAllSet(rightBits));
			rightBits.Set(496);
			EXPECT_TRUE(leftBits.AreAllSet(rightBits));
			rightBits.Set(506);
			EXPECT_TRUE(leftBits.AreAllSet(rightBits));
			rightBits.Set(1);
			EXPECT_FALSE(leftBits.AreAllSet(rightBits));
			rightBits.Set(0);
			EXPECT_FALSE(leftBits.AreAllSet(rightBits));
			rightBits.Set(504);
			EXPECT_FALSE(leftBits.AreAllSet(rightBits));
		}
	}

	UNIT_TEST(Bitset, SetAll)
	{
		{
			constexpr uint16 count = 507;
			Bitset<count> bits;
			EXPECT_EQ(bits.GetNumberOfSetBits(), 0);
			bits.SetAll();
			EXPECT_EQ(bits.GetNumberOfSetBits(), count);
			for (uint16 index = 0; index < count; ++index)
			{
				EXPECT_TRUE(bits.IsSet(index));
			}
		}
		{
			constexpr uint16 count = 512;
			Bitset<count> bits;
			EXPECT_EQ(bits.GetNumberOfSetBits(), 0);
			bits.SetAll();
			EXPECT_EQ(bits.GetNumberOfSetBits(), count);
			for (uint16 index = 0; index < count; ++index)
			{
				EXPECT_TRUE(bits.IsSet(index));
			}
		}
		{
			constexpr uint16 count = 507;
			Bitset<count> bits;
			bits.SetAll(Math::Range<uint16>::Make(0, 0));
			for (uint16 index = 0; index < count; ++index)
			{
				EXPECT_FALSE(bits.IsSet(index));
			}
		}
		{
			constexpr uint16 count = 507;
			Bitset<count> bits;
			bits.SetAll(Math::Range<uint16>::Make(0, 1));
			EXPECT_TRUE(bits.IsSet(0));
			for (uint16 index = 1; index < count; ++index)
			{
				EXPECT_FALSE(bits.IsSet(index));
			}
		}

		{
			constexpr uint16 count = 507;
			Bitset<count> bits;
			bits.SetAll(Math::Range<uint16>::Make(count - 1, 1));
			for (uint16 index = 0; index < (count - 1); ++index)
			{
				EXPECT_FALSE(bits.IsSet(index));
			}
			EXPECT_TRUE(bits.IsSet(count - 1));
		}
		{
			constexpr uint16 count = 512;
			Bitset<count> bits;
			bits.SetAll(Math::Range<uint16>::Make(0, count));
			for (uint16 index = 0; index < count; ++index)
			{
				EXPECT_TRUE(bits.IsSet(index));
			}
		}
		{
			constexpr uint16 count = 507;
			Bitset<count> bits;
			bits.SetAll(Math::Range<uint16>::Make(0, count));
			for (uint16 index = 0; index < count; ++index)
			{
				EXPECT_TRUE(bits.IsSet(index));
			}
		}

		{
			constexpr uint16 count = 507;
			Bitset<count> bits;
			bits.SetAll(Math::Range<uint16>::Make(64, 1));
			for (uint16 index = 0; index < 64; ++index)
			{
				EXPECT_FALSE(bits.IsSet(index));
			}
			EXPECT_TRUE(bits.IsSet(64));
			for (uint16 index = 65; index < count; ++index)
			{
				EXPECT_FALSE(bits.IsSet(index));
			}
		}
		{
			constexpr uint16 count = 513;
			Bitset<count> bits;
			bits.SetAll(Math::Range<uint16>::Make(1, count - 2));
			EXPECT_FALSE(bits.IsSet(0));
			for (uint16 index = 1; index < count - 1; ++index)
			{
				EXPECT_TRUE(bits.IsSet(index));
			}
			EXPECT_FALSE(bits.IsSet(count - 1));
		}
		{
			constexpr uint16 count = 517;
			constexpr uint16 setCount = 94;
			Bitset<count> bits;
			bits.SetAll(Math::Range<uint16>::Make(0, setCount));
			EXPECT_EQ(bits.GetNumberOfSetBits(), setCount);
			for (uint16 index = 0; index < setCount; ++index)
			{
				EXPECT_TRUE(bits.IsSet(index));
			}
		}
		{
			constexpr uint16 count = 517;
			constexpr uint16 setCount = 94;
			Bitset<count> bits;
			bits.SetAll(Math::Range<uint16>::MakeStartToEnd(count - setCount, count - 1));
			EXPECT_EQ(bits.GetNumberOfSetBits(), setCount);
			for (uint16 index = count - setCount; index < count; ++index)
			{
				EXPECT_TRUE(bits.IsSet(index));
			}
		}
	}

	UNIT_TEST(Bitset, ClearAll)
	{
		{
			constexpr uint16 count = 507;
			Bitset<count> bits{Memory::SetAll};
			EXPECT_EQ(bits.GetNumberOfSetBits(), count);
			bits.ClearAll();
			EXPECT_EQ(bits.GetNumberOfSetBits(), 0);
			for (uint16 index = 0; index < count; ++index)
			{
				EXPECT_FALSE(bits.IsSet(index));
			}
		}
		{
			constexpr uint16 count = 512;
			Bitset<count> bits{Memory::SetAll};
			EXPECT_EQ(bits.GetNumberOfSetBits(), count);
			bits.ClearAll();
			EXPECT_EQ(bits.GetNumberOfSetBits(), 0);
			for (uint16 index = 0; index < count; ++index)
			{
				EXPECT_FALSE(bits.IsSet(index));
			}
		}
		{
			constexpr uint16 count = 507;
			Bitset<count> bits{Memory::SetAll};
			bits.ClearAll(Math::Range<uint16>::Make(0, 0));
			for (uint16 index = 0; index < count; ++index)
			{
				EXPECT_TRUE(bits.IsSet(index));
			}
		}
		{
			constexpr uint16 count = 507;
			Bitset<count> bits{Memory::SetAll};
			bits.ClearAll(Math::Range<uint16>::Make(0, 1));
			EXPECT_FALSE(bits.IsSet(0));
			for (uint16 index = 1; index < count; ++index)
			{
				EXPECT_TRUE(bits.IsSet(index));
			}
		}

		{
			constexpr uint16 count = 507;
			Bitset<count> bits{Memory::SetAll};
			bits.ClearAll(Math::Range<uint16>::Make(count - 1, 1));
			for (uint16 index = 0; index < (count - 1); ++index)
			{
				EXPECT_TRUE(bits.IsSet(index));
			}
			EXPECT_FALSE(bits.IsSet(count - 1));
		}
		{
			constexpr uint16 count = 512;
			Bitset<count> bits;
			bits.SetAll(Math::Range<uint16>::Make(0, count));
			for (uint16 index = 0; index < count; ++index)
			{
				EXPECT_TRUE(bits.IsSet(index));
			}
			bits.ClearAll(Math::Range<uint16>::Make(0, count));
			for (uint16 index = 0; index < count; ++index)
			{
				EXPECT_FALSE(bits.IsSet(index));
			}
		}
		{
			constexpr uint16 count = 507;
			Bitset<count> bits;
			bits.SetAll(Math::Range<uint16>::Make(0, count));
			for (uint16 index = 0; index < count; ++index)
			{
				EXPECT_TRUE(bits.IsSet(index));
			}
			bits.ClearAll(Math::Range<uint16>::Make(0, count));
			for (uint16 index = 0; index < count; ++index)
			{
				EXPECT_FALSE(bits.IsSet(index));
			}
		}
		{
			constexpr uint16 count = 507;
			Bitset<count> bits{Memory::SetAll};
			bits.ClearAll(Math::Range<uint16>::Make(64, 1));
			for (uint16 index = 0; index < 64; ++index)
			{
				EXPECT_TRUE(bits.IsSet(index));
			}
			EXPECT_FALSE(bits.IsSet(64));
			for (uint16 index = 65; index < count; ++index)
			{
				EXPECT_TRUE(bits.IsSet(index));
			}
		}
		{
			constexpr uint16 count = 513;
			Bitset<count> bits;
			bits.SetAll(Math::Range<uint16>::Make(0, count));
			for (uint16 index = 0; index < count; ++index)
			{
				EXPECT_TRUE(bits.IsSet(index));
			}
			EXPECT_TRUE(bits.IsSet(0));
			for (uint16 index = 1; index < count - 1; ++index)
			{
				EXPECT_TRUE(bits.IsSet(index));
			}
			EXPECT_TRUE(bits.IsSet(count - 1));
		}
		{
			constexpr uint16 count = 5183;
			Bitset<count> bits{Memory::SetAll};
			bits.ClearAll(Math::Range<uint16>::Make(4030, (4033 - 4030) + 1));
			EXPECT_FALSE(bits.IsSet(4033));
		}

		{
			constexpr uint16 count = 517;
			constexpr uint16 clearedCount = 94;
			Bitset<count> bits{Memory::SetAll};
			EXPECT_EQ(bits.GetNumberOfSetBits(), count);
			bits.ClearAll(Math::Range<uint16>::Make(0, clearedCount));
			EXPECT_EQ(bits.GetNumberOfSetBits(), count - clearedCount);
			for (uint16 index = 0; index < clearedCount; ++index)
			{
				EXPECT_TRUE(bits.IsNotSet(index));
			}
		}
		{
			constexpr uint16 count = 517;
			constexpr uint16 clearedCount = 94;
			Bitset<count> bits{Memory::SetAll};
			bits.ClearAll(Math::Range<uint16>::MakeStartToEnd(count - clearedCount, count - 1));
			EXPECT_EQ(bits.GetNumberOfSetBits(), count - clearedCount);
			for (uint16 index = count - clearedCount; index < count; ++index)
			{
				Assert(bits.IsNotSet(index));
				EXPECT_TRUE(bits.IsNotSet(index));
			}
		}
	}

	UNIT_TEST(AtomicBitset, SetAndClear)
	{
		{
			constexpr uint16 count = 507;
			Threading::AtomicBitset<count> bits;
			EXPECT_FALSE(bits.IsSet(1));
			EXPECT_TRUE(bits.Set(1));
			EXPECT_TRUE(bits.IsSet(1));

			EXPECT_FALSE(bits.IsSet(2));
			EXPECT_TRUE(bits.Set(2));
			EXPECT_TRUE(bits.IsSet(2));

			EXPECT_FALSE(bits.Set(2));
			EXPECT_TRUE(bits.IsSet(2));

			EXPECT_TRUE(bits.Clear(2));
			EXPECT_FALSE(bits.IsSet(2));
			EXPECT_FALSE(bits.Clear(2));
			EXPECT_FALSE(bits.IsSet(2));

			EXPECT_FALSE(bits.Clear(3));
		}
	}

	UNIT_TEST(CompressedBitset, DefaultConstruct)
	{
		constexpr uint16 count = 507;
		CompressedBitset<count> bits;
		EXPECT_TRUE(bits.AreNoneSet());
		EXPECT_FALSE(bits.AreAnySet());
		EXPECT_TRUE(bits.AreAnyNotSet());
		EXPECT_FALSE(bits.AreAllSet());
		for (uint16 i = 0; i < count; ++i)
		{
			EXPECT_FALSE(bits.IsSet(i));
		}
		EXPECT_EQ(bits.GetNumberOfSetBits(), 0u);
		EXPECT_FALSE(bits.GetFirstSetIndex().IsValid());
		EXPECT_FALSE(bits.GetLastSetIndex().IsValid());
		/*EXPECT_FALSE(bits.GetNextSetIndex(0u));

		for ([[maybe_unused]] const uint16 index : bits.GetSetBitsIterator())
		{
		  // Should never be reached
		  EXPECT_TRUE(false);
		}*/
	}

	UNIT_TEST(CompressedBitset, SetAllConstruct)
	{
		constexpr uint16 count = 507;
		CompressedBitset<count> bits{Memory::SetAll};
		EXPECT_FALSE(bits.AreNoneSet());
		EXPECT_TRUE(bits.AreAnySet());
		EXPECT_FALSE(bits.AreAnyNotSet());
		EXPECT_TRUE(bits.AreAllSet());
		for (uint16 i = 0; i < count; ++i)
		{
			EXPECT_TRUE(bits.IsSet(i));
		}
		EXPECT_EQ(bits.GetNumberOfSetBits(), count);
		EXPECT_TRUE(bits.GetFirstSetIndex().IsValid());
		EXPECT_EQ(*bits.GetFirstSetIndex(), 0);
		EXPECT_TRUE(bits.GetLastSetIndex().IsValid());
		EXPECT_EQ(*bits.GetLastSetIndex(), count - 1);
		/*EXPECT_TRUE(bits.GetNextSetIndex(0u));*/
	}

	UNIT_TEST(CompressedBitset, SetAndClear)
	{
		{
			constexpr uint16 count = 507;
			CompressedBitset<count> bits;
			EXPECT_FALSE(bits.IsSet(0));
			EXPECT_FALSE(bits.IsSet(1));
			EXPECT_EQ(bits.GetNumberOfSetBits(), 0);
			bits.Set(1);
			EXPECT_FALSE(bits.IsSet(0));
			EXPECT_TRUE(bits.IsSet(1));
			EXPECT_EQ(bits.GetNumberOfSetBits(), 1);

			EXPECT_FALSE(bits.IsSet(0));
			bits.Set(0);
			EXPECT_TRUE(bits.IsSet(0));
			EXPECT_TRUE(bits.IsSet(1));
			EXPECT_EQ(bits.GetNumberOfSetBits(), 2);

			EXPECT_FALSE(bits.IsSet(2));
			bits.Set(2);
			EXPECT_TRUE(bits.IsSet(0));
			EXPECT_TRUE(bits.IsSet(1));
			EXPECT_TRUE(bits.IsSet(2));
			EXPECT_FALSE(bits.IsSet(3));
			EXPECT_EQ(bits.GetNumberOfSetBits(), 3);

			bits.Set(2);
			EXPECT_TRUE(bits.IsSet(1));
			EXPECT_TRUE(bits.IsSet(2));

			EXPECT_FALSE(bits.IsSet(3));
			EXPECT_FALSE(bits.IsSet(4));
			bits.Set(4);
			EXPECT_TRUE(bits.IsSet(1));
			EXPECT_TRUE(bits.IsSet(4));
			EXPECT_FALSE(bits.IsSet(3));
			EXPECT_EQ(bits.GetNumberOfSetBits(), 4);

			bits.Set(3);
			EXPECT_TRUE(bits.IsSet(1));
			EXPECT_TRUE(bits.IsSet(2));
			EXPECT_TRUE(bits.IsSet(3));
			EXPECT_TRUE(bits.IsSet(4));
			EXPECT_EQ(bits.GetNumberOfSetBits(), 5);

			bits.Clear(2);
			EXPECT_TRUE(bits.IsSet(0));
			EXPECT_TRUE(bits.IsSet(1));
			EXPECT_FALSE(bits.IsSet(2));
			EXPECT_TRUE(bits.IsSet(3));
			EXPECT_TRUE(bits.IsSet(4));
			EXPECT_EQ(bits.GetNumberOfSetBits(), 4);
			bits.Clear(2);
			EXPECT_TRUE(bits.IsSet(0));
			EXPECT_TRUE(bits.IsSet(1));
			EXPECT_FALSE(bits.IsSet(2));
			EXPECT_TRUE(bits.IsSet(3));
			EXPECT_TRUE(bits.IsSet(4));
			EXPECT_EQ(bits.GetNumberOfSetBits(), 4);

			bits.Clear(3);
			EXPECT_TRUE(bits.IsSet(0));
			EXPECT_TRUE(bits.IsSet(1));
			EXPECT_FALSE(bits.IsSet(2));
			EXPECT_FALSE(bits.IsSet(3));
			EXPECT_TRUE(bits.IsSet(4));
			EXPECT_EQ(bits.GetNumberOfSetBits(), 3);
		}
	}
}
