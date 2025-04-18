#pragma once

#include "ForwardDeclarations/Quaternion.h"
#include "Matrix3x3.h"
#include "Vector4.h"

#include <Common/Math/SinCos.h>
#include <Common/Math/Vector3/SinCos.h>
#include <Common/Math/Asin.h>
#include <Common/Math/Acos.h>
#include <Common/Math/Clamp.h>
#include <Common/Math/MathAssert.h>
#include <Common/Math/Vector4/Select.h>
#include <Common/Math/Vector4/Min.h>
#include <Common/Math/Vector4/Abs.h>
#include <Common/Math/Vector4/Sqrt.h>
#include <Common/Math/Vector4/MultiplicativeInverse.h>
#include <Common/Math/Sign.h>
#include <Common/Memory/Containers/ForwardDeclarations/BitView.h>
#include <Common/Platform/TrivialABI.h>
#include <Common/Platform/Assume.h>

#include <Common/Serialization/CanRead.h>
#include <Common/Serialization/CanWrite.h>

namespace ngine::Math
{
	template<typename T>
	struct TRIVIAL_ABI TQuaternion
	{
		inline static constexpr Guid TypeGuid = "{ebb6fa76-432f-4eab-86d0-807b0b98f1ae}"_guid;

		using UnitType = T;
		using Vector3Type = TVector3<T>;
		using Vector4Type = TVector4<T>;
		using MatrixType = TMatrix3x3<T>;

		constexpr TQuaternion() noexcept
		{
		}

		FORCE_INLINE constexpr TQuaternion(const T x, const T y, const T z, const T w) noexcept
			: m_vector{x, y, z, w}
		{
		}
		FORCE_INLINE constexpr TQuaternion(const Vector4Type vector) noexcept
			: m_vector(vector)
		{
		}

		FORCE_INLINE constexpr TQuaternion(const IdentityType) noexcept
			: m_vector{0, 0, 0, 1}
		{
		}
		FORCE_INLINE constexpr TQuaternion(const ForwardType) noexcept
			: m_vector{0, 0, 0, 1}
		{
		}
		FORCE_INLINE constexpr TQuaternion(const BackwardType) noexcept
			: m_vector{0, 0, 1, 0}
		{
		}
		FORCE_INLINE constexpr TQuaternion(const UpType) noexcept
			: m_vector{T(0.707106769), 0, 0, T(0.707106769)}
		{
		}
		FORCE_INLINE constexpr TQuaternion(const DownType) noexcept
			: m_vector{-T(0.707106769), 0, 0, T(0.707106769)}
		{
		}
		FORCE_INLINE constexpr TQuaternion(const RightType) noexcept
			: m_vector{0, 0, -T(0.707106769), T(0.707106769)}
		{
		}
		FORCE_INLINE constexpr TQuaternion(const LeftType) noexcept
			: m_vector{0, 0, T(0.707106769), T(0.707106769)}
		{
		}

