#include "Common/Math/Matrix3x3.h"
#include "Common/Math/Matrix3x4.h"

#include <Common/Serialization/Reader.h>
#include <Common/Serialization/Writer.h>

namespace ngine::Math
{
	template<typename T>
	template<typename S, typename>
	bool TMatrix3x3<T>::Serialize(const Serialization::Reader serializer)
	{
		const Serialization::Value& __restrict currentElement = serializer.GetValue();
		Assert(currentElement.IsArray());
		Assert(currentElement.Size() == 3);
		Math::TEulerAngles<T> eulerAngles;
		eulerAngles.x = Math::TAngle<T>::FromDegrees(static_cast<T>(currentElement[0].GetDouble()));
		eulerAngles.y = Math::TAngle<T>::FromDegrees(static_cast<T>(currentElement[1].GetDouble()));
		eulerAngles.z = Math::TAngle<T>::FromDegrees(static_cast<T>(currentElement[2].GetDouble()));
		*this = TMatrix3x3(eulerAngles);
		return true;
	}

	template<typename T>
	template<typename S, typename>
	bool TMatrix3x3<T>::Serialize(Serialization::Writer serializer) const
	{
		Serialization::Value& __restrict currentElement = serializer.GetValue();
		currentElement = Serialization::Value(rapidjson::Type::kArrayType);
		currentElement.Reserve(3, serializer.GetDocument().GetAllocator());

		const TEulerAngles<T> eulerAngles = GetEulerAngles();
		currentElement.PushBack(eulerAngles.x.GetDegrees(), serializer.GetDocument().GetAllocator());
		currentElement.PushBack(eulerAngles.y.GetDegrees(), serializer.GetDocument().GetAllocator());
		currentElement.PushBack(eulerAngles.z.GetDegrees(), serializer.GetDocument().GetAllocator());
		return true;
	}

	template bool TMatrix3x3<float>::Serialize(const Serialization::Reader);
	template bool TMatrix3x3<float>::Serialize(Serialization::Writer) const;

	template<typename T>
	template<typename S, typename>
	bool TMatrix3x4<T>::Serialize(const Serialization::Reader serializer)
	{
		if (!serializer.Serialize("position", m_location))
		{
			m_location = Math::Zero;
		}

		Math::Matrix3x3f& rotation = AsMatrix3x3();
		if (!serializer.Serialize("rotation", rotation))
		{
			rotation = Math::Identity;
		}

		Math::Vector3f scale;
		if (serializer.Serialize("scale", scale))
		{
			Scale(scale);
		}

		return true;
	}

	template<typename T>
	template<typename S, typename>
	bool TMatrix3x4<T>::Serialize(Serialization::Writer serializer) const
	{
		serializer.Serialize("position", m_location);

		if (!IsIdentity())
		{
			serializer.Serialize("rotation", GetRotationWithoutScale());

			const Math::Vector3f scale = GetScale();
			if (!scale.IsNormalized(0.f))
			{
				serializer.Serialize("scale", scale);
			}
		}

		return true;
	}

	template bool TMatrix3x4<float>::Serialize(const Serialization::Reader);
	template bool TMatrix3x4<float>::Serialize(Serialization::Writer) const;
}
