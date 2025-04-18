#pragma once

#include "ForwardDeclarations/WorldCoordinate.h"

#include <Common/Math/Vector3.h>

#include <Common/Math/Vectorization/Min.h>
#include <Common/Math/Vectorization/Max.h>
#include <Common/Platform/TrivialABI.h>
#include <Common/Math/Vectorization/Abs.h>

namespace ngine::Math
{
	struct TRIVIAL_ABI WorldCoordinate : public TVector3<WorldCoordinateUnitType>
	{
		using BaseType = TVector3<WorldCoordinateUnitType>;

		using TVector3::TVector3;
		FORCE_INLINE constexpr WorldCoordinate(const Vector3f coordinate) noexcept
			: TVector3(coordinate)
		{
		}

		[[nodiscard]] FORCE_INLINE PURE_NOSTATICS Math::Vector3f GetNormalized() const noexcept
		{
			return TVector3::GetNormalized();
		}

		[[nodiscard]] FORCE_INLINE PURE_NOSTATICS constexpr WorldCoordinate operator-() const noexcept
		{
			return TVector3::operator-();
		}

		[[nodiscard]] FORCE_INLINE PURE_NOSTATICS constexpr WorldCoordinate operator+(const TVector3 other) const noexcept
		{
			return TVector3::operator+(other);
		}

		[[nodiscard]] FORCE_INLINE PURE_NOSTATICS constexpr WorldCoordinate operator+(const WorldCoordinate other) const noexcept
		{
			return TVector3::operator+(other);
		}

		constexpr FORCE_INLINE WorldCoordinate& operator+=(const TVector3 other) noexcept
		{
			return static_cast<WorldCoordinate&>(TVector3::operator+=(other));
		}

		constexpr FORCE_INLINE WorldCoordinate& operator+=(const WorldCoordinate other) noexcept
		{
			return static_cast<WorldCoordinate&>(TVector3::operator+=(other));
		}

		[[nodiscard]] FORCE_INLINE PURE_NOSTATICS constexpr WorldCoordinate operator-(const TVector3 other) const noexcept
		{
			return TVector3::operator-(other);
		}

		[[nodiscard]] FORCE_INLINE PURE_NOSTATICS constexpr WorldCoordinate operator-(const WorldCoordinate other) const noexcept
		{
			return TVector3::operator-(other);
		}

		constexpr FORCE_INLINE WorldCoordinate& operator-=(const TVector3 other) noexcept
		{
			return static_cast<WorldCoordinate&>(TVector3::operator-=(other));
		}

		constexpr FORCE_INLINE WorldCoordinate& operator-=(const WorldCoordinate other) noexcept
		{
			return static_cast<WorldCoordinate&>(TVector3::operator-=(other));
		}

		[[nodiscard]] FORCE_INLINE PURE_NOSTATICS constexpr WorldCoordinate operator*(const WorldCoordinateUnitType scalar) const noexcept
		{
			return TVector3::operator*(scalar);
		}

		constexpr FORCE_INLINE WorldCoordinate& operator*=(const WorldCoordinateUnitType scalar) noexcept
		{
			return static_cast<WorldCoordinate&>(TVector3::operator*=(scalar));
		}

		[[nodiscard]] FORCE_INLINE PURE_NOSTATICS constexpr WorldCoordinate operator/(const WorldCoordinateUnitType scalar) const noexcept
		{
			return TVector3::operator/(scalar);
		}

		constexpr FORCE_INLINE WorldCoordinate& operator/=(const WorldCoordinateUnitType scalar) noexcept
		{
			return static_cast<WorldCoordinate&>(TVector3::operator/=(scalar));
		}

		[[nodiscard]] FORCE_INLINE PURE_NOSTATICS constexpr WorldCoordinate operator*(const TVector3 other) const noexcept
		{
			return TVector3::operator*(other);
		}

		[[nodiscard]] FORCE_INLINE PURE_NOSTATICS constexpr WorldCoordinate operator*(const WorldCoordinate other) const noexcept
		{
			return TVector3::operator*(other);
		}

		constexpr FORCE_INLINE WorldCoordinate& operator*=(const TVector3 other) noexcept
		{
			return static_cast<WorldCoordinate&>(TVector3::operator*=(other));
		}

		constexpr FORCE_INLINE WorldCoordinate& operator*=(const WorldCoordinate other) noexcept
		{
			return static_cast<WorldCoordinate&>(TVector3::operator*=(other));
		}

		[[nodiscard]] FORCE_INLINE PURE_NOSTATICS constexpr WorldCoordinate operator/(const TVector3 other) const noexcept
		{
			return TVector3::operator/(other);
		}

		[[nodiscard]] FORCE_INLINE PURE_NOSTATICS constexpr WorldCoordinate operator/(const WorldCoordinate other) const noexcept
		{
			return TVector3::operator/(other);
		}

		constexpr FORCE_INLINE WorldCoordinate& operator/=(const TVector3 other) noexcept
		{
			return static_cast<WorldCoordinate&>(TVector3::operator/=(other));
		}

		constexpr FORCE_INLINE WorldCoordinate& operator/=(const WorldCoordinate other) noexcept
		{
			return static_cast<WorldCoordinate&>(TVector3::operator/=(other));
		}

		bool Serialize(const Serialization::Reader serializer);
		bool Serialize(Serialization::Writer serializer) const;
	};
}
