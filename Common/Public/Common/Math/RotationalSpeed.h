#pragma once

#include "ForwardDeclarations/RotationalSpeed.h"

#include <Common/Platform/ForceInline.h>
#include <Common/Platform/TrivialABI.h>
#include <Common/Math/Constants.h>
#include <Common/Math/Angle.h>
#include <Common/Time/Duration.h>

#include <Common/Serialization/ForwardDeclarations/Reader.h>
#include <Common/Serialization/ForwardDeclarations/Writer.h>

namespace ngine::Math
{
	template<typename T>
	struct TRIVIAL_ABI TRotationalSpeed
	{
		inline static constexpr Guid TypeGuid = "{f7fe785d-341e-46f5-b8a1-457c24de84fc}"_guid;
	protected:
		FORCE_INLINE constexpr TRotationalSpeed(const T value) noexcept
			: m_value(value)
		{
		}
	public:
		constexpr TRotationalSpeed(const ZeroType) noexcept
			: m_value(0)
		{
		}

		constexpr TRotationalSpeed() noexcept
			: m_value(0)
		{
		}

		[[nodiscard]] FORCE_INLINE static constexpr TRotationalSpeed FromRadiansPerSecond(const T value) noexcept
		{
			return TRotationalSpeed(value);
		}
		[[nodiscard]] FORCE_INLINE static constexpr TRotationalSpeed FromRevolutionsPerMinute(const T value) noexcept
		{
			constexpr T rpmToRadiansPerSecond = (T(1) / T(60)) * TConstants<T>::PI2;
			return TRotationalSpeed(value * rpmToRadiansPerSecond);
		}
		[[nodiscard]] FORCE_INLINE static constexpr TRotationalSpeed FromHertz(const T value) noexcept
		{
			constexpr T hzToRadiansPerSecond = TConstants<T>::PI2;
			return TRotationalSpeed(value * hzToRadiansPerSecond);
		}
		[[nodiscard]] FORCE_INLINE static constexpr TRotationalSpeed FromCyclesPerSecond(const T value) noexcept
		{
			return FromHertz(value);
		}

		[[nodiscard]] FORCE_INLINE constexpr T GetRadiansPerSecond() const noexcept
		{
			return m_value;
		}
		[[nodiscard]] FORCE_INLINE constexpr T GetRevolutionsPerMinute() const noexcept
		{
			constexpr T radiansPerSecondToRpm = T(60) / TConstants<T>::PI2;
			return m_value * radiansPerSecondToRpm;
		}
		[[nodiscard]] FORCE_INLINE constexpr T GetHertz() const noexcept
		{
			constexpr T radiansPerSecondToHz = T(1) / TConstants<T>::PI2;
			return m_value * radiansPerSecondToHz;
		}
		[[nodiscard]] FORCE_INLINE constexpr T GetCyclesPerSecond() const noexcept
		{
			return GetHertz();
		}

		constexpr FORCE_INLINE TRotationalSpeed& operator+=(const TRotationalSpeed other) noexcept
		{
			m_value += other.m_value;
			return *this;
		}

		constexpr FORCE_INLINE TRotationalSpeed& operator-=(const TRotationalSpeed other) noexcept
		{
			m_value -= other.m_value;
			return *this;
		}

		[[nodiscard]] FORCE_INLINE constexpr TRotationalSpeed operator-() const noexcept
		{
			return TRotationalSpeed(-m_value);
		}

		[[nodiscard]] FORCE_INLINE constexpr TRotationalSpeed operator*(const T scalar) const noexcept
		{
			return TRotationalSpeed(m_value * scalar);
		}
		FORCE_INLINE constexpr void operator*=(const T scalar) noexcept
		{
			m_value *= scalar;
		}
		[[nodiscard]] FORCE_INLINE constexpr TRotationalSpeed operator*(const TRotationalSpeed other) const noexcept
		{
			return TRotationalSpeed(m_value * other.m_value);
		}
		[[nodiscard]] FORCE_INLINE constexpr TRotationalSpeed operator/(const T scalar) const noexcept
		{
			return TRotationalSpeed(m_value / scalar);
		}
		FORCE_INLINE constexpr void operator/=(const T scalar) noexcept
		{
			m_value /= scalar;
		}
		[[nodiscard]] FORCE_INLINE constexpr TRotationalSpeed operator/(const TRotationalSpeed other) const noexcept
		{
			return TRotationalSpeed(m_value / other.m_value);
		}
		[[nodiscard]] FORCE_INLINE constexpr TRotationalSpeed operator-(const TRotationalSpeed other) const noexcept
		{
			return TRotationalSpeed(m_value - other.m_value);
		}
		[[nodiscard]] FORCE_INLINE constexpr TRotationalSpeed operator+(const TRotationalSpeed other) const noexcept
		{
			return TRotationalSpeed(m_value + other.m_value);
		}
		[[nodiscard]] FORCE_INLINE constexpr bool operator==(const TRotationalSpeed other) const noexcept
		{
			return m_value == other.m_value;
		}
		[[nodiscard]] FORCE_INLINE constexpr bool operator!=(const TRotationalSpeed other) const noexcept
		{
			return !operator==(other);
		}
		[[nodiscard]] FORCE_INLINE constexpr bool operator>(const TRotationalSpeed other) const noexcept
		{
			return m_value > other.m_value;
		}
		[[nodiscard]] FORCE_INLINE constexpr bool operator>=(const TRotationalSpeed other) const noexcept
		{
			return m_value >= other.m_value;
		}
		[[nodiscard]] FORCE_INLINE constexpr bool operator<(const TRotationalSpeed other) const noexcept
		{
			return m_value < other.m_value;
		}
		[[nodiscard]] FORCE_INLINE constexpr bool operator<=(const TRotationalSpeed other) const noexcept
		{
			return m_value <= other.m_value;
		}

		[[nodiscard]] FORCE_INLINE constexpr TAngle<T> operator*(const Time::Duration<T> time) const noexcept
		{
			return TAngle<T>::FromRadians(GetRadiansPerSecond() * time.GetSeconds());
		}

		bool Serialize(const Serialization::Reader serializer);
		bool Serialize(Serialization::Writer serializer) const;
	protected:
		T m_value;
	};

	namespace Literals
	{
		constexpr RotationalSpeedf operator""_rads(unsigned long long value) noexcept
		{
			return RotationalSpeedf::FromRadiansPerSecond(static_cast<float>(value));
		}
		constexpr RotationalSpeedf operator""_rpm(unsigned long long value) noexcept
		{
			return RotationalSpeedf::FromRevolutionsPerMinute(static_cast<float>(value));
		}
		constexpr RotationalSpeedf operator""_cps(unsigned long long value) noexcept
		{
			return RotationalSpeedf::FromCyclesPerSecond(static_cast<float>(value));
		}

		constexpr RotationalSpeedf operator""_rads(long double value) noexcept
		{
			return RotationalSpeedf::FromRadiansPerSecond(static_cast<float>(value));
		}
		constexpr RotationalSpeedf operator""_rpm(long double value) noexcept
		{
			return RotationalSpeedf::FromRevolutionsPerMinute(static_cast<float>(value));
		}
		constexpr RotationalSpeedf operator""_cps(long double value) noexcept
		{
			return RotationalSpeedf::FromCyclesPerSecond(static_cast<float>(value));
		}
	}

	using namespace Literals;
}

namespace ngine
{
	using namespace Math::Literals;
}
