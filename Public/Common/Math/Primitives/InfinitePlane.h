#pragma once

#include <Common/Math/Primitives/ForwardDeclarations/InfinitePlane.h>
#include <Common/Platform/TrivialABI.h>

namespace ngine::Math
{
	template<typename CoordinateType>
	struct TRIVIAL_ABI TInfinitePlane
	{
		using VectorType = CoordinateType;
		using UnitType = typename VectorType::UnitType;

		constexpr TInfinitePlane(const Vector3f normal, const UnitType distance)
			: m_normal(normal)
			, m_distance(distance)
		{
		}
		constexpr TInfinitePlane(const Vector3f normal, const CoordinateType location)
			: m_normal(normal)
			, m_distance(-location.Dot(normal))
		{
		}
		constexpr TInfinitePlane(const CoordinateType location0, const CoordinateType location1, const CoordinateType location2)
			: m_normal(((location2 - location1).Cross(location0 - location1)).GetNormalized())
			, m_distance(-m_normal.Dot(location2))
		{
		}

		[[nodiscard]] FORCE_INLINE Vector3f GetNormal() const
		{
			return m_normal;
		}
		[[nodiscard]] FORCE_INLINE UnitType GetDistance() const
		{
			return m_distance;
		}
	protected:
		Vector3f m_normal;
		UnitType m_distance;
	};
}
