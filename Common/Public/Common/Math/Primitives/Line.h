#pragma once

#include <Common/Math/ForwardDeclarations/Vector3.h>
#include <Common/Math/Clamp.h>
#include <Common/Math/Ratio.h>
#include <Common/Platform/TrivialABI.h>

namespace ngine::Math
{
	template<typename VectorType_>
	struct TRIVIAL_ABI TLine
	{
		using VectorType = VectorType_;
		using UnitType = typename VectorType::UnitType;

		TLine() = default;
		TLine(ZeroType)
			: m_start(Zero)
			, m_end(Zero)
		{
		}
		TLine(const VectorType start, const VectorType end)
			: m_start(start)
			, m_end(end)
		{
		}

		[[nodiscard]] FORCE_INLINE VectorType GetStart() const
		{
			return m_start;
		}
		[[nodiscard]] FORCE_INLINE VectorType GetEnd() const
		{
			return m_end;
		}
		[[nodiscard]] FORCE_INLINE VectorType GetCenter() const
		{
			return (m_start + m_end) * UnitType(0.5);
		}
		[[nodiscard]] FORCE_INLINE VectorType GetDistance() const
		{
			return m_end - m_start;
		}
		[[nodiscard]] FORCE_INLINE UnitType GetLength() const
		{
			return GetDistance().GetLength();
		}
		[[nodiscard]] FORCE_INLINE VectorType GetDirection() const
		{
			return GetDistance().GetNormalized();
		}

		[[nodiscard]] FORCE_INLINE VectorType GetPointAtRatio(const TRatio<UnitType> ratio) const
		{
			return m_end * (UnitType)ratio + m_start * (UnitType(1.0) - (UnitType)ratio);
		}

		[[nodiscard]] TRatio<UnitType> GetClosestPointRatio(const VectorType point) const
		{
			const VectorType lineDistance = GetDistance();
			const UnitType squaredLineLength = lineDistance.GetLengthSquared();
			const VectorType distanceFromLineStart = point - m_start;
			return TRatio<UnitType>(Math::Clamp(distanceFromLineStart.Dot(lineDistance), 0.f, squaredLineLength) / squaredLineLength);
		}

		[[nodiscard]] FORCE_INLINE VectorType GetClosestPoint(const VectorType point) const
		{
			return GetPointAtRatio(GetClosestPointRatio(point));
		}
	protected:
		VectorType m_start;
		VectorType m_end;
	};
}