		FORCE_INLINE explicit TQuaternion(const MatrixType matrix) noexcept
		{
			MathAssert(matrix.IsOrthonormalized());
			T trace = matrix.m_right.x + matrix.m_forward.y + matrix.m_up.z;

			const Vector4Type leftComparison = Vector4Type(trace, matrix.m_right.x, matrix.m_forward.y, matrix.m_up.z);
			typename Vector4Type::BoolType comparisonResult =
				leftComparison >= Vector4Type(T(-Math::NumericLimits<T>::MinPositive), matrix.m_forward.y, matrix.m_right.x, matrix.m_right.x);
			comparisonResult = comparisonResult &
			                   (leftComparison >= Vector4Type(leftComparison.x, matrix.m_up.z, matrix.m_up.z, matrix.m_forward.y));
			const uint8 mask = comparisonResult.GetMask();

			Vector4Type result;
			if (mask & 0x1)
			{
				result.w = Math::Sqrt(T(1) + trace) * T(0.5);
				const UnitType wfactor = Math::MultiplicativeInverse(T(4) * result.w);
				const Vector3Type result3 = (Vector3Type{matrix.m_forward.z, matrix.m_up.x, matrix.m_right.y} -
				                             Vector3Type{matrix.m_up.y, matrix.m_right.z, matrix.m_forward.x}) *
				                            wfactor;
				result = {result3.x, result3.y, result3.z, result.w};
			}
			else if (mask & 0x2)
			{
				trace = matrix.m_right.x - matrix.m_forward.y - matrix.m_up.z;
				result.x = Math::Sqrt(T(1) + trace) * T(0.5);
				const UnitType wfactor = Math::MultiplicativeInverse(T(4) * result.x);
				const Vector3Type result3 = (Vector3Type{matrix.m_right.y, matrix.m_right.z, matrix.m_forward.z} +
				                             Vector3Type{matrix.m_forward.x, matrix.m_up.x, -matrix.m_up.y}) *
				                            wfactor;
				result = {result.x, result3.x, result3.y, result3.z};
			}
			else if (mask & 0x4)
			{
				trace = matrix.m_forward.y - matrix.m_up.z - matrix.m_right.x;
				result.y = Math::Sqrt(T(1) + trace) * T(0.5);
				const UnitType wfactor = Math::MultiplicativeInverse(T(4) * result.y);
				const Vector3Type result3 = (Vector3Type{matrix.m_forward.x, matrix.m_forward.z, matrix.m_up.x} +
				                             Vector3Type{matrix.m_right.y, matrix.m_up.y, -matrix.m_right.z}) *
				                            wfactor;
				result = {result3.x, result.y, result3.y, result3.z};
			}
			else if (mask & 0x8)
			{
				trace = matrix.m_up.z - matrix.m_right.x - matrix.m_forward.y;
				result.z = Math::Sqrt(T(1) + trace) * T(0.5);
				const UnitType wfactor = Math::MultiplicativeInverse(T(4) * result.z);
				const Vector3Type result3 = (Vector3Type{matrix.m_up.x, matrix.m_up.y, matrix.m_right.y} +
				                             Vector3Type{matrix.m_right.z, matrix.m_forward.z, -matrix.m_forward.x}) *
				                            wfactor;
				result = {result3.x, result3.y, result.z, result3.z};
			}
			else
			{
				result = {0, 0, 0, 1};
			}
			m_vector = result;

			MathAssert(matrix.IsEquivalentTo(MatrixType(*this)));
		}

		FORCE_INLINE constexpr explicit TQuaternion(const Vector3Type right, const Vector3Type forward, const Vector3Type up) noexcept
			: TQuaternion(MatrixType{right, forward, up})
		{
		}
		FORCE_INLINE constexpr explicit TQuaternion(const Vector3Type forward, const Vector3Type up) noexcept
			: TQuaternion(MatrixType{forward.Cross(up), forward, up}.GetOrthonormalized())
		{
		}
		FORCE_INLINE constexpr explicit TQuaternion(const Vector3Type direction) noexcept
		{
			const T forwardDot = Math::Abs(direction.Dot(Math::Forward));
			const T upDot = Math::Abs(direction.Dot(Math::Up));

			Vector3Type up;
			if (forwardDot < upDot)
			{
				up = Vector3Type(Math::Forward) * Math::SignNonZero(direction.z);
			}
			else
			{
				up = Vector3Type(Math::Up) * Math::SignNonZero(direction.z);
			}

			const Vector3Type right = direction.Cross(up).GetNormalized();
			up = right.Cross(direction).GetNormalized();
			*this = TQuaternion(right, direction, up);
		}

		constexpr FORCE_INLINE TQuaternion(const CreateRotationAroundAxisType, const TAngle<T> angle, const Vector3Type axis) noexcept
		{
			const T halfAngle = angle.GetRadians() * T(0.5);
			T cos;
			const T sin = Math::SinCos(halfAngle, cos);

			const Vector3Type axisSin = axis * sin;
			m_vector = {axisSin.x, axisSin.y, axisSin.z, cos};
		}

		constexpr FORCE_INLINE TQuaternion(const CreateRotationAroundXAxisType, const TAngle<T> angle) noexcept
		{
			const T halfAngle = angle.GetRadians() * T(0.5);
			T cos;
			const T sin = Math::SinCos(halfAngle, cos);

			m_vector = {sin, 0, 0, cos};
		}

		constexpr FORCE_INLINE TQuaternion(const CreateRotationAroundYAxisType, const TAngle<T> angle) noexcept
		{
			const T halfAngle = angle.GetRadians() * T(0.5);
			T cos;
			const T sin = Math::SinCos(halfAngle, cos);

			m_vector = {0, sin, 0, cos};
		}

		constexpr FORCE_INLINE TQuaternion(const CreateRotationAroundZAxisType, const TAngle<T> angle) noexcept
		{
			const T halfAngle = angle.GetRadians() * T(0.5);
			T cos;
			const T sin = Math::SinCos(halfAngle, cos);

			m_vector = {0, 0, sin, cos};
		}

