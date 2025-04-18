#pragma once

#include "CoreNumericTypes.h"

#include <Common/Guid.h>
#include <Common/Math/Select.h>
#include <Common/Math/Vector4.h>
#include <Common/Math/Vector3.h>
#include <Common/Math/Vector3/Min.h>
#include <Common/Math/Vector3/Max.h>
#include <Common/Math/Vector3/Abs.h>
#include <Common/Math/Vector3/Floor.h>
#include <Common/Math/Vector4/Round.h>
#include <Common/Math/Vector4/Min.h>
#include <Common/Math/Vector4/Max.h>
#include <Common/Math/Clamp.h>
#include <Common/Math/LinearInterpolate.h>
#include <Common/Math/Step.h>
#include <Common/Math/Fract.h>
#include <Common/Memory/Optional.h>
#include <Common/Memory/Containers/StringView.h>
#include <Common/Memory/Containers/ForwardDeclarations/FlatString.h>
#include <Common/Serialization/ForwardDeclarations/Reader.h>
#include <Common/Serialization/ForwardDeclarations/Writer.h>
#include <Common/TypeTraits/IsIntegral.h>
#include <Common/TypeTraits/IsFloatingPoint.h>
#include <Common/Platform/TrivialABI.h>

namespace ngine::Math
{
	namespace Internal
	{
		[[nodiscard]] FORCE_INLINE constexpr bool IsDigit(const char ch) noexcept
		{
			return unsigned(ch - '0') <= 9;
		}

		[[nodiscard]] FORCE_INLINE constexpr bool IsUpper(const char ch) noexcept
		{
			return (ch >= 'A') & (ch <= 'Z');
		}

		[[nodiscard]] FORCE_INLINE constexpr uint8 ParseHexCharacter(uint8 character) noexcept
		{
			if (IsDigit(character))
			{
				character -= '0';
			}
			else
			{
				character -= Math::Select(IsUpper(character), (char)('A' - (char)10), (char)('a' - (char)10));
			}
			return character;
		}

		[[nodiscard]] FORCE_INLINE constexpr uint8 ParseHexString(const char* string) noexcept
		{
			return ParseHexCharacter(string[0]) * 16 + ParseHexCharacter(string[1]);
		}
	}

	template<typename T>
	struct TColor;

	struct TRIVIAL_ABI PackedColor32
	{
		PackedColor32() = default;
		explicit constexpr PackedColor32(const uint32 value)
			: m_value(value)
		{
		}
		constexpr PackedColor32(const uint32 r, const uint32 g, const uint32 b, const uint32 a = 255)
			: m_value{(r << 24) | (g << 16) | (b << 8) | a}
		{
		}

		template<typename UnitType>
		explicit PackedColor32(const TColor<UnitType> color);

		uint32 m_value;
	};

	template<typename T>
	struct TRIVIAL_ABI alignas(TVector4<T>) TColor
	{
		inline static constexpr Guid TypeGuid = "{83198F17-49FA-4B1C-8418-8EE22EAE7690}"_guid;
		using Vector4Type = TVector4<T>;
		using Vector3Type = TVector3<T>;

		FORCE_INLINE TColor()
		{
		}

		FORCE_INLINE constexpr TColor(ZeroType) noexcept
			: r(0)
			, g(0)
			, b(0)
			, a(0)
		{
		}

		explicit FORCE_INLINE constexpr TColor(const T value, const T alpha = (T)1) noexcept
			: r(value)
			, g(value)
			, b(value)
			, a(alpha)
		{
		}

		FORCE_INLINE constexpr TColor(const T _r, const T _g, const T _b, const T alpha = (T)1) noexcept
			: r(_r)
			, g(_g)
			, b(_b)
			, a(alpha)
		{
		}

		FORCE_INLINE constexpr TColor(const Vector4Type value) noexcept
			: m_vector(value)
		{
		}

		FORCE_INLINE constexpr TColor(const PackedColor32 packedColor) noexcept
			: TColor(FromPacked32(packedColor))
		{
		}

		template<typename OtherType>
		[[nodiscard]] FORCE_INLINE constexpr operator TColor<OtherType>() const
		{
			if constexpr (TypeTraits::IsIntegral<OtherType> && TypeTraits::IsFloatingPoint<T>)
			{
				Vector4Type newValue{r, g, b, a};
				newValue *= T(255);
				return TColor<OtherType>(
					TVector4<OtherType>{(OtherType)newValue.x, (OtherType)newValue.y, (OtherType)newValue.z, (OtherType)newValue.w}
				);
			}
			else if constexpr (TypeTraits::IsIntegral<T> && TypeTraits::IsFloatingPoint<OtherType>)
			{
				TVector4<OtherType> newValue{(OtherType)r, (OtherType)g, (OtherType)b, (OtherType)a};
				newValue *= OtherType(1) / OtherType(255);
				return TColor<OtherType>(newValue);
			}
			else
			{
				return TColor<OtherType>(TVector4<OtherType>{(OtherType)r, (OtherType)g, (OtherType)b, (OtherType)a});
			}
		}

