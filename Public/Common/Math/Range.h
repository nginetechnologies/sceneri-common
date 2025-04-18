#pragma once

#include <Common/Math/Round.h>
#include <Common/Math/Clamp.h>
#include <Common/Math/Ratio.h>
#include <Common/Math/ForwardDeclarations/Range.h>
#include <Common/Platform/TrivialABI.h>
#include <Common/TypeTraits/IsFloatingPoint.h>
#include <Common/TypeTraits/Select.h>

namespace ngine::Math
{
	template<typename T>
	struct TRIVIAL_ABI Range
	{
		constexpr Range() = default;

		using FloatType = TypeTraits::Select<TypeTraits::IsFloatingPoint<T>, T, float>;
		using RatioType = Math::TRatio<FloatType>;

		[[nodiscard]] static constexpr Range MakeStartToEnd(const T start, const T end)
		{
			return Range{start, T((end - start) + (T)1)};
		}
		[[nodiscard]] static constexpr Range Make(const T start, const T count)
		{
			return Range{start, count};
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool Contains(const T value) const
		{
			return (value >= m_start) & (value < GetEnd());
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool Contains(const Range other) const
		{
			return (other.m_start >= m_start) & (other.GetEnd() <= GetEnd());
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Range Mask(const Range other) const noexcept
		{
			const T start = Math::Max(m_start, other.m_start);
			const T end = Math::Min(GetEnd(), other.GetEnd());
			return {start, (end - start) * (end >= start)};
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Range GetSubRange(const Range other) const noexcept
		{
			const T start = Math::Max(m_start, other.m_start);
			const T end = Math::Min(m_start + m_count, other.m_start + other.m_count);
			return Range{start, end - start};
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Range GetSubRangeFrom(const T index) const noexcept
		{
			return Range{index, Math::Min(T(m_count - index), m_count)};
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Range GetSubRangeUpTo(const T index) const noexcept
		{
			return Range{m_start, Math::Min(index, m_start + m_count) - m_start};
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool Overlaps(const Range other) const
		{
			return Mask(other).GetSize() > 0;
		}

		template<typename ScalarType>
		[[nodiscard]] Range<ScalarType> PURE_STATICS constexpr operator*(const ScalarType value) const
		{
			return Range<ScalarType>{ScalarType(m_start * value), ScalarType(m_count * value)};
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS T constexpr GetMinimum() const
		{
			return m_start;
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS T constexpr GetMaximum() const
		{
			return m_start + Math::Max(m_count, T(1u)) - T(1u);
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS T constexpr GetEnd() const
		{
			return m_start + m_count;
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS T constexpr GetSize() const
		{
			return m_count;
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS T constexpr GetRange() const
		{
			return GetMaximum() - GetMinimum();
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr T GetClampedValue(const T value) const
		{
			return Math::Clamp(value, m_start, GetMaximum());
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr RatioType GetClampedRatio(T value) const
		{
			value = GetClampedValue(value) - m_start;
			return RatioType((FloatType)value / (FloatType)GetRange());
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr RatioType GetRatio(T value) const
		{
			value = value - m_start;
			return RatioType((FloatType)value / (FloatType)GetRange());
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS T GetValueFromRatio(const RatioType ratio) const
		{
			if constexpr (TypeTraits::IsFloatingPoint<T>)
			{
				return m_start + GetSize() * ratio;
			}
			else
			{
				return m_start + (T)Math::Round((FloatType)GetSize() * ratio);
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS T GetRemappedValue(const T value, const Range otherRange) const
		{
			const RatioType ratio = otherRange.GetClampedRatio(value);
			return GetValueFromRatio(ratio);
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator==(const Range& other) const
		{
			return (m_start == other.m_start) & (m_count == other.m_count);
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator!=(const Range& other) const
		{
			return !operator==(other);
		}

		[[nodiscard]] FORCE_INLINE Range operator+(const T value) const
		{
			Range result = *this;
			result.m_start += value;
			result.m_count -= Math::Min(value, m_count);
			return result;
		}
		FORCE_INLINE Range& operator+=(const T value)
		{
			m_start += value;
			m_count -= Math::Min(value, m_count);
			return *this;
		}
		FORCE_INLINE Range& operator++()
		{
			m_start++;
			m_count -= Math::Min((T)1, m_count);
			return *this;
		}
		[[nodiscard]] FORCE_INLINE Range operator-(const T value) const
		{
			Range result = *this;
			result.m_count -= Math::Min(value, m_count);
			return result;
		}
		FORCE_INLINE Range& operator-=(const T value)
		{
			m_count -= Math::Min(value, m_count);
			return *this;
		}
		FORCE_INLINE Range& operator--()
		{
			m_count -= Math::Min(T(1), m_count);
			return *this;
		}
	private:
		constexpr Range(const T start, const T count)
			: m_start(start)
			, m_count(count)
		{
		}
	protected:
		struct Iterator
		{
			FORCE_INLINE constexpr Iterator(const T value)
				: m_currentValue(value)
			{
			}

			[[nodiscard]] FORCE_INLINE constexpr bool operator==(const Iterator& other) const
			{
				return m_currentValue == other.m_currentValue;
			}

			[[nodiscard]] FORCE_INLINE constexpr bool operator!=(const Iterator& other) const
			{
				return m_currentValue != other.m_currentValue;
			}

			FORCE_INLINE constexpr void operator++()
			{
				m_currentValue++;
			}

			[[nodiscard]] FORCE_INLINE constexpr T operator*() const
			{
				return m_currentValue;
			}
		private:
			T m_currentValue;
		};
	public:
		[[nodiscard]] FORCE_INLINE constexpr Iterator begin() const
		{
			return {m_start};
		}

		[[nodiscard]] FORCE_INLINE constexpr Iterator end() const
		{
			return {GetEnd()};
		}
	protected:
		T m_start{0};
		T m_count{0};
	};

	struct Rangef : public Range<float>
	{
		inline static constexpr Guid TypeGuid = "5c307efa-7b84-4e3f-bdeb-8502a7f7631e"_guid;

		using BaseType = Range<float>;
		using BaseType::BaseType;
		using BaseType::operator=;
		constexpr Rangef(const BaseType range)
			: BaseType(range)
		{
		}
	};

}
