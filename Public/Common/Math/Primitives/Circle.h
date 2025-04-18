#pragma once

#include <Common/Math/Primitives/ForwardDeclarations/Circle.h>
#include <Common/Math/Vector2.h>
#include <Common/Math/Random.h>
#include <Common/Platform/TrivialABI.h>

namespace ngine::Math
{
	template<typename UnitType_>
	struct TRIVIAL_ABI TCircle
	{
		using UnitType = UnitType_;
		using VectorType = TVector2<UnitType>;
		using RadiusType = Radius<UnitType>;

		TCircle(const VectorType position, const RadiusType radius)
			: m_position(position)
			, m_radius(radius)
		{
		}

		void SetCenter(const VectorType center)
		{
			m_position = center;
		}
		[[nodiscard]] FORCE_INLINE VectorType GetCenter() const
		{
			return m_position;
		}
		void SetRadius(const RadiusType radius)
		{
			m_radius = radius;
		}
		[[nodiscard]] FORCE_INLINE RadiusType GetRadius() const
		{
			return m_radius;
		}
		[[nodiscard]] FORCE_INLINE UnitType GetRadiusSquared() const
		{
			return (m_radius * m_radius).GetMeters();
		}
		[[nodiscard]] FORCE_INLINE UnitType GetRadiusCubed() const
		{
			return (m_radius * m_radius * m_radius).GetMeters();
		}

		[[nodiscard]] FORCE_INLINE bool Contains(const VectorType point) const
		{
			return GetRadiusSquared() > (point - m_position).GetLengthSquared();
		}

		[[nodiscard]] VectorType GetRandomLocation() const
		{
			const UnitType u = Random<UnitType>();
			const UnitType v = Random<UnitType>();
			const UnitType theta = u * UnitType(2) * PI.GetRadians();
			const UnitType phi = Acos(UnitType(2) * v - UnitType(1));
			const UnitType r = CubicRoot(Random<UnitType>());
			UnitType cosTheta;
			const UnitType sinTheta = SinCos(theta, cosTheta);
			UnitType cosPhi;
			const UnitType sinPhi = SinCos(phi, cosPhi);

			return m_position + VectorType{m_radius.GetMeters() * r * sinPhi * cosTheta, m_radius.GetMeters() * r * sinPhi * sinTheta};
		}
	protected:
		VectorType m_position;
		RadiusType m_radius;
	};
}
