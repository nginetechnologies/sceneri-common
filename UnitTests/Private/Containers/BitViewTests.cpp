#include <Common/Memory/New.h>

#include <Common/Tests/UnitTest.h>

#include <Common/Memory/Containers/BitView.h>
#include <Common/EnumFlags.h>
#include <Common/Storage/Identifier.h>

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

	UNIT_TEST(BitView, PackAndSkip)
	{
		{
			using Identifier = TIdentifier<uint32, 9>;
			struct PackedData
			{
				Identifier identifier;
				Identifier identifier2;
				Guid guid;
				uint32 size;
			};

			constexpr size BitCount = Identifier::IndexNumBits * 2 + sizeof(Guid) * CharBitCount + sizeof(uint32) * CharBitCount;
			Array<ByteType, (BitCount / CharBitCount) + 1> target{Memory::Zeroed};
			EXPECT_LT(target.GetDataSize(), sizeof(PackedData));
			constexpr Guid expectedGuid = "6c6e4622-9160-4be9-9820-8b890a9d62bf"_guid;

			{
				PackedData data{Identifier::MakeFromIndex(2), Identifier::MakeFromIndex(3), expectedGuid, 1337};
				BitView targetView(target.GetDynamicView(), Math::Range<size>::Make(0, BitCount));
				BitView targetView2 = targetView;

				{
					{
						const Identifier::IndexType index = data.identifier.GetIndex();
						ConstBitView sourceView = ConstBitView::Make(index, Math::Range<size>::Make(0, Identifier::IndexNumBits));
						targetView.PackAndSkip(sourceView);
					}

					{
						Identifier::IndexType readIndex;
						targetView2.UnpackAndSkip(BitView::Make(readIndex, Math::Range<size>::Make(0, Identifier::IndexNumBits)));
						EXPECT_EQ(readIndex, data.identifier.GetIndex());
					}
				}

				{
					{
						const Identifier::IndexType index = data.identifier2.GetIndex();
						ConstBitView sourceView = ConstBitView::Make(index, Math::Range<size>::Make(0, Identifier::IndexNumBits));
						targetView.PackAndSkip(sourceView);
					}

					{
						Identifier::IndexType readIndex;
						targetView2.UnpackAndSkip(BitView::Make(readIndex, Math::Range<size>::Make(0, Identifier::IndexNumBits)));
						EXPECT_EQ(readIndex, data.identifier2.GetIndex());
					}
				}

				{
					{
						ConstBitView sourceView = ConstBitView::Make(data.guid);
						targetView.PackAndSkip(sourceView);
					}
					{
						Array<ByteType, sizeof(Guid)> guidContentsTarget;
						targetView2.UnpackAndSkip(BitView(guidContentsTarget.GetDynamicView()));
						const Guid targetGuid = reinterpret_cast<const Guid&>(*guidContentsTarget.GetData());
						EXPECT_EQ(targetGuid, data.guid);
					}
				}

				{
					{
						ConstBitView sourceView = ConstBitView::Make(data.size);
						targetView.PackAndSkip(sourceView);
					}

					{
						uint32 readSize;
						targetView2.UnpackAndSkip(BitView::Make(readSize));
						EXPECT_EQ(readSize, data.size);
					}
				}
			}

			ConstBitView sourceView(target.GetDynamicView(), Math::Range<size>::Make(0, BitCount));

			PackedData data;

			{
				Identifier::IndexType index;
				sourceView.UnpackAndSkip(BitView::Make(index, Math::Range<size>::Make(0, Identifier::IndexNumBits)));
				data.identifier = Identifier::MakeFromIndex(index);
			}
			{
				Identifier::IndexType index;
				;
				sourceView.UnpackAndSkip(BitView::Make(index, Math::Range<size>::Make(0, Identifier::IndexNumBits)));
				data.identifier2 = Identifier::MakeFromIndex(index);
			}
			{
				sourceView.UnpackAndSkip(BitView::Make(data.guid));
			}
			{
				sourceView.UnpackAndSkip(BitView::Make(data.size));
			}

			EXPECT_EQ(data.identifier.GetIndex(), 2);
			EXPECT_EQ(data.identifier2.GetIndex(), 3);
			EXPECT_EQ(data.guid, expectedGuid);
			EXPECT_EQ(data.size, 1337);
		}

		{
			using Identifier1 = TIdentifier<uint32, 9>;
			using Identifier2 = TIdentifier<uint32, 14>;

			constexpr size BitCount = Identifier1::IndexNumBits + Identifier2::IndexNumBits;
			Array<ByteType, (BitCount / CharBitCount) + 1> target{Memory::Zeroed};

			{
				BitView targetView(target.GetDynamicView(), Math::Range<size>::Make(0, BitCount));
				BitView targetView2 = targetView;
				{
					{
						const Identifier1::IndexType index = Identifier1::MakeFromIndex(1).GetIndex();
						ConstBitView sourceView = ConstBitView::Make(index, Math::Range<size>::Make(0, Identifier1::IndexNumBits));
						targetView.PackAndSkip(sourceView);
					}
					{
						Identifier1::IndexType index;
						targetView2.UnpackAndSkip(BitView::Make(index, Math::Range<size>::Make(0, Identifier1::IndexNumBits)));
						EXPECT_EQ(index, 1);
					}
				}
				{
					{
						const Identifier2::IndexType index = Identifier2::MakeFromIndex(1).GetIndex();
						ConstBitView sourceView = ConstBitView::Make(index, Math::Range<size>::Make(0, Identifier2::IndexNumBits));
						targetView.PackAndSkip(sourceView);
					}

					{
						Identifier2::IndexType index;
						targetView2.UnpackAndSkip(BitView::Make(index, Math::Range<size>::Make(0, Identifier2::IndexNumBits)));
						EXPECT_EQ(index, 1);
					}
				}
			}

			ConstBitView sourceView(target.GetDynamicView(), Math::Range<size>::Make(0, BitCount));

			{
				Identifier1::IndexType index;
				sourceView.UnpackAndSkip(BitView::Make(index, Math::Range<size>::Make(0, Identifier1::IndexNumBits)));
				EXPECT_EQ(index, 1);
			}
			{
				Identifier2::IndexType index;
				sourceView.UnpackAndSkip(BitView::Make(index, Math::Range<size>::Make(0, Identifier2::IndexNumBits)));
				EXPECT_EQ(index, 1);
			}
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
