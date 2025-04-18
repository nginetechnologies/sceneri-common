#include "Common/Math/Transform.h"
#include "Common/Math/Transform2D.h"

#include <Common/Serialization/Reader.h>
#include <Common/Serialization/Writer.h>

namespace ngine::Math
{
	template<typename CoordinateType, typename RotationUnitType>
	bool TTransform<CoordinateType, RotationUnitType>::Serialize(const Serialization::Reader serializer)
	{
		CoordinateType location = m_location;
		serializer.Serialize("position", location);

		TQuaternion<RotationUnitType> rotation = GetRotationQuaternion();
		rotation = serializer.ReadWithDefaultValue("rotation", TQuaternion<RotationUnitType>(Math::Identity));

		TVector3<RotationUnitType> scale = GetScale();
		scale = serializer.ReadWithDefaultValue("scale", TVector3<RotationUnitType>(1.0f));

		*this = TTransform<CoordinateType, RotationUnitType>(rotation, location, scale);
		return true;
	}

	template<typename CoordinateType, typename RotationUnitType>
	bool TTransform<CoordinateType, RotationUnitType>::Serialize(Serialization::Writer serializer) const
	{
		bool setAny = false;

		if (!GetLocation().IsZero())
		{
			serializer.Serialize("position", GetLocation());
			setAny |= true;
		}

		if (!GetRotationQuaternion().IsIdentity())
		{
			serializer.Serialize("rotation", GetRotationQuaternion());
			setAny |= true;
		}

		const TVector3<RotationUnitType> scale = GetScale();
		if (!scale.IsEquivalentTo(Math::Vector3f{1.f}))
		{
			serializer.Serialize("scale", scale);
			setAny |= true;
		}

		return setAny;
	}

	template bool TTransform<Vector3f, float>::Serialize(const Serialization::Reader);
	template bool TTransform<Vector3f, float>::Serialize(Serialization::Writer) const;
	template bool TTransform<WorldCoordinate, WorldRotationUnitType>::Serialize(const Serialization::Reader);
	template bool TTransform<WorldCoordinate, WorldRotationUnitType>::Serialize(Serialization::Writer) const;

	template<typename UnitType>
	bool TTransform2D<UnitType>::Serialize(const Serialization::Reader serializer)
	{
		CoordinateType location;
		if (!serializer.Serialize("position", location))
		{
			location = Math::Zero;
		}

		RotationType rotation;
		if (!serializer.Serialize("rotation", rotation))
		{
			rotation = 0_degrees;
		}

		ScaleType scale = {1.f, 1.f};
		serializer.Serialize("scale", scale);

		*this = TTransform2D<UnitType>(rotation, location, scale);
		return true;
	}

	template<typename UnitType>
	bool TTransform2D<UnitType>::Serialize(Serialization::Writer serializer) const
	{
		bool setAny = false;

		if (!GetLocation().IsZero())
		{
			serializer.Serialize("position", GetLocation());
			setAny |= true;
		}

		if (GetRotation().GetRadians() != 0)
		{
			serializer.Serialize("rotation", GetRotation());
			setAny |= true;
		}

		const ScaleType scale = GetScale();
		if (!scale.IsEquivalentTo(Math::Vector2f{1.f}))
		{
			serializer.Serialize("scale", scale);
			setAny |= true;
		}

		return setAny;
	}

	template bool TTransform2D<WorldCoordinateUnitType>::Serialize(const Serialization::Reader);
	template bool TTransform2D<WorldCoordinateUnitType>::Serialize(Serialization::Writer) const;
}
