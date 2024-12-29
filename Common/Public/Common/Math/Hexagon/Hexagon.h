#pragma once

#include <Common/Platform/Pure.h>
#include <Common/Platform/TrivialABI.h>
#include <Common/Math/Vector3/Abs.h>
#include <Common/Math/Vector3.h>
#include <Common/Math/Vector3/Round.h>

namespace ngine::Math
{
	struct TRIVIAL_ABI Hexagon
	{
		using UnitType = int32;

		FORCE_INLINE constexpr Hexagon(const UnitType _q, const UnitType _r, const UnitType _s) noexcept
			: q(_q)
			, r(_r)
			, s(_s)
		{
		}

		FORCE_INLINE constexpr Hexagon(ZeroType) noexcept
			: q(0)
			, r(0)
			, s(0)
		{
		}

		FORCE_INLINE constexpr Hexagon(const Math::TVector3<UnitType> vector) noexcept
			: m_vector(vector)
		{
		}

		[[nodiscard]] FORCE_INLINE bool operator==(const Hexagon& other) const noexcept
		{
			return (m_vector == other.m_vector).AreAllSet();
		}

		[[nodiscard]] FORCE_INLINE bool operator!=(const Hexagon& other) const noexcept
		{
			return !Hexagon::operator==(other);
		}

		[[nodiscard]] FORCE_INLINE Hexagon operator+(const Hexagon& other) const noexcept
		{
			return m_vector + other.m_vector;
		}

		[[nodiscard]] FORCE_INLINE Hexagon operator-(const Hexagon& other) const noexcept
		{
			return m_vector - other.m_vector;
		}

		[[nodiscard]] FORCE_INLINE Hexagon operator*(const UnitType v) const noexcept
		{
			return m_vector * v;
		}

		FORCE_INLINE Hexagon operator=(const Hexagon& other) noexcept
		{
			m_vector = other.m_vector;
			return *this;
		}

		[[nodiscard]] FORCE_INLINE UnitType GetLength() const noexcept
		{
			return static_cast<UnitType>((Abs(q) + Abs(r) + Abs(s)) / 2);
		}

		[[nodiscard]] FORCE_INLINE UnitType GetDistance(const Hexagon& other) const noexcept
		{
			return (*this - other).GetLength();
		}

		[[nodiscard]] PURE_LOCALS_AND_POINTERS static Math::Hexagon CreateFromFractional(float q, float r, float s)
		{
			Math::Vector3f round = Math::Round(Math::Vector3f(q, r, s));
			const Math::Vector3f abs = Math::Abs(round - Math::Vector3f(q, r, s));
			if ((Vector3f(abs.x, abs.x, abs.y) > Vector3f(abs.y, abs.z, abs.z)).AreAllSet())
			{
				round.x = -round.y - round.z;
			}
			else if (abs.y > abs.z)
			{
				round.y = -round.x - round.z;
			}
			else
			{
				round.z = -round.x - round.y;
			}

			return Math::Hexagon((UnitType)round.x, (UnitType)round.y, (UnitType)round.z);
		}

		union
		{
			struct
			{
				UnitType q, r, s;
			};
			Math::TVector3<UnitType> m_vector;
		};
	};
}
