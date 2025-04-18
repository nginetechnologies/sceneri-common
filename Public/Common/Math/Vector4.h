#pragma once

#include <Common/Math/ForwardDeclarations/Vector4.h>
#include <Common/Guid.h>
#include <Common/Math/CoreNumericTypes.h>
#include <Common/Math/Vectorization/Packed.h>
#include <Common/Math/Vectorization/Select.h>
#include <Common/Math/Vectorization/ISqrt.h>
#include <Common/Math/Select.h>
#include <Common/Math/Abs.h>
#include <Common/Math/Epsilon.h>
#include <Common/Math/Mod.h>
#include <Common/Math/MathAssert.h>
#include <Common/Platform/TrivialABI.h>
#include <Common/Serialization/CanRead.h>
#include <Common/Serialization/CanWrite.h>

namespace ngine::Math
{
	template<typename T>
	struct TBoolVector4;

	template<typename T>
	struct TRIVIAL_ABI alignas(alignof(Vectorization::Packed<T, 4>)) TVector4
	{
		inline static constexpr Guid TypeGuid = "c3150fe5-527d-44bf-8c32-ffacc15a8be2"_guid;
		using UnitType = T;

		using BoolType = TBoolVector4<T>;
		using VectorizedType = Vectorization::Packed<T, 4>;
		inline static constexpr bool IsVectorized = VectorizedType::IsVectorized;

		FORCE_INLINE constexpr TVector4()
		{
		}

		FORCE_INLINE constexpr TVector4(ZeroType) noexcept
			: x(0)
			, y(0)
			, z(0)
			, w(0)
		{
		}

		FORCE_INLINE constexpr TVector4(const VectorizedType vectorized) noexcept
			: m_vectorized(vectorized)
		{
		}

		explicit FORCE_INLINE constexpr TVector4(const T value) noexcept
			: x(value)
			, y(value)
			, z(value)
			, w(value)
		{
		}

		FORCE_INLINE constexpr TVector4(const T _x, const T _y, const T _z, const T _w) noexcept
			: x(_x)
			, y(_y)
			, z(_z)
			, w(_w)
		{
		}

