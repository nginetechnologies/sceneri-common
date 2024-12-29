#pragma once

#include <Common/Platform/ForceInline.h>

#include <Common/Math/ForwardDeclarations/Torque.h>
#include <Common/Serialization/ForwardDeclarations/Reader.h>
#include <Common/Serialization/ForwardDeclarations/Writer.h>
#include <Common/Platform/TrivialABI.h>

namespace ngine::Math
{
	template<typename Type>
	struct TRIVIAL_ABI Torque
	{
		inline static constexpr Guid TypeGuid = "{aef01668-dbcb-42c1-a23a-bc058ac10a11}"_guid;
		using UnitType = Type;

		FORCE_INLINE Torque() = default;

		template<typename OtherType>
		FORCE_INLINE constexpr Torque(const Torque<OtherType> other) noexcept
			: m_value(static_cast<Type>(other.GetNewtonMeters()))
		{
		}

		Torque(const Torque&) = default;
		Torque& operator=(const Torque&) = default;

		[[nodiscard]] FORCE_INLINE PURE_STATICS static constexpr Torque FromNewtonMeters(const Type value) noexcept
		{
			return Torque{value};
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Type GetNewtonMeters() const noexcept
		{
			return m_value;
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Torque operator-(const Torque other) const noexcept
		{
			return Torque{m_value - other.m_value};
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Torque operator+(const Torque other) const noexcept
		{
			return Torque{m_value + other.m_value};
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Torque operator*(const Torque other) const noexcept
		{
			return Torque{m_value * other.m_value};
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Torque operator*(const Type value) const noexcept
		{
			return Torque{Type(m_value * value)};
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Torque operator/(const Torque other) const noexcept
		{
			return Torque{m_value / other.m_value};
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Torque operator/(const Type value) const noexcept
		{
			return Torque{Type(m_value / value)};
		}

		FORCE_INLINE constexpr Torque& operator-=(const Torque other) noexcept
		{
			*this = *this - other;
			return *this;
		}

		FORCE_INLINE constexpr Torque& operator+=(const Torque other) noexcept
		{
			*this = *this + other;
			return *this;
		}

		FORCE_INLINE constexpr Torque& operator*=(const Torque other) noexcept
		{
			*this = *this * other;
			return *this;
		}

		FORCE_INLINE constexpr Torque& operator/=(const Torque other) noexcept
		{
			*this = *this / other;
			return *this;
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Torque operator-() const noexcept
		{
			return Torque{-m_value};
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator>(const Torque other) const noexcept
		{
			return m_value > other.m_value;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator>=(const Torque other) const noexcept
		{
			return m_value >= other.m_value;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator<(const Torque other) const noexcept
		{
			return m_value < other.m_value;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator<=(const Torque other) const noexcept
		{
			return m_value <= other.m_value;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator!=(const Torque other) const noexcept
		{
			return m_value != other.m_value;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator==(const Torque other) const noexcept
		{
			return m_value == other.m_value;
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr UnitType operator-(const UnitType other) const noexcept
		{
			return m_value - other;
		}

		bool Serialize(const Serialization::Reader);
		bool Serialize(Serialization::Writer) const;
	protected:
		FORCE_INLINE constexpr Torque(const Type newtonMeters) noexcept
			: m_value(newtonMeters)
		{
		}
	protected:
		Type m_value;
	};

	template<typename Type>
	[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator<=(const Type left, const Torque<Type> right) noexcept
	{
		return left <= right.GetNewtonMeters();
	}

	template<typename Type>
	[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator<(const Type left, const Torque<Type> right) noexcept
	{
		return left < right.GetNewtonMeters();
	}

	template<typename Type>
	[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator>=(const Type left, const Torque<Type> right) noexcept
	{
		return left >= right.GetNewtonMeters();
	}

	template<typename Type>
	[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator>(const Type left, const Torque<Type> right) noexcept
	{
		return left > right.GetNewtonMeters();
	}

	template<typename Type>
	[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Type operator-(const Type left, const Torque<Type> right) noexcept
	{
		return left - right.GetNewtonMeters();
	}

	namespace Literals
	{
		constexpr Torqued operator""_newtonmeters(unsigned long long value) noexcept
		{
			return Torqued::FromNewtonMeters(static_cast<float>(value));
		}
	}

	using namespace Literals;
}

namespace ngine
{
	using namespace Math::Literals;
}
