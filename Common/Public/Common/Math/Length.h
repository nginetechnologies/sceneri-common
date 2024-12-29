#pragma once

#include <Common/Platform/ForceInline.h>
#include <Common/Platform/TrivialABI.h>

#include <Common/Guid.h>
#include <Common/Math/ForwardDeclarations/Length.h>
#include <Common/Serialization/ForwardDeclarations/Reader.h>
#include <Common/Serialization/ForwardDeclarations/Writer.h>

namespace ngine::Math
{
	template<typename Type>
	struct TRIVIAL_ABI Length
	{
		inline static constexpr Guid TypeGuid = "{D478418C-4ABD-429F-91B8-3CF4DFA052E2}"_guid;

		using UnitType = Type;

		FORCE_INLINE Length() = default;

		template<typename OtherType>
		FORCE_INLINE constexpr Length(const Length<OtherType> other) noexcept
			: m_value(static_cast<Type>(other.GetMeters()))
		{
		}

		FORCE_INLINE constexpr Length(ZeroType) noexcept
			: m_value(0)
		{
		}

		Length(const Length&) = default;
		Length& operator=(const Length&) = default;

		[[nodiscard]] FORCE_INLINE PURE_STATICS static constexpr Length FromKilometers(const Type value) noexcept
		{
			return Length{value * Type(1000.f)};
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS static constexpr Length FromMeters(const Type value) noexcept
		{
			return Length{value};
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS static constexpr Length FromDecimeters(const Type value) noexcept
		{
			return Length{value * Type(0.1f)};
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS static constexpr Length FromCentimeters(const Type value) noexcept
		{
			return Length{value * Type(0.01f)};
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS static constexpr Length FromMillimeters(const Type value) noexcept
		{
			return Length{value * Type(0.001f)};
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS static constexpr Length FromInches(const Type value) noexcept
		{
			return Length{value / Type(39.3700787402f)};
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS static constexpr Length FromUnits(const Type value) noexcept
		{
			return Length{value};
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Length operator-(const Length other) const noexcept
		{
			return Length{m_value - other.m_value};
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Length operator+(const Length other) const noexcept
		{
			return Length{m_value + other.m_value};
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Length operator*(const Length other) const noexcept
		{
			return Length{m_value * other.m_value};
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Length operator*(const Type value) const noexcept
		{
			return Length{Type(m_value * value)};
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Length operator/(const Length other) const noexcept
		{
			return Length{m_value / other.m_value};
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Length operator/(const Type value) const noexcept
		{
			return Length{Type(m_value / value)};
		}

		FORCE_INLINE constexpr Length& operator-=(const Length other) noexcept
		{
			*this = *this - other;
			return *this;
		}

		FORCE_INLINE constexpr Length& operator+=(const Length other) noexcept
		{
			*this = *this + other;
			return *this;
		}

		FORCE_INLINE constexpr Length& operator*=(const Length other) noexcept
		{
			*this = *this * other;
			return *this;
		}

		FORCE_INLINE constexpr Length& operator/=(const Length other) noexcept
		{
			*this = *this / other;
			return *this;
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Length operator-() const noexcept
		{
			return Length{-m_value};
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator>(const Length other) const noexcept
		{
			return m_value > other.m_value;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator>=(const Length other) const noexcept
		{
			return m_value >= other.m_value;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator<(const Length other) const noexcept
		{
			return m_value < other.m_value;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator<=(const Length other) const noexcept
		{
			return m_value <= other.m_value;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator!=(const Length other) const noexcept
		{
			return m_value != other.m_value;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator==(const Length other) const noexcept
		{
			return m_value == other.m_value;
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr UnitType operator-(const UnitType other) const noexcept
		{
			return m_value - other;
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Type GetKilometers() const noexcept
		{
			return m_value * Type(0.001f);
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Type GetMeters() const noexcept
		{
			return m_value;
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Type GetUnits() const noexcept
		{
			return m_value;
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Type GetDecimeters() const noexcept
		{
			return m_value * Type(10.f);
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Type GetCentimeters() const noexcept
		{
			return m_value * Type(100.f);
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Type GetMillimeters() const noexcept
		{
			return m_value * Type(1000.f);
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Type GetInches() const noexcept
		{
			return m_value * Type(39.3700787402f);
		}

		enum class UnitMultipleType : uint8
		{
			Millimeter,
			Centimeter,
			Meter,
			Unit = Meter,
			Kilometer
		};

		struct SimplifiedResult
		{
			UnitMultipleType multiple;
			Type value;
		};

		[[nodiscard]] PURE_STATICS SimplifiedResult GetSimplestUnit() const
		{
			if (m_value >= (Type)1000)
			{
				return {UnitMultipleType::Kilometer, GetKilometers()};
			}
			else if (m_value >= (Type)1)
			{
				return {UnitMultipleType::Meter, GetMeters()};
			}
			else if (m_value >= (Type)0.01)
			{
				return {UnitMultipleType::Centimeter, GetCentimeters()};
			}
			else if (m_value >= (Type)0.001)
			{
				return {UnitMultipleType::Millimeter, GetMillimeters()};
			}

			return {UnitMultipleType::Meter, GetMeters()};
		}

		[[nodiscard]] PURE_STATICS static Math::Lengthf FromMultipleValue(const Type value, const UnitMultipleType unit)
		{
			switch (unit)
			{
				case UnitMultipleType::Millimeter:
					return FromMillimeters(value);
				case UnitMultipleType::Centimeter:
					return FromCentimeters(value);
				case UnitMultipleType::Meter:
					return FromMeters(value);
				case UnitMultipleType::Kilometer:
					return FromKilometers(value);
			}

			ExpectUnreachable();
		}

		[[nodiscard]] PURE_STATICS Type GetMultipleValue(const UnitMultipleType unit) const
		{
			switch (unit)
			{
				case UnitMultipleType::Millimeter:
					return GetMillimeters();
				case UnitMultipleType::Centimeter:
					return GetCentimeters();
				case UnitMultipleType::Meter:
					return GetMeters();
				case UnitMultipleType::Kilometer:
					return GetKilometers();
			}

			ExpectUnreachable();
		}

		bool Serialize(const Serialization::Reader);
		bool Serialize(Serialization::Writer) const;
	protected:
		FORCE_INLINE constexpr Length(const Type meters) noexcept
			: m_value(meters)
		{
		}
	protected:
		Type m_value;
	};

	template<typename Type>
	[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator<=(const Type left, const Length<Type> right) noexcept
	{
		return left <= right.GetMeters();
	}

	template<typename Type>
	[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator<(const Type left, const Length<Type> right) noexcept
	{
		return left < right.GetMeters();
	}

	template<typename Type>
	[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator>=(const Type left, const Length<Type> right) noexcept
	{
		return left >= right.GetMeters();
	}

	template<typename Type>
	[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator>(const Type left, const Length<Type> right) noexcept
	{
		return left > right.GetMeters();
	}

	template<typename Type>
	[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Type operator-(const Type left, const Length<Type> right) noexcept
	{
		return left - right.GetMeters();
	}

	namespace Literals
	{
		constexpr Lengthd operator""_meters(unsigned long long value) noexcept
		{
			return Lengthd::FromMeters(static_cast<float>(value));
		}

		constexpr Lengthd operator""_meters(long double value) noexcept
		{
			return Lengthd::FromMeters(static_cast<float>(value));
		}

		constexpr Lengthd operator""_units(unsigned long long value) noexcept
		{
			return Lengthd::FromMeters(static_cast<float>(value));
		}

		constexpr Lengthd operator""_units(long double value) noexcept
		{
			return Lengthd::FromMeters(static_cast<float>(value));
		}

		constexpr Lengthd operator""_centimeters(unsigned long long value) noexcept
		{
			return Lengthd::FromCentimeters(static_cast<float>(value));
		}

		constexpr Lengthd operator""_centimeters(long double value) noexcept
		{
			return Lengthd::FromCentimeters(static_cast<float>(value));
		}

		constexpr Lengthd operator""_millimeters(unsigned long long value) noexcept
		{
			return Lengthd::FromMillimeters(static_cast<float>(value));
		}

		constexpr Lengthd operator""_millimeters(long double value) noexcept
		{
			return Lengthd::FromMillimeters(static_cast<float>(value));
		}
	}

	using namespace Literals;
}

namespace ngine
{
	using namespace Math::Literals;
}
