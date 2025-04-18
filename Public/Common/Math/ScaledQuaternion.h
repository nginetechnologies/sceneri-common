#pragma once

#include "Quaternion.h"
#include "ForwardDeclarations/ScaledQuaternion.h"

#include <Common/Math/Vector3/Abs.h>
#include <Common/Math/Vector3/SignNonZero.h>
#include <Common/Math/Vector3/MultiplicativeInverse.h>
#include <Common/Platform/TrivialABI.h>

namespace ngine::Math
{
	template<typename T>
	struct TRIVIAL_ABI TScaledQuaternion
	{
		using QuaternionType = TQuaternion<T>;
		using ScaleType = TVector3<T>;
		using RotationMatrixType = TMatrix3x3<T>;

		TScaledQuaternion() = default;

		FORCE_INLINE TScaledQuaternion(const IdentityType, const ScaleType scale = {1, 1, 1}) noexcept
			: m_rotation(Identity)
			, m_scale(scale)
		{
		}

		FORCE_INLINE TScaledQuaternion(const QuaternionType rotation, const ScaleType scale = {1, 1, 1}) noexcept
			: m_rotation(rotation)
			, m_scale(scale)
		{
		}

		FORCE_INLINE TScaledQuaternion(const RotationMatrixType rotation) noexcept
			: m_rotation(rotation.GetWithoutScale().GetOrthonormalized())
			, m_scale(rotation.GetScale())
		{
			const RotationMatrixType correctedMatrix = rotation.GetWithoutScale().GetOrthonormalized();
			const Math::Vector3f scaleSignChange = {
				correctedMatrix.m_right.Dot(rotation.m_right),
				correctedMatrix.m_forward.Dot(rotation.m_forward),
				correctedMatrix.m_up.Dot(rotation.m_up)
			};
			m_scale *= Math::SignNonZero(scaleSignChange);
		}

		FORCE_INLINE void SetRotation(const QuaternionType rotation) noexcept
		{
			m_rotation = rotation;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS Vector3f TransformDirection(const Vector3f location) const noexcept
		{
			return m_rotation.TransformDirection(location) * m_scale;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS Vector3f TransformDirectionWithoutScale(const Vector3f location) const noexcept
		{
			return m_rotation.TransformDirection(location);
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS Vector3f InverseTransformDirection(const Vector3f location) const noexcept
		{
			return m_rotation.InverseTransformDirection(location * Math::MultiplicativeInverse(m_scale));
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS Vector3f InverseTransformDirectionWithoutScale(const Vector3f location
		) const noexcept
		{
			return m_rotation.InverseTransformDirection(location);
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS ScaleType TransformScale(const Vector3f scale) const noexcept
		{
			return ScaleType(scale) * m_scale;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS ScaleType InverseTransformScale(const Vector3f scale) const noexcept
		{
			return scale * Math::MultiplicativeInverse(m_scale);
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS QuaternionType TransformRotation(const QuaternionType rotation) const noexcept
		{
			return m_rotation.TransformRotation(rotation);
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS QuaternionType InverseTransformRotation(const QuaternionType rotation
		) const noexcept
		{
			return m_rotation.InverseTransformRotation(rotation);
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS explicit operator RotationMatrixType() const noexcept
		{
			RotationMatrixType matrix(m_rotation);
			matrix.Scale(m_scale);
			return matrix;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS explicit operator QuaternionType() const noexcept
		{
			return m_rotation;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS Vector3f GetRightColumn() const noexcept
		{
			return m_rotation.GetRightColumn() * m_scale;
		}
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS Vector3f GetForwardColumn() const noexcept
		{
			return m_rotation.GetForwardColumn() * m_scale;
		}
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS Vector3f GetUpColumn() const noexcept
		{
			return m_rotation.GetUpColumn() * m_scale;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS ScaleType GetScale() const noexcept
		{
			return m_scale;
		}
		FORCE_INLINE void Scale(const ScaleType scale) noexcept
		{
			m_scale *= scale;
		}
		FORCE_INLINE void SetScale(const ScaleType scale) noexcept
		{
			m_scale = scale;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS bool IsIdentity() const noexcept
		{
			return m_rotation.IsIdentity() & m_scale.IsEquivalentTo({T(1), T(1), T(1)});
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS bool
		IsEquivalentTo(const TScaledQuaternion other, const T epsilon = Math::NumericLimits<T>::Epsilon) const noexcept
		{
			return m_rotation.IsEquivalentTo(other.m_rotation, epsilon) & m_scale.IsEquivalentTo(other.m_scale, epsilon);
		}
	protected:
		QuaternionType m_rotation;
		ScaleType m_scale;
	};
}