		constexpr explicit FORCE_INLINE TQuaternion(const TEulerAngles<T> eulerAngles) noexcept
		{
			const TEulerAngles<T> halfAngles = eulerAngles * 0.5f;
			const typename TEulerAngles<T>::SinCosResult sinCos = halfAngles.SinCos();

			const TVector3<T> sin = sinCos.sin.ToRawRadians();
			const TVector3<T> cos = sinCos.cos.ToRawRadians();

			m_vector = Vector4Type{cos.z, cos.z, sin.z, cos.z} * Vector4Type{sin.x, cos.x, cos.x, cos.x} *
			             Vector4Type{cos.y, sin.y, cos.y, cos.y} +
			           Vector4Type{-sin.z, sin.z, -cos.z, sin.z} * Vector4Type{cos.x, sin.x, sin.x, sin.x} *
			             Vector4Type{sin.y, cos.y, sin.y, sin.y};
		}

		constexpr explicit FORCE_INLINE TQuaternion(const TYawPitchRoll<T> yawPitchRoll) noexcept
		{
			const typename TYawPitchRoll<T>::SinCosResult anglesSinCos = (yawPitchRoll * 0.5f).SinCos();

			const Vector4Type left =
				Vector4Type{
					anglesSinCos.sin.GetPitch().GetRadians(),
					anglesSinCos.cos.GetPitch().GetRadians(),
					anglesSinCos.cos.GetPitch().GetRadians(),
					anglesSinCos.cos.GetPitch().GetRadians()
				} *
				Vector4Type{
					anglesSinCos.cos.GetRoll().GetRadians(),
					anglesSinCos.sin.GetRoll().GetRadians(),
					anglesSinCos.cos.GetRoll().GetRadians(),
					anglesSinCos.cos.GetRoll().GetRadians()
				} *
				Vector4Type{
					anglesSinCos.cos.GetYaw().GetRadians(),
					anglesSinCos.cos.GetYaw().GetRadians(),
					anglesSinCos.sin.GetYaw().GetRadians(),
					anglesSinCos.cos.GetYaw().GetRadians()
				};

			const Vector4Type right =
				Vector4Type{
					anglesSinCos.cos.GetPitch().GetRadians(),
					anglesSinCos.sin.GetPitch().GetRadians(),
					anglesSinCos.sin.GetPitch().GetRadians(),
					anglesSinCos.sin.GetPitch().GetRadians()
				} *
				Vector4Type{
					anglesSinCos.sin.GetRoll().GetRadians(),
					anglesSinCos.cos.GetRoll().GetRadians(),
					anglesSinCos.sin.GetRoll().GetRadians(),
					anglesSinCos.sin.GetRoll().GetRadians()
				} *
				Vector4Type{
					anglesSinCos.sin.GetYaw().GetRadians(),
					anglesSinCos.sin.GetYaw().GetRadians(),
					anglesSinCos.cos.GetYaw().GetRadians(),
					anglesSinCos.sin.GetYaw().GetRadians()
				};

			m_vector = left + right * Vector4Type{T(-1), T(1), T(-1), T(1)};
		}

		FORCE_INLINE void SetRotation(const TQuaternion rotation) noexcept
		{
			*this = rotation;
		}

