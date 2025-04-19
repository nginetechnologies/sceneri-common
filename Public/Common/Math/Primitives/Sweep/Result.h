#pragma once

#include <Common/Math/Primitives/Sweep/ForwardDeclarations/Result.h>

namespace ngine::Math
{
	enum class SweepIntersectionType : uint8
	{
		None,
		InitialOverlap,
		Intersection
	};

	template<typename VectorType>
	struct TSweepResult
	{
		[[nodiscard]] FORCE_INLINE operator bool() const
		{
			return m_type != SweepIntersectionType::None;
		}

		SweepIntersectionType m_type = SweepIntersectionType::None;
		typename VectorType::UnitType m_intersectionTime;
		VectorType m_intersectionPoint;
	};
}
