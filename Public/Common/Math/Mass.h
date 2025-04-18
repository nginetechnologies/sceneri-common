#pragma once

#include <Common/Platform/ForceInline.h>
#include <Common/Platform/TrivialABI.h>

#include <Common/Guid.h>
#include <Common/Math/ForwardDeclarations/Mass.h>
#include <Common/Serialization/ForwardDeclarations/Reader.h>
#include <Common/Serialization/ForwardDeclarations/Writer.h>

namespace ngine::Math
{
	template<typename Type>
	struct TRIVIAL_ABI Mass
	{
		inline static constexpr Guid TypeGuid = "{F0357577-5C4A-4DD2-8874-7BDDFD1C6163}"_guid;

		using UnitType = Type;

		FORCE_INLINE Mass() = default;

		template<typename OtherType>
		FORCE_INLINE constexpr Mass(const Mass<OtherType> other) noexcept
			: m_value(static_cast<Type>(other.GetKilograms()))
		{
		}

		Mass(const Mass&) = default;
		Mass& operator=(const Mass&) = default;

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Type GetDecigrams() const noexcept
		{
			return m_value * 10000.f;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS static constexpr Mass FromDecigrams(const Type value) noexcept
		{
			return Mass{value * 0.0001f};
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Type GetGrams() const noexcept
		{
			return m_value * 1000.f;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS static constexpr Mass FromGrams(const Type value) noexcept
		{
			return Mass{value * 0.001f};
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS static constexpr Mass FromKilograms(const Type value) noexcept
		{
			return Mass{value};
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Type GetKilograms() const noexcept
		{
			return m_value;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Type GetTons() const noexcept
		{
			return m_value * 0.001f;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS static constexpr Mass FromTons(const Type value) noexcept
		{
			return Mass{value * 1000.f};
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Mass operator-(const Mass other) const noexcept
		{
			return Mass{m_value - other.m_value};
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Mass operator+(const Mass other) const noexcept
		{
			return Mass{m_value + other.m_value};
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Mass operator*(const Mass other) const noexcept
		{
			return Mass{m_value * other.m_value};
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Mass operator*(const Type value) const noexcept
		{
			return Mass{Type(m_value * value)};
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Mass operator/(const Mass other) const noexcept
		{
			return Mass{m_value / other.m_value};
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Mass operator/(const Type value) const noexcept
		{
			return Mass{Type(m_value / value)};
		}

		FORCE_INLINE constexpr Mass& operator-=(const Mass other) noexcept
		{
			*this = *this - other;
			return *this;
		}

		FORCE_INLINE constexpr Mass& operator+=(const Mass other) noexcept
		{
			*this = *this + other;
			return *this;
		}

		FORCE_INLINE constexpr Mass& operator*=(const Mass other) noexcept
		{
			*this = *this * other;
			return *this;
		}

		FORCE_INLINE constexpr Mass& operator/=(const Mass other) noexcept
		{
			*this = *this / other;
			return *this;
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Mass operator-() const noexcept
		{
			return Mass{-m_value};
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator>(const Mass other) const noexcept
		{
			return m_value > other.m_value;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator>=(const Mass other) const noexcept
		{
			return m_value >= other.m_value;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator<(const Mass other) const noexcept
		{
			return m_value < other.m_value;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator<=(const Mass other) const noexcept
		{
			return m_value <= other.m_value;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator!=(const Mass other) const noexcept
		{
			return m_value != other.m_value;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator==(const Mass other) const noexcept
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
		FORCE_INLINE constexpr Mass(const Type meters) noexcept
			: m_value(meters)
		{
		}
	protected:
		Type m_value;
	};

	template<typename Type>
	[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator<=(const Type left, const Mass<Type> right) noexcept
	{
		return left <= right.GetKilograms();
	}

	template<typename Type>
	[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator<(const Type left, const Mass<Type> right) noexcept
	{
		return left < right.GetKilograms();
	}

	template<typename Type>
	[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator>=(const Type left, const Mass<Type> right) noexcept
	{
		return left >= right.GetKilograms();
	}

	template<typename Type>
	[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator>(const Type left, const Mass<Type> right) noexcept
	{
		return left > right.GetKilograms();
	}

	template<typename Type>
	[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Type operator-(const Type left, const Mass<Type> right) noexcept
	{
		return left - right.GetKilograms();
	}

	namespace Literals
	{
		constexpr Massd operator""_decigrams(unsigned long long value) noexcept
		{
			return Massd::FromDecigrams(static_cast<float>(value));
		}
		constexpr Massd operator""_decigrams(long double value) noexcept
		{
			return Massd::FromDecigrams(static_cast<float>(value));
		}
		constexpr Massd operator""_grams(unsigned long long value) noexcept
		{
			return Massd::FromGrams(static_cast<float>(value));
		}
		constexpr Massd operator""_grams(long double value) noexcept
		{
			return Massd::FromGrams(static_cast<float>(value));
		}
		constexpr Massd operator""_kilograms(unsigned long long value) noexcept
		{
			return Massd::FromKilograms(static_cast<float>(value));
		}
		constexpr Massd operator""_kilograms(long double value) noexcept
		{
			return Massd::FromKilograms(static_cast<float>(value));
		}
		constexpr Massd operator""_tons(unsigned long long value) noexcept
		{
			return Massd::FromTons(static_cast<float>(value));
		}
		constexpr Massd operator""_tons(long double value) noexcept
		{
			return Massd::FromTons(static_cast<float>(value));
		}
	}

	using namespace Literals;
}

namespace ngine
{
	using namespace Math::Literals;
}