		[[nodiscard]] FORCE_INLINE explicit constexpr operator MatrixType() const noexcept
		{
			const Vector3Type v{m_vector.x, m_vector.y, m_vector.z};
			const Vector3Type v2 = v + v;
			const Vector3Type v2v = v2 * v;

			const Vector3Type primaryAxes = (Vector3Type{1} - v2v.yxx()) - v2v.zzy();
			const Vector3Type xyyzxz = v.xyx() * v2.yzz();
			const Vector3Type vvw = v2 * m_vector.w;
			const Vector3Type xyyzxzadd = xyyzxz + vvw.zxy();
			const Vector3Type xyyzxzsub = xyyzxz.zxy() - vvw.yzx();

			return MatrixType{
				Vector3Type{primaryAxes.x, xyyzxzadd.x, xyyzxzsub.x},
				Vector3Type{xyyzxzsub.y, primaryAxes.y, xyyzxzadd.y},
				Vector3Type{xyyzxzadd.z, xyyzxzsub.z, primaryAxes.z}
			};
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr bool IsIdentity() const noexcept
		{
			return IsEquivalentTo(Math::Identity);
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS bool
		IsEquivalentTo(const TQuaternion other, const T epsilon = Math::NumericLimits<T>::Epsilon) const noexcept
		{
			const Vector4Type vEpsilon{epsilon};

			const Vector4Type absDiff = Math::Abs(m_vector - other.m_vector);
			const Vector4Type absSum = Math::Abs(m_vector + other.m_vector);

			const typename Vector4Type::BoolType resultMask = Math::Min(absDiff, absSum) <= vEpsilon;
			return resultMask.AreAllSet();
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TQuaternion GetInverted() const noexcept
		{
			return m_vector * Vector4Type{-1.f, -1.f, -1.f, 1.f};
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TQuaternion GetNormalized() const noexcept
		{
			return TQuaternion{m_vector.GetNormalized()};
		}
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr bool IsNormalized(const T epsilon = (T)0.05) const noexcept
		{
			return m_vector.IsNormalized(epsilon);
		}
		void Normalize()
		{
			*this = GetNormalized();
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TQuaternion operator-() const
		{
			return TQuaternion(-m_vector);
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector3<T> GetRightColumn() const noexcept
		{
			return TransformDirection(Vector3Type(Right));
		}
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector3<T> GetForwardColumn() const noexcept
		{
			return TransformDirection(Vector3Type(Forward));
		}
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TVector3<T> GetUpColumn() const noexcept
		{
			return TransformDirection(Vector3Type(Up));
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TQuaternion TransformRotation(const TQuaternion other) const noexcept
		{
			const Vector3Type thisV{m_vector.x, m_vector.y, m_vector.z};
			const Vector3Type otherV{other.m_vector.x, other.m_vector.y, other.m_vector.z};
			const Vector3Type resultV = thisV.Cross(otherV) + Vector3Type{m_vector.w} * otherV + Vector3Type{other.m_vector.w} * thisV;
			return TQuaternion(Vector4Type{
				resultV.x,
				resultV.y,
				resultV.z,
				m_vector.w * other.m_vector.w - (thisV * otherV).GetSum(),
			});
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TQuaternion InverseTransformRotation(const TQuaternion other
		) const noexcept
		{
			return GetInverted().TransformRotation(other);
		}

		template<typename Vector3Type = TVector3<T>>
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr Vector3Type TransformDirection(const Vector3Type direction) const noexcept
		{
			const Vector3Type axis{m_vector.x, m_vector.y, m_vector.z};
			const Vector3Type tangent = axis.Cross(direction) * T(2);
			return direction + (tangent * m_vector.w) + axis.Cross(tangent);
		}

		template<typename Vector3Type = TVector3<T>>
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr Vector3Type TransformDirection(const UpType) const noexcept
		{
			return TransformDirection(Vector3Type{Up});
		}

		template<typename Vector3Type = TVector3<T>>
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr Vector3Type TransformDirection(const DownType) const noexcept
		{
			return TransformDirection(Vector3Type{Down});
		}

		template<typename Vector3Type = TVector3<T>>
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr Vector3Type TransformDirection(const ForwardType) const noexcept
		{
			return TransformDirection(Vector3Type{Forward});
		}

		template<typename Vector3Type = TVector3<T>>
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr Vector3Type TransformDirection(const BackwardType) const noexcept
		{
			return TransformDirection(Vector3Type{Backward});
		}

		template<typename Vector3Type = TVector3<T>>
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr Vector3Type TransformDirection(const LeftType) const noexcept
		{
			return TransformDirection(Vector3Type{Left});
		}

		template<typename Vector3Type = TVector3<T>>
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr Vector3Type TransformDirection(const RightType) const noexcept
		{
			return TransformDirection(Vector3Type{Right});
		}

		template<typename Vector3Type = TVector3<T>>
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr Vector3Type InverseTransformDirection(const Vector3Type direction
		) const noexcept
		{
			return GetInverted().TransformDirection(direction);
		}

		template<typename Vector3Type = TVector3<T>>
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr Vector3Type InverseTransformDirection(const UpType) const noexcept
		{
			return GetInverted().TransformDirection(Up);
		}

		template<typename Vector3Type = TVector3<T>>
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr Vector3Type InverseTransformDirection(const DownType) const noexcept
		{
			return GetInverted().TransformDirection(Down);
		}

		template<typename Vector3Type = TVector3<T>>
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr Vector3Type InverseTransformDirection(const ForwardType) const noexcept
		{
			return GetInverted().TransformDirection(Forward);
		}

		template<typename Vector3Type = TVector3<T>>
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr Vector3Type InverseTransformDirection(const BackwardType) const noexcept
		{
			return GetInverted().TransformDirection(Backward);
		}

		template<typename Vector3Type = TVector3<T>>
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr Vector3Type InverseTransformDirection(const LeftType) const noexcept
		{
			return GetInverted().TransformDirection(Left);
		}

		template<typename Vector3Type = TVector3<T>>
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr Vector3Type InverseTransformDirection(const RightType) const noexcept
		{
			return GetInverted().TransformDirection(Right);
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS TAngle<T> GetYaw() const noexcept
		{
			return MatrixType(*this).GetYaw();
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS TAngle<T> GetPitch() const noexcept
		{
			return MatrixType(*this).GetPitch();
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS TAngle<T> GetRoll() const noexcept
		{
			return MatrixType(*this).GetRoll();
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS TYawPitchRoll<T> GetYawPitchRoll() const noexcept
		{
			return MatrixType(*this).GetYawPitchRoll();
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS TEulerAngles<T> GetEulerAngles() const noexcept
		{
			TEulerAngles<T> result;
			result.y = TAngle<T>::FromRadians(Math::Asin(Math::Clamp(-(m_vector.x * m_vector.z - m_vector.w * m_vector.y) * 2, T(-1), T(1))));
			if (Math::Abs(Math::Abs(result.y.GetRadians()) - Math::TConstants<T>::PI * T(0.5)) < (T)0.01)
			{
				result.x = TAngle<T>::FromRadians(0);
				result.z = TAngle<T>::FromRadians(
					Math::Atan2(-2 * (m_vector.x * m_vector.y - m_vector.w * m_vector.z), 1 - (m_vector.x * m_vector.x + m_vector.z * m_vector.z) * 2)
				);
			}
			else
			{
				result.x = TAngle<T>::FromRadians(
					Math::Atan2((m_vector.y * m_vector.z + m_vector.w * m_vector.x) * 2, 1 - (m_vector.x * m_vector.x + m_vector.y * m_vector.y) * 2)
				);
				result.z = TAngle<T>::FromRadians(
					Math::Atan2((m_vector.x * m_vector.y + m_vector.w * m_vector.z) * 2, 1 - (m_vector.z * m_vector.z + m_vector.y * m_vector.y) * 2)
				);
			}
			return result;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS static TQuaternion
		LinearInterpolation(const TQuaternion from, TQuaternion to, const T ratio)
		{
			to = Math::Select(from.m_vector.Dot(to.m_vector) >= T(0), to, -to);
			const T inverseRatio = T(1) - ratio;
			return TQuaternion{from.m_vector * inverseRatio + to.m_vector * ratio};
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS static TQuaternion
		SphericalInterpolation(const TQuaternion from, TQuaternion to, const T ratio)
		{
			T cosine = from.m_vector.Dot(to.m_vector);

			// Negate if interpolation would've taken the long way around the sphere
			to = Math::Select(cosine >= T(0), to, -to);
			cosine = Math::Abs(cosine);

			const T inverseRatio = T(1) - ratio;
			if (cosine > T(0.9999))
			{
				return TQuaternion{from.m_vector * inverseRatio + to.m_vector * ratio};
			}

			const T angle = Math::Acos(cosine);
			const Vector3Type sine = Math::Sin(Vector3Type{inverseRatio * angle, ratio * angle, angle});
			return TQuaternion{(from.m_vector * sine.x + to.m_vector * sine.y) / sine.z};
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr T& operator[](const uint8 index) noexcept
		{
			ASSUME(index < 4);
			return *(&x + index);
		}
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr T operator[](const uint8 index) const noexcept
		{
			ASSUME(index < 4);
			return *(&x + index);
		}

		template<typename S = T, typename E = EnableIf<Serialization::Internal::CanRead<S> && TypeTraits::IsConvertibleTo<double, S>, bool>>
		bool Serialize(const Serialization::Reader serializer);

		template<typename S = T, typename E = EnableIf<Serialization::Internal::CanWrite<S> && TypeTraits::IsConvertibleTo<S, double>, bool>>
		bool Serialize(Serialization::Writer serializer) const;

		[[nodiscard]] static constexpr uint32 CalculateCompressedDataSize()
		{
			// One bit to indicate whether the maximum value is 1
			// Two bits to indicate maximum index
			// 12 bits per non-max axis
			return 1 + 2 + 12 * 3;
		}
		bool Compress(BitView& target) const;
		bool Decompress(ConstBitView& source);

		union
		{
			Vector4Type m_vector;
			struct
			{
				T x, y, z, w;
			};
		};
	};

	using Quaternionf = TQuaternion<float>;
}
