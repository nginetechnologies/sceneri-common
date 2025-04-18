#include <Common/Memory/New.h>

#include <Common/Tests/UnitTest.h>

#include <Common/Math/Abs.h>
#include <Common/Math/Ceil.h>
#include <Common/Math/Clamp.h>
#include <Common/Math/LinearInterpolate.h>
#include <Common/Math/Sign.h>
#include <Common/Math/SignNonZero.h>
#include <Common/Math/MultiplicativeInverse.h>
#include <Common/Math/Range.h>
#include <Common/Math/Acos.h>
#include <Common/Math/Asin.h>
#include <Common/Math/Atan.h>
#include <Common/Math/Cos.h>
#include <Common/Math/Sin.h>
#include <Common/Math/Tan.h>
#include <Common/Math/Ceil.h>
#include <Common/Math/Floor.h>
#include <Common/Math/Wrap.h>
#include <Common/Math/ISqrt.h>
#include <Common/Math/Log.h>
#include <Common/Math/Log2.h>
#include <Common/Math/Mod.h>
#include <Common/Math/Power.h>
#include <Common/Math/Sqrt.h>
#include <Common/Math/Round.h>
#include <Common/Math/IsEquivalentTo.h>
#include <Common/Math/Quantize.h>
#include <Common/Memory/Optional.h>

#include <Common/Memory/CountBits.h>
#include <Common/Memory/Containers/FlatVector.h>

#include <math.h>
#include <stdlib.h>
#include <tgmath.h>

namespace ngine::Tests
{
	UNIT_TEST(Math, Int128)
	{
		EXPECT_TRUE(int128(0) == int128(0));
		EXPECT_TRUE(int128(0) != int128(1));
		EXPECT_TRUE(int128(0) < int128(1));
		EXPECT_TRUE(int128(0) <= int128(1));
		EXPECT_TRUE(int128(1) > int128(0));
		EXPECT_TRUE(int128(1) >= int128(0));
		EXPECT_TRUE(-int128(1) < int128(0));
		EXPECT_TRUE(Math::NumericLimits<int128>::Min < int128(0));
		EXPECT_TRUE(Math::NumericLimits<int128>::Max > int128(Math::NumericLimits<int64>::Max));
#if __SIZEOF_INT128__
		EXPECT_TRUE(Math::NumericLimits<int128>::Min == (-Math::NumericLimits<int128>::Max) - 1);
#else
		EXPECT_TRUE(Math::NumericLimits<int128>::Min == (-Math::NumericLimits<int128>::Max)--);
#endif
		EXPECT_TRUE(Math::NumericLimits<int128>::Max == (int128)((~uint128(0)) >> uint128(1)));
	}

	UNIT_TEST(Math, Uint128)
	{
		EXPECT_TRUE(uint128(0) == uint128(0));
		EXPECT_TRUE(uint128(0) != uint128(1));
		EXPECT_TRUE(uint128(0) < uint128(1));
		EXPECT_TRUE(uint128(0) <= uint128(1));
		EXPECT_TRUE(uint128(1) > uint128(0));
		EXPECT_TRUE(uint128(1) >= uint128(0));
		EXPECT_TRUE(~uint128(0) > uint128(0));
		EXPECT_TRUE((uint128(0) & uint128(0)) == uint128(0));
		EXPECT_TRUE((uint128(3) & uint128(2)) == uint128(2));
		EXPECT_TRUE((uint128(1) | uint128(2)) == uint128(3));
		EXPECT_TRUE((uint128(1) << uint128(0)) == uint128(1));
		EXPECT_TRUE((uint128(1) << uint128(1)) == uint128(2));
		EXPECT_TRUE((uint128(2) >> uint128(1)) == uint128(1));
		EXPECT_TRUE(Math::NumericLimits<uint128>::Min == uint128(0));
		EXPECT_TRUE(
			Math::NumericLimits<uint128>::Max == ((uint128(Math::NumericLimits<uint64>::Max) << 64) | uint128(Math::NumericLimits<uint64>::Max))
		);
		EXPECT_TRUE(Math::NumericLimits<uint128>::Max > uint128(Math::NumericLimits<uint64>::Max));
	}

	UNIT_TEST(Math, Abs)
	{
		EXPECT_EQ(Math::Abs((uint8)0), (uint8)0);
		EXPECT_EQ(Math::Abs((uint8)128), (uint8)128);
		EXPECT_EQ(Math::Abs((uint8)255), (uint8)255);
		EXPECT_EQ(Math::Abs((uint16)0), (uint16)0);
		EXPECT_EQ(Math::Abs((uint16)1456), (uint16)1456);
		EXPECT_EQ(Math::Abs((uint16)65240), (uint16)65240);
		EXPECT_EQ(Math::Abs((uint32)0), (uint32)0);
		EXPECT_EQ(Math::Abs((uint32)45645647), (uint32)45645647);
		EXPECT_EQ(Math::Abs((uint32)78978976), (uint32)78978976);
		EXPECT_EQ(Math::Abs((uint64)456454564647), (uint64)456454564647);
		EXPECT_EQ(Math::Abs((uint64)6478974578976), (uint64)6478974578976);

		EXPECT_EQ(Math::Abs((int8)-0), (int8)0);
		EXPECT_EQ(Math::Abs((int8)0), (int8)0);
		EXPECT_EQ(Math::Abs((int8)-65), (int8)65);
		EXPECT_EQ(Math::Abs((int8)65), (int8)65);
		EXPECT_EQ(Math::Abs((int8)-126), (int8)126);
		EXPECT_EQ(Math::Abs((int8)126), (int8)126);
		EXPECT_EQ(Math::Abs((int16)-51), (int16)51);
		EXPECT_EQ(Math::Abs((int16)46), (int16)46);
		EXPECT_EQ(Math::Abs((int16)-32502), (int16)32502);
		EXPECT_EQ(Math::Abs((int32)-128977), (int32)128977);
		EXPECT_EQ(Math::Abs((int32)975664564), (int32)975664564);
		EXPECT_EQ(Math::Abs((int32)-786745656), (int32)786745656);
		EXPECT_EQ(Math::Abs((int64)-128454564977), (int64)128454564977);
		EXPECT_EQ(Math::Abs((int64)9756477564564), (int64)9756477564564);
		EXPECT_EQ(Math::Abs((int64)-786745664456), (int64)786745664456);

		EXPECT_EQ(Math::Abs(-128134.1023f), 128134.1023f);
		EXPECT_EQ(Math::Abs(128134.1023f), 128134.1023f);
		EXPECT_EQ(Math::Abs(-819231923.5123), 819231923.5123);
		EXPECT_EQ(Math::Abs(819231923.5123), 819231923.5123);
	}

