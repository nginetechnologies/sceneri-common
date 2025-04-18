#include <Common/Memory/New.h>

#include <Common/Tests/UnitTest.h>

#include <Common/Math/Quaternion.h>
#include <Common/Math/ScaledQuaternion.h>
#include <Common/Math/Ceil.h>
#include <Common/Compression/Compressor.h>
#include <Common/Compression/Compress.h>
#include <Common/Compression/Decompress.h>

namespace ngine::Tests
{
	UNIT_TEST(Math, Quaternion_IsEquivalentTo)
	{
		{
			const Math::Quaternionf quaternion{Math::Identity};
			ASSERT_TRUE(quaternion.IsIdentity());
			const Math::Quaternionf otherQuaternion{Math::Identity};
			ASSERT_TRUE(quaternion.IsEquivalentTo(otherQuaternion));
		}

		{
			Math::Quaternionf quaternion{Math::CreateRotationAroundXAxis, 5_degrees};
			const Math::Quaternionf otherQuaternion{Math::Identity};
			ASSERT_TRUE(!quaternion.IsEquivalentTo(otherQuaternion));
		}

		{
			Math::Quaternionf quaternion{Math::CreateRotationAroundXAxis, 5_degrees};
			const Math::Quaternionf otherQuaternion{Math::CreateRotationAroundXAxis, 5_degrees};
			ASSERT_TRUE(quaternion.IsEquivalentTo(otherQuaternion));
		}

		{
			Math::Quaternionf quaternion{Math::CreateRotationAroundXAxis, 5_degrees};
			const Math::Quaternionf otherQuaternion{Math::CreateRotationAroundZAxis, 10_degrees};
			ASSERT_TRUE(!quaternion.IsEquivalentTo(otherQuaternion));
		}
	}

	UNIT_TEST(Math, ScaledQuaternion_IsEquivalentTo)
	{
		{
			const Math::ScaledQuaternionf quaternion{Math::Identity};
			ASSERT_TRUE(quaternion.IsIdentity());
			const Math::ScaledQuaternionf otherQuaternion{Math::Identity};
			ASSERT_TRUE(quaternion.IsEquivalentTo(otherQuaternion));
		}

		{
			Math::ScaledQuaternionf quaternion{Math::Quaternionf(Math::CreateRotationAroundXAxis, 5_degrees)};
			ASSERT_FALSE(quaternion.IsIdentity());
			const Math::ScaledQuaternionf otherQuaternion{Math::Identity};
			ASSERT_TRUE(!quaternion.IsEquivalentTo(otherQuaternion));
		}

		{
			Math::ScaledQuaternionf quaternion{Math::Quaternionf(Math::CreateRotationAroundXAxis, 5_degrees), {2.f, 2.f, 2.f}};
			const Math::ScaledQuaternionf otherQuaternion{Math::Quaternionf(Math::CreateRotationAroundXAxis, 5_degrees), {2.f, 2.f, 2.f}};
			ASSERT_TRUE(quaternion.IsEquivalentTo(otherQuaternion));
		}

		{
			Math::ScaledQuaternionf quaternion{Math::Quaternionf(Math::CreateRotationAroundXAxis, 5_degrees)};
			const Math::ScaledQuaternionf otherQuaternion{Math::Quaternionf(Math::CreateRotationAroundZAxis, 10_degrees)};
			ASSERT_TRUE(!quaternion.IsEquivalentTo(otherQuaternion));
		}
	}

