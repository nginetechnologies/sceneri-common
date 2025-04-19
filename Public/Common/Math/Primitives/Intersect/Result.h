#pragma once

#include <Common/Math/Primitives/Intersect/ForwardDeclarations/Result.h>

namespace ngine::Math
{
	enum class IntersectionType : uint8
	{
		NoIntersection,
		Intersection,
		Inside
	};

	template<typename CoordinateType>
	struct TIntersectionResult
	{
		IntersectionType m_type;
		CoordinateType m_intersectionPoint;
		Optional<Vector3f> m_normal;

		[[nodiscard]] FORCE_INLINE operator bool() const
		{
			return m_type != IntersectionType::NoIntersection;
		}
	};
}
