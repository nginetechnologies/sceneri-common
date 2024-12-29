#pragma once

#include <Common/Math/ForwardDeclarations/Vector2.h>
#include <Common/Guid.h>
#include <Common/Math/Sqrt.h>
#include <Common/Math/ISqrt.h>
#include <Common/Math/Abs.h>
#include <Common/Math/Epsilon.h>
#include <Common/Math/Mod.h>
#include <Common/Math/MathAssert.h>
#include <Common/Math/Vectorization/Select.h>
#include <Common/Math/Vectorization/Packed.h>
#include <Common/Platform/ForceInline.h>
#include <Common/Platform/TrivialABI.h>
#include <Common/Memory/Containers/ForwardDeclarations/BitView.h>
#include <Common/Math/MathAssert.h>

#include <Common/Serialization/CanRead.h>
#include <Common/Serialization/CanWrite.h>
#include <Common/TypeTraits/IsConvertibleTo.h>
#include <Common/TypeTraits/IsIntegral.h>
#include <Common/TypeTraits/IsFloatingPoint.h>

namespace ngine::Math
{
	template<typename T>
	struct TBoolVector2;

	template<typename T>
	struct [[nodiscard]] TRIVIAL_ABI alignas(alignof(Vectorization::Packed<T, 2>)) TVector2
	{
		using UnitType = T;

		using BoolType = TBoolVector2<T>;
		using VectorizedType = Vectorization::Packed<T, 2>;
		inline static constexpr bool IsVectorized = VectorizedType::IsVectorized;

		static constexpr Guid TypeGuid = "D8A11E69-3EC6-408A-B8AD-49D7C841889C"_guid;

		FORCE_INLINE TVector2() noexcept
		{
		}

		FORCE_INLINE constexpr TVector2(ZeroType) noexcept
			: x(0)
			, y(0)
		{
		}
		FORCE_INLINE constexpr TVector2(UpType) noexcept
			: x(0)
			, y((T)1)
		{
		}
		FORCE_INLINE constexpr TVector2(DownType) noexcept
			: x(0)
			, y((T)-1)
		{
		}
		FORCE_INLINE constexpr TVector2(ForwardType) noexcept
			: x(0)
			, y((T)1)
		{
		}
		FORCE_INLINE constexpr TVector2(BackwardType) noexcept
			: x(0)
			, y((T)-1)
		{
		}
		FORCE_INLINE constexpr TVector2(RightType) noexcept
			: x((T)1)
			, y(0)
		{
		}
		FORCE_INLINE constexpr TVector2(LeftType) noexcept
			: x((T)-1)
			, y(0)
		{
		}

		FORCE_INLINE constexpr TVector2(const T _x, const T _y) noexcept
			: x(_x)
			, y(_y)
		{
		}

		FORCE_INLINE explicit constexpr TVector2(const T scalar) noexcept
			: x(scalar)
			, y(scalar)
		{
		}

		FORCE_INLINE explicit constexpr TVector2(const VectorizedType vectorized) noexcept
			: x(vectorized[0])
			, y(vectorized[1])
		{
		}

