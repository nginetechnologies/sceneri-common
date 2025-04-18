#include <Common/Memory/New.h>

#include <Common/Tests/UnitTest.h>

#include <Common/Math/Primitives/BoundingBox.h>
#include <Common/Math/Radius.h>

namespace ngine::Tests
{
	UNIT_TEST(Math, BoundingBox_Contains)
	{
		Math::BoundingBox testBox(Math::Radiusf(0.5_meters));
		Math::Vector3f outside(2.0f, 1.0f, 0.0);
		Math::Vector3f inside(0.1f, -0.1f, 0.2f);
		Math::Vector3f zero(Math::Zero);
		EXPECT_TRUE(testBox.Contains(outside) == false);
		EXPECT_TRUE(testBox.Contains(inside) == true);
		EXPECT_TRUE(testBox.Contains(zero) == true);
	}
}