		[[nodiscard]] static constexpr TColor Parse(const char* string, uint32 length)
		{
			const bool hasHashtag = length > 0 && string[0] == '#';
			string += hasHashtag;
			length -= hasHashtag;

			switch (length)
			{
				case 0:
					return Convert<uint8>(255, 255, 255, 255);
				case 1:
				{
					const uint8 value = Internal::ParseHexCharacter(string[0]) * 16;
					return Convert<T>(value, value, value, 255);
				}
				case 2:
				{
					const uint8 value = Internal::ParseHexString(string);
					return Convert<T>(value, value, value, 255);
				}
				case 3:
				{
					// Shorthand for 6 digit hex code with repeated signs
					// i.e. #fc9 = #ffcc99
					const uint8 r = Internal::ParseHexCharacter(string[0]);
					const uint8 g = Internal::ParseHexCharacter(string[1]);
					const uint8 b = Internal::ParseHexCharacter(string[2]);

					return Convert<T>(r * 16 + r, g * 16 + g, b * 16 + b, 255);
				}
				case 4:
				{
					const uint8 r = Internal::ParseHexString(string);
					string += 2;
					const uint8 g = Internal::ParseHexString(string);
					return Convert<T>(r, g, 255, 255);
				}
				case 5:
				{
					const uint8 r = Internal::ParseHexString(string);
					string += 2;
					const uint8 g = Internal::ParseHexString(string);
					string += 2;
					const uint8 b = Internal::ParseHexCharacter(string[0]) * 16;
					return Convert<T>(r, g, b, 255);
				}
				case 6:
				{
					const uint8 r = Internal::ParseHexString(string);
					string += 2;
					const uint8 g = Internal::ParseHexString(string);
					string += 2;
					const uint8 b = Internal::ParseHexString(string);
					return Convert<T>(r, g, b, 255);
				}
				case 7:
				{
					const uint8 r = Internal::ParseHexString(string);
					string += 2;
					const uint8 g = Internal::ParseHexString(string);
					string += 2;
					const uint8 b = Internal::ParseHexString(string);
					string += 2;
					const uint8 a = Internal::ParseHexCharacter(string[0]) * 16;
					return Convert<T>(r, g, b, a);
				}
				case 8:
				default:
				{
					const uint8 r = Internal::ParseHexString(string);
					string += 2;
					const uint8 g = Internal::ParseHexString(string);
					string += 2;
					const uint8 b = Internal::ParseHexString(string);
					string += 2;
					const uint8 a = Internal::ParseHexString(string);
					return Convert<T>(r, g, b, a);
				}
			}
		}
		[[nodiscard]] static Optional<TColor> TryParse(const char* string, uint32 length)
		{
			return Parse(string, length);
		}

		[[nodiscard]] FORCE_INLINE constexpr bool operator==(const TColor other) const noexcept
		{
			return (m_vector == other.m_vector).AreAllSet();
		}
		[[nodiscard]] FORCE_INLINE constexpr bool operator!=(const TColor other) const noexcept
		{
			return !operator==(other);
		}

		[[nodiscard]] FORCE_INLINE constexpr TColor operator+(const TColor other) const noexcept
		{
			return TColor(m_vector + other.m_vector);
		}

		constexpr FORCE_INLINE TColor& operator+=(const TColor other) noexcept
		{
			m_vector += other.m_vector;
			return *this;
		}

		[[nodiscard]] FORCE_INLINE constexpr TColor operator-(const TColor other) const noexcept
		{
			return TColor(m_vector - other.m_vector);
		}

		constexpr FORCE_INLINE TColor& operator-=(const TColor other) noexcept
		{
			m_vector -= other.m_vector;
			return *this;
		}

		[[nodiscard]] FORCE_INLINE constexpr TColor operator*(const T scalar) const noexcept
		{
			return TColor(m_vector * scalar);
		}

		constexpr FORCE_INLINE TColor& operator*=(const T scalar) noexcept
		{
			m_vector *= scalar;
			return *this;
		}

		[[nodiscard]] FORCE_INLINE constexpr TColor operator*(const TColor other) const noexcept
		{
			return TColor(m_vector * other.m_vector);
		}

		constexpr FORCE_INLINE TColor& operator*=(const TColor other) noexcept
		{
			m_vector *= other.m_vector;
			return *this;
		}

		[[nodiscard]] FORCE_INLINE constexpr TColor operator/(const TColor other) const noexcept
		{
			return TColor(m_vector / other.m_vector);
		}

