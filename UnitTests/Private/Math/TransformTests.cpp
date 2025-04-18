#include <Common/Memory/New.h>

#include <Common/Tests/UnitTest.h>

#include <Common/Math/Transform.h>

namespace ngine::Tests
{
	UNIT_TEST(Math, Transform_GetTransformRelativeTo)
	{
		using TransformType = Math::TTransform<Math::Vector3f, float>;
		ASSERT_TRUE(TransformType(Math::Identity).IsIdentity());

		{
			TransformType transform{Math::Quaternionf{Math::Identity}, Math::Vector3f{0, 1, 2}, Math::Vector3f{1, 1, 1}};
			TransformType parentTransform{Math::Quaternionf{Math::Identity}, Math::Vector3f{0, 0, 2}, Math::Vector3f{1, 1, 1}};
			const TransformType relativeTransform = parentTransform.GetTransformRelativeTo(transform);
			const TransformType result = parentTransform.Transform(relativeTransform);
			ASSERT_TRUE(result.GetStoredRotation().IsEquivalentTo(transform.GetStoredRotation()));
			ASSERT_TRUE(result.IsEquivalentTo(transform));
		}

		{
			TransformType transform{
				Math::Quaternionf{Math::CreateRotationAroundXAxis, 5_degrees},
				Math::Vector3f{0, 1, 2},
				Math::Vector3f{1, 1, 1}
			};
			TransformType parentTransform{Math::Quaternionf{Math::Identity}, Math::Vector3f{0, 0, 2}, Math::Vector3f{1, 1, 1}};
			const TransformType relativeTransform = parentTransform.GetTransformRelativeTo(transform);
			const TransformType result = parentTransform.Transform(relativeTransform);
			ASSERT_TRUE(result.IsEquivalentTo(transform));
		}

		{
			TransformType transform{
				Math::Quaternionf{Math::CreateRotationAroundXAxis, 5_degrees},
				Math::Vector3f{0, 1, 2},
				Math::Vector3f{1, 1, 1}
			};
			TransformType parentTransform{
				Math::Quaternionf{Math::CreateRotationAroundZAxis, 10_degrees},
				Math::Vector3f{0, 0, 2},
				Math::Vector3f{1, 1, 1}
			};
			const TransformType relativeTransform = parentTransform.GetTransformRelativeTo(transform);
			const TransformType result = parentTransform.Transform(relativeTransform);
			ASSERT_TRUE(result.IsEquivalentTo(transform));
		}

		// Negative scale
		{
			TransformType transform{Math::Quaternionf{Math::Forward}, Math::Zero, Math::Vector3f{1, 1, 1}};
			TransformType parentTransform{Math::Quaternionf{Math::Forward}, Math::Zero, Math::Vector3f{1, -1, 1}};
			const TransformType relativeTransform = parentTransform.GetTransformRelativeTo(transform);
			ASSERT_TRUE(
				parentTransform.GetTransformRelativeToAsMatrix(transform).IsEquivalentTo(parentTransform.GetTransformRelativeToIdeal(transform))
			);
			const TransformType result = parentTransform.Transform(relativeTransform);
			ASSERT_TRUE(parentTransform.TransformAsMatrix(relativeTransform).IsEquivalentTo(parentTransform.TransformIdeal(relativeTransform)));
			ASSERT_TRUE(parentTransform.TransformAsMatrix(relativeTransform)
			              .GetForwardColumn()
			              .IsEquivalentTo(parentTransform.TransformIdeal(relativeTransform).GetForwardColumn()));
			ASSERT_TRUE(result.IsEquivalentTo(transform));
		}

		{
			TransformType transform{Math::Identity, Math::Vector3f{14.1572237f, 11.156455f, 0.00808489322f}, Math::Vector3f{1.5f}};
			TransformType parentTransform{Math::Identity, Math::Vector3f{14.1623793f, 11.1070652f, 0}, Math::Vector3f{1.5f}};

			const TransformType relativeTransform = parentTransform.GetTransformRelativeTo(transform);
			const TransformType relativeTransformAsMatrix = parentTransform.GetTransformRelativeToAsMatrix(transform);
			const TransformType relativeTransformIdeal = parentTransform.GetTransformRelativeToIdeal(transform);
			ASSERT_TRUE(relativeTransformAsMatrix.IsEquivalentTo(relativeTransformIdeal));

			const TransformType result = parentTransform.Transform(relativeTransform);
			ASSERT_TRUE(parentTransform.TransformAsMatrix(relativeTransform).IsEquivalentTo(parentTransform.TransformIdeal(relativeTransform)));
			ASSERT_TRUE(result.IsEquivalentTo(transform));
		}

		{
			TransformType transform{
				Math::Quaternionf{Math::CreateRotationAroundXAxis, 5_degrees},
				Math::Vector3f{0, 1, 2},
				Math::Vector3f{1, 1, 1}
			};
			TransformType parentTransform{
				Math::Quaternionf{Math::CreateRotationAroundZAxis, 10_degrees},
				Math::Vector3f{0, 0, 2},
				Math::Vector3f{1, 1, 1}
			};
			const TransformType relativeTransform = parentTransform.GetTransformRelativeTo(transform);
			const TransformType result = parentTransform.Transform(relativeTransform);
			ASSERT_TRUE(result.IsEquivalentTo(transform));
		}
	}
}
