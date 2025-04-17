#include <Common/Memory/New.h>

#include <Common/Tests/UnitTest.h>

#include <Common/Compression/Bool.h>
#include <Common/Compression/Enum.h>
#include <Common/Compression/Float.h>
#include <Common/Compression/Compress.h>
#include <Common/Compression/Decompress.h>
#include <Common/Math/Vector3.h>
#include <Common/Storage/Compression/Identifier.h>
#include <Common/Memory/Compression/Bitset.h>
#include <Common/Memory/Containers/Compression/ArrayView.h>
#include <Common/Memory/Containers/Compression/Vector.h>
#include <Common/Memory/Containers/Vector.h>

namespace ngine::Tests
{
	UNIT_TEST(Compression, Bool)
	{
		Array<uint8, 1> data{Memory::Zeroed};
		{
			BitView packedTarget(data.GetDynamicView());
			static_assert(!Compression::IsDynamicallyCompressed<bool>());
			static_assert(Compression::CalculateFixedDataSize<bool>() == 1);
			EXPECT_TRUE(Compression::Compress<bool>(true, packedTarget));
			EXPECT_TRUE(Compression::Compress<bool>(false, packedTarget));
			EXPECT_TRUE(Compression::Compress<bool>(true, packedTarget));
			EXPECT_TRUE(Compression::Compress<bool>(false, packedTarget));
			EXPECT_TRUE(Compression::Compress<bool>(true, packedTarget));
			EXPECT_TRUE(Compression::Compress<bool>(false, packedTarget));
		}

		ConstBitView packedSource(data.GetDynamicView());
		bool target;
		EXPECT_TRUE(Compression::Decompress<bool>(target, packedSource));
		EXPECT_EQ(target, true);
		EXPECT_TRUE(Compression::Decompress<bool>(target, packedSource));
		EXPECT_EQ(target, false);
		EXPECT_TRUE(Compression::Decompress<bool>(target, packedSource));
		EXPECT_EQ(target, true);
		EXPECT_TRUE(Compression::Decompress<bool>(target, packedSource));
		EXPECT_EQ(target, false);
		EXPECT_TRUE(Compression::Decompress<bool>(target, packedSource));
		EXPECT_EQ(target, true);
		EXPECT_TRUE(Compression::Decompress<bool>(target, packedSource));
		EXPECT_EQ(target, false);
	}

	UNIT_TEST(Compression, Float)
	{
		Array<uint8, 320> data{Memory::Zeroed};
		{
			BitView packedTarget(data.GetDynamicView());
			EXPECT_TRUE(Compression::Compress<float>(0.f, packedTarget));
			EXPECT_TRUE(Compression::Compress<float>(0.05f, packedTarget));
			EXPECT_TRUE(Compression::Compress<float>(0.1f, packedTarget));
			EXPECT_TRUE(Compression::Compress<float>(0.25f, packedTarget));
			EXPECT_TRUE(Compression::Compress<float>(0.5f, packedTarget));
			EXPECT_TRUE(Compression::Compress<float>(0.51f, packedTarget));
			EXPECT_TRUE(Compression::Compress<float>(0.7f, packedTarget));
			EXPECT_TRUE(Compression::Compress<float>(0.9f, packedTarget));
			EXPECT_TRUE(Compression::Compress<float>(1.f, packedTarget));
		}

		ConstBitView packedSource(data.GetDynamicView());
		float target;
		EXPECT_TRUE(Compression::Decompress<float>(target, packedSource));
		EXPECT_EQ(target, 0.f);
		EXPECT_TRUE(Compression::Decompress<float>(target, packedSource));
		EXPECT_NEAR(target, 0.05f, 0.05f);
		EXPECT_TRUE(Compression::Decompress<float>(target, packedSource));
		EXPECT_NEAR(target, 0.1f, 0.05f);
		EXPECT_TRUE(Compression::Decompress<float>(target, packedSource));
		EXPECT_NEAR(target, 0.25f, 0.05f);
		EXPECT_TRUE(Compression::Decompress<float>(target, packedSource));
		EXPECT_NEAR(target, 0.5f, 0.05f);
		EXPECT_TRUE(Compression::Decompress<float>(target, packedSource));
		EXPECT_NEAR(target, 0.51f, 0.05f);
		EXPECT_TRUE(Compression::Decompress<float>(target, packedSource));
		EXPECT_NEAR(target, 0.7f, 0.05f);
		EXPECT_TRUE(Compression::Decompress<float>(target, packedSource));
		EXPECT_NEAR(target, 0.9f, 0.05f);
		EXPECT_TRUE(Compression::Decompress<float>(target, packedSource));
		EXPECT_EQ(target, 1.f);
	}

