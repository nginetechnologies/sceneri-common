#pragma once

#include "Hexagon.h"

#include <Common/Platform/Pure.h>
#include <Common/Math/Vector3.h>
#include <Common/Math/Abs.h>
#include <Common/Math/Round.h>
#include <Common/Math/WorldCoordinate.h>

namespace ngine::Math
{
	namespace Internal
	{
		inline constexpr static Math::Vector3f QDirection = Math::Vector3f(1, 0, 0);
		inline constexpr static Math::Vector3f RDirection = Math::Vector3f(0.5f, -0.866f, 0.f);

		inline static constexpr Math::Hexagon Directions[6] = {
			Math::Hexagon(1, 0, -1),
			Math::Hexagon(1, -1, 0),
			Math::Hexagon(0, -1, 1),
			Math::Hexagon(-1, 0, 1),
			Math::Hexagon(-1, 1, 0),
			Math::Hexagon(0, 1, -1)
		};
	}

	class HexagonWorld
	{
	public:
		explicit HexagonWorld(float hexagonSize = 1.f, Math::Vector3f worldCenter = Math::Zero, Math::Vector3f normal = Math::Up) noexcept
			: m_hexagonSize(hexagonSize)
			, m_center(worldCenter)
			, m_normal(normal)
		{
		}

		[[nodiscard]] FORCE_INLINE float GetHexagonSize() const noexcept
		{
			return m_hexagonSize;
		}
		[[nodiscard]] FORCE_INLINE Math::Vector3f GetCenter() const noexcept
		{
			return m_center;
		}
		[[nodiscard]] FORCE_INLINE Math::Vector3f GetNormal() const noexcept
		{
			return m_normal;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS Math::WorldCoordinate GetWorldCoordinateFromHexagon(Math::Hexagon hexagon
		) const noexcept
		{
			const Math::Vector3f vq = Internal::QDirection * static_cast<float>(hexagon.q);
			const Math::Vector3f vr = Internal::RDirection * static_cast<float>(hexagon.r);

			return ((vq + vr) + m_center) * m_hexagonSize;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS Math::Hexagon GetHexagonFromWorldCoordinate(Math::WorldCoordinate worldCoordinate
		) const noexcept
		{
			worldCoordinate += m_center;

			worldCoordinate.y *= -1.f;
			float q = (Math::Sqrt(3.f) / 3.f * worldCoordinate.x - 1.f / 3.f * worldCoordinate.y) / m_hexagonSize;
			float r = (2.f / 3.f * worldCoordinate.y) / m_hexagonSize;

			return Hexagon::CreateFromFractional(q, r, -q - r);
		}

		[[nodiscard]] FORCE_INLINE static Hexagon GetDirection(uint8 index) noexcept
		{
			MathAssert(0 <= index && index < 6);
			return Internal::Directions[index];
		}
	private:
		const float m_hexagonSize;
		const Math::Vector3f m_center;
		const Math::Vector3f m_normal;
	};

}