	UNIT_TEST(Math, Ceil)
	{
		EXPECT_EQ(Math::Ceil(1336.5), 1337.0);
		EXPECT_EQ(Math::Ceil(1336.1), 1337.0);
		EXPECT_EQ(Math::Ceil(1337.0), 1337.0);
		EXPECT_EQ(Math::Ceil(1336.6), 1337.0);
		EXPECT_EQ(Math::Ceil(9000.243f), 9001.f);
		EXPECT_EQ(Math::Ceil(9000.5f), 9001.f);
		EXPECT_EQ(Math::Ceil(9000.779112f), 9001.f);
		EXPECT_EQ(Math::Ceil(9001.f), 9001.f);
	}

	UNIT_TEST(Math, Floor)
	{
		EXPECT_EQ(Math::Floor(1336.5), 1336.0);
		EXPECT_EQ(Math::Floor(1336.1), 1336.0);
		EXPECT_EQ(Math::Floor(1337.0), 1337.0);
		EXPECT_EQ(Math::Floor(1336.6), 1336.0);
		EXPECT_EQ(Math::Floor(9000.243f), 9000.f);
		EXPECT_EQ(Math::Floor(9000.5f), 9000.f);
		EXPECT_EQ(Math::Floor(9000.779112f), 9000.f);
		EXPECT_EQ(Math::Floor(9001.f), 9001.f);
	}

	UNIT_TEST(Math, Wrap)
	{
		EXPECT_EQ(Math::Wrap(0, 0, 4), 0);
		EXPECT_EQ(Math::Wrap(1, 0, 4), 1);
		EXPECT_EQ(Math::Wrap(2, 0, 4), 2);
		EXPECT_EQ(Math::Wrap(3, 0, 4), 3);
		EXPECT_EQ(Math::Wrap(4, 0, 4), 4);
		EXPECT_EQ(Math::Wrap(5, 0, 4), 0);
		EXPECT_EQ(Math::Wrap(6, 0, 4), 1);
		EXPECT_EQ(Math::Wrap(-1, 0, 4), 4);

		EXPECT_NEAR(Math::Wrap(0.1, 0.0, 4.0), 0.1, Math::NumericLimits<double>::Epsilon);
		EXPECT_NEAR(Math::Wrap(1.1, 0.0, 4.0), 1.1, Math::NumericLimits<double>::Epsilon);
		EXPECT_NEAR(Math::Wrap(2.1, 0.0, 4.0), 2.1, Math::NumericLimits<double>::Epsilon);
		EXPECT_NEAR(Math::Wrap(3.1, 0.0, 4.0), 3.1, Math::NumericLimits<double>::Epsilon);
		EXPECT_NEAR(Math::Wrap(4.1, 0.0, 4.0), 0.1, Math::NumericLimits<double>::Epsilon);
		EXPECT_NEAR(Math::Wrap(5.1, 0.0, 4.0), 1.1, Math::NumericLimits<double>::Epsilon);
		EXPECT_NEAR(Math::Wrap(6.1, 0.0, 4.0), 2.1, Math::NumericLimits<double>::Epsilon);
		EXPECT_NEAR(Math::Wrap(-1.1, 0.0, 4.0), 2.9, Math::NumericLimits<double>::Epsilon);

		EXPECT_NEAR(Math::Wrap(-1.0, -1.0, 1.0), -1.0, Math::NumericLimits<double>::Epsilon);
		EXPECT_NEAR(Math::Wrap(1.0, -1.0, 1.0), 1.0, Math::NumericLimits<double>::Epsilon);
		EXPECT_NEAR(Math::Wrap(0.1, -1.0, 1.0), 0.1, Math::NumericLimits<double>::Epsilon);
		EXPECT_NEAR(Math::Wrap(-0.1, -1.0, 1.0), -0.1, Math::NumericLimits<double>::Epsilon);
		EXPECT_NEAR(Math::Wrap(-1.0 - 0.1, -1.0, 1.0), 1.0 - 0.1, Math::NumericLimits<double>::Epsilon);
		EXPECT_NEAR(Math::Wrap(1.0 + 0.1, -1.0, 1.0), -1.0 + 0.1, Math::NumericLimits<double>::Epsilon);
	}

	UNIT_TEST(Math, Round)
	{
		EXPECT_EQ(Math::Round(1336.1), 1336.0);
		EXPECT_EQ(Math::Round(1337.0), 1337.0);
		EXPECT_EQ(Math::Round(1336.6), 1337.0);
		EXPECT_EQ(Math::Round(9000.243f), 9000.f);
		EXPECT_EQ(Math::Round(9000.779112f), 9001.f);
		EXPECT_EQ(Math::Round(9001.f), 9001.f);
	}

	UNIT_TEST(Math, Clamp)
	{
		EXPECT_EQ(Math::Clamp(0, -1, 1), 0);
		EXPECT_EQ(Math::Clamp(1, -1, 1), 1);
		EXPECT_EQ(Math::Clamp(-1, -1, 1), -1);
		EXPECT_EQ(Math::Clamp(-137, -1, 1), -1);
		EXPECT_EQ(Math::Clamp(2687, -1, 1), 1);

		EXPECT_EQ(Math::Clamp(0.5f, -1.f, 1.f), 0.5f);
		EXPECT_EQ(Math::Clamp(-1.f, -1.f, 1.f), -1.f);
		EXPECT_EQ(Math::Clamp(1.f, -1.f, 1.f), 1.f);
		EXPECT_EQ(Math::Clamp(-275.53f, -1.f, 1.f), -1.f);
		EXPECT_EQ(Math::Clamp(679987.623f, -1.f, 1.f), 1.f);

		EXPECT_EQ(Math::Clamp(0.5, -1.0, 1.0), 0.5);
		EXPECT_EQ(Math::Clamp(-1.0, -1.0, 1.0), -1.0);
		EXPECT_EQ(Math::Clamp(1.0, -1.0, 1.0), 1.0);
		EXPECT_EQ(Math::Clamp(-275.53, -1.0, 1.0), -1.0);
		EXPECT_EQ(Math::Clamp(679987.623, -1.0, 1.0), 1.0);
	}