	UNIT_TEST(Compression, Vector3)
	{
		Array<uint8, 96> data{Memory::Zeroed};
		{
			BitView packedTarget(data.GetDynamicView());
			EXPECT_TRUE(Compression::Compress<Math::Vector3f>({1, 2, 3}, packedTarget));
		}

		ConstBitView packedSource(data.GetDynamicView());
		Math::Vector3f target;
		EXPECT_TRUE(Compression::Decompress<Math::Vector3f>(target, packedSource));
		EXPECT_TRUE(target.IsEquivalentTo(Math::Vector3f{1, 2, 3}, 0.01f));
	}

	enum class TestEnum : uint8
	{
		Zero,
		One,
		Two,
		Three
	};

	enum class TestEnumFlags : uint8
	{
		One = 1 << 0,
		Two = 1 << 1,
		Four = 1 << 2,
		Eight = 1 << 3,
		Sixteen = 1 << 4,
		ThirtyTwo = 1 << 5,
		SixtyFour = 1 << 6
	};
	ENUM_FLAG_OPERATORS(TestEnumFlags);
}

namespace ngine::Reflection
{
	template<>
	struct ReflectedType<Tests::TestEnum>
	{
		inline static constexpr auto Type = Reflection::Reflect<Tests::TestEnum>(
			"bfc57938-33ae-477d-a0eb-15459777c2e9"_guid,
			MAKE_UNICODE_LITERAL("Test Enum"),
			Reflection::TypeFlags{},
			Reflection::Tags{},
			Reflection::Properties{},
			Reflection::Functions{},
			Reflection::Events{},
			Reflection::Extensions{Reflection::EnumTypeExtension{
				Reflection::EnumTypeEntry{Tests::TestEnum::Zero, MAKE_UNICODE_LITERAL("0")},
				Reflection::EnumTypeEntry{Tests::TestEnum::One, MAKE_UNICODE_LITERAL("1")},
				Reflection::EnumTypeEntry{Tests::TestEnum::Two, MAKE_UNICODE_LITERAL("2")},
				Reflection::EnumTypeEntry{Tests::TestEnum::Three, MAKE_UNICODE_LITERAL("3")}
			}}
		);
	};

	template<>
	struct ReflectedType<Tests::TestEnumFlags>
	{
		inline static constexpr auto Type = Reflection::Reflect<Tests::TestEnumFlags>(
			"ae8c06e2-f432-47bc-b75d-c5493a58495f"_guid,
			MAKE_UNICODE_LITERAL("Test Enum Flags"),
			Reflection::TypeFlags{},
			Reflection::Tags{},
			Reflection::Properties{},
			Reflection::Functions{},
			Reflection::Events{},
			Reflection::Extensions{Reflection::EnumTypeExtension{
				Reflection::EnumTypeEntry{Tests::TestEnumFlags::One, MAKE_UNICODE_LITERAL("1")},
				Reflection::EnumTypeEntry{Tests::TestEnumFlags::Two, MAKE_UNICODE_LITERAL("2")},
				Reflection::EnumTypeEntry{Tests::TestEnumFlags::Four, MAKE_UNICODE_LITERAL("4")},
				Reflection::EnumTypeEntry{Tests::TestEnumFlags::Eight, MAKE_UNICODE_LITERAL("8")},
				Reflection::EnumTypeEntry{Tests::TestEnumFlags::Sixteen, MAKE_UNICODE_LITERAL("16")},
				Reflection::EnumTypeEntry{Tests::TestEnumFlags::ThirtyTwo, MAKE_UNICODE_LITERAL("32")},
				Reflection::EnumTypeEntry{Tests::TestEnumFlags::SixtyFour, MAKE_UNICODE_LITERAL("64")}
			}}
		);
	};
}

