#pragma once

#include <Common/Math/Vector2.h>
#include <Common/Math/Vector2/Min.h>
#include <Common/Math/Vector2/Max.h>
#include <Common/Math/Vector2/Ceil.h>
#include <Common/Math/Vector2/Select.h>
#include <Common/Platform/TrivialABI.h>
#include "ForwardDeclarations/Rectangle.h"

namespace ngine::Math
{
	template<typename T>
	struct TRIVIAL_ABI TRectangle
	{
		using UnitType = T;
		using VectorType = TVector2<T>;

		TRectangle() = default;
		constexpr TRectangle(const Math::ZeroType)
			: m_position(Math::Zero)
			, m_size(Math::Zero)
		{
		}
		constexpr TRectangle(const VectorType position, const VectorType size)
			: m_position(position)
			, m_size(size)
		{
		}
		template<typename OtherUnitType>
		constexpr explicit TRectangle(const TRectangle<OtherUnitType>& other)
			: m_position(static_cast<VectorType>(other.GetPosition()))
			, m_size(static_cast<VectorType>(other.GetSize()))
		{
		}

		[[nodiscard]] FORCE_INLINE VectorType GetPosition() const
		{
			return m_position;
		}
		[[nodiscard]] FORCE_INLINE VectorType GetSize() const
		{
			return m_size;
		}
		[[nodiscard]] FORCE_INLINE bool HasSize() const
		{
			return m_size.GetComponentLength() > 0;
		}
		[[nodiscard]] FORCE_INLINE VectorType GetEndPosition() const
		{
			return m_position + m_size;
		}
		[[nodiscard]] FORCE_INLINE VectorType GetCenterPosition() const
		{
			return m_position + (VectorType)Math::Ceil((Math::Vector2f)m_size * 0.5f);
		}

		FORCE_INLINE void SetPosition(const VectorType position)
		{
			m_position = position;
		}
		FORCE_INLINE void SetEndPosition(const VectorType newEndPosition)
		{
			m_size = Math::Max(newEndPosition - m_position, (VectorType)Math::Zero);
		}
		FORCE_INLINE void SetSize(const VectorType size)
		{
			m_size = size;
		}

		[[nodiscard]] FORCE_INLINE bool Contains(const TRectangle other) const
		{
			return ((m_position <= other.GetPosition()) & (other.GetEndPosition() <= GetEndPosition())).AreAllSet();
		}

		[[nodiscard]] FORCE_INLINE bool Contains(const VectorType otherPosition) const
		{
			return ((m_position <= otherPosition) & (otherPosition <= GetEndPosition())).AreAllSet();
		}

		[[nodiscard]] FORCE_INLINE bool Overlaps(const TRectangle other) const
		{
			return Mask(other).HasSize();
		}

		template<typename OtherUnitType>
		[[nodiscard]] FORCE_INLINE TRectangle operator*(const TRectangle<OtherUnitType>& other) const
		{
			return {m_position * VectorType(other.m_position), m_size * VectorType(other.m_size)};
		}

		[[nodiscard]] FORCE_INLINE TRectangle operator+(const VectorType offset) const
		{
			const VectorType newPosition = m_position + offset;
			const VectorType previousEndPosition = GetEndPosition();

			return {newPosition, Math::Select(previousEndPosition >= newPosition, previousEndPosition - newPosition, VectorType{Math::Zero})};
		}

		FORCE_INLINE TRectangle& operator+=(const VectorType offset)
		{
			*this = *this + offset;
			return *this;
		}

		[[nodiscard]] FORCE_INLINE TRectangle operator-(const VectorType offset) const
		{
			const VectorType newEndPosition = GetEndPosition() - offset;
			return {m_position, Math::Select(newEndPosition >= m_position, newEndPosition - m_position, VectorType{Math::Zero})};
		}

		FORCE_INLINE TRectangle& operator-=(const VectorType offset)
		{
			*this = *this - offset;
			return *this;
		}

		[[nodiscard]] FORCE_INLINE TRectangle operator/(const VectorType divisor) const
		{
			return {m_position / divisor, m_size / divisor};
		}

		FORCE_INLINE TRectangle& operator/=(const VectorType divisor)
		{
			*this = *this / divisor;
			return *this;
		}

		[[nodiscard]] FORCE_INLINE TRectangle operator*(const VectorType factor) const
		{
			return {m_position * factor, m_size * factor};
		}

		FORCE_INLINE TRectangle& operator*=(const VectorType factor)
		{
			*this = *this * factor;
			return *this;
		}

		template<typename OtherUnitType>
		[[nodiscard]] FORCE_INLINE bool operator==(const TRectangle<OtherUnitType>& other) const
		{
			return ((m_position == other.m_position) & (m_size == other.m_size)).AreAllSet();
		}

		template<typename OtherUnitType>
		[[nodiscard]] FORCE_INLINE bool operator!=(const TRectangle<OtherUnitType>& other) const
		{
			return !operator==(other);
		}

		[[nodiscard]] TRectangle Merge(const TRectangle other) const
		{
			if (!HasSize())
			{
				return other;
			}
			else if (!other.HasSize())
			{
				return *this;
			}

			const VectorType minimiumPosition = Math::Min(m_position, other.m_position);
			const VectorType newEnd = Math::Max(GetEndPosition(), other.GetEndPosition());

			return TRectangle{minimiumPosition, newEnd - minimiumPosition};
		}

		[[nodiscard]] TRectangle Mask(const TRectangle other) const
		{
			if (!HasSize() | !other.HasSize())
			{
				return Math::Zero;
			}

			const VectorType maskedPosition = Math::Max(m_position, other.m_position);
			const VectorType endPosition = Math::Min(GetEndPosition(), other.GetEndPosition());

			return TRectangle{maskedPosition, Math::Select(endPosition >= maskedPosition, endPosition - maskedPosition, VectorType{Math::Zero})};
		}

		[[nodiscard]] TRectangle Constrain(const TRectangle other) const
		{
			Math::Vector2i position = Math::Max(m_position, other.GetPosition());
			position = Math::Min(position + m_size, other.GetSize()) - m_size;
			return TRectangle{position, m_size};
		}

		[[nodiscard]] VectorType GetClosestPoint(const VectorType point) const
		{
			return Math::Min(Math::Max(point, m_position), m_position + m_size);
		}

		VectorType m_position;
		VectorType m_size;
	};
}