	UNIT_TEST(Math, LinearInterpolate)
	{
		EXPECT_EQ(Math::LinearInterpolate(-1.f, 1.f, 0.5f), 0.f);
		EXPECT_EQ(Math::LinearInterpolate(-1.f, 1.f, 0.f), -1.f);
		EXPECT_EQ(Math::LinearInterpolate(-1.f, 1.f, 1.f), 1.f);
	}

	UNIT_TEST(Math, Sign)
	{
		EXPECT_EQ(Math::Sign(2u), 1u);
		EXPECT_EQ(Math::Sign(0u), 0u);
		EXPECT_EQ(Math::Sign(2), 1);
		EXPECT_EQ(Math::Sign(0), 0);
		EXPECT_EQ(Math::Sign(-0), 0);
		EXPECT_EQ(Math::Sign(-2), -1);
		EXPECT_EQ(Math::Sign(1.f), 1.f);
		EXPECT_EQ(Math::Sign(0.f), 0.f);
		EXPECT_EQ(Math::Sign(-0.f), 0.f);
		EXPECT_EQ(Math::Sign(-1.f), -1.f);
		EXPECT_EQ(Math::Sign(7.5f), 1.f);
		EXPECT_EQ(Math::Sign(-7.5f), -1.f);
	}

	UNIT_TEST(Math, SignNonZero)
	{
		EXPECT_EQ(Math::SignNonZero(2u), 1u);
		EXPECT_EQ(Math::SignNonZero(0u), 1u);
		EXPECT_EQ(Math::SignNonZero(2), 1);
		EXPECT_EQ(Math::SignNonZero(0), 1);
		EXPECT_EQ(Math::SignNonZero(-2), -1);
		EXPECT_EQ(Math::SignNonZero(1.f), 1.f);
		EXPECT_EQ(Math::SignNonZero(0.f), 1.f);
		EXPECT_EQ(Math::SignNonZero(-0.f), -1.f);
		EXPECT_EQ(Math::SignNonZero(-1.f), -1.f);
		EXPECT_EQ(Math::SignNonZero(7.5f), 1.f);
		EXPECT_EQ(Math::SignNonZero(-7.5f), -1.f);
	}

	UNIT_TEST(Math, MultiplicativeInverse)
	{
		const bool isInRange = Math::Abs(Math::MultiplicativeInverse(2.f) - 0.5f) <= 0.01f;
		EXPECT_TRUE(isInRange);
	}

	// TODO: Matrices, quat & vectors

	UNIT_TEST(Math, GetNumberOfLeadingZeros)
	{
		{
			const uint64 value = 0b0001000000000100000000000000000000000000000000000000000000000000;
			EXPECT_EQ(Memory::GetNumberOfLeadingZeros(value), 3u);
			static_assert(Memory::GetNumberOfLeadingZeros(value) == 3);
			const uint64 zeroValue = 0b0000000000000000000000000000000000000000000000000000000000000000;
			EXPECT_EQ(Memory::GetNumberOfLeadingZeros(zeroValue), 64u);
			static_assert(Memory::GetNumberOfLeadingZeros(zeroValue) == 64u);
			const uint64 allValue = 0b1111111111111111111111111111111111111111111111111111111111111111;
			EXPECT_EQ(Memory::GetNumberOfLeadingZeros(allValue), 0u);
			static_assert(Memory::GetNumberOfLeadingZeros(allValue) == 0u);
		}
		{
			const uint32 value = 0b00000100000000010000000000000000;
			EXPECT_EQ(Memory::GetNumberOfLeadingZeros(value), 5u);
			static_assert(Memory::GetNumberOfLeadingZeros(value) == 5u);
			const uint32 zeroValue = 0b00000000000000000000000000000000;
			EXPECT_EQ(Memory::GetNumberOfLeadingZeros(zeroValue), 32u);
			static_assert(Memory::GetNumberOfLeadingZeros(zeroValue) == 32u);
			const uint32 allValue = 0b11111111111111111111111111111111;
			EXPECT_EQ(Memory::GetNumberOfLeadingZeros(allValue), 0u);
			static_assert(Memory::GetNumberOfLeadingZeros(allValue) == 0u);
		}
		{
			const uint16 value = 0b0000001000010000;
			EXPECT_EQ(Memory::GetNumberOfLeadingZeros(value), 6u);
			static_assert(Memory::GetNumberOfLeadingZeros(value) == 6u);
			const uint16 zeroValue = 0b0000000000000000;
			EXPECT_EQ(Memory::GetNumberOfLeadingZeros(zeroValue), 16u);
			static_assert(Memory::GetNumberOfLeadingZeros(zeroValue) == 16u);
			const uint16 allValue = 0b1111111111111111;
			EXPECT_EQ(Memory::GetNumberOfLeadingZeros(allValue), 0u);
			static_assert(Memory::GetNumberOfLeadingZeros(allValue) == 0u);
		}
		{
			const uint8 value = 0b00000110;
			EXPECT_EQ(Memory::GetNumberOfLeadingZeros(value), 5u);
			static_assert(Memory::GetNumberOfLeadingZeros(value) == 5u);
			const uint8 zeroValue = 0b00000000;
			EXPECT_EQ(Memory::GetNumberOfLeadingZeros(zeroValue), 8u);
			static_assert(Memory::GetNumberOfLeadingZeros(zeroValue) == 8u);
			const uint8 allValue = 0b11111111;
			EXPECT_EQ(Memory::GetNumberOfLeadingZeros(allValue), 0u);
			static_assert(Memory::GetNumberOfLeadingZeros(allValue) == 0u);
		}
	}

