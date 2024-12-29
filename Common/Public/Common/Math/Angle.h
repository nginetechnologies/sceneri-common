#pragma once

#include <Common/Platform/ForceInline.h>
#include <Common/Platform/TrivialABI.h>

#include <Common/Guid.h>
#include <Common/Math/Constants.h>
#include <Common/Math/Abs.h>
#include <Common/Math/Sin.h>
#include <Common/Math/Cos.h>
#include <Common/Math/Tan.h>
#include <Common/Math/Round.h>
#include <Common/Math/Wrap.h>
#include <Common/Math/Epsilon.h>
#include <Common/Math/Vector2.h>

#include <Common/Serialization/ForwardDeclarations/Reader.h>
#include <Common/Serialization/ForwardDeclarations/Writer.h>

namespace ngine::Math
{
	template<typename T>
	struct TRIVIAL_ABI TAngle
	{
		inline static constexpr Guid TypeGuid = "{D10F67FB-88A6-4573-8D81-82BDF75EBBB7}"_guid;
	protected:
		template<class F>
		friend struct TVector3;
		template<class F>
		friend struct TBoolVector3;
		FORCE_INLINE constexpr TAngle(const T value) noexcept
			: m_value(value)
		{
		}
	public:
		constexpr TAngle(const ZeroType) noexcept
			: m_value(0)
		{
		}

		constexpr TAngle() noexcept
			: m_value(0)
		{
		}

		constexpr TAngle(const Epsilon<T> value)
			: m_value(value)
		{
		}

		[[nodiscard]] FORCE_INLINE static constexpr TAngle FromDegrees(const T value) noexcept
		{
			return TAngle(value * TConstants<T>::DegToRad);
		}

		[[nodiscard]] FORCE_INLINE static constexpr TAngle FromRadians(const T value) noexcept
		{
			return TAngle(value);
		}

		constexpr FORCE_INLINE TAngle& operator+=(const TAngle other) noexcept
		{
			m_value += other.m_value;
			return *this;
		}

		constexpr FORCE_INLINE TAngle& operator-=(const TAngle other) noexcept
		{
			m_value -= other.m_value;
			return *this;
		}

		[[nodiscard]] FORCE_INLINE constexpr TAngle operator-() const noexcept
		{
			return TAngle(-m_value);
		}

		[[nodiscard]] FORCE_INLINE constexpr TAngle operator*(const T scalar) const noexcept
		{
			return TAngle(m_value * scalar);
		}
		FORCE_INLINE constexpr void operator*=(const T scalar) noexcept
		{
			m_value *= scalar;
		}
		[[nodiscard]] FORCE_INLINE constexpr TAngle operator*(const TAngle other) const noexcept
		{
			return TAngle(m_value * other.m_value);
		}
		[[nodiscard]] FORCE_INLINE constexpr TAngle operator/(const T scalar) const noexcept
		{
			return TAngle(m_value / scalar);
		}
		FORCE_INLINE constexpr void operator/=(const T scalar) noexcept
		{
			m_value /= scalar;
		}
		[[nodiscard]] FORCE_INLINE constexpr TAngle operator/(const TAngle other) const noexcept
		{
			return TAngle(m_value / other.m_value);
		}
		[[nodiscard]] FORCE_INLINE constexpr TAngle operator-(const TAngle other) const noexcept
		{
			return TAngle(m_value - other.m_value);
		}
		[[nodiscard]] FORCE_INLINE constexpr TAngle operator+(const TAngle other) const noexcept
		{
			return TAngle(m_value + other.m_value);
		}
		[[nodiscard]] FORCE_INLINE constexpr bool operator==(const TAngle other) const noexcept
		{
			return m_value == other.m_value;
		}
		[[nodiscard]] FORCE_INLINE constexpr bool operator!=(const TAngle other) const noexcept
		{
			return !operator==(other);
		}
		[[nodiscard]] FORCE_INLINE constexpr bool operator>(const TAngle other) const noexcept
		{
			return m_value > other.m_value;
		}
		[[nodiscard]] FORCE_INLINE constexpr bool operator>=(const TAngle other) const noexcept
		{
			return m_value >= other.m_value;
		}
		[[nodiscard]] FORCE_INLINE constexpr bool operator<(const TAngle other) const noexcept
		{
			return m_value < other.m_value;
		}
		[[nodiscard]] FORCE_INLINE constexpr bool operator<=(const TAngle other) const noexcept
		{
			return m_value <= other.m_value;
		}