	UNIT_TEST(Math, Quaternion_MatrixEquivalents)
	{
		{
			const Math::Quaternionf quaternion{Math::Identity};
			const Math::Matrix3x3f matrix{Math::Identity};
			ASSERT_TRUE(matrix.IsOrthonormalized());
			ASSERT_TRUE(quaternion.IsEquivalentTo((Math::Quaternionf)matrix));
		}
		{
			const Math::Quaternionf quaternion{Math::Forward};
			const Math::Matrix3x3f matrix{Math::Forward};
			ASSERT_TRUE(matrix.IsOrthonormalized());
			ASSERT_TRUE(quaternion.IsEquivalentTo((Math::Quaternionf)matrix));
		}
		{
			const Math::Quaternionf quaternion{Math::Backward};
			const Math::Matrix3x3f matrix{Math::Backward};
			ASSERT_TRUE(matrix.IsOrthonormalized());
			ASSERT_TRUE(quaternion.IsEquivalentTo((Math::Quaternionf)matrix));
		}
		{
			const Math::Quaternionf quaternion{Math::Up};
			const Math::Matrix3x3f matrix{Math::Up};
			ASSERT_TRUE(matrix.IsOrthonormalized());
			ASSERT_TRUE(quaternion.IsEquivalentTo((Math::Quaternionf)matrix));
		}
		{
			const Math::Quaternionf quaternion{Math::Down};
			const Math::Matrix3x3f matrix{Math::Down};
			ASSERT_TRUE(matrix.IsOrthonormalized());
			const Math::Quaternionf result{(Math::Quaternionf)matrix};
			ASSERT_TRUE(quaternion.IsEquivalentTo(result));
		}
		{
			const Math::Quaternionf quaternion{Math::Right};
			const Math::Matrix3x3f matrix{Math::Right};
			ASSERT_TRUE(matrix.IsOrthonormalized());
			ASSERT_TRUE(quaternion.IsEquivalentTo((Math::Quaternionf)matrix));
		}
		{
			const Math::Quaternionf quaternion{Math::Left};
			const Math::Matrix3x3f matrix{Math::Left};
			ASSERT_TRUE(matrix.IsOrthonormalized());
			ASSERT_TRUE(quaternion.IsEquivalentTo((Math::Quaternionf)matrix));
		}
		{
			const Math::Quaternionf quaternion{Math::CreateRotationAroundAxis, 5_degrees, Math::Up};
			const Math::Matrix3x3f matrix{Math::CreateRotationAroundAxis, 5_degrees, Math::Up};
			ASSERT_TRUE(matrix.IsOrthonormalized());
			ASSERT_TRUE(quaternion.IsEquivalentTo((Math::Quaternionf)matrix));
		}
		{
			const Math::Quaternionf quaternion{Math::CreateRotationAroundXAxis, 5_degrees};
			const Math::Matrix3x3f matrix{Math::CreateRotationAroundXAxis, 5_degrees};
			ASSERT_TRUE(matrix.IsOrthonormalized());
			ASSERT_TRUE(quaternion.IsEquivalentTo((Math::Quaternionf)matrix));
		}
		{
			const Math::Quaternionf quaternion{Math::CreateRotationAroundYAxis, 5_degrees};
			const Math::Matrix3x3f matrix{Math::CreateRotationAroundYAxis, 5_degrees};
			ASSERT_TRUE(matrix.IsOrthonormalized());
			ASSERT_TRUE(quaternion.IsEquivalentTo((Math::Quaternionf)matrix));
		}
		{
			const Math::Quaternionf quaternion{Math::CreateRotationAroundZAxis, 5_degrees};
			const Math::Matrix3x3f matrix{Math::CreateRotationAroundZAxis, 5_degrees};
			ASSERT_TRUE(matrix.IsOrthonormalized());
			ASSERT_TRUE(quaternion.IsEquivalentTo((Math::Quaternionf)matrix));
		}
		{
			const Math::Quaternionf quaternion{Math::EulerAnglesf{5_degrees, 4_degrees, 3_degrees}};
			const Math::Matrix3x3f matrix{Math::EulerAnglesf{5_degrees, 4_degrees, 3_degrees}};
			ASSERT_TRUE(matrix.IsOrthonormalized());
			ASSERT_TRUE(quaternion.IsEquivalentTo((Math::Quaternionf)matrix));
		}
		/*{
		  const Math::Quaternionf quaternion{Math::YawPitchRollf{5_degrees, 4_degrees, 3_degrees}};
		  const Math::Matrix3x3f matrix{Math::YawPitchRollf{5_degrees, 4_degrees, 3_degrees}};
		  ASSERT_TRUE(quaternion.IsEquivalentTo((Math::Quaternionf)matrix));
		}*/
		{
			const Math::Quaternionf quaternion{Math::Forward, Math::Up};
			ASSERT_TRUE(quaternion.IsEquivalentTo(Math::Quaternionf(Math::Right, Math::Forward, Math::Up)));
			const Math::Matrix3x3f matrix{Math::Forward, Math::Up};
			ASSERT_TRUE(matrix.IsOrthonormalized());
			ASSERT_TRUE(matrix.IsEquivalentTo(Math::Matrix3x3f(Math::Right, Math::Forward, Math::Up)));
			ASSERT_TRUE(quaternion.IsEquivalentTo((Math::Quaternionf)matrix));
		}
		{
			const Math::Quaternionf quaternion{Math::Down, Math::Forward};
			ASSERT_TRUE(quaternion.IsEquivalentTo(Math::Quaternionf(Math::Right, Math::Down, Math::Forward)));
			const Math::Matrix3x3f matrix{Math::Down, Math::Forward};
			ASSERT_TRUE(matrix.IsOrthonormalized());
			ASSERT_TRUE(matrix.IsEquivalentTo(Math::Matrix3x3f(Math::Right, Math::Down, Math::Forward)));
			ASSERT_TRUE(quaternion.IsEquivalentTo((Math::Quaternionf)matrix));
		}
	}

