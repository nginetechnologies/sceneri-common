#pragma once

#include "Vector2.h"
#include "Vector3.h"
#include "Angle3.h"

#include "MultiplicativeInverse.h"

#include "ForwardDeclarations/Matrix3x3.h"

#include <Common/Memory/Containers/Array.h>

#include <Common/Math/Vector3/Abs.h>
#include <Common/Math/Vector3/SignNonZero.h>
#include <Common/Math/Vector3/Select.h>

#include <Common/Math/Atan2.h>
#include <Common/Math/Asin.h>
#include <Common/Platform/TrivialABI.h>
#include <Common/Platform/Assume.h>

#include <Common/Serialization/CanRead.h>
#include <Common/Serialization/CanWrite.h>

namespace ngine::Math
{
	template<typename T>
	struct TMatrix3x4;
	template<typename T>
	struct TMatrix4x4;
	template<typename T>
	struct TQuaternion;

	enum CreateRotationAroundAxisType : uint8
	{
		CreateRotationAroundAxis
	};
	enum CreateRotationAroundXAxisType : uint8
	{
		CreateRotationAroundXAxis
	};
	enum CreateRotationAroundYAxisType : uint8
	{
		CreateRotationAroundYAxis
	};
	enum CreateRotationAroundZAxisType : uint8
	{
		CreateRotationAroundZAxis
	};

	/*	{ [0][0] right.x, [0][1] forward.x, [0][2] up.x },
	  { [1][0] right.y, [1][1] forward.y, [1][2] up.y },
	  { [2][0] right.z, [2][1] forward.z, [2][2] up.z }
	*/
	template<typename T>
	struct TRIVIAL_ABI TMatrix3x3
	{
		using UnitType = T;
		using VectorType = TVector3<T>;

		constexpr FORCE_INLINE TMatrix3x3() noexcept
		{
		}

		constexpr FORCE_INLINE TMatrix3x3(IdentityType) noexcept
			: m_right{1, 0, 0}
			, m_forward{0, 1, 0}
			, m_up{0, 0, 1}
		{
		}
		constexpr FORCE_INLINE TMatrix3x3(ForwardType) noexcept
			: TMatrix3x3(Identity)
		{
		}
		constexpr FORCE_INLINE TMatrix3x3(BackwardType) noexcept
			: m_right{-1, 0, 0}
			, m_forward{0, -1, 0}
			, m_up{0, 0, 1}
		{
		}
		constexpr FORCE_INLINE TMatrix3x3(UpType) noexcept
			: m_right{1, 0, 0}
			, m_forward{0, 0, 1}
			, m_up{0, -1, 0}
		{
		}
		constexpr FORCE_INLINE TMatrix3x3(DownType) noexcept
			: m_right{1, 0, 0}
			, m_forward{0, 0, -1}
			, m_up{0, 1, 0}
		{
		}
		constexpr FORCE_INLINE TMatrix3x3(RightType) noexcept
			: m_right{0, -1, 0}
			, m_forward{1, 0, 0}
			, m_up{0, 0, 1}
		{
		}
		constexpr FORCE_INLINE TMatrix3x3(LeftType) noexcept
			: m_right{0, 1, 0}
			, m_forward{-1, 0, 0}
			, m_up{0, 0, 1}
		{
		}

		constexpr FORCE_INLINE TMatrix3x3(const TVector3<T> right, const TVector3<T> forward, const TVector3<T> up) noexcept
			: m_right(right)
			, m_forward(forward)
			, m_up(up)
		{
		}

		constexpr FORCE_INLINE TMatrix3x3(const TVector3<T> forward, const TVector3<T> up = Math::Up) noexcept
			: TMatrix3x3(forward.Cross(up), forward, up)
		{
		}

		constexpr FORCE_INLINE
		TMatrix3x3(const TVector3<T> right, const TVector3<T> forward, const TVector3<T> up, const TVector3<T> scale) noexcept
			: m_right(right * scale.x)
			, m_forward(forward * scale.y)
			, m_up(up * scale.z)
		{
		}