namespace ngine::Tests
{
	UNIT_TEST(Compression, Enum)
	{
		Array<uint8, 4> data{Memory::Zeroed};
		{
			BitView packedTarget(data.GetDynamicView());
			constexpr auto range = Reflection::GetEnum<TestEnum>().GetRange();
			static_assert(range.GetMinimum() == uint8(TestEnum::Zero));
			static_assert(range.GetMaximum() == uint8(TestEnum::Three));
			static_assert(range.GetSize() > 0);
			static_assert(range.GetSize() == 4);
			static_assert(range.GetMinimum() == 0);
			static_assert(!Compression::IsDynamicallyCompressed<TestEnum>());
			static_assert(Compression::CalculateFixedDataSize<TestEnum>() == 3);

			EXPECT_TRUE(Compression::Compress<TestEnum>(TestEnum::Zero, packedTarget));
			EXPECT_TRUE(Compression::Compress<TestEnum>(TestEnum::One, packedTarget));
			EXPECT_TRUE(Compression::Compress<TestEnum>(TestEnum::Two, packedTarget));
			EXPECT_TRUE(Compression::Compress<TestEnum>(TestEnum::Three, packedTarget));
		}

		ConstBitView packedSource(data.GetDynamicView());
		TestEnum target;
		EXPECT_TRUE(Compression::Decompress<TestEnum>(target, packedSource));
		EXPECT_EQ(target, TestEnum::Zero);
		EXPECT_TRUE(Compression::Decompress<TestEnum>(target, packedSource));
		EXPECT_EQ(target, TestEnum::One);
		EXPECT_TRUE(Compression::Decompress<TestEnum>(target, packedSource));
		EXPECT_EQ(target, TestEnum::Two);
		EXPECT_TRUE(Compression::Decompress<TestEnum>(target, packedSource));
		EXPECT_EQ(target, TestEnum::Three);
	}

	UNIT_TEST(Compression, EnumFlags)
	{
		Array<uint8, 8> data{Memory::Zeroed};
		{
			BitView packedTarget(data.GetDynamicView());

			static_assert(!Compression::IsDynamicallyCompressed<EnumFlags<TestEnumFlags>>());
			static_assert(
				Compression::CalculateFixedDataSize<EnumFlags<TestEnumFlags>>() ==
				Math::Log2((UNDERLYING_TYPE(TestEnumFlags))TestEnumFlags::SixtyFour) + 1u
			);

			EXPECT_TRUE(Compression::Compress<EnumFlags<TestEnumFlags>>(TestEnumFlags{}, packedTarget));
			EXPECT_TRUE(Compression::Compress<EnumFlags<TestEnumFlags>>(TestEnumFlags::One, packedTarget));
			EXPECT_TRUE(Compression::Compress<EnumFlags<TestEnumFlags>>(TestEnumFlags::Two, packedTarget));
			EXPECT_TRUE(Compression::Compress<EnumFlags<TestEnumFlags>>(TestEnumFlags::Four, packedTarget));
			EXPECT_TRUE(Compression::Compress<EnumFlags<TestEnumFlags>>(TestEnumFlags::Eight, packedTarget));
			EXPECT_TRUE(Compression::Compress<EnumFlags<TestEnumFlags>>(TestEnumFlags::Sixteen, packedTarget));
			EXPECT_TRUE(Compression::Compress<EnumFlags<TestEnumFlags>>(TestEnumFlags::ThirtyTwo, packedTarget));
			EXPECT_TRUE(Compression::Compress<EnumFlags<TestEnumFlags>>(TestEnumFlags::SixtyFour, packedTarget));
			EXPECT_TRUE(Compression::Compress<EnumFlags<TestEnumFlags>>(
				TestEnumFlags::One | TestEnumFlags::Four | TestEnumFlags::Sixteen | TestEnumFlags::ThirtyTwo,
				packedTarget
			));
		}

		ConstBitView packedSource(data.GetDynamicView());
		EnumFlags<TestEnumFlags> target;
		EXPECT_TRUE(Compression::Decompress<EnumFlags<TestEnumFlags>>(target, packedSource));
		EXPECT_TRUE(target.AreNoneSet());
		EXPECT_TRUE(Compression::Decompress<EnumFlags<TestEnumFlags>>(target, packedSource));
		EXPECT_TRUE(target == TestEnumFlags::One);
		EXPECT_TRUE(Compression::Decompress<EnumFlags<TestEnumFlags>>(target, packedSource));
		EXPECT_TRUE(target == TestEnumFlags::Two);
		EXPECT_TRUE(Compression::Decompress<EnumFlags<TestEnumFlags>>(target, packedSource));
		EXPECT_TRUE(target == TestEnumFlags::Four);
		EXPECT_TRUE(Compression::Decompress<EnumFlags<TestEnumFlags>>(target, packedSource));
		EXPECT_TRUE(target == TestEnumFlags::Eight);
		EXPECT_TRUE(Compression::Decompress<EnumFlags<TestEnumFlags>>(target, packedSource));
		EXPECT_TRUE(target == TestEnumFlags::Sixteen);
		EXPECT_TRUE(Compression::Decompress<EnumFlags<TestEnumFlags>>(target, packedSource));
		EXPECT_TRUE(target == TestEnumFlags::ThirtyTwo);
		EXPECT_TRUE(Compression::Decompress<EnumFlags<TestEnumFlags>>(target, packedSource));
		EXPECT_TRUE(target == TestEnumFlags::SixtyFour);
		EXPECT_TRUE(Compression::Decompress<EnumFlags<TestEnumFlags>>(target, packedSource));
		EXPECT_TRUE(target == (TestEnumFlags::One | TestEnumFlags::Four | TestEnumFlags::Sixteen | TestEnumFlags::ThirtyTwo));
	}

