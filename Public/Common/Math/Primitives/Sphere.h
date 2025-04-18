#pragma once

#include <Common/Math/Primitives/ForwardDeclarations/Sphere.h>
#include <Common/Math/ForwardDeclarations/Radius.h>
#include <Common/Math/Radius.h>
#include <Common/Math/Random.h>
#include <Common/Math/SinCos.h>
#include <Common/Math/Acos.h>
#include <Common/Math/CubicRoot.h>
#include <Common/Platform/TrivialABI.h>

namespace ngine::Math
{
	template<typename _VectorType>
	struct TRIVIAL_ABI TSphere
	{
		using VectorType = _VectorType;
		using UnitType = typename VectorType::UnitType;
		using RadiusType = Radius<UnitType>;

		TSphere(const VectorType position, const RadiusType radius)
			: m_position(position)
			, m_radius(radius)
		{
		}

		[[nodiscard]] FORCE_INLINE VectorType GetPosition() const
		{
			return m_position;
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

		[[nodiscard]] FORCE_INLINE UnitType GetSurfaceArea() const
		{
			return UnitType(4) * PI.GetRadians() * GetRadiusSquared();
		}

		[[nodiscard]] FORCE_INLINE UnitType GetVolume() const
		{
			static constexpr UnitType fourThreeQuarters = UnitType(4) / UnitType(3);
			return fourThreeQuarters * PI.GetRadians() * GetRadiusCubed();
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

			return m_position + VectorType{
														m_radius.GetMeters() * r * sinPhi * cosTheta,
														m_radius.GetMeters() * r * sinPhi * sinTheta,
														m_radius.GetMeters() * r * cosPhi
													};
		}
	protected:
		VectorType m_position;
		RadiusType m_radius;
	};
}
