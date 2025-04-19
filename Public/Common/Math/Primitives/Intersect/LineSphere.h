#pragma once

#include <Common/Math/Primitives/Line.h>
#include <Common/Math/Primitives/Sphere.h>
#include <Common/Math/Primitives/Intersect/Result.h>

namespace ngine::Math
{
	template<typename CoordinateType>
	struct LineSphereIntersectionResult
	{
		IntersectionType m_type;
		Optional<CoordinateType> m_entryIntersection;
		Optional<CoordinateType> m_exitIntersection;

		[[nodiscard]] FORCE_INLINE operator bool() const
		{
			return m_type != IntersectionType::NoIntersection;
		}
	};

	template<typename VectorType>
	[[nodiscard]] LineSphereIntersectionResult<VectorType> Intersects(const TLine<VectorType> line, const TSphere<VectorType> sphere)
	{
		/*if (sphere.Contains(line.GetStart()) & sphere.Contains(line.GetEnd()))
		{
		  return LineSphereIntersectionResult<VectorType>{ IntersectionType::Inside };
		}*/

		const VectorType distance = line.GetDistance();
		const float lengthSquared = distance.GetLengthSquared();

		const VectorType distanceToCenter = line.GetStart() - sphere.GetPosition();
		const float b = distance.Dot(distanceToCenter) * 2.0f;
		const float c = distanceToCenter.GetLengthSquared() - sphere.GetRadiusSquared();
		const float desc = (b * b) - (4 * lengthSquared * c);

		if (desc >= 0.0f)
		{
			const float lamba0 = (-b - Math::Sqrt(desc)) / (2.0f * lengthSquared);
			Optional<VectorType> entryPoint;
			if (lamba0 > 0.f)
			{
				entryPoint = line.GetStart() + (distance * lamba0);

				if ((*entryPoint - line.GetEnd()).Dot(distance) > 0)
				{
					return LineSphereIntersectionResult<VectorType>{IntersectionType::NoIntersection};
				}
			}

			const float lamba1 = (-b + Math::Sqrt(desc)) / (2.0f * lengthSquared);
			if (lamba1 > 0.0f)
			{
				const VectorType exitPoint = line.GetStart() + (distance * lamba1);
				if ((exitPoint - line.GetEnd()).Dot(distance) > 0)
				{
					if (entryPoint.IsValid())
					{
						return LineSphereIntersectionResult<VectorType>{IntersectionType::Intersection, entryPoint};
					}
					else
					{
						return LineSphereIntersectionResult<VectorType>{IntersectionType::NoIntersection};
					}
				}

				return LineSphereIntersectionResult<VectorType>{IntersectionType::Intersection, entryPoint, exitPoint};
			}
			else if (entryPoint.IsValid())
			{
				return LineSphereIntersectionResult<VectorType>{IntersectionType::Intersection, entryPoint};
			}
		}

		return LineSphereIntersectionResult<VectorType>{IntersectionType::NoIntersection};
	}

	template<typename VectorType>
	[[nodiscard]] LineSphereIntersectionResult<VectorType> Intersects(const TSphere<VectorType> sphere, const TLine<VectorType> line)
	{
		return Intersects<VectorType>(line, sphere);
	}
}