	UNIT_TEST(Compression, Identifier)
	{
		using Identifier = TIdentifier<uint32, 7>;

		constexpr uint32 requiredByteCount = (uint32)Math::Ceil((float)(Identifier::IndexNumBits * 6) / 8.f);
		Array<uint8, requiredByteCount> data{Memory::Zeroed};
		{
			BitView packedTarget(data.GetDynamicView());
			static_assert(!Compression::IsDynamicallyCompressed<bool>());
			static_assert(Compression::CalculateFixedDataSize<Identifier>() == Identifier::IndexNumBits);
			EXPECT_TRUE(Compression::Compress<Identifier>(Identifier::MakeFromIndex(0), packedTarget));
			EXPECT_TRUE(Compression::Compress<Identifier>(Identifier::MakeFromIndex(7), packedTarget));
			EXPECT_TRUE(Compression::Compress<Identifier>(Identifier::MakeFromIndex(115), packedTarget));
			EXPECT_TRUE(Compression::Compress<Identifier>(Identifier::MakeFromIndex(19), packedTarget));
			EXPECT_TRUE(Compression::Compress<Identifier>(Identifier::MakeFromIndex(98), packedTarget));
			EXPECT_TRUE(Compression::Compress<Identifier>(Identifier::MakeFromIndex(Identifier::MaximumCount - 1), packedTarget));
		}

		ConstBitView packedSource(data.GetDynamicView());
		Identifier target;
		EXPECT_TRUE(Compression::Decompress<Identifier>(target, packedSource));
		EXPECT_EQ(target.GetIndex(), 0);
		EXPECT_TRUE(Compression::Decompress<Identifier>(target, packedSource));
		EXPECT_EQ(target.GetIndex(), 7);
		EXPECT_TRUE(Compression::Decompress<Identifier>(target, packedSource));
		EXPECT_EQ(target.GetIndex(), 115);
		EXPECT_TRUE(Compression::Decompress<Identifier>(target, packedSource));
		EXPECT_EQ(target.GetIndex(), 19);
		EXPECT_TRUE(Compression::Decompress<Identifier>(target, packedSource));
		EXPECT_EQ(target.GetIndex(), 98);
		EXPECT_TRUE(Compression::Decompress<Identifier>(target, packedSource));
		EXPECT_EQ(target.GetIndex(), Identifier::MaximumCount - 1);
	}