	UNIT_TEST(Math, GetNumberOfTrailingZeros)
	{
		{
			const uint64 value = 0b0001000000000100000000000000000000000000000000000000000000000000;
			EXPECT_EQ(Memory::GetNumberOfTrailingZeros(value), 50u);
			static_assert(Memory::GetNumberOfTrailingZeros(value) == 50u);
			const uint64 zeroValue = 0b0000000000000000000000000000000000000000000000000000000000000000;
			EXPECT_EQ(Memory::GetNumberOfTrailingZeros(zeroValue), 64u);
			static_assert(Memory::GetNumberOfTrailingZeros(zeroValue) == 64u);
			const uint64 allValue = 0b1111111111111111111111111111111111111111111111111111111111111111;
			EXPECT_EQ(Memory::GetNumberOfTrailingZeros(allValue), 0u);
			static_assert(Memory::GetNumberOfTrailingZeros(allValue) == 0u);
		}
		{
			const uint32 value = 0b00000100000000010000000000000000;
			EXPECT_EQ(Memory::GetNumberOfTrailingZeros(value), 16u);
			static_assert(Memory::GetNumberOfTrailingZeros(value) == 16u);
			const uint32 zeroValue = 0b00000000000000000000000000000000;
			EXPECT_EQ(Memory::GetNumberOfTrailingZeros(zeroValue), 32u);
			static_assert(Memory::GetNumberOfTrailingZeros(zeroValue) == 32u);
			const uint32 allValue = 0b11111111111111111111111111111111;
			EXPECT_EQ(Memory::GetNumberOfTrailingZeros(allValue), 0u);
			static_assert(Memory::GetNumberOfTrailingZeros(allValue) == 0u);
		}
		{
			const uint16 value = 0b0000001000010000;
			EXPECT_EQ(Memory::GetNumberOfTrailingZeros(value), 4u);
			static_assert(Memory::GetNumberOfTrailingZeros(value) == 4u);
			const uint16 zeroValue = 0b0000000000000000;
			EXPECT_EQ(Memory::GetNumberOfTrailingZeros(zeroValue), 16u);
			static_assert(Memory::GetNumberOfTrailingZeros(zeroValue) == 16u);
			const uint16 allValue = 0b1111111111111111;
			EXPECT_EQ(Memory::GetNumberOfTrailingZeros(allValue), 0u);
			static_assert(Memory::GetNumberOfTrailingZeros(allValue) == 0u);
		}
		{
			const uint8 value = 0b00000110;
			EXPECT_EQ(Memory::GetNumberOfTrailingZeros(value), 1u);
			static_assert(Memory::GetNumberOfTrailingZeros(value) == 1u);
			const uint8 zeroValue = 0b00000000;
			EXPECT_EQ(Memory::GetNumberOfTrailingZeros(zeroValue), 8u);
			static_assert(Memory::GetNumberOfTrailingZeros(zeroValue) == 8u);
			const uint8 allValue = 0b11111111;
			EXPECT_EQ(Memory::GetNumberOfTrailingZeros(allValue), 0u);
			static_assert(Memory::GetNumberOfTrailingZeros(allValue) == 0u);
		}
	}

	UNIT_TEST(Math, GetNumberOfSetBits)
	{
		{
			const uint64 value = 0b0001000000000100000000000000000000000000000000000000000000000000;
			EXPECT_EQ(Memory::GetNumberOfSetBits(value), 2u);
			static_assert(Memory::GetNumberOfSetBits(value) == 2u);
			const uint64 zeroValue = 0b0000000000000000000000000000000000000000000000000000000000000000;
			EXPECT_EQ(Memory::GetNumberOfSetBits(zeroValue), 0u);
			static_assert(Memory::GetNumberOfSetBits(zeroValue) == 0u);
			const uint64 allValue = 0b1111111111111111111111111111111111111111111111111111111111111111;
			EXPECT_EQ(Memory::GetNumberOfSetBits(allValue), 64u);
			static_assert(Memory::GetNumberOfSetBits(allValue) == 64u);
		}
		{
			const uint32 value = 0b00000100000000010000000000000000;
			EXPECT_EQ(Memory::GetNumberOfSetBits(value), 2u);
			static_assert(Memory::GetNumberOfSetBits(value) == 2u);
			const uint32 zeroValue = 0b00000000000000000000000000000000;
			EXPECT_EQ(Memory::GetNumberOfSetBits(zeroValue), 0u);
			static_assert(Memory::GetNumberOfSetBits(zeroValue) == 0u);
			const uint32 allValue = 0b11111111111111111111111111111111;
			EXPECT_EQ(Memory::GetNumberOfSetBits(allValue), 32u);
			static_assert(Memory::GetNumberOfSetBits(allValue) == 32u);
		}
	}

	UNIT_TEST(Math, GetBitWidth)
	{
		{
			const uint64 value = 0b0000000000000000000000000000000000000000000000000000000000000001;
			EXPECT_EQ(Memory::GetBitWidth(value), 1u);
			static_assert(Memory::GetBitWidth(value) == 1u);
		}
		{
			const uint64 value = 0b0000000000000000000000000000000000000000000000000000000000000010;
			EXPECT_EQ(Memory::GetBitWidth(value), 2u);
			static_assert(Memory::GetBitWidth(value) == 2u);
		}
		{
			const uint64 value = 0b1000000000000000000000000000000000000000000000000000000000000010;
			EXPECT_EQ(Memory::GetBitWidth(value), 64u);
			static_assert(Memory::GetBitWidth(value) == 64u);
		}
	}

	UNIT_TEST(Math, GetFirstSetIndex)
	{
		EXPECT_FALSE(Memory::GetFirstSetIndex((uint64)0ull).IsValid());
		static_assert(!Memory::GetFirstSetIndex((uint64)0ull).IsValid());
		EXPECT_TRUE(Memory::GetFirstSetIndex(uint8(1u)).IsValid());
		static_assert(Memory::GetFirstSetIndex(uint8(1u)).IsValid());
		EXPECT_TRUE(Memory::GetFirstSetIndex(uint8(255)).IsValid());
		static_assert(Memory::GetFirstSetIndex(uint8(255)).IsValid());
		EXPECT_EQ(*Memory::GetFirstSetIndex(1u), 0u);
		static_assert(*Memory::GetFirstSetIndex(1u) == 0u);
		EXPECT_EQ(*Memory::GetFirstSetIndex(2u), 1u);
		static_assert(*Memory::GetFirstSetIndex(2u) == 1u);
		EXPECT_EQ(*Memory::GetFirstSetIndex(3u), 0u);
		static_assert(*Memory::GetFirstSetIndex(3u) == 0u);
		EXPECT_EQ(*Memory::GetFirstSetIndex((uint64)0b1000000000000000000000000000000000000000000000000000000000000000ull), 63ull);
		static_assert(*Memory::GetFirstSetIndex((uint64)0b1000000000000000000000000000000000000000000000000000000000000000ull) == 63ull);
		EXPECT_EQ(*Memory::GetFirstSetIndex((uint64)0b0000000000000000000000000000000000000000000000000000000000000001ull), 0ull);
		static_assert(*Memory::GetFirstSetIndex((uint64)0b0000000000000000000000000000000000000000000000000000000000000001ull) == 0ull);
		EXPECT_EQ(*Memory::GetFirstSetIndex((uint64)0b0001000000000100000000000000000000000000000000000000000000000000ull), 50ull);
		static_assert(*Memory::GetFirstSetIndex((uint64)0b0001000000000100000000000000000000000000000000000000000000000000ull) == 50ull);
		EXPECT_EQ(*Memory::GetFirstSetIndex((uint64)0b1111111111111111111111111111111111111111111111111111111111111111ull), 0ull);
		static_assert(*Memory::GetFirstSetIndex((uint64)0b1111111111111111111111111111111111111111111111111111111111111111ull) == 0ull);
		EXPECT_FALSE(Memory::GetFirstSetIndex((uint64)0b000000000000000000000000000000000000000000000000000000000000000ull).IsValid());
		static_assert(!Memory::GetFirstSetIndex((uint64)0b000000000000000000000000000000000000000000000000000000000000000ull).IsValid());
	}

