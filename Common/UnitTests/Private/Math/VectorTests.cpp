#include <Common/Memory/New.h>

#include <Common/Tests/UnitTest.h>

#include <Common/Math/Vector2.h>
#include <Common/Math/Vector3.h>
#include <Common/Math/Vector4.h>

namespace ngine::Tests
{
	UNIT_TEST(Math, Vector3_Construct)
	{
		const Math::Vector3f right{Math::Right};
		EXPECT_TRUE(right.IsEquivalentTo(Math::Vector3f{1.f, 0.f, 0.f}));
		const Math::Vector3f left{Math::Left};
		EXPECT_TRUE(left.IsEquivalentTo(Math::Vector3f{-1.f, 0.f, 0.f}));
		const Math::Vector3f forward{Math::Forward};
		EXPECT_TRUE(forward.IsEquivalentTo(Math::Vector3f{0.f, 1.f, 0.f}));
		const Math::Vector3f back{Math::Backward};
		EXPECT_TRUE(back.IsEquivalentTo(Math::Vector3f{0.f, -1.f, 0.f}));
		const Math::Vector3f up{Math::Up};
		EXPECT_TRUE(up.IsEquivalentTo(Math::Vector3f{0.f, 0.f, 1.f}));
		const Math::Vector3f down{Math::Down};
		EXPECT_TRUE(down.IsEquivalentTo(Math::Vector3f{0.f, 0.f, -1.f}));
	}

	UNIT_TEST(Math, Vector3_Dot)
	{
		EXPECT_NEAR(Math::Vector3f(0.5f, 0.5f, 1.f).Dot(Math::Vector3f{1.f, 0.f, 0.f}), 0.5f, 0.05f);
	}

	UNIT_TEST(Math, Vector3_Cross)
	{
		const Math::Vector3f right{Math::Right};
		EXPECT_TRUE(right.Cross(Math::Forward).IsEquivalentTo(Math::Up));
		EXPECT_TRUE(right.Cross(Math::Down).IsEquivalentTo(Math::Forward));
		const Math::Vector3f left{Math::Left};
		EXPECT_TRUE(left.Cross(Math::Up).IsEquivalentTo(Math::Forward));
		const Math::Vector3f forward{Math::Forward};
		EXPECT_TRUE(forward.Cross(Math::Right).IsEquivalentTo(Math::Down));
	}

	UNIT_TEST(Math, Vector3_Normalize)
	{
		EXPECT_TRUE(Math::Vector3f(0.5f, 0.2f, 0.3f).GetNormalized().IsEquivalentTo(Math::Vector3f{0.811107f, 0.324443f, 0.486664f}));
	}

	UNIT_TEST(Math, Vector3_IsZero)
	{
		EXPECT_TRUE(Math::Vector3f(0.f, 0.f, 0.f).IsZero());
		EXPECT_TRUE(Math::Vector3f(0.f, 0.f, 0.01f).IsZero());
		EXPECT_FALSE(Math::Vector3f(0.f, 0.f, 0.2f).IsZero());
		EXPECT_TRUE(Math::Vector3f(0.f, 0.f, 0.f).IsZeroExact());
		EXPECT_FALSE(Math::Vector3f(0.f, 0.f, 0.01f).IsZeroExact());
	}

	UNIT_TEST(Math, Vector3_IsUnit)
	{
		EXPECT_TRUE(Math::Vector3f(1.f, 0.f, 0.f).IsUnit());
		EXPECT_FALSE(Math::Vector3f(0.f, 0.f, 0.f).IsUnit());
		EXPECT_FALSE(Math::Vector3f(0.f, 0.f, 0.01f).IsUnit());
		EXPECT_FALSE(Math::Vector3f(0.f, 0.f, 0.8f).IsUnit());
		EXPECT_TRUE(Math::Vector3f(0.f, 0.f, 1.f).IsUnit());
	}

	UNIT_TEST(Math, Vector3_Length)
	{
		EXPECT_NEAR(Math::Vector3f(1.f, 0.f, 0.f).GetLength(), 1.f, 0.05f);
		EXPECT_NEAR(Math::Vector3f(0.2f, 0.f, 0.f).GetLength(), 0.2f, 0.05f);
		EXPECT_NEAR(Math::Vector3f(0.2f, 0.2f, 0.f).GetLength(), 0.2828f, 0.05f);
	}

	UNIT_TEST(Math, Vector3_Swizzle)
	{
		EXPECT_TRUE((Math::Vector3f(1.f, 2.f, 3.f).Swizzle<0, 0, 0>().IsEquivalentTo(Math::Vector3f{1.f, 1.f, 1.f})));
		EXPECT_TRUE((Math::Vector3f(1.f, 2.f, 3.f).Swizzle<0, 1, 2>().IsEquivalentTo(Math::Vector3f{1.f, 2.f, 3.f})));
		EXPECT_TRUE((Math::Vector3f(1.f, 2.f, 3.f).Swizzle<2, 1, 0>().IsEquivalentTo(Math::Vector3f{3.f, 2.f, 1.f})));
	}

	UNIT_TEST(Math, Vector3_Equality)
	{
		EXPECT_TRUE((Math::Vector3f(1.f, 2.f, 3.f) == Math::Vector3f(1.f, 2.f, 3.f)).GetMask() == 0b111);
		EXPECT_TRUE((Math::Vector3f(1.f, 2.f, 3.f) == Math::Vector3f(1.f, 2.f, 7.f)).GetMask() == 0b011);
	}
}