	UNIT_TEST(Math, Quaternion_Compression)
	{
		EXPECT_FALSE(Compression::IsDynamicallyCompressed<Math::Quaternionf>());
		constexpr uint32 requiredByteCount = (uint32)Math::Ceil((float)Compression::CalculateFixedDataSize<Math::Quaternionf>() / 8.f);
		Array<char, requiredByteCount> data;

		{
			const Math::Quaternionf referenceQuaternion{Math::Identity};
			{
				data.GetView().ZeroInitialize();
				BitView packedTarget(data.GetDynamicView());
				EXPECT_TRUE(Compression::Compress(referenceQuaternion, packedTarget));
			}
			{
				ConstBitView packedSource(data.GetDynamicView());
				Math::Quaternionf decompressed;
				EXPECT_TRUE(Compression::Decompress<Math::Quaternionf>(decompressed, packedSource));
				EXPECT_TRUE(decompressed.IsEquivalentTo(referenceQuaternion));
				EXPECT_TRUE(decompressed.IsIdentity());
			}
		}

		{
			Math::Quaternionf referenceQuaternion{Math::CreateRotationAroundXAxis, 5_degrees};
			EXPECT_FALSE(referenceQuaternion.IsEquivalentTo(Math::Identity));
			{
				data.GetView().ZeroInitialize();
				BitView packedTarget(data.GetDynamicView());
				EXPECT_TRUE(Compression::Compress(referenceQuaternion, packedTarget));
			}
			{
				ConstBitView packedSource(data.GetDynamicView());
				Math::Quaternionf decompressed;
				EXPECT_TRUE(Compression::Decompress<Math::Quaternionf>(decompressed, packedSource));
				EXPECT_TRUE(decompressed.IsEquivalentTo(referenceQuaternion));
			}
		}

		{
			Math::Quaternionf referenceQuaternion{Math::CreateRotationAroundXAxis, 0.1_degrees};
			{
				data.GetView().ZeroInitialize();
				BitView packedTarget(data.GetDynamicView());
				EXPECT_TRUE(Compression::Compress(referenceQuaternion, packedTarget));
			}
			{
				ConstBitView packedSource(data.GetDynamicView());
				Math::Quaternionf decompressed;
				EXPECT_TRUE(Compression::Decompress<Math::Quaternionf>(decompressed, packedSource));
				EXPECT_TRUE(decompressed.IsEquivalentTo(referenceQuaternion));
			}
		}

		{
			Math::Quaternionf referenceQuaternion{Math::CreateRotationAroundYAxis, 14.5_degrees};
			{
				data.GetView().ZeroInitialize();
				BitView packedTarget(data.GetDynamicView());
				EXPECT_TRUE(Compression::Compress(referenceQuaternion, packedTarget));
			}
			{
				ConstBitView packedSource(data.GetDynamicView());
				Math::Quaternionf decompressed;
				EXPECT_TRUE(Compression::Decompress<Math::Quaternionf>(decompressed, packedSource));
				EXPECT_TRUE(decompressed.IsEquivalentTo(referenceQuaternion));
			}
		}

		{
			Math::Quaternionf referenceQuaternion{Math::CreateRotationAroundZAxis, 78.3_degrees};
			{
				data.GetView().ZeroInitialize();
				BitView packedTarget(data.GetDynamicView());
				EXPECT_TRUE(Compression::Compress(referenceQuaternion, packedTarget));
			}
			{
				ConstBitView packedSource(data.GetDynamicView());
				Math::Quaternionf decompressed;
				EXPECT_TRUE(Compression::Decompress<Math::Quaternionf>(decompressed, packedSource));
				EXPECT_TRUE(decompressed.IsEquivalentTo(referenceQuaternion));
			}
		}
	}

}