		constexpr FORCE_INLINE TMatrix3x3(const CreateRotationAroundAxisType, const TAngle<T> angle, const TVector3<T> axis) noexcept
			: TMatrix3x3(CreateRotationAroundAxis(angle, axis))
		{
		}

		constexpr FORCE_INLINE TMatrix3x3(const CreateRotationAroundXAxisType, const TAngle<T> angle) noexcept
			: TMatrix3x3(CreateRotationAroundXAxis(angle))
		{
		}

		constexpr FORCE_INLINE TMatrix3x3(const CreateRotationAroundYAxisType, const TAngle<T> angle) noexcept
			: TMatrix3x3(CreateRotationAroundYAxis(angle))
		{
		}

		constexpr FORCE_INLINE TMatrix3x3(const CreateRotationAroundZAxisType, const TAngle<T> angle) noexcept
			: TMatrix3x3(CreateRotationAroundZAxis(angle))
		{
		}

		constexpr explicit FORCE_INLINE TMatrix3x3(const TEulerAngles<T> eulerAngles) noexcept
			: TMatrix3x3(FromEulerAngles(eulerAngles))
		{
		}

		constexpr explicit FORCE_INLINE TMatrix3x3(const TYawPitchRoll<T> yawPitchRoll) noexcept = delete;

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS TAngle<T> GetYaw() const noexcept
		{
			const T length = Math::TVector2<T>(m_forward.x, m_forward.y).GetLength();
			return TAngle<T>::FromRadians(Math::Atan2(-m_forward.x / length, m_forward.y / length));
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS TAngle<T> GetPitch() const noexcept
		{
			const T length = Math::TVector2<T>(m_forward.x, m_forward.y).GetLength();
			return TAngle<T>::FromRadians(Math::Atan2(m_forward.z, length));
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS TAngle<T> GetRoll() const noexcept
		{
			return TAngle<T>::FromRadians(Math::Atan2(-m_right.z, m_up.z));
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS TYawPitchRoll<T> GetYawPitchRoll() const noexcept
		{
			const T l = Math::TVector3<T>(m_forward.x, m_forward.y, 0.0f).GetLength();
			if (l > (T)0.0001)
			{
				return TYawPitchRoll{
					TAngle<T>::FromRadians(Math::Atan2(-m_forward.x / l, m_forward.y / l)),
					TAngle<T>::FromRadians(Math::Atan2(m_forward.z, l)),
					TAngle<T>::FromRadians(Math::Atan2(-m_right.z / l, m_up.z / l))
				};
			}

			return TYawPitchRoll{TAngle<T>::FromRadians(0), TAngle<T>::FromRadians(Math::Atan2(m_forward.z, l)), TAngle<T>::FromRadians(0)};
		}

		constexpr FORCE_INLINE PURE_LOCALS_AND_POINTERS TMatrix3x3 GetInverted() const noexcept
		{
			TMatrix3x3 result = {
				TVector3<T>{
					m_up.z * m_forward.y - m_up.y * m_forward.z,
					m_up.y * m_right.z - m_up.z * m_right.y,
					m_right.y * m_forward.z - m_right.z * m_forward.y
				},
				TVector3<T>{
					m_up.x * m_forward.z - m_up.z * m_forward.x,
					m_up.z * m_right.x - m_up.x * m_right.z,
					m_right.z * m_forward.x - m_right.x * m_forward.z
				},
				TVector3<T>{
					m_up.y * m_forward.x - m_up.x * m_forward.y,
					m_up.x * m_right.y - m_up.y * m_right.x,
					m_right.x * m_forward.y - m_right.y * m_forward.x
				}
			};

			const T inverseDeterminant =
				Math::MultiplicativeInverse(result.m_right.x * m_right.x + result.m_right.y * m_forward.x + result.m_right.z * m_up.x);

			result.m_right *= inverseDeterminant;
			result.m_forward *= inverseDeterminant;
			result.m_up *= inverseDeterminant;

			return result;
		}

		constexpr FORCE_INLINE PURE_LOCALS_AND_POINTERS TMatrix3x3 GetMagnitude() const noexcept
		{
			return TMatrix3x3{Math::Abs(m_right), Math::Abs(m_forward), Math::Abs(m_up)};
		}

		FORCE_INLINE void Orthonormalize() noexcept
		{
			m_right.Normalize();
			m_forward = m_up.GetNormalized().Cross(m_right).GetNormalized();
			m_up = m_right.Cross(m_forward);
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS TMatrix3x3 GetOrthonormalized() const noexcept
		{
			TMatrix3x3 result = *this;
			result.Orthonormalize();
			return result;
		}

		FORCE_INLINE void OrthonormalizeLeftHanded() noexcept
		{
			m_right.Normalize();
			m_forward = m_right.Cross(m_up).GetNormalized();
			m_up = m_forward.Cross(m_right);
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS bool IsOrthonormalizedLeftHanded() const noexcept
		{
			return m_right.IsEquivalentTo(m_up.Cross(m_forward)) & m_right.IsUnit() & m_forward.IsEquivalentTo(m_right.Cross(m_up)) &
			       m_forward.IsUnit() & m_up.IsEquivalentTo(m_forward.Cross(m_right)) & m_up.IsUnit();
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS bool IsOrthonormalized() const noexcept
		{
			return m_right.IsEquivalentTo(m_forward.Cross(m_up)) & m_right.IsUnit() & m_forward.IsEquivalentTo(m_up.Cross(m_right)) &
			       m_forward.IsUnit() & m_up.IsEquivalentTo(m_right.Cross(m_forward)) & m_up.IsUnit();
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS bool IsOrthonormalizedAnyHanded() const noexcept
		{
			const VectorType crossDot = Math::Abs(VectorType{m_right.Dot(m_forward), m_right.Dot(m_up), m_forward.Dot(m_up)});
			const VectorType epsilon(T(0.001));
			const VectorType crossDotMask = crossDot > epsilon;

			const VectorType selfDot = Math::Abs(VectorType(1) - VectorType{m_right.Dot(m_right), m_forward.Dot(m_forward), m_up.Dot(m_up)});
			const VectorType selfDotMask = selfDot > epsilon;
			return !(crossDotMask | selfDotMask).AreAnySet();
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS TEulerAngles<T> GetEulerAngles() const noexcept
		{
			const T y = Math::Asin(-m_right.z);
			if (Math::Abs(Math::Abs(y) - (TConstants<T>::PI * (T)0.5)) < (T)0.01)
			{
				return TEulerAngles<T>{
					TAngle<T>::FromRadians(0),
					TAngle<T>::FromRadians(y),
					TAngle<T>::FromRadians(Math::Atan2(-m_forward.x, m_forward.y))
				};
			}

			return TEulerAngles<T>{
				TAngle<T>::FromRadians(Math::Atan2(m_forward.z, m_up.z)),
				TAngle<T>::FromRadians(y),
				TAngle<T>::FromRadians(Math::Atan2(m_right.y, m_right.x))
			};
		}

		template<typename VectorType = TVector3<T>>
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr VectorType TransformDirection(const VectorType direction) const noexcept
		{
			return m_right * direction.x + m_forward * direction.y + m_up * direction.z;
		}

		template<typename VectorType = TVector3<T>>
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr VectorType TransformDirectionWithoutScale(const VectorType direction
		) const noexcept
		{
			return GetWithoutScale().TransformDirection(direction);
		}

		template<typename VectorType = TVector3<T>>
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr VectorType InverseTransformDirection(const VectorType direction
		) const noexcept
		{
			return GetInverted().TransformDirection(direction);
		}

		template<typename VectorType = TVector3<T>>
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr VectorType
		InverseTransformDirectionWithoutScale(const VectorType direction) const noexcept
		{
			return GetWithoutScale().InverseTransformDirection(direction);
		}

		template<typename VectorType = TVector3<T>>
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr VectorType TransformScale(const VectorType scale) const noexcept
		{
			return TransformDirectionWithoutScale(scale * GetScale());
		}

		template<typename VectorType = TVector3<T>>
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr VectorType InverseTransformScale(const VectorType scale) const noexcept
		{
			return Math::MultiplicativeInverse(InverseTransformDirectionWithoutScale(scale)) * GetScale();
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr const TVector3<T> GetRightColumn() const noexcept
		{
			return m_right;
		}
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr const TVector3<T> GetForwardColumn() const noexcept
		{
			return m_forward;
		}
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr const TVector3<T> GetUpColumn() const noexcept
		{
			return m_up;
		}
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr const TVector3<T> GetRightColumnWithoutScale() const noexcept
		{
			return m_right;
		}
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr const TVector3<T> GetForwardColumnWithoutScale() const noexcept
		{
			return m_forward;
		}
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr const TVector3<T> GetUpColumnWithoutScale() const noexcept
		{
			return m_up;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TMatrix3x3 TransformRotation(const TMatrix3x3 other) const noexcept
		{
			return TMatrix3x3{
				m_right * VectorType(other.m_right.x) + m_forward * VectorType(other.m_right.y) + m_up * VectorType(other.m_right.z),
				m_right * VectorType(other.m_forward.x) + m_forward * VectorType(other.m_forward.y) + m_up * VectorType(other.m_forward.z),
				m_right * VectorType(other.m_up.x) + m_forward * VectorType(other.m_up.y) + m_up * VectorType(other.m_up.z)
			};
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TMatrix3x3 InverseTransformRotation(const TMatrix3x3 other) const noexcept
		{
			return GetInverted().TransformRotation(other);
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS bool IsIdentity() const noexcept
		{
			return (Math::Abs(Math::Vector3f{(T)1 - m_right.x, m_right.y, m_right.z}).GetSum() +
			        Math::Abs(Math::Vector3f{m_forward.x, (T)1 - m_forward.y, m_forward.z}).GetSum() +
			        Math::Abs(Math::Vector3f{m_up.x, m_up.y, (T)1 - m_up.z}).GetSum()) == (T)0;
		}

		FORCE_INLINE void Scale(const TVector3<T> scale) noexcept
		{
			m_right *= scale.x;
			m_forward *= scale.y;
			m_up *= scale.z;
		}

		void SetScale(const TVector3<T> scale) noexcept
		{
			m_right = m_right.GetNormalized() * scale.x;
			m_forward = m_forward.GetNormalized() * scale.y;
			m_up = m_up.GetNormalized() * scale.z;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS TVector3<T> GetScale() const noexcept
		{
			return {
				m_right.GetNormalized().Dot(m_right),
				m_forward.GetNormalized().Dot(m_forward),
				m_up.GetNormalized().Dot(m_up),
			};
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS TVector3<T> GetInverseScale() const noexcept
		{
			return Math::MultiplicativeInverse(GetScale());
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS bool IsScaleUnit() const noexcept
		{
			return m_right.IsUnit() & m_forward.IsUnit() & m_up.IsUnit();
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr VectorType& operator[](const uint8 index) noexcept
		{
			ASSUME(index < 3);
			return *(&m_right + index);
		}
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr VectorType operator[](const uint8 index) const noexcept
		{
			ASSUME(index < 3);
			return *(&m_right + index);
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS TMatrix3x3 GetWithoutScale() const noexcept
		{
			return TMatrix3x3{m_right.GetNormalized(), m_forward.GetNormalized(), m_up.GetNormalized()};
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS TMatrix3x3 GetTransposed() const noexcept
		{
			return {{m_right.x, m_forward.x, m_up.x}, {m_right.y, m_forward.y, m_up.y}, {m_right.z, m_forward.z, m_up.z}};
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS bool
		IsEquivalentTo(const TMatrix3x3 other, const T epsilon = T(0.05)) const noexcept
		{
			return m_right.IsEquivalentTo(other.m_right, epsilon) & m_forward.IsEquivalentTo(other.m_forward, epsilon) &
			       m_up.IsEquivalentTo(other.m_up, epsilon);
		}

		template<typename S = T, typename E = EnableIf<Serialization::Internal::CanRead<S> && TypeTraits::IsConvertibleTo<double, S>, bool>>
		bool Serialize(const Serialization::Reader serializer);

		template<typename S = T, typename E = EnableIf<Serialization::Internal::CanWrite<S> && TypeTraits::IsConvertibleTo<S, double>, bool>>
		bool Serialize(Serialization::Writer serializer) const;
	protected:
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS static TMatrix3x3 FromEulerAngles(const TEulerAngles<T> eulerAngles) noexcept
		{
			const TVector3<T> angleCos = eulerAngles.Cos().ToRawRadians();
			const TVector3<T> angleSin = eulerAngles.Sin().ToRawRadians();

			const T sinYMultCosZ = angleSin.y * angleCos.z;
			const T sinYMultSinZ = angleSin.y * angleSin.z;

			return TMatrix3x3{
				TVector3<T>{angleCos.y * angleCos.z, angleCos.y * angleSin.z, -angleSin.y}.GetNormalized(),
				TVector3<T>{
					sinYMultCosZ * angleSin.x - angleCos.x * angleSin.z,
					sinYMultSinZ * angleSin.x + angleCos.x * angleCos.z,
					angleCos.y * angleSin.x
				}
					.GetNormalized(),
				TVector3<T>{
					sinYMultCosZ * angleCos.x + angleSin.x * angleSin.z,
					sinYMultSinZ * angleCos.x - angleSin.x * angleCos.z,
					angleCos.y * angleCos.x
				}
					.GetNormalized()
			};
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS static constexpr TMatrix3x3
		CreateRotationAroundAxis(const TAngle<T> angle, const TVector3<T> axis) noexcept
		{
			const TAngle<T> angleCos = angle.Cos();
			const TAngle<T> angleSin = angle.Sin();

			const TVector3<T> temp(axis * (T(1) - angleCos.GetRadians()));

			return TMatrix3x3{
				TVector3<T>{
					temp.x * axis.x + angleCos.GetRadians(),
					temp.y * axis.x + axis.z * angleSin.GetRadians(),
					temp.z * axis.x - axis.y * angleSin.GetRadians()
				}
					.GetNormalized(),
				TVector3<T>{
					temp.x * axis.y - axis.z * angleSin.GetRadians(),
					temp.y * axis.y + angleCos.GetRadians(),
					temp.z * axis.y + axis.x * angleSin.GetRadians()
				}
					.GetNormalized(),
				TVector3<T>{
					temp.x * axis.z + axis.y * angleSin.GetRadians(),
					temp.y * axis.z - axis.x * angleSin.GetRadians(),
					temp.z * axis.z + angleCos.GetRadians()
				}
					.GetNormalized()
			};
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS static constexpr TMatrix3x3 CreateRotationAroundXAxis(const TAngle<T> angle
		) noexcept
		{
			const TAngle<T> angleCos = angle.Cos();
			const TAngle<T> angleSin = angle.Sin();

			return TMatrix3x3{
				TVector3<T>{1.f, 0.f, 0.f},
				TVector3<T>{0.f, angleCos.GetRadians(), angleSin.GetRadians()},
				TVector3<T>{0.f, -angleSin.GetRadians(), angleCos.GetRadians()}
			};
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS static constexpr TMatrix3x3 CreateRotationAroundYAxis(const TAngle<T> angle
		) noexcept
		{
			const TAngle<T> angleCos = angle.Cos();
			const TAngle<T> angleSin = angle.Sin();

			return TMatrix3x3{
				TVector3<T>{angleCos.GetRadians(), 0.f, -angleSin.GetRadians()},
				TVector3<T>{0.f, 1.f, 0.f},
				TVector3<T>{angleSin.GetRadians(), 0.f, angleCos.GetRadians()}
			};
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS static constexpr TMatrix3x3 CreateRotationAroundZAxis(const TAngle<T> angle
		) noexcept
		{
			const TAngle<T> angleCos = angle.Cos();
			const TAngle<T> angleSin = angle.Sin();

			return TMatrix3x3{
				TVector3<T>{angleCos.GetRadians(), angleSin.GetRadians(), 0.f},
				TVector3<T>{-angleSin.GetRadians(), angleCos.GetRadians(), 0.f},
				TVector3<T>{0.f, 0.f, 1.f}
			};
		}
	public:
		friend struct TMatrix3x4<T>;
		friend struct TMatrix4x4<T>;
		friend struct TQuaternion<T>;

		TVector3<T> m_right;
		TVector3<T> m_forward;
		TVector3<T> m_up;
	};

	using Matrix3x3f = TMatrix3x3<float>;
}
