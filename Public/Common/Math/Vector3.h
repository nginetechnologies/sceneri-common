#pragma once

#include "ForwardDeclarations/Vector3.h"

#include <Common/Guid.h>
#include <Common/Math/Sqrt.h>
#include <Common/Math/Vectorization/Packed.h>
#include <Common/Math/Vectorization/Select.h>
#include <Common/Math/Abs.h>
#include <Common/Math/Mod.h>
#include <Common/Math/MathAssert.h>
#include <Common/Math/Vectorization/ISqrt.h>
#include <Common/Math/Epsilon.h>
#include <Common/Platform/TrivialABI.h>
#include <Common/Platform/Assume.h>

#include <Common/Serialization/CanRead.h>
#include <Common/Serialization/CanWrite.h>
#include <Common/TypeTraits/IsConvertibleTo.h>
#include <Common/TypeTraits/IsIntegral.h>
#include <Common/TypeTraits/IsFloatingPoint.h>

#include <Common/Memory/Containers/FixedArrayView.h>
#include <Common/Memory/Containers/ForwardDeclarations/BitView.h>

namespace ngine::Math
{
	template<typename T>
	struct TBoolVector3;

	template<typename T>
	struct [[nodiscard]] TRIVIAL_ABI alignas(alignof(Vectorization::Packed<T, 4>)) TVector3
	{
		inline static constexpr Guid TypeGuid = "6a4e032e-1c26-4cb9-beb3-5e5964e514f3"_guid;

		using UnitType = T;

		using VectorizedType = Vectorization::Packed<T, 4>;
		inline static constexpr bool IsVectorized = VectorizedType::IsVectorized;

		using BoolType = TBoolVector3<T>;
		using ViewType = FixedArrayView<T, 3>;
		using ConstViewType = FixedArrayView<const T, 3>;

		FORCE_INLINE TVector3() noexcept
		{
		}

		FORCE_INLINE constexpr TVector3(ZeroType) noexcept
			: x(0)
			, y(0)
			, z(0)
		{
		}
		FORCE_INLINE constexpr TVector3(UpType) noexcept
			: x(0)
			, y(0)
			, z((T)1)
		{
		}
		FORCE_INLINE constexpr TVector3(DownType) noexcept
			: x(0)
			, y(0)
			, z((T)-1)
		{
		}
		FORCE_INLINE constexpr TVector3(ForwardType) noexcept
			: x(0)
			, y((T)1)
			, z(0)
		{
		}
		FORCE_INLINE constexpr TVector3(BackwardType) noexcept
			: x(0)
			, y((T)-1)
			, z(0)
		{
		}
		FORCE_INLINE constexpr TVector3(RightType) noexcept
			: x((T)1)
			, y(0)
			, z(0)
		{
		}
		FORCE_INLINE constexpr TVector3(LeftType) noexcept
			: x((T)-1)
			, y(0)
			, z(0)
		{
		}

		explicit FORCE_INLINE constexpr TVector3(const T value) noexcept
			: x(value)
			, y(value)
			, z(value)
		{
		}

		FORCE_INLINE constexpr TVector3(const T _x, const T _y, const T _z) noexcept
			: x(_x)
			, y(_y)
			, z(_z)
		{
		}

		FORCE_INLINE explicit constexpr TVector3(const VectorizedType vectorized) noexcept
			: m_vectorized(vectorized)
		{
		}

		template<typename OtherType>
		FORCE_INLINE explicit constexpr TVector3(const TVector3<OtherType> otherVector) noexcept
			: x((T)otherVector.x)
			, y((T)otherVector.y)
			, z((T)otherVector.z)
		{
		}

