#pragma once

#include "ForwardDeclarations/RotationalSpeed.h"
#include "Frequency.h"

#include <Common/Math/Angle.h>
#include <Common/Time/Duration.h>

namespace ngine::Math
{
	template<typename T>
	struct TRIVIAL_ABI TRotationalSpeed : public TFrequency<T>
	{
		using BaseType = TFrequency<T>;

		inline static constexpr Guid TypeGuid = "{f7fe785d-341e-46f5-b8a1-457c24de84fc}"_guid;

		using BaseType::BaseType;
		constexpr TRotationalSpeed(const BaseType frequency)
			: BaseType(frequency)
		{
		}

		[[nodiscard]] FORCE_INLINE static constexpr TRotationalSpeed FromRadiansPerSecond(const T value) noexcept
		{
			return TRotationalSpeed::FromHertz((T(1) / TConstants<T>::PI2) * value);
		}
		[[nodiscard]] FORCE_INLINE static constexpr TRotationalSpeed FromRevolutionsPerMinute(const T value) noexcept
		{
			return TRotationalSpeed::FromHertz(value / T(60));
		}

		[[nodiscard]] FORCE_INLINE constexpr T GetRadiansPerSecond() const noexcept
		{
			return BaseType::GetHertz() * TConstants<T>::PI2;
		}
		[[nodiscard]] FORCE_INLINE constexpr T GetRevolutionsPerMinute() const noexcept
		{
			return BaseType::GetHertz() * T(60);
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
