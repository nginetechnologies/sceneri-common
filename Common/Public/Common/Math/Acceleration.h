#pragma once

#include "ForwardDeclarations/Acceleration.h"

#include <Common/Platform/ForceInline.h>
#include <Common/Platform/TrivialABI.h>

#include <Common/Serialization/ForwardDeclarations/Reader.h>
#include <Common/Serialization/ForwardDeclarations/Writer.h>

namespace ngine::Math
{
	template<typename T>
	struct TRIVIAL_ABI TAcceleration
	{
		inline static constexpr Guid TypeGuid = "8cbf9777-d09c-42cd-83fb-7cb9da2a2de7"_guid;
	protected:
		FORCE_INLINE constexpr TAcceleration(const T value) noexcept
			: m_value(value)
		{
		}
	public:
		constexpr TAcceleration(const ZeroType) noexcept
			: m_value(0)
		{
		}

		constexpr TAcceleration() noexcept
			: m_value(0)
		{
		}

		[[nodiscard]] FORCE_INLINE static constexpr TAcceleration FromMetersPerSecondSquared(const T value) noexcept
		{
			return TAcceleration(value);
		}
		[[nodiscard]] FORCE_INLINE constexpr T GetMetersPerSecondSquared() const noexcept
		{
			return m_value;
		}

		constexpr FORCE_INLINE TAcceleration& operator+=(const TAcceleration other) noexcept
		{
			m_value += other.m_value;
			return *this;
		}

		constexpr FORCE_INLINE TAcceleration& operator-=(const TAcceleration other) noexcept
		{
			m_value -= other.m_value;
			return *this;
		}

		[[nodiscard]] FORCE_INLINE constexpr TAcceleration operator-() const noexcept
		{
			return TAcceleration(-m_value);
		}

		[[nodiscard]] FORCE_INLINE constexpr TAcceleration operator*(const T scalar) const noexcept
		{
			return TAcceleration(m_value * scalar);
		}
		FORCE_INLINE constexpr void operator*=(const T scalar) noexcept
		{
			m_value *= scalar;
		}
		[[nodiscard]] FORCE_INLINE constexpr TAcceleration operator*(const TAcceleration other) const noexcept
		{
			return TAcceleration(m_value * other.m_value);
		}
		[[nodiscard]] FORCE_INLINE constexpr TAcceleration operator/(const T scalar) const noexcept
		{
			return TAcceleration(m_value / scalar);
		}
		FORCE_INLINE constexpr void operator/=(const T scalar) noexcept
		{
			m_value /= scalar;
		}
		[[nodiscard]] FORCE_INLINE constexpr TAcceleration operator/(const TAcceleration other) const noexcept
		{
			return TAcceleration(m_value / other.m_value);
		}
		[[nodiscard]] FORCE_INLINE constexpr TAcceleration operator-(const TAcceleration other) const noexcept
		{
			return TAcceleration(m_value - other.m_value);
		}
		[[nodiscard]] FORCE_INLINE constexpr TAcceleration operator+(const TAcceleration other) const noexcept
		{
			return TAcceleration(m_value + other.m_value);
		}
		[[nodiscard]] FORCE_INLINE constexpr bool operator==(const TAcceleration other) const noexcept
		{
			return m_value == other.m_value;
		}
		[[nodiscard]] FORCE_INLINE constexpr bool operator!=(const TAcceleration other) const noexcept
		{
			return !operator==(other);
		}
		[[nodiscard]] FORCE_INLINE constexpr bool operator>(const TAcceleration other) const noexcept
		{
			return m_value > other.m_value;
		}
		[[nodiscard]] FORCE_INLINE constexpr bool operator>=(const TAcceleration other) const noexcept
		{
			return m_value >= other.m_value;
		}
		[[nodiscard]] FORCE_INLINE constexpr bool operator<(const TAcceleration other) const noexcept
		{
			return m_value < other.m_value;
		}
		[[nodiscard]] FORCE_INLINE constexpr bool operator<=(const TAcceleration other) const noexcept
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
		constexpr Accelerationf operator""_m_per_second_squared(unsigned long long value) noexcept
		{
			return Accelerationf::FromMetersPerSecondSquared(static_cast<float>(value));
		}
		constexpr Accelerationf operator""_m_per_second_squared(long double value) noexcept
		{
			return Accelerationf::FromMetersPerSecondSquared(static_cast<float>(value));
		}
	}

	using namespace Literals;
}

namespace ngine
{
	using namespace Math::Literals;
}