	UNIT_TEST(Math, GetLastSetIndex)
	{
		EXPECT_FALSE(Memory::GetLastSetIndex((uint64)0ull).IsValid());
		static_assert(!Memory::GetLastSetIndex((uint64)0ull).IsValid());
		EXPECT_TRUE(Memory::GetLastSetIndex(uint8(1u)).IsValid());
		static_assert(Memory::GetLastSetIndex(uint8(1u)).IsValid());
		EXPECT_TRUE(Memory::GetLastSetIndex(uint8(255)).IsValid());
		EXPECT_EQ(*Memory::GetLastSetIndex(1u), 0u);
		static_assert(*Memory::GetLastSetIndex(1u) == 0u);
		EXPECT_EQ(*Memory::GetLastSetIndex(2u), 1u);
		static_assert(*Memory::GetLastSetIndex(2u) == 1u);
		EXPECT_EQ(*Memory::GetLastSetIndex(3u), 1u);
		static_assert(*Memory::GetLastSetIndex(3u) == 1u);
		EXPECT_EQ(*Memory::GetLastSetIndex((uint64)0b1000000000000000000000000000000000000000000000000000000000000000ull), 63ull);
		static_assert(*Memory::GetLastSetIndex((uint64)0b1000000000000000000000000000000000000000000000000000000000000000ull) == 63ull);
		EXPECT_EQ(*Memory::GetLastSetIndex((uint64)0b0000000000000000000000000000000000000000000000000000000000000001ull), 0ull);
		static_assert(*Memory::GetLastSetIndex((uint64)0b0000000000000000000000000000000000000000000000000000000000000001ull) == 0ull);
		EXPECT_EQ(*Memory::GetLastSetIndex((uint64)0b0001000000000100000000000000000000000000000000000000000000000000ull), 60ull);
		static_assert(*Memory::GetLastSetIndex((uint64)0b0001000000000100000000000000000000000000000000000000000000000000ull) == 60ull);
		EXPECT_EQ(*Memory::GetLastSetIndex((uint64)0b1111111111111111111111111111111111111111111111111111111111111111ull), 63ull);
		static_assert(*Memory::GetLastSetIndex((uint64)0b1111111111111111111111111111111111111111111111111111111111111111ull) == 63ull);
		EXPECT_FALSE(Memory::GetLastSetIndex((uint64)0b000000000000000000000000000000000000000000000000000000000000000ull).IsValid());
		static_assert(!Memory::GetLastSetIndex((uint64)0b000000000000000000000000000000000000000000000000000000000000000ull).IsValid());
	}

	UNIT_TEST(Math, GetSetBitsIterator)
	{
		{
			const uint8 value = 0b10101000;
			uint8 valueOut = 0;
			FlatVector<uint8, 8> order;
			Memory::IterateSetBits(
				value,
				[&valueOut, &order](const uint8 bitIndex) -> bool
				{
					valueOut |= 1 << bitIndex;
					order.EmplaceBack(bitIndex);
					return true;
				}
			);

			EXPECT_EQ(value, valueOut);

			const Array<uint8, 3> expectedOrder{(uint8)3u, (uint8)5u, (uint8)7u};
			EXPECT_EQ(order.GetView(), expectedOrder.GetDynamicView());
		}

		{
			const uint8 value = 0b10101000;
			uint8 valueOut = 0;
			FlatVector<uint8, 8> order;
			for (const uint8 bitIndex : Memory::GetSetBitsIterator(value))
			{
				valueOut |= 1 << bitIndex;
				order.EmplaceBack(bitIndex);
			}

			EXPECT_EQ(value, valueOut);

			const Array<uint8, 3> expectedOrder{(uint8)3u, (uint8)5u, (uint8)7u};
			EXPECT_EQ(order.GetView(), expectedOrder.GetDynamicView());
		}
	}

	UNIT_TEST(Math, GetUnsetBitsIterator)
	{
		{
			const uint8 value = 0b10101000;
			uint8 valueOut = 0;
			Memory::IterateSetBits(
				uint8(~value),
				[&valueOut](const uint8 bitIndex) -> bool
				{
					valueOut |= 1 << bitIndex;
					return true;
				}
			);

			EXPECT_EQ(value, uint8(~valueOut));
		}

		{
			const uint8 value = 0b10101000;
			uint8 valueOut = 0;
			for (const uint8 bitIndex : Memory::GetUnsetBitsIterator(value))
			{
				valueOut |= 1 << bitIndex;
			}

			EXPECT_EQ(value, uint8(~valueOut));
		}
	}

	UNIT_TEST(Math, GetSetBitsReverseIterator)
	{
		{
			const uint8 value = 0b10101000;
			uint8 valueOut = 0;
			FlatVector<uint8, 8> order;
			for (const uint8 bitIndex : Memory::GetSetBitsReverseIterator(value))
			{
				valueOut |= 1 << bitIndex;
				order.EmplaceBack(bitIndex);
			}

			EXPECT_EQ(value, valueOut);

			const Array<uint8, 3> expectedOrder{(uint8)7u, (uint8)5u, (uint8)3u};
			EXPECT_EQ(order.GetView(), expectedOrder.GetDynamicView());
		}
	}

	UNIT_TEST(Math, GetSetBitsRangeIterator)
	{
		{
			const uint8 value = 0b11101011;
			FlatVector<Math::Range<uint8>, 8> order;
			for (const Math::Range<uint8> setBitsRange : Memory::GetSetBitRangesIterator(value))
			{
				order.EmplaceBack(setBitsRange);
			}

			const Array<Math::Range<uint8>, 3> expectedOrder{
				Math::Range<uint8>::MakeStartToEnd(0, 1),
				Math::Range<uint8>::MakeStartToEnd(3, 3),
				Math::Range<uint8>::MakeStartToEnd(5, 7)
			};
			EXPECT_EQ(order.GetView(), expectedOrder.GetDynamicView());
		}
	}

