#include <Common/Memory/New.h>

#include <Common/Tests/UnitTest.h>

#include <Common/Memory/Containers/BitView.h>
#include <Common/EnumFlags.h>

namespace ngine::Tests
{
	UNIT_TEST(BitView, DefaultConstruct)
	{
		BitView view;
		EXPECT_EQ(view.GetData(), nullptr);
		EXPECT_EQ(view.GetIndex(), 0);
		EXPECT_EQ(view.GetCount(), 0);
	}
	UNIT_TEST(BitView, ConstructFromType)
	{
		{
			uint32 type;
			BitView view = BitView::Make(type);
			EXPECT_EQ(view.GetData(), reinterpret_cast<ByteType*>(&type));
			EXPECT_EQ(view.GetIndex(), 0);
			EXPECT_EQ(view.GetCount(), sizeof(type) * CharBitCount);
		}
	}

	UNIT_TEST(BitView, PackFlags)
	{
		{
			enum class FlagsTest : uint8
			{
				One = 1 << 0,
				Two = 1 << 1,
				Three = 1 << 2
			};

			Array<ByteType, 1> target{Memory::Zeroed};

			{
				EnumFlags<FlagsTest> flags;
				flags |= FlagsTest::One;
				flags |= FlagsTest::Two;
				flags |= FlagsTest::Three;
				ConstBitView sourceView = ConstBitView::Make(flags, Math::Range<size>::Make(0, 3));

				BitView(target.GetDynamicView()).Pack(sourceView);
			}

			EnumFlags<FlagsTest> flags;
			ConstBitView(target.GetDynamicView()).Unpack(BitView::Make(flags, Math::Range<size>::Make(0, 3)));

			EXPECT_TRUE(flags.IsSet(FlagsTest::One));
			EXPECT_TRUE(flags.IsSet(FlagsTest::Two));
			EXPECT_TRUE(flags.IsSet(FlagsTest::Three));
			EXPECT_EQ(flags.GetNumberOfSetFlags(), 3);
		}
	}
	
	UNIT_TEST(BitView, ViewToView)
	{
		const Array<const ByteType, 13> source{
			(ByteType)10,
			(ByteType)44,
			(ByteType)8,
			(ByteType)0,
			(ByteType)206,
			(ByteType)204,
			(ByteType)22,
			(ByteType)12,
			(ByteType)165,
			(ByteType)140,
			(ByteType)0,
			(ByteType)0,
			(ByteType)0
		};
		const ConstBitView sourceView = ConstBitView::Make(source, Math::Range<size>::Make(9, 95));
		Array<ByteType, 12> target{Memory::Zeroed};
		const BitView targetView = BitView::Make(target, Math::Range<size>::Make(0, 95));
		targetView.Pack(sourceView);

		Array<ByteType, 13> finalTarget{Memory::Zeroed};
		BitView::Make(finalTarget, Math::Range<size>::Make(0, 9)).Pack(ConstBitView::Make(source, Math::Range<size>::Make(0, 9)));
		BitView::Make(finalTarget, Math::Range<size>::Make(9, 95)).Pack(ConstBitView{targetView.GetByteView(), targetView.GetBitRange()});

		EXPECT_EQ(source.GetDynamicView(), finalTarget.GetDynamicView());
	}
}
