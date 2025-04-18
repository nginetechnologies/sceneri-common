#pragma once

#include "ForwardDeclarations/Transform.h"
#include <Common/Math/ForwardDeclarations/Quaternion.h>
#include <Common/Serialization/ForwardDeclarations/Reader.h>
#include <Common/Serialization/ForwardDeclarations/Writer.h>
#include "ScaledQuaternion.h"
#include "Matrix3x4.h"
#include "WorldCoordinate.h"

#include <Common/Guid.h>
#include <Common/TypeTraits/IsSame.h>
#include <Common/Platform/TrivialABI.h>

namespace ngine::Math
{
	template<typename CoordinateType, typename RotationUnitType>
	struct TRIVIAL_ABI TTransform
	{
		inline static constexpr Guid TypeGuid = "{EF6B3901-2C33-439B-9B3A-96B05E96557F}"_guid;

		using QuaternionType = TQuaternion<RotationUnitType>;
		using ScaledQuaternionType = TScaledQuaternion<RotationUnitType>;
		using ScaleType = TVector3<RotationUnitType>;
		using MatrixType = TMatrix3x4<RotationUnitType>;
		using Matrix4x4Type = TMatrix4x4<RotationUnitType>;
		using RotationMatrixType = TMatrix3x3<RotationUnitType>;

		using StoredRotationType = ScaledQuaternionType;

		TTransform() = default;

		FORCE_INLINE
		TTransform(const IdentityType, const CoordinateType location = Math::Zero, const ScaleType scale = {1.f, 1.f, 1.f}) noexcept
			: m_rotation(Identity, scale)
			, m_location(location)
		{
		}

		FORCE_INLINE TTransform(const MatrixType& matrix) noexcept
			: m_rotation(matrix.GetRotation())
			, m_location(matrix.GetLocation())
		{
		}

		FORCE_INLINE TTransform(const Matrix4x4Type& matrix) noexcept
			: m_rotation(matrix.GetRotation())
			, m_location(matrix.GetLocation())
		{
		}

		FORCE_INLINE
		TTransform(const QuaternionType rotation, const CoordinateType location = Math::Zero, const ScaleType scale = {1.f, 1.f, 1.f}) noexcept
			: m_rotation(rotation, scale)
			, m_location(location)
		{
		}

		TTransform(const RotationMatrixType rotation, const CoordinateType location = Math::Zero) noexcept
			: m_rotation(rotation)
			, m_location(location)
		{
		}