	UNIT_TEST(Math, ClearTrailingSetBits)
	{
		EXPECT_EQ(Memory::ClearTrailingSetBits((uint16)0b1011000100100100, 3u), 0b1011000000000000);
	}

	UNIT_TEST(Math, ClearLeadingSetBits)
	{
		EXPECT_EQ(Memory::ClearLeadingSetBits((uint16)0b1011000100100100, 3u), 0b0000000100100100);
	}

	UNIT_TEST(Math, GetRemappedRange)
	{
		{
			Math::Rangef sourceRange = Math::Rangef::Make(-32768.f, 65535.f);
			Math::Rangef targetRange = Math::Rangef::Make(-1.f, 2.f);
			EXPECT_NEAR(targetRange.GetRemappedValue(-32768.f, sourceRange), -1.f, 0.01f);
			EXPECT_NEAR(targetRange.GetRemappedValue(32767.f, sourceRange), 1.f, 0.01f);
		}
	}

	UNIT_TEST(Math, ReverseBits)
	{
		EXPECT_EQ(Memory::ReverseBits((uint8)0b10110010), (uint8)0b01001101);
		EXPECT_EQ(Memory::ReverseBits((uint16)0b0010000010110010), (uint16)0b0100110100000100);
		EXPECT_EQ(Memory::ReverseBits((uint32)0b10000000000000000000000010110010), (uint32)0b01001101000000000000000000000001);
		EXPECT_EQ(
			Memory::ReverseBits((uint64)0b0100000000000000000000000000000000000000000000000000000010110010),
			(uint64)0b0100110100000000000000000000000000000000000000000000000000000010
		);
	}

	UNIT_TEST(Math, Acos)
	{
		EXPECT_TRUE(Math::IsEquivalentTo(Math::Acos(0.8f), acosf(0.8f)));
		EXPECT_TRUE(Math::IsEquivalentTo(Math::Acos(0.8), acos(0.8)));
	}

	UNIT_TEST(Math, Asin)
	{
		EXPECT_TRUE(Math::IsEquivalentTo(Math::Asin(0.8f), asinf(0.8f)));
		EXPECT_TRUE(Math::IsEquivalentTo(Math::Asin(0.8), asin(0.8)));
	}

	UNIT_TEST(Math, Atan)
	{
		EXPECT_TRUE(Math::IsEquivalentTo(Math::Atan(1.5f), atanf(1.5f)));
		EXPECT_TRUE(Math::IsEquivalentTo(Math::Atan(1.5), atan(1.5)));
	}

	UNIT_TEST(Math, Cos)
	{
		EXPECT_TRUE(Math::IsEquivalentTo(Math::Cos(1.5f), cosf(1.5f)));
		EXPECT_TRUE(Math::IsEquivalentTo(Math::Cos(1.5), cos(1.5)));
	}

	UNIT_TEST(Math, Sin)
	{
		EXPECT_TRUE(Math::IsEquivalentTo(Math::Sin(1.5f), sinf(1.5f)));
		EXPECT_TRUE(Math::IsEquivalentTo(Math::Sin(1.5), sin(1.5)));
	}

	UNIT_TEST(Math, Tan)
	{
		EXPECT_TRUE(Math::IsEquivalentTo(Math::Tan(1.5f), tanf(1.5f)));
		EXPECT_TRUE(Math::IsEquivalentTo(Math::Tan(1.5), tan(1.5)));
	}

	UNIT_TEST(Math, ISqrt)
	{
		EXPECT_TRUE(Math::IsEquivalentTo(Math::Isqrt(1.5f), 1.f / sqrt(1.5f)));
		EXPECT_TRUE(Math::IsEquivalentTo(Math::Isqrt(1.5), 1.0 / sqrt(1.5)));
	}

	UNIT_TEST(Math, Sqrt)
	{
		EXPECT_TRUE(Math::IsEquivalentTo(Math::Sqrt(1.5f), sqrt(1.5f)));
		EXPECT_TRUE(Math::IsEquivalentTo(Math::Sqrt(1.5), sqrt(1.5)));
	}

	UNIT_TEST(Math, Mod)
	{
		EXPECT_TRUE(Math::IsEquivalentTo(Math::Mod(3.6f, 1.5f), fmodf(3.6f, 1.5f)));
		EXPECT_TRUE(Math::IsEquivalentTo(Math::Mod(3.6, 1.5), fmod(3.6, 1.5)));
	}

	UNIT_TEST(Math, Power)
	{
		EXPECT_TRUE(Math::IsEquivalentTo(Math::Power(3.6f, 1.5f), powf(3.6f, 1.5f)));
		EXPECT_TRUE(Math::IsEquivalentTo(Math::Power(3.6, 1.5), pow(3.6, 1.5)));
	}

	UNIT_TEST(Math, Power2)
	{
		EXPECT_TRUE(Math::IsEquivalentTo(Math::Power2(3.6f), powf(2.f, 3.6f)));
		EXPECT_TRUE(Math::IsEquivalentTo(Math::Power2(3.6), pow(2.0, 3.6)));
	}

	UNIT_TEST(Math, Power10)
	{
		EXPECT_TRUE(Math::IsEquivalentTo(Math::Power10(3.6f), powf(10.f, 3.6f)));
		EXPECT_TRUE(Math::IsEquivalentTo(Math::Power10(3.6), pow(10.0, 3.6)));
	}

	UNIT_TEST(Math, Log)
	{
		EXPECT_TRUE(Math::IsEquivalentTo(Math::Log(1.5f), 0.405465f));
		EXPECT_TRUE(Math::IsEquivalentTo(Math::Log(512.f), 6.238324f));

		EXPECT_TRUE(Math::IsEquivalentTo(Math::Log(1.5), 0.405465));
		EXPECT_TRUE(Math::IsEquivalentTo(Math::Log(512.0), 6.238324));
	}

