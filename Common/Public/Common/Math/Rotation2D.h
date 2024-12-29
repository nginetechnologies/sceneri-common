#pragma once

#include "ForwardDeclarations/Rotation2D.h"
#include "Vector2.h"
#include "Angle.h"
#include "SinCos.h"
#include "Atan2.h"

#include <Common/Serialization/ForwardDeclarations/Reader.h>
#include <Common/Serialization/ForwardDeclarations/Writer.h>
#include <Common/Platform/TrivialABI.h>
#include <Common/Math/IsEquivalentTo.h>

namespace ngine::Math
{
	template<typename UnitType>
	struct TRIVIAL_ABI TRotation2D : public TAngle<UnitType>
	{
		inline static constexpr Guid TypeGuid = "b2301d2c-4e60-4fc5-9677-3de925ee8555"_guid;

		using BaseType = TAngle<UnitType>;

		using BaseType::BaseType;

		constexpr TRotation2D(TAngle<UnitType> angle)
			: TAngle<UnitType>(angle)
		{
		}
		TRotation2D(const TVector2<UnitType> direction)
			: TAngle<UnitType>(Atan2(direction.x, direction.y))
		{
		}

		[[nodiscard]] FORCE_INLINE constexpr TRotation2D GetInverted() const
		{
			return {-m_value};
		}

		[[nodiscard]] FORCE_INLINE constexpr TRotation2D TransformRotation(const TRotation2D other) const
		{
			return {Math::Wrap(m_value + other.m_value, -TConstants<UnitType>::PI, TConstants<UnitType>::PI)};
		}
		[[nodiscard]] FORCE_INLINE constexpr TRotation2D InverseTransformRotation(const TRotation2D other) const
		{
			return GetInverted().TransformRotation(other);
		}

		[[nodiscard]] FORCE_INLINE TVector2<UnitType> TransformDirection(TVector2<UnitType> vector) const noexcept
		{
			PUSH_GCC_WARNINGS
			DISABLE_GCC_WARNING("-Wuninitialized")
			UnitType cos;
			const UnitType sin = SinCos(m_value, cos);
			return {vector.x * cos - vector.y * sin, vector.x * sin + vector.y * cos};
			POP_GCC_WARNINGS
		}
		[[nodiscard]] FORCE_INLINE TVector2<UnitType> InverseTransformDirection(TVector2<UnitType> vector) const noexcept
		{
			return GetInverted().TransformDirection(vector);
		}

		[[nodiscard]] FORCE_INLINE TVector2<UnitType> GetForwardColumn() const
		{
			TVector2<UnitType> direction;
			direction.x = SinCos(m_value, direction.y);
			return direction.GetNormalized();
		}
		[[nodiscard]] FORCE_INLINE TVector2<UnitType> GetUpColumn() const
		{
			return GetForwardColumn().GetPerpendicularCounterClockwise();
		}

		[[nodiscard]] bool FORCE_INLINE
		IsEquivalentTo(const TRotation2D other, const UnitType epsilon = Math::NumericLimits<UnitType>::Epsilon) const
		{
			return Math::IsEquivalentTo(BaseType::GetRadians(), other.BaseType::GetRadians(), epsilon);
		}
	protected:
		using TAngle<UnitType>::m_value;
	};
}