		template<typename OtherType>
		FORCE_INLINE explicit constexpr TVector2(const TVector2<OtherType> other) noexcept
			: x((T)other.x)
			, y((T)other.y)
		{
			/*MathAssert(other.x >= Math::NumericLimits<T>::Min);
			MathAssert(other.x <= Math::NumericLimits<T>::Max);
			MathAssert(other.y >= Math::NumericLimits<T>::Min);
			MathAssert(other.y <= Math::NumericLimits<T>::Max);*/
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS explicit constexpr operator BoolType() const noexcept
		{
			if constexpr (IsVectorized)
			{
				return BoolType(m_vectorized);
			}
			else
			{
				return {x, y};
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS BoolType operator==(const TVector2 other) const noexcept
		{
			if constexpr (IsVectorized)
			{
				return BoolType(m_vectorized == other.m_vectorized);
			}
			else
			{
				return {T(x == other.x), T(y == other.y)};
			}
		}
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS BoolType operator!=(const TVector2 other) const noexcept
		{
			if constexpr (IsVectorized)
			{
				return BoolType(m_vectorized != other.m_vectorized);
			}
			else
			{
				return {T(x != other.x), T(y != other.y)};
			}
		}

		template<typename OtherUnitType>
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS BoolType operator==(const TVector2<OtherUnitType> other) const
		{
			return *this == TVector2(other);
		}

		template<typename OtherUnitType>
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS BoolType operator!=(const TVector2<OtherUnitType> other) const
		{
			return *this != TVector2(other);
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr BoolType operator>(const TVector2 other) const noexcept
		{
			if constexpr (IsVectorized)
			{
				return BoolType(m_vectorized > other.m_vectorized);
			}
			else
			{
				return {T(x > other.x), T(y > other.y)};
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr BoolType operator>=(const TVector2 other) const noexcept
		{
			if constexpr (IsVectorized)
			{
				return BoolType(m_vectorized >= other.m_vectorized);
			}
			else
			{
				return {T(x >= other.x), T(y >= other.y)};
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr BoolType operator<(const TVector2 other) const noexcept
		{
			if constexpr (IsVectorized)
			{
				return BoolType(m_vectorized < other.m_vectorized);
			}
			else
			{
				return {T(x < other.x), T(y < other.y)};
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr BoolType operator<=(const TVector2 other) const noexcept
		{
			if constexpr (IsVectorized)
			{
				return BoolType(m_vectorized <= other.m_vectorized);
			}
			else
			{
				return {T(x <= other.x), T(y <= other.y)};
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr BoolType operator&(const TVector2 other) const noexcept
		{
			if constexpr (IsVectorized)
			{
				return BoolType(m_vectorized & other.m_vectorized);
			}
			else
			{
				return {T((uint32)x & (uint32)other.x), T((uint32)y & (uint32)other.y)};
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr BoolType operator|(const TVector2 other) const noexcept
		{
			if constexpr (IsVectorized)
			{
				return BoolType(m_vectorized | other.m_vectorized);
			}
			else
			{
				return {T((uint32)x | (uint32)other.x), T((uint32)y | (uint32)other.y)};
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr BoolType operator^(const TVector2 other) const noexcept
		{
			if constexpr (IsVectorized)
			{
				return BoolType(m_vectorized ^ other.m_vectorized);
			}
			else
			{
				return {T((uint32)x ^ (uint32)other.x), T((uint32)y ^ (uint32)other.y)};
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
				return {T(!x), T(!y)};
				POP_CLANG_WARNINGS
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector2 operator~() const noexcept
		{
			if constexpr (IsVectorized)
			{
				return TVector2(~m_vectorized);
			}
			else
			{
				return {~x, ~y};
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector2 operator>>(const TVector2 other) const noexcept
		{
			if constexpr (IsVectorized)
			{
				return m_vectorized >> other.m_vectorized;
			}
			else
			{
				return TVector2(x >> other.x, y >> other.y);
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector2 operator<<(const TVector2 other) const noexcept
		{
			if constexpr (IsVectorized)
			{
				return m_vectorized << other.m_vectorized;
			}
			else
			{
				return TVector2(x << other.x, y << other.y);
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector2 operator>>(const T scalar) const noexcept
		{
			return *this >> TVector2(scalar);
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector2 operator<<(const T scalar) const noexcept
		{
			return *this << TVector2(scalar);
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS bool IsEquivalentTo(const TVector2 other, const T epsilon = T(0.005)) const noexcept
		{
			const T epsilonSquared = epsilon * epsilon;
			return (*this - other).GetLengthSquared() <= Epsilon<T>(epsilonSquared);
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr VectorizedType GetVectorized() const noexcept
		{
			return m_vectorized;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector2 operator+(const TVector2 other) const noexcept
		{
			if constexpr (IsVectorized)
			{
				return TVector2(m_vectorized + other.m_vectorized);
			}
			else
			{
				return TVector2(x + other.x, y + other.y);
			}
		}

		constexpr FORCE_INLINE TVector2& operator+=(const TVector2 other) noexcept
		{
			*this = *this + other;
			return *this;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector2 operator-(const TVector2 other) const noexcept
		{
			if constexpr (IsVectorized)
			{
				return TVector2(m_vectorized - other.m_vectorized);
			}
			else
			{
				return TVector2(x - other.x, y - other.y);
			}
		}

		constexpr FORCE_INLINE TVector2& operator-=(const TVector2 other) noexcept
		{
			*this = *this - other;
			return *this;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector2 operator*(const T scalar) const noexcept
		{
			return *this * TVector2(scalar);
		}

		constexpr FORCE_INLINE TVector2& operator*=(const T scalar) noexcept
		{
			*this = *this * scalar;
			return *this;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector2 operator*(const TVector2 other) const noexcept
		{
			if constexpr (IsVectorized)
			{
				return TVector2(m_vectorized * other.m_vectorized);
			}
			else
			{
				return TVector2(x * other.x, y * other.y);
			}
		}

		constexpr FORCE_INLINE TVector2& operator*=(const TVector2 other) noexcept
		{
			*this = *this * other;
			return *this;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector2 operator/(const T scalar) const noexcept
		{
			return *this / TVector2(scalar);
		}

		constexpr FORCE_INLINE TVector2& operator/=(const T scalar) noexcept
		{
			*this = *this / scalar;
			return *this;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector2 operator/(const TVector2 other) const noexcept
		{
			if constexpr (IsVectorized)
			{
				return TVector2(m_vectorized / other.m_vectorized);
			}
			else
			{
				return TVector2(x / other.x, y / other.y);
			}
		}

		constexpr FORCE_INLINE TVector2& operator/=(const TVector2 other) noexcept
		{
			*this = *this / other;
			return *this;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector2 operator%(const TVector2 other) const noexcept
		{
			/*if constexpr (IsVectorized)
			{
			  return TVector2(m_vectorized % other.m_vectorized);
			}
			else*/
			{
				return {Math::Mod(x, other.x), Math::Mod(y, other.y)};
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr T GetLengthSquared() const noexcept
		{
			if constexpr (IsVectorized)
			{
				return m_vectorized.GetLengthSquared2Scalar();
			}
			else
			{
				return x * x + y * y;
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS T GetLength() const noexcept
		{
			return Math::Sqrt(GetLengthSquared());
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS T GetInverseLength() const noexcept
		{
			return Math::Isqrt(GetLengthSquared());
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr T GetComponentLength() const noexcept
		{
			return x * y;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS TVector2 GetNormalized() const noexcept
		{
			MathAssert(GetLengthSquared() > 0);
			return operator/(GetLength());
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS TVector2 GetNormalizedSafe(const TVector2 fallbackValue = Math::Zero) const noexcept
		{
			if constexpr (IsVectorized)
			{
				const VectorizedType vectorizedThis = m_vectorized;
				const VectorizedType lengthSquared = vectorizedThis.GetLengthSquared2();
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

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS bool IsNormalized(const T epsilon = (T)0.05) const noexcept
		{
			return Math::Abs((T)1 - GetLength()) <= epsilon;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS bool IsUnit() const noexcept
		{
			return Math::Abs(T(1) - GetLengthSquared()) <= 0.05f;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS bool IsZero(const T epsilon = 0) const noexcept
		{
			return GetLengthSquared() <= epsilon;
		}
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS bool IsZeroExact() const noexcept
		{
			return (x == T(0)) & (y == T(0));
		}

		[[nodiscard]] FORCE_INLINE constexpr T GetSum() const noexcept
		{
			/*if constexpr(IsVectorized)
			{
			  return m_vectorized.GetHorizontalSum().GetSingle();
			}
			else*/
			{
				return x + y;
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr T Dot(const TVector2 other) const noexcept
		{
			if constexpr (IsVectorized)
			{
				return TVector2(m_vectorized.Dot(other.m_vectorized).GetSingle());
			}
			else
			{
				return x * other.x + y * other.y;
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr UnitType Cross(const TVector2 other) const noexcept
		{

			return x * other.y - y * other.x;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector2 Project(const TVector2 normal) const noexcept
		{
			return *this - normal * normal.Dot(*this);
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector2 Reflect(const TVector2 normal) const noexcept
		{
			return (normal * Dot(normal) * T(2)) - *this;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector2 Refract(const TVector2 normal, const float eta) const noexcept
		{
			const T cosI = Dot(normal);
			const T k = T(1) - eta * eta * (T(1) - cosI * cosI);
			if (k < 0.0f)
			{
				return Math::Zero;
			}
			return *this * eta - normal * (eta * cosI + Math::Sqrt(k));
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector2 GetPerpendicularClockwise()
		{
			return {y, -x};
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector2 GetPerpendicularCounterClockwise()
		{
			return {-y, x};
		}

		template<uint8 X, uint8 Y>
		[[nodiscard]] FORCE_INLINE TVector2 Swizzle() const
		{
			if constexpr (IsVectorized)
			{
				return TVector2{m_vectorized.template Swizzle<X, Y>()};
			}
			else
			{
				return {operator[](X), operator[](Y)};
			}
		}

		[[nodiscard]] FORCE_INLINE TVector2 xy() const
		{
			return Swizzle<0, 1>();
		}
		[[nodiscard]] FORCE_INLINE TVector2 xx() const
		{
			return Swizzle<0, 0>();
		}
		[[nodiscard]] FORCE_INLINE TVector2 yx() const
		{
			return Swizzle<1, 0>();
		}
		[[nodiscard]] FORCE_INLINE TVector2 yy() const
		{
			return Swizzle<1, 1>();
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr T& operator[](const uint8 index) noexcept
		{
			MathExpect(index < 2);
			return *(&x + index);
		}
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr T operator[](const uint8 index) const noexcept
		{
			MathExpect(index < 2);
			return *(&x + index);
		}

		template<typename S = T, typename E = EnableIf<Serialization::Internal::CanRead<S> && TypeTraits::IsConvertibleTo<double, S>, bool>>
		bool Serialize(const Serialization::Reader serializer);

		template<typename S = T, typename E = EnableIf<Serialization::Internal::CanWrite<S> && TypeTraits::IsConvertibleTo<S, double>, bool>>
		bool Serialize(Serialization::Writer serializer) const;

		[[nodiscard]] static constexpr uint32 CalculateCompressedDataSize()
		{
			return sizeof(UnitType) * 8 * 2;
		}
		template<typename S = T, typename E = EnableIf<TypeTraits::IsFloatingPoint<S>, bool>>
		bool Compress(BitView& target) const;
		template<typename S = T, typename E = EnableIf<TypeTraits::IsFloatingPoint<S>, bool>>
		bool Decompress(ConstBitView& source);

		union
		{
			struct
			{
				T x, y;
			};
			T m_data[2];
			VectorizedType m_vectorized;
		};
	};

	using Vector2d = TVector2<double>;
	using Vector2f = TVector2<float>;
	using Vector2i = TVector2<int>;
	using Vector2l = TVector2<long>;
	using Vector2ui = TVector2<uint32>;

	template<typename T>
	struct TBoolVector2 : public TVector2<T>
	{
		using BaseType = TVector2<T>;
		using TVector2<T>::TVector2;

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS uint8 GetMask() const
		{
			if constexpr (BaseType::IsVectorized)
			{
				return BaseType::m_vectorized.GetMask() & 0x3;
			}
			else
			{
				uint8 mask = 0;
				mask |= 0x1 * (BaseType::x > 0);
				mask |= 0x2 * (BaseType::y > 0);
				return mask;
			}
		}
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS bool AreAllSet() const
		{
			return (GetMask() & 0x3) == 0x3;
		}
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS bool AreAnySet() const
		{
			return (GetMask() & 0x3) != 0;
		}
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS bool AreNoneSet() const
		{
			return (GetMask() & 0x3) == 0;
		}
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS bool IsXSet() const
		{
			return (GetMask() & 0x1) == 0x1;
		}
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS bool IsYSet() const
		{
			return (GetMask() & 0x2) == 0x2;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS explicit operator bool() const noexcept
		{
			return AreAnySet();
		}
	};

	template<typename T>
	struct TRIVIAL_ABI UnalignedVector2
	{
		using UnitType = T;

		UnalignedVector2() = default;

		FORCE_INLINE constexpr UnalignedVector2(ZeroType) noexcept
			: x(0)
			, y(0)
		{
		}
		FORCE_INLINE constexpr UnalignedVector2(UpType) noexcept
			: x(0)
			, y((T)1)
		{
		}
		FORCE_INLINE constexpr UnalignedVector2(DownType) noexcept
			: x(0)
			, y((T)-1)
		{
		}
		FORCE_INLINE constexpr UnalignedVector2(RightType) noexcept
			: x((T)1)
			, y(0)
		{
		}
		FORCE_INLINE constexpr UnalignedVector2(LeftType) noexcept
			: x((T)-1)
			, y(0)
		{
		}

		FORCE_INLINE constexpr UnalignedVector2(const T _x, const T _y) noexcept
			: x(_x)
			, y(_y)
		{
		}
		FORCE_INLINE constexpr UnalignedVector2(const TVector2<T> vector) noexcept
			: x(vector.x)
			, y(vector.y)
		{
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr operator TVector2<T>() const noexcept
		{
			return {x, y};
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
			T x, y;
		};
	};
}