	UNIT_TEST(Math, Log2)
	{
		static_assert(Math::Log2(0u) == 0);
		static_assert(Math::Log2(1u) == 0);
		static_assert(Math::Log2(2u) == 1);
		static_assert(Math::Log2(4u) == 2);
		static_assert(Math::Log2(1u << 5u) == 5);

		EXPECT_TRUE(Math::IsEquivalentTo(Math::Log2(1.5f), 0.584963f));
		EXPECT_TRUE(Math::IsEquivalentTo(Math::Log2(512.f), 9.f));

		EXPECT_TRUE(Math::IsEquivalentTo(Math::Log2(1.5), 0.584963));
		EXPECT_TRUE(Math::IsEquivalentTo(Math::Log2(512.0), 9.0));

		EXPECT_EQ(Math::Log2((uint8)128u), (uint8)7u);
		EXPECT_EQ(Math::Log2((uint16)512u), (uint16)9u);
		EXPECT_EQ(Math::Log2((uint32)65536), 16u);
		EXPECT_EQ(Math::Log2((uint64)65536), (uint64)16u);
	}

	UNIT_TEST(Math, Range)
	{
		{
			Math::Range<uint32> range = Math::Range<uint32>::MakeStartToEnd(0, 3);
			EXPECT_TRUE(range.Contains(0));
			EXPECT_TRUE(range.Contains(1));
			EXPECT_TRUE(range.Contains(2));
			EXPECT_TRUE(range.Contains(3));
			EXPECT_TRUE(range.Contains(Math::Range<uint32>::MakeStartToEnd(0, 1)));
			EXPECT_TRUE(range.Contains(Math::Range<uint32>::MakeStartToEnd(0, 2)));
			EXPECT_TRUE(range.Contains(Math::Range<uint32>::MakeStartToEnd(0, 3)));
			EXPECT_TRUE(range.Contains(Math::Range<uint32>::MakeStartToEnd(1, 1)));
			EXPECT_TRUE(range.Contains(Math::Range<uint32>::MakeStartToEnd(1, 2)));
			EXPECT_TRUE(range.Contains(Math::Range<uint32>::MakeStartToEnd(1, 3)));
			EXPECT_TRUE(range.Contains(Math::Range<uint32>::MakeStartToEnd(2, 1)));
			EXPECT_TRUE(range.Contains(Math::Range<uint32>::MakeStartToEnd(2, 2)));
			EXPECT_TRUE(range.Contains(Math::Range<uint32>::MakeStartToEnd(2, 3)));
			EXPECT_TRUE(range.Contains(Math::Range<uint32>::MakeStartToEnd(3, 1)));
			EXPECT_TRUE(range.Contains(Math::Range<uint32>::MakeStartToEnd(3, 2)));
			EXPECT_TRUE(range.Contains(Math::Range<uint32>::MakeStartToEnd(3, 3)));
			EXPECT_TRUE(!range.Contains(Math::Range<uint32>::MakeStartToEnd(3, 4)));

			EXPECT_EQ(range.GetMinimum(), 0u);
			EXPECT_EQ(range.GetMaximum(), 3u);
			EXPECT_EQ(range.GetSize(), 4u);
			EXPECT_EQ(range.GetClampedValue(4), 3u);

			uint32 counter = 0;
			for (const uint32 value : range)
			{
				EXPECT_EQ(value, counter++);
			}
			EXPECT_EQ(counter, 4u);
		}
		{
			Math::Range<uint32> range = Math::Range<uint32>::MakeStartToEnd(1, 0);
			EXPECT_EQ(range.GetSize(), 0u);
			EXPECT_EQ(range.begin(), range.end());
		}
		{
			Math::Range<int32> range = Math::Range<int32>::MakeStartToEnd(-1, 1);
			EXPECT_EQ(range.GetSize(), 3u);
			EXPECT_EQ(range.GetMinimum(), -1);
			EXPECT_EQ(range.GetMaximum(), 1);
			EXPECT_FALSE(range.Contains(-2));
			EXPECT_TRUE(range.Contains(-1));
			EXPECT_TRUE(range.Contains(0));
			EXPECT_TRUE(range.Contains(1));
			EXPECT_FALSE(range.Contains(2));
			EXPECT_EQ(range.GetClampedRatio(-1), 0_percent);
			EXPECT_EQ(range.GetClampedRatio(0), 50_percent);
			EXPECT_EQ(range.GetClampedRatio(1), 100_percent);
		}
	}

