#pragma once

#include <Common/Platform/ForceInline.h>
#include <Common/Platform/TrivialABI.h>

#include <Common/Guid.h>
#include <Common/Math/ForwardDeclarations/Density.h>
#include <Common/Serialization/ForwardDeclarations/Reader.h>
#include <Common/Serialization/ForwardDeclarations/Writer.h>

namespace ngine::Math
{
	template<typename Type>
	struct TRIVIAL_ABI Density
	{
		inline static constexpr Guid TypeGuid = "f8b2f900-20ff-4172-8c05-d05ecc073104"_guid;

		using UnitType = Type;

		FORCE_INLINE Density() = default;

		template<typename OtherType>
		FORCE_INLINE constexpr Density(const Density<OtherType> other) noexcept
			: m_value(static_cast<Type>(other.GetKilogramsCubed()))
		{
		}

		Density(const Density&) = default;
		Density& operator=(const Density&) = default;

		[[nodiscard]] FORCE_INLINE PURE_STATICS static constexpr Density FromKilogramsCubed(const Type value) noexcept
		{
			return Density{value};
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Type GetKilogramsCubed() const noexcept
		{
			return m_value;
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Density operator-(const Density other) const noexcept
		{
			return Density{m_value - other.m_value};
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Density operator+(const Density other) const noexcept
		{
			return Density{m_value + other.m_value};
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Density operator*(const Density other) const noexcept
		{
			return Density{m_value * other.m_value};
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Density operator*(const Type value) const noexcept
		{
			return Density{Type(m_value * value)};
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Density operator/(const Density other) const noexcept
		{
			return Density{m_value / other.m_value};
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Density operator/(const Type value) const noexcept
		{
			return Density{Type(m_value / value)};
		}

		FORCE_INLINE constexpr Density& operator-=(const Density other) noexcept
		{
			*this = *this - other;
			return *this;
		}

		FORCE_INLINE constexpr Density& operator+=(const Density other) noexcept
		{
			*this = *this + other;
			return *this;
		}

		FORCE_INLINE constexpr Density& operator*=(const Density other) noexcept
		{
			*this = *this * other;
			return *this;
		}

		FORCE_INLINE constexpr Density& operator/=(const Density other) noexcept
		{
			*this = *this / other;
			return *this;
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Density operator-() const noexcept
		{
			return Density{-m_value};
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator>(const Density other) const noexcept
		{
			return m_value > other.m_value;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator>=(const Density other) const noexcept
		{
			return m_value >= other.m_value;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator<(const Density other) const noexcept
		{
			return m_value < other.m_value;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator<=(const Density other) const noexcept
		{
			return m_value <= other.m_value;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator!=(const Density other) const noexcept
		{
			return m_value != other.m_value;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator==(const Density other) const noexcept
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
		FORCE_INLINE constexpr Density(const Type metersCubed) noexcept
			: m_value(metersCubed)
		{
		}
	protected:
		Type m_value;
	};

	template<typename Type>
	[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator<=(const Type left, const Density<Type> right) noexcept
	{
		return left <= right.GetKilogramsCubed();
	}

	template<typename Type>
	[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator<(const Type left, const Density<Type> right) noexcept
	{
		return left < right.GetKilogramsCubed();
	}

	template<typename Type>
	[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator>=(const Type left, const Density<Type> right) noexcept
	{
		return left >= right.GetKilogramsCubed();
	}

	template<typename Type>
	[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator>(const Type left, const Density<Type> right) noexcept
	{
		return left > right.GetKilogramsCubed();
	}

	template<typename Type>
	[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Type operator-(const Type left, const Density<Type> right) noexcept
	{
		return left - right.GetKilogramsCubed();
	}

	namespace Literals
	{
		constexpr Densityf operator""_kilograms_cubed(unsigned long long value) noexcept
		{
			return Densityf::FromKilogramsCubed(static_cast<float>(value));
		}
		constexpr Densityf operator""_kilograms_cubed(long double value) noexcept
		{
			return Densityf::FromKilogramsCubed(static_cast<float>(value));
		}
	}

	using namespace Literals;
}

namespace ngine
{
	using namespace Math::Literals;
}