		template<typename OtherType>
		FORCE_INLINE explicit constexpr TVector4(const TVector4<OtherType> otherVector) noexcept
			: x((T)otherVector.x)
			, y((T)otherVector.y)
			, z((T)otherVector.z)
			, w((T)otherVector.w)
		{
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr operator VectorizedType() const noexcept
		{
			return m_vectorized;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS explicit constexpr operator BoolType() const noexcept
		{
			if constexpr (IsVectorized)
			{
				return BoolType(m_vectorized);
			}
			else
			{
				return {x, y, z, w};
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr BoolType operator==(const TVector4 other) const noexcept
		{
			if constexpr (IsVectorized)
			{
				return BoolType(m_vectorized == other.m_vectorized);
			}
			else
			{
				return {T(x == other.x), T(y == other.y), T(z == other.z), T(w == other.w)};
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr BoolType operator!=(const TVector4 other) const noexcept
		{
			if constexpr (IsVectorized)
			{
				return BoolType(m_vectorized != other.m_vectorized);
			}
			else
			{
				return {T(x != other.x), T(y != other.y), T(z != other.z), T(w != other.w)};
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr BoolType operator>(const TVector4 other) const noexcept
		{
			if constexpr (IsVectorized)
			{
				return BoolType(m_vectorized > other.m_vectorized);
			}
			else
			{
				return {T(x > other.x), T(y > other.y), T(z > other.z), T(w > other.w)};
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr BoolType operator>=(const TVector4 other) const noexcept
		{
			if constexpr (IsVectorized)
			{
				return BoolType(m_vectorized >= other.m_vectorized);
			}
			else
			{
				return {T(x >= other.x), T(y >= other.y), T(z >= other.z), T(w >= other.w)};
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr BoolType operator<(const TVector4 other) const noexcept
		{
			if constexpr (IsVectorized)
			{
				return BoolType(m_vectorized < other.m_vectorized);
			}
			else
			{
				return {T(x < other.x), T(y < other.y), T(z < other.z), T(w < other.w)};
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr BoolType operator<=(const TVector4 other) const noexcept
		{
			if constexpr (IsVectorized)
			{
				return BoolType(m_vectorized <= other.m_vectorized);
			}
			else
			{
				return {T(x <= other.x), T(y <= other.y), T(z <= other.z), T(w <= other.w)};
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr BoolType operator&(const TVector4 other) const noexcept
		{
			if constexpr (IsVectorized)
			{
				return BoolType(m_vectorized & other.m_vectorized);
			}
			else
			{
				return {
					T((uint32)x & (uint32)other.x),
					T((uint32)y & (uint32)other.y),
					T((uint32)z & (uint32)other.z),
					T((uint32)w & (uint32)other.w)
				};
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr BoolType operator|(const TVector4 other) const noexcept
		{
			if constexpr (IsVectorized)
			{
				return BoolType(m_vectorized | other.m_vectorized);
			}
			else
			{
				return {
					T((uint32)x | (uint32)other.x),
					T((uint32)y | (uint32)other.y),
					T((uint32)z | (uint32)other.z),
					T((uint32)w | (uint32)other.w)
				};
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr BoolType operator^(const TVector4 other) const noexcept
		{
			if constexpr (IsVectorized)
			{
				return BoolType(m_vectorized ^ other.m_vectorized);
			}
			else
			{
				return {
					T((uint32)x ^ (uint32)other.x),
					T((uint32)y ^ (uint32)other.y),
					T((uint32)z ^ (uint32)other.z),
					T((uint32)w ^ (uint32)other.w)
				};
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr BoolType operator!() const noexcept
		{
			if constexpr (IsVectorized)
			{
				return BoolType(!m_vectorized);
			}
			else
			{
				PUSH_CLANG_WARNINGS
				DISABLE_CLANG_WARNING("-Wfloat-conversion")
				return {T(!x), T(!y), T(!z), T(!w)};
				POP_CLANG_WARNINGS
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector4 operator~() const noexcept
		{
			if constexpr (IsVectorized)
			{
				return TVector4(~m_vectorized);
			}
			else
			{
				return {~x, ~y, ~z, ~w};
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector4 operator>>(const TVector4 other) const noexcept
		{
			if constexpr (IsVectorized)
			{
				return m_vectorized >> other.m_vectorized;
			}
			else
			{
				return TVector4(x >> other.x, y >> other.y, z >> other.z, w >> other.w);
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector4 operator<<(const TVector4 other) const noexcept
		{
			if constexpr (IsVectorized)
			{
				return m_vectorized << other.m_vectorized;
			}
			else
			{
				return TVector4(x << other.x, y << other.y, z << other.z, w << other.w);
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector4 operator>>(const T scalar) const noexcept
		{
			return *this >> TVector4(scalar);
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector4 operator<<(const T scalar) const noexcept
		{
			return *this << TVector4(scalar);
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector4 operator-() const noexcept
		{
			if constexpr (IsVectorized)
			{
				return TVector4(-m_vectorized);
			}
			else
			{
				return {-x, -y, -z, -w};
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr VectorizedType& GetVectorized() noexcept
		{
			return m_vectorized;
		}
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr VectorizedType GetVectorized() const noexcept
		{
			return m_vectorized;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector4 operator+(const TVector4 other) const noexcept
		{
			if constexpr (IsVectorized)
			{
				return TVector4(m_vectorized + other.m_vectorized);
			}
			else
			{
				return {T(x + other.x), T(y + other.y), T(z + other.z), T(w + other.w)};
			}
		}

		constexpr FORCE_INLINE TVector4& operator+=(const TVector4 other) noexcept
		{
			*this = *this + other;
			return *this;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector4 operator-(const TVector4 other) const noexcept
		{
			if constexpr (IsVectorized)
			{
				return TVector4(m_vectorized - other.m_vectorized);
			}
			else
			{
				return {T(x - other.x), T(y - other.y), T(z - other.z), T(w - other.w)};
			}
		}

		constexpr FORCE_INLINE TVector4& operator-=(const TVector4 other) noexcept
		{
			*this = *this - other;
			return *this;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector4 operator*(const T scalar) const noexcept
		{
			return *this * TVector4(scalar);
		}

		constexpr FORCE_INLINE TVector4& operator*=(const T scalar) noexcept
		{
			*this = *this * scalar;
			return *this;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector4 operator*(const TVector4 other) const noexcept
		{
			if constexpr (IsVectorized)
			{
				return TVector4(m_vectorized * other.m_vectorized);
			}
			else
			{
				return {T(x * other.x), T(y * other.y), T(z * other.z), T(w * other.w)};
			}
		}

		constexpr FORCE_INLINE TVector4& operator*=(const TVector4 other) noexcept
		{
			*this = *this * other;
			return *this;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector4 operator/(const T scalar) const noexcept
		{
			return *this / TVector4(scalar);
		}

		constexpr FORCE_INLINE TVector4& operator/=(const T scalar) noexcept
		{
			*this = *this / scalar;
			return *this;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector4 operator/(const TVector4 other) const noexcept
		{
			if constexpr (IsVectorized)
			{
				return TVector4(m_vectorized / other.m_vectorized);
			}
			else
			{
				return {T(x / other.x), T(y / other.y), T(z / other.z), T(w / other.w)};
			}
		}

		constexpr FORCE_INLINE TVector4& operator/=(const TVector4 other) noexcept
		{
			*this = *this / other;
			return *this;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector4 operator%(const TVector4 other) const noexcept
		{
			/*if constexpr (IsVectorized)
			{
			  return TVector4(m_vectorized % other.m_vectorized);
			}
			else*/
			{
				return {Math::Mod(x, other.x), Math::Mod(y, other.y), Math::Mod(z, other.z), Math::Mod(w, other.w)};
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS bool
		IsEquivalentTo(const TVector4 other, const T epsilon = Math::NumericLimits<T>::Epsilon) const noexcept
		{
			const T epsilonSquared = epsilon * epsilon;
			return (*this - other).GetLengthSquared() <= Epsilon<T>(epsilonSquared);
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector4 GetSquared() const noexcept
		{
			return *this * *this;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr T GetSum() const noexcept
		{
			/*if constexpr(IsVectorized)
			{
			  return m_vectorized.GetHorizontalSum().GetSingle();
			}
			else*/
			{
				return x + y + z + w;
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr T GetLengthSquared() const noexcept
		{
			if constexpr (IsVectorized)
			{
				return m_vectorized.GetLengthSquared4Scalar();
			}
			else
			{
				return x * x + y * y + z * z + w * w;
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS T GetLength() const noexcept
		{
			return Math::Sqrt(GetLengthSquared());
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr T GetComponentLength() const noexcept
		{
			return x * y * z * w;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS T GetInverseLength() const noexcept
		{
			return Math::Isqrt(GetLengthSquared());
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS TVector4 GetNormalized() const noexcept
		{
			MathAssert(GetLengthSquared() > 0);
			if constexpr (IsVectorized)
			{
				return TVector4{m_vectorized.GetNormalized4()};
			}
			else
			{
				return operator/(GetLength());
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS TVector4 GetNormalizedSafe(const TVector4 fallbackValue = Math::Zero) const noexcept
		{
			if constexpr (IsVectorized)
			{
				const VectorizedType vectorizedThis = m_vectorized;
				const VectorizedType lengthSquared = vectorizedThis.GetLengthSquared4();
				const VectorizedType result = vectorizedThis / Math::Sqrt(lengthSquared);

				return TVector4(Math::Select(lengthSquared > VectorizedType(T(0.00000001)), result, fallbackValue.m_vectorized));
			}
			else
			{
				const T lengthSquared = GetLengthSquared();
				return Math::Select(lengthSquared > T(0.00000001), operator/(Math::Sqrt(lengthSquared)), fallbackValue);
			}
		}

		FORCE_INLINE void Normalize() noexcept
		{
			*this = GetNormalized();
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS bool IsNormalized(const T epsilon = (T)0.05) const noexcept
		{
			return Math::Abs((T)1 - GetLength()) <= epsilon;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS bool IsUnit() const noexcept
		{
			return Math::Abs(T(1) - GetLengthSquared()) <= 0.05f;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS bool IsZero() const noexcept
		{
			return GetLengthSquared() <= 0.05f;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr T Dot(const TVector4 other) const noexcept
		{
			if constexpr (IsVectorized)
			{
				return m_vectorized.Dot4Scalar(other.m_vectorized);
			}
			else
			{
				return x * other.x + y * other.y + z * other.z + w * other.w;
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr T& operator[](const uint8 index) noexcept
		{
			MathExpect(index < 4);
			return *(&x + index);
		}
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr T operator[](const uint8 index) const noexcept
		{
			MathExpect(index < 4);
			return *(&x + index);
		}

		template<typename S = T, typename E = EnableIf<Serialization::Internal::CanRead<S> && TypeTraits::IsConvertibleTo<double, S>, bool>>
		bool Serialize(const Serialization::Reader serializer);

		template<typename S = T, typename E = EnableIf<Serialization::Internal::CanWrite<S> && TypeTraits::IsConvertibleTo<S, double>, bool>>
		bool Serialize(Serialization::Writer serializer) const;

		union
		{
			VectorizedType m_vectorized;
			struct
			{
				T x, y, z, w;
			};
		};
	};

	template<typename T>
	struct TBoolVector4 : public TVector4<T>
	{
		using BaseType = TVector4<T>;
		using TVector4<T>::TVector4;
		explicit TBoolVector4(const TVector4<T> base)
			: BaseType(base)
		{
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS uint8 GetMask() const
		{
			if constexpr (BaseType::IsVectorized)
			{
				return BaseType::m_vectorized.GetMask() & 0xF;
			}
			else
			{
				uint8 mask = 0;
				mask |= 0x1 * (BaseType::x > 0);
				mask |= 0x2 * (BaseType::y > 0);
				mask |= 0x4 * (BaseType::z > 0);
				mask |= 0x8 * (BaseType::w > 0);
				return mask;
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS bool AreAllSet() const
		{
			return GetMask() == 0xf;
		}
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS bool AreAnySet() const
		{
			return GetMask() != 0;
		}
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS bool AreNoneSet() const
		{
			return GetMask() == 0;
		}
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS bool IsXSet() const
		{
			return (GetMask() & 0x1) == 0x1;
		}
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS bool IsYSet() const
		{
			return (GetMask() & 0x2) == 0x2;
		}
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS bool IsZSet() const
		{
			return (GetMask() & 0x4) == 0x4;
		}
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS bool IsWSet() const
		{
			return (GetMask() & 0x8) == 0x8;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS explicit operator bool() const noexcept
		{
			return AreAllSet();
		}
	};

	template<typename T>
	struct TRIVIAL_ABI UnalignedVector4
	{
		using UnitType = T;

		UnalignedVector4() = default;

		FORCE_INLINE constexpr UnalignedVector4(ZeroType) noexcept
			: x(0)
			, y(0)
			, z(0)
			, w(0)
		{
		}

		FORCE_INLINE constexpr UnalignedVector4(const T _x, const T _y, const T _z, const T _w) noexcept
			: x(_x)
			, y(_y)
			, z(_z)
			, w(_w)
		{
		}
		FORCE_INLINE constexpr UnalignedVector4(const TVector4<T> vector) noexcept
			: x(vector.x)
			, y(vector.y)
			, z(vector.z)
			, w(vector.w)
		{
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr operator TVector4<T>() const noexcept
		{
			return {x, y, z, w};
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr T& operator[](const uint8 index) noexcept
		{
			MathExpect(index < 4);
			return *(&x + index);
		}
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr T operator[](const uint8 index) const noexcept
		{
			MathExpect(index < 4);
			return *(&x + index);
		}

		struct
		{
			T x, y, z, w;
		};
	};
	using Vector4d = TVector4<double>;
	using Vector4f = TVector4<float>;
	using Vector4i = TVector4<int>;
	using Vector4ui = TVector4<unsigned int>;
}
