#pragma once

#include "ForwardDeclarations/Transform2D.h"
#include "Vector4.h"
#include "Vector2.h"
#include "Angle.h"
#include "Rotation2D.h"

#include <Common/Math/Vector2/MultiplicativeInverse.h>

#include <Common/Serialization/ForwardDeclarations/Reader.h>
#include <Common/Serialization/ForwardDeclarations/Writer.h>
#include <Common/Platform/TrivialABI.h>

namespace ngine::Math
{
	template<typename UnitType>
	struct TRIVIAL_ABI TTransform2D
	{
		inline static constexpr Guid TypeGuid = "9b32862e-7484-4f69-b509-2d5f21c50aee"_guid;

		using Vector2Type = TVector2<UnitType>;
		using Vector4Type = TVector4<UnitType>;

		using RotationType = TRotation2D<UnitType>;
		using CoordinateType = Vector2Type;
		using ScaleType = Vector2Type;

		TTransform2D() = default;
		FORCE_INLINE TTransform2D(IdentityType)
			: m_locationAndScale(0, 0, 0, 0)
			, m_rotation(0_degrees)
		{
		}
		FORCE_INLINE TTransform2D(
			const RotationType rotation, const CoordinateType location = Math::Zero, const ScaleType scale = {UnitType(1), UnitType(1)}
		)
			: m_locationAndScale(location.x, location.y, scale.x, scale.y)
			, m_rotation(rotation)
		{
		}

		[[nodiscard]] FORCE_INLINE Vector2Type GetLocation() const noexcept
		{
			return {m_locationAndScale.x, m_locationAndScale.y};
		}
		[[nodiscard]] FORCE_INLINE Vector2Type GetScale() const noexcept
		{
			return {m_locationAndScale.z, m_locationAndScale.w};
		}
		[[nodiscard]] FORCE_INLINE RotationType GetRotation() const noexcept
		{
			return m_rotation;
		}

		FORCE_INLINE void SetLocation(const Vector2Type location)
		{
			m_locationAndScale.x = location.x;
			m_locationAndScale.y = location.y;
		}
		FORCE_INLINE void SetScale(const Vector2Type scale)
		{
			m_locationAndScale.z = scale.x;
			m_locationAndScale.w = scale.y;
		}
		FORCE_INLINE void SetRotation(const RotationType rotation)
		{
			m_rotation = rotation;
		}

		[[nodiscard]] FORCE_INLINE RotationType TransformRotation(const RotationType rotation) const
		{
			return m_rotation.TransformRotation(rotation);
		}
		[[nodiscard]] FORCE_INLINE RotationType InverseTransformRotation(const RotationType rotation) const
		{
			return m_rotation.InverseTransformRotation(rotation);
		}

		[[nodiscard]] FORCE_INLINE RotationType TransformDirection(const Vector2Type direction) const
		{
			return m_rotation.TransformDirection(direction);
		}
		[[nodiscard]] FORCE_INLINE RotationType InverseTransformDirection(const Vector2Type direction) const
		{
			return m_rotation.InverseTransformDirection(direction);
		}

		[[nodiscard]] FORCE_INLINE Vector2Type TransformLocation(const Vector2Type location) const
		{
			return m_rotation.TransformDirection(location) * GetScale() + GetLocation();
		}
		[[nodiscard]] FORCE_INLINE Vector2Type InverseTransformLocation(const Vector2Type location) const
		{
			return m_rotation.InverseTransformDirection(location) * Math::MultiplicativeInverse(GetScale()) - GetLocation();
		}

		[[nodiscard]] FORCE_INLINE Vector2Type TransformLocationWithoutScale(const Vector2Type location) const
		{
			return m_rotation.TransformDirection(location) + GetLocation();
		}
		[[nodiscard]] FORCE_INLINE Vector2Type InverseTransformLocationWithoutScale(const Vector2Type location) const
		{
			return m_rotation.InverseTransformDirection(location) - GetLocation();
		}

		[[nodiscard]] FORCE_INLINE Vector2Type TransformScale(const Vector2Type scale) const
		{
			return scale * GetScale();
		}
		[[nodiscard]] FORCE_INLINE Vector2Type InverseTransformScale(const Vector2Type scale) const
		{
			return scale * Math::MultiplicativeInverse(GetScale());
		}

		[[nodiscard]] FORCE_INLINE TTransform2D Transform(const TTransform2D other) const
		{
			return {TransformRotation(other.GetRotation()), TransformLocation(other.GetLocation()), TransformScale(other.GetScale())};
		}
		[[nodiscard]] FORCE_INLINE TTransform2D InverseTransform(const TTransform2D other) const
		{
			return {
				InverseTransformRotation(other.GetRotation()),
				InverseTransformLocation(other.GetLocation()),
				InverseTransformScale(other.GetScale())
			};
		}

		[[nodiscard]] FORCE_INLINE Vector2Type GetForwardColumn() const
		{
			return m_rotation.GetForwardColumn();
		}
		[[nodiscard]] FORCE_INLINE Vector2Type GetUpColumn() const
		{
			return m_rotation.GetUpColumn();
		}

		[[nodiscard]] bool FORCE_INLINE
		IsEquivalentTo(const TTransform2D other, const UnitType epsilon = Math::NumericLimits<UnitType>::Epsilon) const
		{
			return m_locationAndScale.IsEquivalentTo(other.m_locationAndScale, epsilon) & m_rotation.IsEquivalentTo(other.m_rotation, epsilon);
		}

		[[nodiscard]] PURE_LOCALS_AND_POINTERS bool operator==(const TTransform2D& other) const
		{
			return IsEquivalentTo(other);
		}

		bool Serialize(const Serialization::Reader);
		bool Serialize(Serialization::Writer) const;
	private:
		Vector4Type m_locationAndScale;
		RotationType m_rotation;
	};
}
