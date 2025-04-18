#pragma once

#include "ForwardDeclarations/Speed.h"

#include <Common/Platform/ForceInline.h>
#include <Common/Platform/TrivialABI.h>

#include <Common/Serialization/ForwardDeclarations/Reader.h>
#include <Common/Serialization/ForwardDeclarations/Writer.h>

namespace ngine::Math
{
	template<typename T>
	struct TRIVIAL_ABI TSpeed
	{
		inline static constexpr Guid TypeGuid = "37de82f9-2178-4eae-a4ab-868f1c6770a1"_guid;
	protected:
		FORCE_INLINE constexpr TSpeed(const T value) noexcept
			: m_value(value)
		{
		}
	public:
		constexpr TSpeed(const ZeroType) noexcept
			: m_value(0)
		{
		}

		constexpr TSpeed() noexcept
			: m_value(0)
		{
		}

		[[nodiscard]] FORCE_INLINE static constexpr TSpeed FromMetersPerSecond(const T value) noexcept
		{
			return TSpeed(value);
		}
		[[nodiscard]] FORCE_INLINE static constexpr TSpeed FromKilometersPerHour(const T value) noexcept
		{
			return TSpeed(value / 3.6f);
		}
		[[nodiscard]] FORCE_INLINE static constexpr TSpeed FromMilesPerHour(const T value) noexcept
		{
			return TSpeed(value / 2.237f);
		}
		[[nodiscard]] FORCE_INLINE static constexpr TSpeed FromKnots(const T value) noexcept
		{
			return TSpeed(value / 1.94384f);
		}
		[[nodiscard]] FORCE_INLINE static constexpr TSpeed FromFeetPerSecond(const T value) noexcept
		{
			return TSpeed(value / 3.28084f);
		}

		[[nodiscard]] FORCE_INLINE constexpr T GetMetersPerSecond() const noexcept
		{
			return m_value;
		}
		[[nodiscard]] FORCE_INLINE constexpr T GetKilometersPerHour() const noexcept
		{
			return m_value * 3.6f;
		}
		[[nodiscard]] FORCE_INLINE constexpr T GetMilesPerHour() const noexcept
		{
			return m_value * 2.237f;
		}
		[[nodiscard]] FORCE_INLINE constexpr T GetKnots() const noexcept
		{
			return m_value * 1.94384f;
		}
		[[nodiscard]] FORCE_INLINE constexpr T GetFeetPerSecond() const noexcept
		{
			return m_value * 3.28084f;
		}

		constexpr FORCE_INLINE TSpeed& operator+=(const TSpeed other) noexcept
		{
			m_value += other.m_value;
			return *this;
		}

		constexpr FORCE_INLINE TSpeed& operator-=(const TSpeed other) noexcept
		{
			m_value -= other.m_value;
			return *this;
		}

		[[nodiscard]] FORCE_INLINE constexpr TSpeed operator-() const noexcept
		{
			return TSpeed(-m_value);
		}

		[[nodiscard]] FORCE_INLINE constexpr TSpeed operator*(const T scalar) const noexcept
		{
			return TSpeed(m_value * scalar);
		}
		FORCE_INLINE constexpr void operator*=(const T scalar) noexcept
		{
			m_value *= scalar;
		}
		[[nodiscard]] FORCE_INLINE constexpr TSpeed operator*(const TSpeed other) const noexcept
		{
			return TSpeed(m_value * other.m_value);
		}
		[[nodiscard]] FORCE_INLINE constexpr TSpeed operator/(const T scalar) const noexcept
		{
			return TSpeed(m_value / scalar);
		}
		FORCE_INLINE constexpr void operator/=(const T scalar) noexcept
		{
			m_value /= scalar;
		}
		[[nodiscard]] FORCE_INLINE constexpr TSpeed operator/(const TSpeed other) const noexcept
		{
			return TSpeed(m_value / other.m_value);
		}
		[[nodiscard]] FORCE_INLINE constexpr TSpeed operator-(const TSpeed other) const noexcept
		{
			return TSpeed(m_value - other.m_value);
		}
		[[nodiscard]] FORCE_INLINE constexpr TSpeed operator+(const TSpeed other) const noexcept
		{
			return TSpeed(m_value + other.m_value);
		}
		[[nodiscard]] FORCE_INLINE constexpr bool operator==(const TSpeed other) const noexcept
		{
			return m_value == other.m_value;
		}
		[[nodiscard]] FORCE_INLINE constexpr bool operator!=(const TSpeed other) const noexcept
		{
			return !operator==(other);
		}
		[[nodiscard]] FORCE_INLINE constexpr bool operator>(const TSpeed other) const noexcept
		{
			return m_value > other.m_value;
		}
		[[nodiscard]] FORCE_INLINE constexpr bool operator>=(const TSpeed other) const noexcept
		{
			return m_value >= other.m_value;
		}
		[[nodiscard]] FORCE_INLINE constexpr bool operator<(const TSpeed other) const noexcept
		{
			return m_value < other.m_value;
		}
		[[nodiscard]] FORCE_INLINE constexpr bool operator<=(const TSpeed other) const noexcept
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
		constexpr Speedf operator""_mps(unsigned long long value) noexcept
		{
			return Speedf::FromMetersPerSecond(static_cast<float>(value));
		}
		constexpr Speedf operator""_kmh(unsigned long long value) noexcept
		{
			return Speedf::FromKilometersPerHour(static_cast<float>(value));
		}
		constexpr Speedf operator""_mph(unsigned long long value) noexcept
		{
			return Speedf::FromMilesPerHour(static_cast<float>(value));
		}
		constexpr Speedf operator""_kn(unsigned long long value) noexcept
		{
			return Speedf::FromKnots(static_cast<float>(value));
		}
		constexpr Speedf operator""_fts(unsigned long long value) noexcept
		{
			return Speedf::FromFeetPerSecond(static_cast<float>(value));
		}

		constexpr Speedf operator""_mps(long double value) noexcept
		{
			return Speedf::FromMetersPerSecond(static_cast<float>(value));
		}
		constexpr Speedf operator""_kmh(long double value) noexcept
		{
			return Speedf::FromKilometersPerHour(static_cast<float>(value));
		}
		constexpr Speedf operator""_mph(long double value) noexcept
		{
			return Speedf::FromMilesPerHour(static_cast<float>(value));
		}
		constexpr Speedf operator""_kn(long double value) noexcept
		{
			return Speedf::FromKnots(static_cast<float>(value));
		}
		constexpr Speedf operator""_fts(long double value) noexcept
		{
			return Speedf::FromFeetPerSecond(static_cast<float>(value));
		}
	}

	using namespace Literals;
}

namespace ngine
{
	using namespace Math::Literals;
}