		TTransform(const ScaledQuaternionType rotation, const CoordinateType location = Math::Zero) noexcept
			: m_rotation(rotation)
			, m_location(location)
		{
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS Vector3f TransformDirection(const Vector3f location) const noexcept
		{
			return m_rotation.TransformDirection(location);
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS Vector3f TransformDirectionWithoutScale(const Vector3f location) const noexcept
		{
			return m_rotation.TransformDirectionWithoutScale(location);
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS Vector3f InverseTransformDirection(const Vector3f location) const noexcept
		{
			return m_rotation.InverseTransformDirection(location);
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS Vector3f InverseTransformDirectionWithoutScale(const Vector3f location
		) const noexcept
		{
			return m_rotation.InverseTransformDirectionWithoutScale(location);
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS CoordinateType TransformLocation(const Vector3f location) const noexcept
		{
			return CoordinateType(TransformDirection(location)) + m_location;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS CoordinateType TransformLocationWithoutScale(const Vector3f location) const noexcept
		{
			return CoordinateType(TransformDirectionWithoutScale(location)) + m_location;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS CoordinateType InverseTransformLocation(const Vector3f location) const noexcept
		{
			return CoordinateType(m_rotation.InverseTransformDirection(location - m_location));
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS CoordinateType InverseTransformLocationWithoutScale(const Vector3f location
		) const noexcept
		{
			return CoordinateType(m_rotation.InverseTransformDirectionWithoutScale(location - m_location));
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS ScaleType TransformScale(const Vector3f scale) const noexcept
		{
			return m_rotation.TransformScale(scale);
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS CoordinateType InverseTransformScale(const Vector3f scale) const noexcept
		{
			return m_rotation.InverseTransformScale(scale);
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS QuaternionType TransformRotation(const QuaternionType rotation) const noexcept
		{
			return (QuaternionType)m_rotation.TransformRotation(rotation);
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS QuaternionType InverseTransformRotation(const QuaternionType rotation
		) const noexcept
		{
			return (QuaternionType)m_rotation.InverseTransformRotation(rotation);
		}

		template<typename OtherCoordinateType, typename OtherRotationType>
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS TTransform
		TransformAsMatrix(const TTransform<OtherCoordinateType, OtherRotationType>& other) const noexcept
		{
			const Math::Matrix3x4f thisMatrix(*this);
			const Math::Matrix3x4f otherMatrix(other);
			return TTransform(thisMatrix.TransformMatrix(otherMatrix));
		}

		template<typename OtherCoordinateType, typename OtherRotationType>
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS TTransform
		TransformIdeal(const TTransform<OtherCoordinateType, OtherRotationType>& other) const noexcept
		{
			return TTransform(
				TransformRotation(other.GetRotationQuaternion()),
				TransformLocation(other.GetLocation()),
				TransformScale(other.GetScale())
			);
		}

		template<typename OtherCoordinateType, typename OtherRotationType>
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS TTransform Transform(const TTransform<OtherCoordinateType, OtherRotationType>& other
		) const noexcept
		{
			// Convert to matrix so that negative scale has an effect on the rotation
			return TransformAsMatrix(other);
			// TransformIdeal(other);
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS TTransform GetTransformRelativeToAsMatrix(const TTransform other) const noexcept
		{
			const Math::Matrix3x4f thisMatrix(*this);
			const Math::Matrix3x4f otherMatrix(other);
			return TTransform(thisMatrix.GetInvertedRotationAndLocation().TransformMatrix(otherMatrix));
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS TTransform GetTransformRelativeToIdeal(const TTransform other) const noexcept
		{
			return TTransform(
				InverseTransformRotation(other.GetRotationQuaternion()),
				InverseTransformLocation(other.GetLocation()),
				InverseTransformScale(other.GetScale())
			);
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS TTransform GetTransformRelativeTo(const TTransform other) const noexcept
		{
			// Convert to matrix so that negative scale has an effect on the rotation
			return GetTransformRelativeToAsMatrix(other);
			// GetTransformRelativeToIdeal(other);
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS explicit operator Math::Matrix3x4f() const noexcept
		{
			return Math::Matrix3x4f{GetRotationMatrix(), GetLocation()};
		}

		FORCE_INLINE void SetLocation(const CoordinateType location) noexcept
		{
			m_location = location;
		}

		FORCE_INLINE void AddLocation(const CoordinateType location) noexcept
		{
			m_location += location;
		}

		FORCE_INLINE void SetRotation(const QuaternionType rotation) noexcept
		{
			m_rotation.SetRotation(rotation);
		}

		FORCE_INLINE void SetRotation(const RotationMatrixType rotation) noexcept
		{
			m_rotation = rotation;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS const StoredRotationType GetStoredRotation() const noexcept
		{
			return m_rotation;
		}
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS QuaternionType GetRotationQuaternion() const noexcept
		{
			return (QuaternionType)m_rotation;
		}
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS RotationMatrixType GetRotationMatrix() const noexcept
		{
			return (RotationMatrixType)m_rotation;
		}
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS const CoordinateType GetLocation() const noexcept
		{
			return m_location;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS Vector3f GetRightColumn() const noexcept
		{
			return m_rotation.GetRightColumn();
		}
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS Vector3f GetForwardColumn() const noexcept
		{
			return m_rotation.GetForwardColumn();
		}
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS Vector3f GetUpColumn() const noexcept
		{
			return m_rotation.GetUpColumn();
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS ScaleType GetScale() const noexcept
		{
			return m_rotation.GetScale();
		}
		FORCE_INLINE void Scale(const ScaleType scale) noexcept
		{
			m_rotation.Scale(scale);
		}
		FORCE_INLINE void SetScale(const ScaleType scale) noexcept
		{
			m_rotation.SetScale(scale);
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS bool IsIdentity() const noexcept
		{
			return m_rotation.IsIdentity() & m_location.IsZero();
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS bool IsEquivalentTo(
			const TTransform other,
			const typename CoordinateType::UnitType locationEpsilon = Math::NumericLimits<typename CoordinateType::UnitType>::Epsilon,
			const RotationUnitType rotationEpsilon = Math::NumericLimits<RotationUnitType>::Epsilon
		) const noexcept
		{
			return m_rotation.IsEquivalentTo(other.m_rotation, rotationEpsilon) & m_location.IsEquivalentTo(other.m_location, locationEpsilon);
		}

		[[nodiscard]] PURE_LOCALS_AND_POINTERS bool operator==(const TTransform& other) const
		{
			return IsEquivalentTo(other);
		}

		bool Serialize(const Serialization::Reader);
		bool Serialize(Serialization::Writer) const;
	protected:
		StoredRotationType m_rotation;
		CoordinateType m_location;
	};

	struct LocalTransform : public Transform3Df
	{
		using BaseType = Transform3Df;

		using BaseType::BaseType;
		explicit LocalTransform(const BaseType& base) noexcept
			: BaseType(base)
		{
		}

		template<typename OtherCoordinateType, typename OtherRotationType>
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS LocalTransform
		Transform(const TTransform<OtherCoordinateType, OtherRotationType>& other) const noexcept
		{
			return LocalTransform(BaseType::Transform(other));
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS LocalTransform GetTransformRelativeTo(const LocalTransform other) const noexcept
		{
			return LocalTransform(BaseType::GetTransformRelativeTo(other));
		}
	};

	struct WorldTransform : public TTransform<WorldCoordinate, WorldRotationUnitType>
	{
		using BaseType = TTransform<WorldCoordinate, WorldRotationUnitType>;

		using BaseType::BaseType;
		explicit WorldTransform(const BaseType& base) noexcept
			: BaseType(base)
		{
		}

		template<typename OtherCoordinateType, typename OtherRotationType>
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS WorldTransform
		Transform(const TTransform<OtherCoordinateType, OtherRotationType>& other) const noexcept
		{
			return WorldTransform(BaseType::Transform(other));
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS WorldTransform GetTransformRelativeTo(const WorldTransform other) const noexcept
		{
			return WorldTransform(BaseType::GetTransformRelativeTo(other));
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS LocalTransform GetTransformRelativeToAsLocal(const WorldTransform other
		) const noexcept
		{
			const WorldTransform transform = GetTransformRelativeTo(other);

			return LocalTransform{transform.GetStoredRotation(), transform.GetLocation()};
		}
	};

	using WorldMatrix3x3 = typename WorldTransform::RotationMatrixType;
}
