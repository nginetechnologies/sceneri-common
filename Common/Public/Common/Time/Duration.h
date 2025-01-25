#pragma once

#include <Common/Platform/ForceInline.h>

#include <Common/Time/ForwardDeclarations/Duration.h>
#include <Common/Memory/Containers/ForwardDeclarations/FlatString.h>
#include <Common/Serialization/ForwardDeclarations/Reader.h>
#include <Common/Serialization/ForwardDeclarations/Writer.h>

namespace ngine::Time
{
	//! Represents a duration intended for high-performance and per-session contexts
	//! Stores the value in seconds
	template<typename Type>
	struct Duration
	{
		using UnitType = Type;

		FORCE_INLINE Duration() = default;

		template<typename OtherType>
		FORCE_INLINE constexpr Duration(const Duration<OtherType> other)
			: m_value(static_cast<Type>(other.GetSeconds()))
		{
		}

		//! Gets the current high-performance time relative to system up-time
		[[nodiscard]] PURE_STATICS static Duration GetCurrentSystemUptime();

		[[nodiscard]] FORCE_INLINE static constexpr Duration FromHours(const Type value)
		{
			return Duration{value * 3600.f};
		}

		[[nodiscard]] FORCE_INLINE static constexpr Duration FromMinutes(const Type value)
		{
			return Duration{value * 60.f};
		}

		[[nodiscard]] FORCE_INLINE static constexpr Duration FromSeconds(const Type value)
		{
			return Duration{value};
		}

		[[nodiscard]] FORCE_INLINE static constexpr Duration FromMilliseconds(const uint64 value)
		{
			return Duration{Type((double)value * 0.001f)};
		}

		[[nodiscard]] FORCE_INLINE static constexpr Duration FromNanoseconds(const uint64 value)
		{
			return Duration{Type((double)value / 1000000000.0)};
		}

		[[nodiscard]] FORCE_INLINE constexpr Duration operator-(const Duration other) const
		{
			return Duration{m_value - other.m_value};
		}
		[[nodiscard]] FORCE_INLINE constexpr Duration operator+(const Duration other) const
		{
			return Duration{m_value + other.m_value};
		}
		[[nodiscard]] FORCE_INLINE constexpr Duration operator*(const Duration other) const
		{
			return Duration{m_value * other.m_value};
		}
		[[nodiscard]] FORCE_INLINE constexpr Duration operator/(const Duration other) const
		{
			return Duration{m_value / other.m_value};
		}

		[[nodiscard]] FORCE_INLINE bool operator>(const Duration other) const
		{
			return m_value > other.m_value;
		}
		[[nodiscard]] FORCE_INLINE bool operator>=(const Duration other) const
		{
			return m_value >= other.m_value;
		}
		[[nodiscard]] FORCE_INLINE bool operator<(const Duration other) const
		{
			return m_value < other.m_value;
		}
		[[nodiscard]] FORCE_INLINE bool operator<=(const Duration other) const
		{
			return m_value <= other.m_value;
		}
		[[nodiscard]] FORCE_INLINE bool operator!=(const Duration other) const
		{
			return m_value != other.m_value;
		}
		[[nodiscard]] FORCE_INLINE bool operator==(const Duration other) const
		{
			return m_value == other.m_value;
		}

		[[nodiscard]] FORCE_INLINE constexpr Duration operator*(const UnitType scalar) const
		{
			return Duration{m_value * scalar};
		}
		[[nodiscard]] FORCE_INLINE constexpr Duration operator/(const UnitType scalar) const
		{
			return Duration{m_value / scalar};
		}

		[[nodiscard]] FORCE_INLINE constexpr Duration operator-() const
		{
			return Duration{-m_value};
		}

		FORCE_INLINE constexpr Duration& operator-=(const Duration other)
		{
			m_value -= other.m_value;
			return *this;
		}
		FORCE_INLINE constexpr Duration& operator+=(const Duration other)
		{
			m_value += other.m_value;
			return *this;
		}
		FORCE_INLINE constexpr Duration& operator*=(const Duration other)
		{
			m_value *= other.m_value;
			return *this;
		}
		FORCE_INLINE constexpr Duration& operator/=(const Duration other)
		{
			m_value /= other.m_value;
			return *this;
		}
		FORCE_INLINE constexpr Duration& operator*=(const UnitType scalar)
		{
			m_value *= scalar;
			return *this;
		}
		FORCE_INLINE constexpr Duration& operator/=(const UnitType scalar)
		{
			m_value /= scalar;
			return *this;
		}

		[[nodiscard]] FORCE_INLINE constexpr Type GetHours() const
		{
			return m_value / 3600.f;
		}

		[[nodiscard]] FORCE_INLINE constexpr Type GetMinutes() const
		{
			return m_value / 60.f;
		}

		[[nodiscard]] FORCE_INLINE constexpr Type GetSeconds() const
		{
			return m_value;
		}

		[[nodiscard]] FORCE_INLINE constexpr Type GetMilliseconds() const
		{
			return m_value * 1000.f;
		}

		[[nodiscard]] FORCE_INLINE constexpr int64 GetNanoseconds() const
		{
			return (int64)GetMilliseconds() * 1000000;
		}

		//! Returns a string with the format minutes:seconds
		[[nodiscard]] FlatString<40> ToString() const;

		bool Serialize(const Serialization::Reader serializer);
		bool Serialize(Serialization::Writer serializer) const;
	protected:
		FORCE_INLINE constexpr Duration(const Type seconds)
			: m_value(seconds)
		{
		}
	protected:
		Type m_value;
	};

	extern template struct Duration<float>;
	extern template struct Duration<double>;

	namespace Literals
	{
		constexpr Durationd operator""_hours(unsigned long long value)
		{
			return Durationd::FromHours(static_cast<float>(value));
		}

		constexpr Durationd operator""_hours(long double value)
		{
			return Durationd::FromHours(static_cast<float>(value));
		}

		constexpr Durationd operator""_minutes(unsigned long long value)
		{
			return Durationd::FromMinutes(static_cast<float>(value));
		}

		constexpr Durationd operator""_minutes(long double value)
		{
			return Durationd::FromMinutes(static_cast<float>(value));
		}

		constexpr Durationd operator""_seconds(unsigned long long value)
		{
			return Durationd::FromSeconds(static_cast<float>(value));
		}

		constexpr Durationd operator""_seconds(long double value)
		{
			return Durationd::FromSeconds(static_cast<float>(value));
		}

		constexpr Durationd operator""_milliseconds(unsigned long long value)
		{
			return Durationd::FromMilliseconds(static_cast<uint64>(value));
		}

		constexpr Durationd operator""_milliseconds(long double value)
		{
			return Durationd::FromMilliseconds(static_cast<uint64>(value));
		}
	}

	using namespace Literals;
}

namespace ngine
{
	using namespace Time::Literals;
}
