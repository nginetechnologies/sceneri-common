#include <Common/Memory/New.h>

#include <Common/Tests/UnitTest.h>

#include <Common/Math/CompressedDirectionAndSign.h>

namespace ngine::Tests
{
	UNIT_TEST(Math, CompressedDirectionAndSign)
	{
		{
			const Math::Vector4f input{1, 0, 0, 1};
			const Math::CompressedDirectionAndSign compressed{input};
			const Math::Vector4f decompressed{compressed};

			EXPECT_TRUE(input.IsEquivalentTo(decompressed, 0.01f));
		}

		{
			const Math::Vector4f input{0.5f, 0.5f, 0, 1};
			const Math::CompressedDirectionAndSign compressed{input};
			const Math::Vector4f decompressed{compressed};

			EXPECT_TRUE(input.IsEquivalentTo(decompressed, 0.01f));
		}

		{
			const Math::Vector4f input{0.5f, 0.3f, 0.2f, -1};
			const Math::CompressedDirectionAndSign compressed{input};
			const Math::Vector4f decompressed{compressed};

			EXPECT_TRUE(input.IsEquivalentTo(decompressed, 0.01f));
		}
	}
}
