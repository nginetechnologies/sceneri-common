#pragma once

#include "Matrix3x3.h"

#include <Common/Math/Vector3/MultiplicativeInverse.h>
#include <Common/Memory/New.h>
#include <Common/Platform/TrivialABI.h>

#include <Common/Serialization/CanRead.h>
#include <Common/Serialization/CanWrite.h>

namespace ngine::Math
{
	/*
	  { [0][0] m_right.x, [0][1] m_forward.x, [0][2] m_up.x, [0][3] m_location.x },
	  { [1][0] m_right.y, [1][1] m_forward.y, [1][2] m_up.y, [1][3] m_location.y },
	  { [2][0] m_right.z, [2][1] m_forward.z, [2][2] m_up.z, [2][3] m_location.z }
	*/
	template<typename T>
	struct TRIVIAL_ABI TMatrix3x4 : protected TMatrix3x3<T>
	{
	protected:
		using BaseType = TMatrix3x3<T>;
	public:
		constexpr TMatrix3x4() noexcept
		{
		}

		using BaseType::m_right;
		using BaseType::m_forward;
		using BaseType::m_up;

		using BaseType::GetYaw;
		using BaseType::GetPitch;
		using BaseType::GetRoll;
		using BaseType::GetYawPitchRoll;
		using BaseType::GetEulerAngles;
		using BaseType::TransformDirection;
		using BaseType::InverseTransformDirection;
		using BaseType::TransformRotation;
		using BaseType::InverseTransformRotation;
		using BaseType::IsIdentity;
		using BaseType::Scale;
		using BaseType::GetScale;
		using BaseType::GetInverseScale;
		using BaseType::GetRightColumn;
		using BaseType::GetForwardColumn;
		using BaseType::GetUpColumn;
		using BaseType::Orthonormalize;

		constexpr FORCE_INLINE TMatrix3x4(IdentityType, const TVector3<T> translation = Zero) noexcept
			: BaseType(Identity)
			, m_location(translation)
		{
		}

		constexpr FORCE_INLINE TMatrix3x4(const BaseType& other, const TVector3<T> translation = Zero) noexcept
			: BaseType(other)
			, m_location(translation)
		{
		}

		constexpr FORCE_INLINE
		TMatrix3x4(const TVector3<T> right, const TVector3<T> forward, const TVector3<T> up, const TVector3<T> translation = Zero) noexcept
			: BaseType(right, forward, up)
			, m_location(translation)
		{
		}

		constexpr FORCE_INLINE TMatrix3x4(
			const TVector3<T> right,
			const TVector3<T> forward,
			const TVector3<T> up,
			const TVector3<T> scale,
			const TVector3<T> translation = Zero
		) noexcept
			: BaseType(right, forward, up, scale)
			, m_location(translation)
		{
		}

		constexpr FORCE_INLINE TMatrix3x4(
			const CreateRotationAroundAxisType, const TAngle<T> angle, const TVector3<T> axis, const TVector3<T> translation = Zero
		) noexcept
			: BaseType(CreateRotationAroundAxis, angle, axis)
			, m_location(translation)
		{
		}

		constexpr FORCE_INLINE
		TMatrix3x4(const CreateRotationAroundXAxisType, const TAngle<T> angle, const TVector3<T> translation = Zero) noexcept
			: BaseType(CreateRotationAroundXAxis, angle)
			, m_location(translation)
		{
		}

		constexpr FORCE_INLINE
		TMatrix3x4(const CreateRotationAroundYAxisType, const TAngle<T> angle, const TVector3<T> translation = Zero) noexcept
			: BaseType(CreateRotationAroundYAxis, angle)
			, m_location(translation)
		{
		}

		constexpr FORCE_INLINE
		TMatrix3x4(const CreateRotationAroundZAxisType, const TAngle<T> angle, const TVector3<T> translation = Zero) noexcept
			: BaseType(CreateRotationAroundZAxis, angle)
			, m_location(translation)
		{
		}

		explicit FORCE_INLINE TMatrix3x4(const TEulerAngles<T> eulerAngles, const TVector3<T> translation = Zero) noexcept
			: BaseType(eulerAngles)
			, m_location(translation)
		{
		}