		constexpr FORCE_INLINE TColor& operator/=(const TColor other) noexcept
		{
			m_vector /= other.m_vector;
			return *this;
		}

		FlatString<10> ToString() const;
		[[nodiscard]] PackedColor32 ToPacked32() const
		{
			if constexpr (TypeTraits::IsFloatingPoint<T>)
			{
				const Vector4ui packed = (Vector4ui)(Math::Round(Math::Clamp(m_vector, Vector4Type{T(0)}, Vector4Type{T(1)}) * Vector4Type{T(255)})
				);
				return PackedColor32{packed.x, packed.y, packed.z, packed.w};
			}
			else
			{
				const Vector4ui packed = (Vector4ui)Math::Round(Math::Clamp(m_vector, Vector4Type{T(0)}, Vector4Type{T(255)}));
				return PackedColor32{packed.x, packed.y, packed.z, packed.w};
			}
		}
		[[nodiscard]] constexpr static TColor FromPacked32(const PackedColor32 packedColor)
		{
			const Vector4ui packedColors = Vector4ui{packedColor.m_value} >> Vector4ui{24u, 16u, 8u, 0u};
			if constexpr (TypeTraits::IsFloatingPoint<T>)
			{
				return TColor{Vector4Type(packedColors & Vector4ui{0xFF}) / Vector4Type{T(255)}};
			}
			else
			{
				return TColor{Vector4Type(packedColors & Vector4ui{0xFF})};
			}
		}

		//! Converts to HSV (hue saturation
		[[nodiscard]] Vector4Type GetHSV() const
		{
			const Vector4Type rgba{m_vector};
			const Vector4Type K{T(0), T(-1) / T(3), T(2) / T(3), T(-1)};
			const Vector4Type p =
				LinearInterpolate(Vector4Type{rgba.z, rgba.y, K.w, K.z}, Vector4Type{rgba.y, rgba.z, K.x, K.y}, Step(rgba.z, rgba.y));
			const Vector4Type q = LinearInterpolate(Vector4Type{p.x, p.y, p.w, rgba.x}, Vector4Type{rgba.x, p.y, p.z, p.x}, Step(p.x, rgba.x));

			T d = q.x - Min(q.w, q.y);
			T e = T(1.0e-10);
			return {static_cast<T>(Abs(q.z + (q.w - q.y) / (T(6) * d + e))), static_cast<T>(d / (q.x + e)), q.x, rgba.w};
		}
		[[nodiscard]] T GetHue() const
		{
			return GetHSV().x;
		}
		[[nodiscard]] T GetSaturation() const
		{
			return GetHSV().y;
		}
		[[nodiscard]] T GetBrightness() const
		{
			return GetHSV().z;
		}

		[[nodiscard]] static TColor FromHSV(const Vector4Type hsv)
		{
			const Vector4Type K{T(1), T(2) / T(3), T(1) / T(3), T(3)};
			const Vector3Type p = Abs(Fract(Vector3Type{hsv.x} + Vector3Type{K.x, K.y, K.z}) * T(6) - Vector3Type{K.w});
			const Vector3Type color{
				hsv.z * LinearInterpolate(Vector3Type{K.x}, Clamp(p - Vector3Type{K.x}, Vector3Type{T(0)}, Vector3Type{T(1)}), hsv.y)
			};
			return {color.x, color.y, color.z, hsv.w};
		}

		bool Serialize(const Serialization::Reader serializer);
		bool Serialize(Serialization::Writer serializer) const;

		union
		{
			struct
			{
				T r, g, b, a;
			};
			Vector4Type m_vector;
		};
	private:
		//! Specific constexpr friendly conversion
		template<typename OtherType>
		[[nodiscard]] FORCE_INLINE static constexpr TColor<OtherType> Convert(const uint8 r, const uint8 g, const uint8 b, const uint8 a)
		{
			if constexpr (TypeTraits::IsFloatingPoint<OtherType>)
			{
				return TColor<OtherType>{r / 255.f, g / 255.f, b / 255.f, a / 255.f};
			}
			else
			{
				return TColor<OtherType>{r, g, b, a};
			}
		}
	};

	using Color = TColor<float>;
	using ColorByte = TColor<uint8>;

	template<typename UnitType>
	inline PackedColor32::PackedColor32(const TColor<UnitType> color)
		: PackedColor32{color.ToPacked32()}
	{
	}

	namespace Literals
	{
		constexpr Math::ColorByte operator""_color(const char* szInput, size n) noexcept
		{
			return Math::ColorByte::Parse(szInput, (uint8)n);
		}

		constexpr Math::Color operator""_colorf(const char* szInput, size n) noexcept
		{
			return Math::Color::Parse(szInput, (uint8)n);
		}
	}

	using namespace Literals;
}

namespace ngine
{
	using namespace Math::Literals;
}