		[[nodiscard]] FORCE_INLINE constexpr T GetRadians() const noexcept
		{
			return m_value;
		}
		[[nodiscard]] FORCE_INLINE constexpr T GetDegrees() const noexcept
		{
			return m_value * TConstants<T>::RadToDeg;
		}
		[[nodiscard]] FORCE_INLINE TAngle Cos() const noexcept
		{
			return TAngle(Math::Cos(m_value));
		}
		[[nodiscard]] FORCE_INLINE TAngle Sin() const noexcept
		{
			return TAngle(Math::Sin(m_value));
		}
		[[nodiscard]] FORCE_INLINE TAngle Tan() const noexcept
		{
			return TAngle(Math::Tan(m_value));
		}

		[[nodiscard]] FORCE_INLINE static constexpr TAngle PI() noexcept
		{
			return TAngle(TConstants<T>::PI);
		}
		[[nodiscard]] FORCE_INLINE static constexpr TAngle PI2() noexcept
		{
			return TAngle(TConstants<T>::PI2);
		}

		[[nodiscard]] FORCE_INLINE TAngle Snap(const TAngle snap) const noexcept
		{
			return TAngle(Math::Round(m_value / snap.m_value) * snap.m_value);
		}
		FORCE_INLINE void Wrap() noexcept
		{
			m_value = Math::Wrap(m_value, -TConstants<T>::PI, TConstants<T>::PI);
		}

		bool Serialize(const Serialization::Reader serializer);
		bool Serialize(Serialization::Writer serializer) const;
	protected:
		T m_value;
	};

	template<typename T>
	struct Epsilon<TAngle<T>> : public Epsilon<T>
	{
		using BaseType = Epsilon<T>;
		using BaseType::BaseType;
		using BaseType::operator=;
	};

	using Anglef = TAngle<float>;

	namespace Literals
	{
		constexpr Anglef operator""_radians(unsigned long long value) noexcept
		{
			return Anglef::FromRadians(static_cast<float>(value));
		}

		constexpr Anglef operator""_degrees(unsigned long long value) noexcept
		{
			return Anglef::FromDegrees(static_cast<float>(value));
		}

		constexpr Anglef operator""_radians(long double value) noexcept
		{
			return Anglef::FromRadians(static_cast<float>(value));
		}

		constexpr Anglef operator""_degrees(long double value) noexcept
		{
			return Anglef::FromDegrees(static_cast<float>(value));
		}
	}

	using namespace Literals;

	template<typename T>
	[[nodiscard]] FORCE_INLINE TAngle<T> Round(const TAngle<T> value) noexcept
	{
		return TAngle<T>::FromRadians(Math::Round(value.GetRadians()));
	}

	template<typename T>
	[[nodiscard]] FORCE_INLINE TAngle<T> Abs(const TAngle<T> value) noexcept
	{
		return TAngle<T>::FromRadians(Math::Abs(value.GetRadians()));
	}

	inline static constexpr Anglef PI = Anglef::PI();
	inline static constexpr Anglef PI2 = Anglef::PI2();

	template<typename T>
	struct NumericLimits<TAngle<T>>
	{
		inline static constexpr TAngle<T> NumBits = TAngle<T>::FromRadians(NumericLimits<T>::NumBits);
		inline static constexpr TAngle<T> Min = TAngle<T>::FromRadians(NumericLimits<T>::Min);
		inline static constexpr TAngle<T> Max = TAngle<T>::FromRadians(NumericLimits<T>::Max);
		inline static constexpr TAngle<T> Epsilon = TAngle<T>::FromRadians(NumericLimits<T>::Epsilon);
		inline static constexpr bool IsUnsigned = NumericLimits<T>::IsUnsigned;
	};
}

namespace ngine
{
	using namespace Math::Literals;
	using Math::PI;
	using Math::PI2;
}
