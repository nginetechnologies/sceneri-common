#pragma once

#include "ForwardDeclarations/Frequency.h"

#include <Common/Guid.h>
#include <Common/Platform/ForceInline.h>
#include <Common/Platform/TrivialABI.h>
#include <Common/Math/Constants.h>
#include <Common/Time/Duration.h>

#include <Common/Serialization/ForwardDeclarations/Reader.h>
#include <Common/Serialization/ForwardDeclarations/Writer.h>

namespace ngine::Math
{
	template<typename T>
	struct TRIVIAL_ABI TFrequency
	{
		using UnitType = T;
		using DurationType = Time::Duration<T>;

		inline static constexpr Guid TypeGuid = "{da381ccd-1b64-482b-83e1-268207a5fd5d}"_guid;
	protected:
		FORCE_INLINE constexpr TFrequency(const T value) noexcept
			: m_value(value)
		{
		}
	public:
		constexpr TFrequency(const ZeroType) noexcept
			: m_value(0)
		{
		}

		constexpr TFrequency() noexcept
			: m_value(0)
		{
		}

		[[nodiscard]] FORCE_INLINE static constexpr TFrequency FromHertz(const T value) noexcept
		{
			return TFrequency(value);
		}
		[[nodiscard]] FORCE_INLINE static constexpr TFrequency FromCyclesPerSecond(const T value) noexcept
		{
			return FromHertz(value);
		}

		[[nodiscard]] FORCE_INLINE constexpr T GetHertz() const noexcept
		{
			return m_value;
		}
		[[nodiscard]] FORCE_INLINE constexpr T GetCyclesPerSecond() const noexcept
		{
			return GetHertz();
		}

		[[nodiscard]] FORCE_INLINE constexpr DurationType GetDuration() const
		{
			return DurationType::FromSeconds(T(1) / m_value);
		}

		constexpr FORCE_INLINE TFrequency& operator+=(const TFrequency other) noexcept
		{
			m_value += other.m_value;
			return *this;
		}

		constexpr FORCE_INLINE TFrequency& operator-=(const TFrequency other) noexcept
		{
			m_value -= other.m_value;
			return *this;
		}

		[[nodiscard]] FORCE_INLINE constexpr TFrequency operator-() const noexcept
		{
			return TFrequency(-m_value);
		}

		[[nodiscard]] FORCE_INLINE constexpr TFrequency operator*(const T scalar) const noexcept
		{
			return TFrequency(m_value * scalar);
		}
		FORCE_INLINE constexpr void operator*=(const T scalar) noexcept
		{
			m_value *= scalar;
		}
		[[nodiscard]] FORCE_INLINE constexpr TFrequency operator*(const TFrequency other) const noexcept
		{
			return TFrequency(m_value * other.m_value);
		}
		[[nodiscard]] FORCE_INLINE constexpr TFrequency operator/(const T scalar) const noexcept
		{
			return TFrequency(m_value / scalar);
		}
		FORCE_INLINE constexpr void operator/=(const T scalar) noexcept
		{
			m_value /= scalar;
		}
		[[nodiscard]] FORCE_INLINE constexpr TFrequency operator/(const TFrequency other) const noexcept
		{
			return TFrequency(m_value / other.m_value);
		}
		[[nodiscard]] FORCE_INLINE constexpr TFrequency operator-(const TFrequency other) const noexcept
		{
			return TFrequency(m_value - other.m_value);
		}
		[[nodiscard]] FORCE_INLINE constexpr TFrequency operator+(const TFrequency other) const noexcept
		{
			return TFrequency(m_value + other.m_value);
		}
		[[nodiscard]] FORCE_INLINE constexpr bool operator==(const TFrequency other) const noexcept
		{
			return m_value == other.m_value;
		}
		[[nodiscard]] FORCE_INLINE constexpr bool operator!=(const TFrequency other) const noexcept
		{
			return !operator==(other);
		}
		[[nodiscard]] FORCE_INLINE constexpr bool operator>(const TFrequency other) const noexcept
		{
			return m_value > other.m_value;
		}
		[[nodiscard]] FORCE_INLINE constexpr bool operator>=(const TFrequency other) const noexcept
		{
			return m_value >= other.m_value;
		}
		[[nodiscard]] FORCE_INLINE constexpr bool operator<(const TFrequency other) const noexcept
		{
			return m_value < other.m_value;
		}
		[[nodiscard]] FORCE_INLINE constexpr bool operator<=(const TFrequency other) const noexcept
		{
			return m_value <= other.m_value;
		}

		bool Serialize(const Serialization::Reader serializer);
		bool Serialize(Serialization::Writer serializer) const;
	protected:
		T m_value;
	};

	namespace Literals
	{
		constexpr Frequencyd operator""_hz(unsigned long long value) noexcept
		{
			return Frequencyd::FromHertz(static_cast<double>(value));
		}
		constexpr Frequencyd operator""_hz(long double value) noexcept
		{
			return Frequencyd::FromHertz(static_cast<double>(value));
		}
	}

	using namespace Literals;
}

namespace ngine
{
	using namespace Math::Literals;
}