		explicit FORCE_INLINE TMatrix3x4(const TYawPitchRoll<T> yawPitchRoll, const TVector3<T> translation = Zero) noexcept
			: BaseType(yawPitchRoll)
			, m_location(translation)
		{
		}

		[[nodiscard]] constexpr FORCE_INLINE PURE_LOCALS_AND_POINTERS TVector3<T> CalculateInvertedLocation() const noexcept
		{
			const BaseType rotation = BaseType::GetWithoutScale();
			const TVector3<T> scale{
				rotation.GetRightColumn().Dot(BaseType::m_right),
				rotation.GetForwardColumn().Dot(BaseType::m_forward),
				rotation.GetUpColumn().Dot(BaseType::m_up)
			};
			const TVector3<T> inverseScale = Math::MultiplicativeInverse(scale);

			return (TVector3<T>{-m_location.x} * TVector3<T>{rotation.m_right.x, rotation.m_forward.x, rotation.m_up.x} -
			        TVector3<T>{m_location.y} * TVector3<T>{rotation.m_right.y, rotation.m_forward.y, rotation.m_up.y} -
			        TVector3<T>{m_location.z} * TVector3<T>{rotation.m_right.z, rotation.m_forward.z, rotation.m_up.z}) *
			       inverseScale;
		}

		[[nodiscard]] constexpr FORCE_INLINE PURE_LOCALS_AND_POINTERS TMatrix3x4 GetInvertedLocation() const noexcept
		{
			return TMatrix3x4(BaseType(*this), CalculateInvertedLocation());
		}

		[[nodiscard]] constexpr FORCE_INLINE PURE_LOCALS_AND_POINTERS TMatrix3x4 GetInvertedRotationAndLocation() const noexcept
		{
			return TMatrix3x4(BaseType(BaseType::GetInverted()), CalculateInvertedLocation());
		}

		[[nodiscard]] constexpr FORCE_INLINE PURE_LOCALS_AND_POINTERS const TMatrix3x3<T> GetRotation() const noexcept
		{
			return *this;
		}

		[[nodiscard]] constexpr FORCE_INLINE PURE_LOCALS_AND_POINTERS TMatrix3x3<T> GetInvertedRotation() const noexcept
		{
			return BaseType::GetInverted();
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS TMatrix3x3<T> GetRotationWithoutScale() const noexcept
		{
			return TMatrix3x3<T>::GetWithoutScale();
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS TMatrix3x4<T> GetTransposed() const noexcept
		{
			return {TMatrix3x3<T>::GetTransposed(), m_location};
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS TVector3<T> TransformLocation(const TVector3<T> point) const noexcept
		{
			return TransformDirection(point) + m_location;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS TVector3<T> InverseTransformLocation(const TVector3<T> point) const noexcept
		{
			return InverseTransformDirection(point - m_location);
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS TMatrix3x4 TransformMatrix(const TMatrix3x4& other) const noexcept
		{
			return TMatrix3x4{TransformRotation(other), TransformLocation(other.GetLocation())};
		}

		FORCE_INLINE void SetRotation(const BaseType& matrix) noexcept
		{
			m_right = matrix.m_right;
			m_forward = matrix.m_forward;
			m_up = matrix.m_up;
		}

		using BaseType::SetScale;

		FORCE_INLINE void SetLocation(const TVector3<T> location) noexcept
		{
			m_location = location;
		}

		FORCE_INLINE void AddLocation(const TVector3<T> location) noexcept
		{
			m_location += location;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS const TVector3<T> GetLocation() const noexcept
		{
			return m_location;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS BaseType& AsMatrix3x3() noexcept
		{
			return *this;
		}
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS const BaseType& AsMatrix3x3() const noexcept
		{
			return *this;
		}

		template<typename S = T, typename E = EnableIf<Serialization::Internal::CanRead<S> && TypeTraits::IsConvertibleTo<double, S>, bool>>
		bool Serialize(const Serialization::Reader serializer);

		template<typename S = T, typename E = EnableIf<Serialization::Internal::CanWrite<S> && TypeTraits::IsConvertibleTo<S, double>, bool>>
		bool Serialize(Serialization::Writer serializer) const;

		TVector3<T> m_location;
	};

	using Matrix3x4f = TMatrix3x4<float>;
}