	UNIT_TEST(Math, QuantizeFloat)
	{
		auto check = [](const float value, const Math::QuantizationMode mode, const Math::Rangef range, const uint32 bitCount)
		{
			const uint32 quantized = Math::Quantize(value, mode, range, bitCount);
			const float dequantized = Math::Dequantize(quantized, range, bitCount);
			EXPECT_NEAR(dequantized, value, 0.05f);
			if (mode == Math::QuantizationMode::AlwaysRoundUp)
			{
				EXPECT_GE(dequantized, value);
			}
		};

		check(0.f, Math::QuantizationMode::Truncate, Math::Rangef::MakeStartToEnd(0.f, 1.f), 16);
		check(0.05f, Math::QuantizationMode::Truncate, Math::Rangef::MakeStartToEnd(0.f, 1.f), 16);
		check(0.1f, Math::QuantizationMode::Truncate, Math::Rangef::MakeStartToEnd(0.f, 1.f), 16);
		check(0.25f, Math::QuantizationMode::Truncate, Math::Rangef::MakeStartToEnd(0.f, 1.f), 16);
		check(0.5f, Math::QuantizationMode::Truncate, Math::Rangef::MakeStartToEnd(0.f, 1.f), 16);
		check(0.51f, Math::QuantizationMode::Truncate, Math::Rangef::MakeStartToEnd(0.f, 1.f), 16);
		check(0.7f, Math::QuantizationMode::Truncate, Math::Rangef::MakeStartToEnd(0.f, 1.f), 16);
		check(0.9f, Math::QuantizationMode::Truncate, Math::Rangef::MakeStartToEnd(0.f, 1.f), 16);
		check(1.f, Math::QuantizationMode::Truncate, Math::Rangef::MakeStartToEnd(0.f, 1.f), 16);

		check(1.f, Math::QuantizationMode::Truncate, Math::Rangef::MakeStartToEnd(1.f, 2.f), 16);
		check(1.05f, Math::QuantizationMode::Truncate, Math::Rangef::MakeStartToEnd(1.f, 2.f), 16);
		check(1.1f, Math::QuantizationMode::Truncate, Math::Rangef::MakeStartToEnd(1.f, 2.f), 16);
		check(1.25f, Math::QuantizationMode::Truncate, Math::Rangef::MakeStartToEnd(1.f, 2.f), 16);
		check(1.5f, Math::QuantizationMode::Truncate, Math::Rangef::MakeStartToEnd(1.f, 2.f), 16);
		check(1.51f, Math::QuantizationMode::Truncate, Math::Rangef::MakeStartToEnd(1.f, 2.f), 16);
		check(1.7f, Math::QuantizationMode::Truncate, Math::Rangef::MakeStartToEnd(1.f, 2.f), 16);
		check(1.9f, Math::QuantizationMode::Truncate, Math::Rangef::MakeStartToEnd(1.f, 2.f), 16);
		check(2.f, Math::QuantizationMode::Truncate, Math::Rangef::MakeStartToEnd(1.f, 2.f), 16);

		check(0.f, Math::QuantizationMode::Round, Math::Rangef::MakeStartToEnd(0.f, 1.f), 16);
		check(0.05f, Math::QuantizationMode::Round, Math::Rangef::MakeStartToEnd(0.f, 1.f), 16);
		check(0.1f, Math::QuantizationMode::Round, Math::Rangef::MakeStartToEnd(0.f, 1.f), 16);
		check(0.25f, Math::QuantizationMode::Round, Math::Rangef::MakeStartToEnd(0.f, 1.f), 16);
		check(0.5f, Math::QuantizationMode::Round, Math::Rangef::MakeStartToEnd(0.f, 1.f), 16);
		check(0.51f, Math::QuantizationMode::Round, Math::Rangef::MakeStartToEnd(0.f, 1.f), 16);
		check(0.7f, Math::QuantizationMode::Round, Math::Rangef::MakeStartToEnd(0.f, 1.f), 16);
		check(0.9f, Math::QuantizationMode::Round, Math::Rangef::MakeStartToEnd(0.f, 1.f), 16);
		check(1.f, Math::QuantizationMode::Round, Math::Rangef::MakeStartToEnd(0.f, 1.f), 16);

		check(1.f, Math::QuantizationMode::Round, Math::Rangef::MakeStartToEnd(1.f, 2.f), 16);
		check(1.05f, Math::QuantizationMode::Round, Math::Rangef::MakeStartToEnd(1.f, 2.f), 16);
		check(1.1f, Math::QuantizationMode::Round, Math::Rangef::MakeStartToEnd(1.f, 2.f), 16);
		check(1.25f, Math::QuantizationMode::Round, Math::Rangef::MakeStartToEnd(1.f, 2.f), 16);
		check(1.5f, Math::QuantizationMode::Round, Math::Rangef::MakeStartToEnd(1.f, 2.f), 16);
		check(1.51f, Math::QuantizationMode::Round, Math::Rangef::MakeStartToEnd(1.f, 2.f), 16);
		check(1.7f, Math::QuantizationMode::Round, Math::Rangef::MakeStartToEnd(1.f, 2.f), 16);
		check(1.9f, Math::QuantizationMode::Round, Math::Rangef::MakeStartToEnd(1.f, 2.f), 16);
		check(2.f, Math::QuantizationMode::Round, Math::Rangef::MakeStartToEnd(1.f, 2.f), 16);

		check(0.f, Math::QuantizationMode::AlwaysRoundUp, Math::Rangef::MakeStartToEnd(0.f, 1.f), 16);
		check(0.05f, Math::QuantizationMode::AlwaysRoundUp, Math::Rangef::MakeStartToEnd(0.f, 1.f), 16);
		check(0.1f, Math::QuantizationMode::AlwaysRoundUp, Math::Rangef::MakeStartToEnd(0.f, 1.f), 16);
		check(0.25f, Math::QuantizationMode::AlwaysRoundUp, Math::Rangef::MakeStartToEnd(0.f, 1.f), 16);
		check(0.5f, Math::QuantizationMode::AlwaysRoundUp, Math::Rangef::MakeStartToEnd(0.f, 1.f), 16);
		check(0.51f, Math::QuantizationMode::AlwaysRoundUp, Math::Rangef::MakeStartToEnd(0.f, 1.f), 16);
		check(0.7f, Math::QuantizationMode::AlwaysRoundUp, Math::Rangef::MakeStartToEnd(0.f, 1.f), 16);
		check(0.9f, Math::QuantizationMode::AlwaysRoundUp, Math::Rangef::MakeStartToEnd(0.f, 1.f), 16);
		check(1.f, Math::QuantizationMode::AlwaysRoundUp, Math::Rangef::MakeStartToEnd(0.f, 1.f), 16);

		check(1.f, Math::QuantizationMode::AlwaysRoundUp, Math::Rangef::MakeStartToEnd(1.f, 2.f), 16);
		check(1.05f, Math::QuantizationMode::AlwaysRoundUp, Math::Rangef::MakeStartToEnd(1.f, 2.f), 16);
		check(1.1f, Math::QuantizationMode::AlwaysRoundUp, Math::Rangef::MakeStartToEnd(1.f, 2.f), 16);
		check(1.25f, Math::QuantizationMode::AlwaysRoundUp, Math::Rangef::MakeStartToEnd(1.f, 2.f), 16);
		check(1.5f, Math::QuantizationMode::AlwaysRoundUp, Math::Rangef::MakeStartToEnd(1.f, 2.f), 16);
		check(1.51f, Math::QuantizationMode::AlwaysRoundUp, Math::Rangef::MakeStartToEnd(1.f, 2.f), 16);
		check(1.7f, Math::QuantizationMode::AlwaysRoundUp, Math::Rangef::MakeStartToEnd(1.f, 2.f), 16);
		check(1.9f, Math::QuantizationMode::AlwaysRoundUp, Math::Rangef::MakeStartToEnd(1.f, 2.f), 16);
		check(2.f, Math::QuantizationMode::AlwaysRoundUp, Math::Rangef::MakeStartToEnd(1.f, 2.f), 16);
	}

	UNIT_TEST(Math, OptionalFloat)
	{
		EXPECT_TRUE(Optional<float>{1.f}.IsValid());
		EXPECT_TRUE(Optional<float>{0.f}.IsValid());
		EXPECT_TRUE(Optional<float>{-1.f}.IsValid());
		EXPECT_FALSE(Optional<float>{Invalid}.IsValid());
		EXPECT_TRUE(Optional<float>{Invalid}.IsInvalid());
	}

	UNIT_TEST(Math, OptionalDouble)
	{
		EXPECT_TRUE(Optional<double>{1.0}.IsValid());
		EXPECT_TRUE(Optional<double>{0.0}.IsValid());
		EXPECT_TRUE(Optional<double>{-1.0}.IsValid());
		EXPECT_FALSE(Optional<double>{Invalid}.IsValid());
		EXPECT_TRUE(Optional<double>{Invalid}.IsInvalid());
	}
}