		FORCE_INLINE constexpr TVector3(const ConstViewType view) noexcept
			: x(view[0])
			, y(view[1])
			, z(view[2])
		{
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr operator ConstViewType() const
		{
			return ConstViewType(m_data);
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr operator ViewType()
		{
			return ViewType(m_data);
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr VectorizedType& GetVectorized() noexcept
		{
			return m_vectorized;
		}
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr VectorizedType GetVectorized() const noexcept
		{
			return m_vectorized;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector3 operator-() const noexcept
		{
			if constexpr (IsVectorized)
			{
				return TVector3(-m_vectorized);
			}
			else
			{
				return {-x, -y, -z};
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector3 operator+(const TVector3 other) const noexcept
		{
			if constexpr (IsVectorized)
			{
				return TVector3(m_vectorized + other.m_vectorized);
			}
			else
			{
				return {static_cast<T>(x + other.x), static_cast<T>(y + other.y), static_cast<T>(z + other.z)};
			}
		}

		constexpr FORCE_INLINE TVector3& operator+=(const TVector3 other) noexcept
		{
			*this = *this + other;
			return *this;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector3 operator-(const TVector3 other) const noexcept
		{
			if constexpr (IsVectorized)
			{
				return TVector3(m_vectorized - other.m_vectorized);
			}
			else
			{
				return {static_cast<T>(x - other.x), static_cast<T>(y - other.y), static_cast<T>(z - other.z)};
			}
		}

		constexpr FORCE_INLINE TVector3& operator-=(const TVector3 other) noexcept
		{
			*this = *this - other;
			return *this;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector3 operator*(const T scalar) const noexcept
		{
			return *this * TVector3(scalar);
		}

		constexpr FORCE_INLINE TVector3& operator*=(const T scalar) noexcept
		{
			*this = *this * scalar;
			return *this;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector3 operator*(const TVector3 other) const noexcept
		{
			if constexpr (IsVectorized)
			{
				return TVector3(m_vectorized * other.m_vectorized);
			}
			else
			{
				return {static_cast<T>(x * other.x), static_cast<T>(y * other.y), static_cast<T>(z * other.z)};
			}
		}

		constexpr FORCE_INLINE TVector3& operator*=(const TVector3 other) noexcept
		{
			*this = *this * other;
			return *this;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector3 operator/(const T scalar) const noexcept
		{
			return *this / TVector3(scalar);
		}

		constexpr FORCE_INLINE TVector3& operator/=(const T scalar) noexcept
		{
			*this = *this / scalar;
			return *this;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector3 operator/(const TVector3 other) const noexcept
		{
			if constexpr (IsVectorized)
			{
				return TVector3(m_vectorized / VectorizedType(other.x, other.y, other.z, 1));
			}
			else
			{
				return {x / other.x, y / other.y, z / other.z};
			}
		}

		constexpr FORCE_INLINE TVector3& operator/=(const TVector3 other) noexcept
		{
			*this = *this / other;
			return *this;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector3 operator%(const TVector3 other) const noexcept
		{
			/*if constexpr (IsVectorized)
			{
			  return TVector3(m_vectorized % other.m_vectorized);
			}
			else*/
			{
				return {Math::Mod(x, other.x), Math::Mod(y, other.y), Math::Mod(z, other.z)};
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS explicit constexpr operator BoolType() const noexcept
		{
			if constexpr (IsVectorized)
			{
				return BoolType(m_vectorized);
			}
			else
			{
				return {x, y, z};
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr BoolType operator==(const TVector3 other) const noexcept
		{
			if constexpr (IsVectorized)
			{
				return BoolType(m_vectorized == other.m_vectorized);
			}
			else
			{
				return {T(x == other.x), T(y == other.y), T(z == other.z)};
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr BoolType operator!=(const TVector3 other) const noexcept
		{
			if constexpr (IsVectorized)
			{
				return BoolType(m_vectorized != other.m_vectorized);
			}
			else
			{
				return {T(x != other.x), T(y != other.y), T(z != other.z)};
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr BoolType operator>(const TVector3 other) const noexcept
		{
			if constexpr (IsVectorized)
			{
				return BoolType(m_vectorized > other.m_vectorized);
			}
			else
			{
				return {T(x > other.x), T(y > other.y), T(z > other.z)};
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr BoolType operator>=(const TVector3 other) const noexcept
		{
			if constexpr (IsVectorized)
			{
				return BoolType(m_vectorized >= other.m_vectorized);
			}
			else
			{
				return {T(x >= other.x), T(y >= other.y), T(z >= other.z)};
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr BoolType operator<(const TVector3 other) const noexcept
		{
			if constexpr (IsVectorized)
			{
				return BoolType(m_vectorized < other.m_vectorized);
			}
			else
			{
				return {T(x < other.x), T(y < other.y), T(z < other.z)};
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr BoolType operator<=(const TVector3 other) const noexcept
		{
			if constexpr (IsVectorized)
			{
				return BoolType(m_vectorized <= other.m_vectorized);
			}
			else
			{
				return {T(x <= other.x), T(y <= other.y), T(z <= other.z)};
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr BoolType operator&(const TVector3 other) const noexcept
		{
			if constexpr (IsVectorized)
			{
				return BoolType(m_vectorized & other.m_vectorized);
			}
			else
			{
				return {T((uint32)x & (uint32)other.x), T((uint32)y & (uint32)other.y), T((uint32)z & (uint32)other.z)};
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr BoolType operator|(const TVector3 other) const noexcept
		{
			if constexpr (IsVectorized)
			{
				return BoolType(m_vectorized | other.m_vectorized);
			}
			else
			{
				return {T((uint32)x | (uint32)other.x), T((uint32)y | (uint32)other.y), T((uint32)z | (uint32)other.z)};
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr BoolType operator^(const TVector3 other) const noexcept
		{
			if constexpr (IsVectorized)
			{
				return BoolType(m_vectorized ^ other.m_vectorized);
			}
			else
			{
				return {T((uint32)x ^ (uint32)other.x), T((uint32)y ^ (uint32)other.y), T((uint32)z ^ (uint32)other.z)};
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
				return {T(!x), T(!y), T(!z)};
				POP_CLANG_WARNINGS
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector3 operator~() const noexcept
		{
			if constexpr (IsVectorized)
			{
				return TVector3(~m_vectorized);
			}
			else
			{
				return {~x, ~y, ~z};
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector3 operator>>(const TVector3 other) const noexcept
		{
			if constexpr (IsVectorized)
			{
				return TVector3(m_vectorized >> other.m_vectorized);
			}
			else
			{
				return TVector3(x >> other.x, y >> other.y, z >> other.z);
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector3 operator<<(const TVector3 other) const noexcept
		{
			if constexpr (IsVectorized)
			{
				return TVector3(m_vectorized << other.m_vectorized);
			}
			else
			{
				return TVector3(x << other.x, y << other.y, z << other.z);
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector3 operator>>(const T scalar) const noexcept
		{
			return *this >> TVector3(scalar);
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector3 operator<<(const T scalar) const noexcept
		{
			return *this << TVector3(scalar);
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS bool
		IsEquivalentTo(const TVector3 other, const T epsilon = Math::NumericLimits<T>::Epsilon) const noexcept
		{
			const T epsilonSquared = epsilon * epsilon;
			return (*this - other).GetLengthSquared() <= Epsilon<T>(epsilonSquared);
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector3 GetSquared() const noexcept
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
				return x + y + z;
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr T GetLengthSquared() const noexcept
		{
			if constexpr (IsVectorized)
			{
				return m_vectorized.GetLengthSquared3Scalar();
			}
			else
			{
				return x * x + y * y + z * z;
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr T GetLength2DSquared() const noexcept
		{
			return x * x + y * y;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr T GetComponentLength() const noexcept
		{
			return x * y * z;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS T GetLength() const noexcept
		{
			return Math::Sqrt(GetLengthSquared());
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS T GetLength2D() const noexcept
		{
			return Math::Sqrt(GetLength2DSquared());
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS T GetInverseLength() const noexcept
		{
			return Math::Isqrt(Math::Max(GetLengthSquared(), Math::NumericLimits<T>::MinPositive));
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS T GetInverseLength2D() const noexcept
		{
			return Math::Isqrt(Math::Max(GetLength2DSquared(), Math::NumericLimits<T>::MinPositive));
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS TVector3 GetNormalized() const noexcept
		{
			MathAssert(GetLengthSquared() > 0);
			if constexpr (IsVectorized)
			{
				return TVector3{m_vectorized.GetNormalized3()};
			}
			else
			{
				return operator/(GetLength());
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS TVector3 GetNormalizedSafe(const TVector3 fallbackValue = Math::Zero) const noexcept
		{
			if constexpr (IsVectorized)
			{
				const VectorizedType vectorizedThis = m_vectorized;
				const VectorizedType lengthSquared = vectorizedThis.GetLengthSquared3();
				const VectorizedType result = vectorizedThis / Math::Sqrt(lengthSquared);

				return TVector3(Math::Select(lengthSquared > VectorizedType(T(0.00000001)), result, fallbackValue.m_vectorized));
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

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS bool IsNormalized(const Epsilon<T> epsilon = (T)0.01) const noexcept
		{
			return Math::Abs((T)1 - GetLengthSquared()) <= epsilon;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS bool IsUnit() const noexcept
		{
			return Math::Abs(T(1) - GetLengthSquared()) <= Epsilon<T>(0.01f);
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS TVector3 Get2D() const noexcept
		{
			return {x, y, T(0)};
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS bool IsZeroExact() const noexcept
		{
			return (x == T(0)) & (y == T(0)) & (z == T(0));
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS bool
		IsZero(const Epsilon<T> epsilon = Math::NumericLimits<T>::Epsilon) const noexcept
		{
			return GetLengthSquared() <= epsilon;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS bool IsUniform() const noexcept
		{
			return IsEquivalentTo(TVector3{x});
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS bool ContainsNaN() const noexcept
		{
			return Math::Select(*this == *this, TVector3{1, 1, 1}, Math::Zero).GetComponentLength() == 0;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr T Dot(const TVector3 other) const noexcept
		{
			if constexpr (IsVectorized)
			{
				return m_vectorized.Dot3Scalar(other.m_vectorized);
			}
			else
			{
				return x * other.x + y * other.y + z * other.z;
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector3 Cross(const TVector3 other) const noexcept
		{
			const TVector3 a = yzx() * other.zxy();
			const TVector3 b = zxy() * other.yzx();
			return a - b;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector3 Project(const TVector3 normal) const noexcept
		{
			return *this - normal * normal.Dot(*this);
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector3 Reflect(const TVector3 normal) const noexcept
		{
			return (normal * Dot(normal) * T(2)) - *this;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector3 Refract(const TVector3 normal, const float eta) const noexcept
		{
			const T cosI = Dot(normal);
			const T k = T(1) - eta * eta * (T(1) - cosI * cosI);
			if (k < 0.0f)
			{
				return Math::Zero;
			}
			return *this * eta - normal * (eta * cosI + Math::Sqrt(k));
		}

		template<uint8 X, uint8 Y, uint8 Z>
		[[nodiscard]] FORCE_INLINE TVector3 Swizzle() const
		{
			if constexpr (IsVectorized)
			{
				return TVector3{m_vectorized.template Swizzle<X, Y, Z, 0>()};
			}
			else
			{
				return {operator[](X), operator[](Y), operator[](Z)};
			}
		}

		[[nodiscard]] FORCE_INLINE TVector3 xyx() const
		{
			return Swizzle<0, 1, 0>();
		}
		[[nodiscard]] FORCE_INLINE TVector3 yxx() const
		{
			return Swizzle<1, 0, 0>();
		}
		[[nodiscard]] FORCE_INLINE TVector3 yzx() const
		{
			return Swizzle<1, 2, 0>();
		}
		[[nodiscard]] FORCE_INLINE TVector3 yzz() const
		{
			return Swizzle<1, 2, 2>();
		}
		[[nodiscard]] FORCE_INLINE TVector3 zxy() const
		{
			return Swizzle<2, 0, 1>();
		}
		[[nodiscard]] FORCE_INLINE TVector3 zzy() const
		{
			return Swizzle<2, 2, 1>();
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr T& operator[](const uint8 index) noexcept
		{
			ASSUME(index < 3);
			return *(&x + index);
		}
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr T operator[](const uint8 index) const noexcept
		{
			ASSUME(index < 3);
			return *(&x + index);
		}

		template<typename S = T, typename E = EnableIf<Serialization::Internal::CanRead<S> && TypeTraits::IsConvertibleTo<double, S>, bool>>
		bool Serialize(const Serialization::Reader serializer);

		template<typename S = T, typename E = EnableIf<Serialization::Internal::CanWrite<S> && TypeTraits::IsConvertibleTo<S, double>, bool>>
		bool Serialize(Serialization::Writer serializer) const;

		[[nodiscard]] static constexpr uint32 CalculateCompressedDataSize()
		{
			return sizeof(UnitType) * 8 * 3;
		}
		template<typename S = T, typename E = EnableIf<TypeTraits::IsFloatingPoint<S>, bool>>
		bool Compress(BitView& target) const;
		template<typename S = T, typename E = EnableIf<TypeTraits::IsFloatingPoint<S>, bool>>
		bool Decompress(ConstBitView& source);

		union
		{
			struct
			{
				T x, y, z;
			};
			T m_data[3];
			VectorizedType m_vectorized;
		};
	};

	template<typename T>
	struct TBoolVector3 : public TVector3<T>
	{
		using BaseType = TVector3<T>;
		using TVector3<T>::TVector3;

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS uint8 GetMask() const
		{
			if constexpr (BaseType::IsVectorized)
			{
				return BaseType::m_vectorized.GetMask() & 0x7;
			}
			else
			{
				uint8 mask = 0;
				mask |= 0x1 * (BaseType::x > 0);
				mask |= 0x2 * (BaseType::y > 0);
				mask |= 0x4 * (BaseType::z > 0);
				return mask;
			}
		}
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS bool AreAllSet() const
		{
			return GetMask() == 0x7;
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

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS explicit operator bool() const noexcept
		{
			return AreAllSet();
		}
	};

	template<typename T>
	struct TRIVIAL_ABI UnalignedVector3
	{
		using UnitType = T;

		UnalignedVector3() = default;

		FORCE_INLINE constexpr UnalignedVector3(ZeroType) noexcept
			: x(0)
			, y(0)
			, z(0)
		{
		}
		FORCE_INLINE constexpr UnalignedVector3(UpType) noexcept
			: x(0)
			, y(0)
			, z((T)1)
		{
		}
		FORCE_INLINE constexpr UnalignedVector3(DownType) noexcept
			: x(0)
			, y(0)
			, z((T)-1)
		{
		}
		FORCE_INLINE constexpr UnalignedVector3(ForwardType) noexcept
			: x(0)
			, y((T)1)
			, z(0)
		{
		}
		FORCE_INLINE constexpr UnalignedVector3(BackwardType) noexcept
			: x(0)
			, y((T)-1)
			, z(0)
		{
		}
		FORCE_INLINE constexpr UnalignedVector3(RightType) noexcept
			: x((T)1)
			, y(0)
			, z(0)
		{
		}
		FORCE_INLINE constexpr UnalignedVector3(LeftType) noexcept
			: x((T)-1)
			, y(0)
			, z(0)
		{
		}

		FORCE_INLINE constexpr UnalignedVector3(const T _x, const T _y, const T _z) noexcept
			: x(_x)
			, y(_y)
			, z(_z)
		{
		}
		FORCE_INLINE constexpr UnalignedVector3(const TVector3<T> vector) noexcept
			: x(vector.x)
			, y(vector.y)
			, z(vector.z)
		{
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr operator TVector3<T>() const noexcept
		{
			return {x, y, z};
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr T& operator[](const uint8 index) noexcept
		{
			MathExpect(index < 3);
			return *(&x + index);
		}
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr T operator[](const uint8 index) const noexcept
		{
			MathExpect(index < 3);
			return *(&x + index);
		}

		struct
		{
			T x, y, z;
		};
	};

	using Vector3r = TVector3<double>;
	using Vector3f = TVector3<float>;
	using Vector3i = TVector3<int>;
	using Vector3ui = TVector3<unsigned int>;

	template<typename T>
	[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector3<T> operator*(const T scalar, const TVector3<T> vector) noexcept
	{
		return vector * scalar;
	}

	// Cannot remove this without MSVC compiler bug popping up and causing compiler errors
	namespace Internal::MSVCBugFix
	{
		inline static constexpr Vector3f UpVector = Vector3f{0, 0, 1.f};
		inline static constexpr Vector3d UpVectord = Vector3d{0, 0, 1.f};
	}
}