	UNIT_TEST(Compression, Bitset)
	{
		using TestBitset = Bitset<43>;

		TestBitset reference;
		reference.Set(0);
		reference.Set(1);
		reference.Set(19);
		reference.Set(42);

		constexpr uint32 requiredByteCount = (uint32)Math::Ceil((float)(43) / 8.f);
		Array<uint8, requiredByteCount> data{Memory::Zeroed};
		{
			BitView packedTarget(data.GetDynamicView());
			static_assert(!Compression::IsDynamicallyCompressed<TestBitset>());
			static_assert(Compression::CalculateFixedDataSize<TestBitset>() == 43);

			EXPECT_TRUE(Compression::Compress<TestBitset>(reference, packedTarget));
		}

		ConstBitView packedSource(data.GetDynamicView());
		TestBitset target;
		EXPECT_TRUE(Compression::Decompress<TestBitset>(target, packedSource));
		EXPECT_EQ(target, reference);
	}

	UNIT_TEST(Compression, Vector)
	{
		Array<uint8, 4> data{Memory::Zeroed};
		{
			BitView packedTarget(data.GetDynamicView());

			static_assert(Compression::IsDynamicallyCompressed<Vector<bool, uint8>>());
			static_assert(Compression::CalculateFixedDataSize<Vector<bool, uint8>>() == 0);
			EXPECT_EQ((Compression::CalculateDynamicDataSize<Vector<bool, uint8>>({})), sizeof(uint8) * 8);
			EXPECT_EQ((Compression::CalculateDynamicDataSize<Vector<bool, uint8>>(Vector<bool, uint8>{true, false})), sizeof(uint8) * 8 + 2);
			EXPECT_TRUE((Compression::Compress<Vector<bool, uint8>>({}, packedTarget)));
			EXPECT_TRUE((Compression::Compress<Vector<bool, uint8>>({true, false, true}, packedTarget)));
		}

		ConstBitView packedSource(data.GetDynamicView());
		Vector<bool, uint8> target;
		EXPECT_TRUE((Compression::Decompress<Vector<bool, uint8>>(target, packedSource)));
		EXPECT_EQ(target.GetSize(), 0);
		EXPECT_TRUE((Compression::Decompress<Vector<bool, uint8>>(target, packedSource)));
		EXPECT_EQ(target.GetSize(), 3);
		EXPECT_TRUE(target[0]);
		EXPECT_FALSE(target[1]);
		EXPECT_TRUE(target[2]);
	}

	UNIT_TEST(Compression, ArrayView)
	{
		Array<uint8, 4> data{Memory::Zeroed};
		{
			BitView packedTarget(data.GetDynamicView());
			static_assert(Compression::IsDynamicallyCompressed<ArrayView<bool, uint8>>());
			static_assert(Compression::CalculateFixedDataSize<ArrayView<bool, uint8>>() == 0);
			static_assert(Compression::CalculateDynamicDataSize<ArrayView<bool, uint8>>({}) == sizeof(uint8) * 8);
			static_assert(Compression::CalculateDynamicDataSize<ArrayView<bool, uint8>>(Array<bool, 2>{true, false}) == sizeof(uint8) * 8 + 2);
			EXPECT_TRUE((Compression::Compress<ArrayView<bool, uint8>>({}, packedTarget)));
			EXPECT_TRUE((Compression::Compress<ArrayView<bool, uint8>>(Array<bool, 3>{true, false, true}, packedTarget)));
		}

		ConstBitView packedSource(data.GetDynamicView());
		Vector<bool, uint8> target;
		EXPECT_TRUE((Compression::Decompress<Vector<bool, uint8>>(target, packedSource)));
		EXPECT_EQ(target.GetSize(), 0);
		EXPECT_TRUE((Compression::Decompress<Vector<bool, uint8>>(target, packedSource)));
		EXPECT_EQ(target.GetSize(), 3);
		EXPECT_TRUE(target[0]);
		EXPECT_FALSE(target[1]);
		EXPECT_TRUE(target[2]);
	}

}
