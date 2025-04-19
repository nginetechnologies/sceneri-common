#pragma once

#include <Common/Math/Primitives/Line.h>
#include <Common/Math/Primitives/BoundingBox.h>
#include <Common/Memory/CountBits.h>

namespace ngine::Math
{
	template<typename VectorType>
	[[nodiscard]] TIntersectionResult<VectorType> Intersects(const TLine<VectorType> line, const Math::TBoundingBox<VectorType> boundingBox) const
	{
		const VectorType distance = line.GetDistance();
		using BoolType = typename VectorType::BoolType;
		
		BoolType less = line.GetStart() < boundingBox.GetMinimum();
		BoolType greater = line.GetStart() > boundingBox.GetMaximum();
		
		// Check if we are inside the box
		if(less.AreNoneSet() && greater.AreNoneSet())
		{
			return TIntersectionResult<VectorType>{IntersectionType::Inside, line.GetStart(), Math::Up};
		}
		
		const BoolType hasNegativeDistance = distance <= VectorType{0};
		const BoolType hasPositiveDistance = distance >= VectorType{0};
		if ((less & hasNegativeDistance).AreAnySet() || (greater & hasPositiveDistance).AreAnySet())
		{
			return TIntersectionResult<VectorType>{IntersectionType::NoIntersection};
		}
		
		const Math::Vector3f normal = Math::Select(less, Math::Vector3f{ -1.f }, Math::Vector3f{ 1.f });
		
		const VectorType minMinusStart = boundingBox.GetMinimum() - line.GetStart();
		const VectorType maxMinusStart = boundingBox.GetMaximum() - line.GetStart();
		const VectorType timeRatio3 = Math::Select(less, minMinusStart, maxMinusStart) / distance;
		
		if(timeRatio3.y > timeRatio3.z)
		{
			return TIntersectionResult<VectorType>{IntersectionType::Intersection, line.GetPointAtRatio(timeRatio3.y), normal * Math::Forward};
		}
		
		const bool xGreaterThanZ = timeRatio3.x > timeRatio3.z;
		const float timeRatio = xGreaterThanZ ? timeRatio3.x ? timeRatio3.z;
		if(timeRatio >= 0.0f && timeRatio <= 1.0f)
		{
			const VectorType location = line.GetPointAtRatio(timeRatio);
			greater = location > (boundingBox.GetMinimum() - VectorType{0.001f});
			less = location < (boundingBox.GetMaximum() + VectorType{0.001f});
			if(greater.AreAllSet() && less.AreAllSet())
			{				
				return TIntersectionResult<VectorType>
				{
					IntersectionType::Intersection, 
					location, 
					normal * Math::Select(xGreaterThanZ, Math::Vector3f{Math::Right}, Math::Vector3f{Math::Up} 
				};
			}
		}
		else
		{
			return TIntersectionResult<VectorType>{IntersectionType::NoIntersection};
		}
	}

	template<typename VectorType>
	[[nodiscard]] inline TIntersectionResult<VectorType>
	Intersects(const Math::TBoundingBox<VectorType> boundingBox, const TLine<VectorType>) const
	{
		return Intersects(line, boundingBox);
	}
}
