#pragma once

#include <Common/Math/Vector3.h>
#include <Common/Math/Vector3/Min.h>
#include <Common/Math/Vector3/Max.h>
#include <Common/Math/Vector3/Select.h>
#include <Common/Math/Vector2.h>
#include <Common/Math/Vector2/Min.h>
#include <Common/Math/Vector2/Max.h>
#include <Common/Math/Vector2/Select.h>
#include <Common/Math/Length.h>
#include <Common/Math/ForwardDeclarations/Radius.h>
#include <Common/Platform/TrivialABI.h>

namespace ngine::Math
{
	template<typename CoordinateType>
	struct TRIVIAL_ABI TBoundingBox
	{
		using VectorType = CoordinateType;
		using UnitType = typename CoordinateType::UnitType;
		using RadiusType = Math::Radius<UnitType>;

		TBoundingBox()
		{
		}

		FORCE_INLINE constexpr TBoundingBox(const RadiusType radius)
			: m_minimum(-radius.GetMeters())
			, m_maximum(radius.GetMeters())
		{
		}

		FORCE_INLINE constexpr TBoundingBox(const VectorType minimum, const VectorType maximum)
			: m_minimum(minimum)
			, m_maximum(maximum)
		{
		}

		explicit FORCE_INLINE constexpr TBoundingBox(const VectorType coordinate)
			: m_minimum(coordinate)
			, m_maximum(coordinate)
		{
		}

		FORCE_INLINE constexpr TBoundingBox(const VectorType coordinate, const RadiusType radius)
			: m_minimum(coordinate - VectorType{radius.GetMeters()})
			, m_maximum(coordinate + VectorType{radius.GetMeters()})
		{
		}

		template<typename OtherCoordinateType>
		FORCE_INLINE constexpr TBoundingBox(const TBoundingBox<OtherCoordinateType>& other)
			: m_minimum((VectorType)other.m_minimum)
			, m_maximum((VectorType)other.m_maximum)
		{
		}

		FORCE_INLINE constexpr TBoundingBox(const ZeroType)
			: m_minimum(Zero)
			, m_maximum(Zero)
		{
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS VectorType GetSize() const
		{
			return m_maximum - m_minimum;
		}
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS bool IsZero() const
		{
			return Math::Abs(GetSize().GetLength()) == 0.f;
		}

		FORCE_INLINE constexpr void Expand(const TBoundingBox other)
		{
			m_minimum = Math::Min(m_minimum, other.m_minimum);
			m_maximum = Math::Max(m_maximum, other.m_maximum);
		}

		FORCE_INLINE void Expand(const VectorType other)
		{
			m_minimum = Math::Min(m_minimum, other);
			m_maximum = Math::Max(m_maximum, other);
		}

		FORCE_INLINE constexpr Array<VectorType, 4> GetCorners() const
		{
			return {
				VectorType{m_minimum.x, m_minimum.y, m_minimum.z},
				VectorType{m_maximum.x, m_minimum.y, m_minimum.z},
				VectorType{m_minimum.x, m_maximum.y, m_minimum.z},
				VectorType{m_minimum.x, m_minimum.y, m_maximum.z},
			};
		}

		FORCE_INLINE constexpr Array<VectorType, 8> GetAllCorners() const
		{
			return {
				VectorType{m_minimum.x, m_minimum.y, m_minimum.z},
				VectorType{m_maximum.x, m_minimum.y, m_minimum.z},
				VectorType{m_minimum.x, m_maximum.y, m_minimum.z},
				VectorType{m_maximum.x, m_maximum.y, m_minimum.z},
				VectorType{m_minimum.x, m_minimum.y, m_maximum.z},
				VectorType{m_maximum.x, m_minimum.y, m_maximum.z},
				VectorType{m_minimum.x, m_maximum.y, m_maximum.z},
				VectorType{m_maximum.x, m_maximum.y, m_maximum.z}
			};
		}

		FORCE_INLINE constexpr void Reset()
		{
			m_minimum = Math::Zero;
			m_maximum = Math::Zero;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr bool Contains(const VectorType point) const
		{
			return ((point >= m_minimum) & (point <= m_maximum)).AreAllSet();
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr bool Contains(const TBoundingBox other) const
		{
			return Contains(other.m_minimum) & Contains(other.m_maximum);
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr UnitType GetClosestDistanceSquared(const VectorType point) const
		{
			VectorType minimumDistance = Math::Max(m_minimum - point, VectorType{Math::Zero});
			minimumDistance *= minimumDistance;

			VectorType maximumDistance = Math::Max(point - m_maximum, VectorType{Math::Zero});
			maximumDistance *= maximumDistance;

			return minimumDistance.GetSum() + maximumDistance.GetSum();
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr UnitType GetClosestDistance(const VectorType point) const
		{
			return Math::Sqrt(GetClosestDistanceSquared(point));
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr bool IsVisibleFromFrustum() const
		{
			// TODO
			return true;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr RadiusType GetRadius() const
		{
			const VectorType size = GetSize();
			return RadiusType::FromMeters(Math::Max(size.x, size.y, size.z) * UnitType(0.5));
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr UnitType GetRadiusSquared() const
		{
			const RadiusType radius = GetRadius();
			return (radius * radius).GetMeters();
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr UnitType GetVolume() const
		{
			const VectorType size = GetSize();
			return size.x * size.y * Math::Select(Math::Abs(size.z) > 0.0005f, size.z, 1.f);
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr VectorType GetMinimum() const
		{
			return m_minimum;
		}
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr VectorType GetMaximum() const
		{
			return m_maximum;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr VectorType GetCenter() const
		{
			return (m_minimum + m_maximum) * UnitType(0.5);
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TBoundingBox operator+(const VectorType other) const
		{
			return TBoundingBox{m_minimum + other, m_maximum + other};
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TBoundingBox operator-(const VectorType other) const
		{
			return TBoundingBox{m_minimum - other, m_maximum - other};
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TBoundingBox operator*(const VectorType other) const
		{
			return TBoundingBox{m_minimum * other, m_maximum * other};
		}

		VectorType m_minimum;
		VectorType m_maximum;
	};

	using BoundingBox = TBoundingBox<TVector3<float>>;
}
