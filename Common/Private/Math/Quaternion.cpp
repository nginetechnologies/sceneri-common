#include "Common/Math/Quaternion.h"

#include <Common/Serialization/Reader.h>
#include <Common/Serialization/Writer.h>

#include <Common/Math/Vector3/Quantize.h>

namespace ngine::Math
{
	template<typename T>
	template<typename S, typename>
	bool TQuaternion<T>::Serialize(const Serialization::Reader serializer)
	{
		const Serialization::Value& __restrict currentElement = serializer.GetValue();
		Assert(currentElement.IsArray());
		Assert(currentElement.Size() == 3);
		Math::TEulerAngles<T> eulerAngles;
		eulerAngles.x = Math::TAngle<T>::FromDegrees(static_cast<T>(currentElement[0].GetDouble()));
		eulerAngles.y = Math::TAngle<T>::FromDegrees(static_cast<T>(currentElement[1].GetDouble()));
		eulerAngles.z = Math::TAngle<T>::FromDegrees(static_cast<T>(currentElement[2].GetDouble()));
		*this = TQuaternion<T>(eulerAngles);
		return true;
	}

	template<typename T>
	template<typename S, typename>
	bool TQuaternion<T>::Serialize(Serialization::Writer serializer) const
	{
		Serialization::Value& __restrict currentElement = serializer.GetValue();
		currentElement = Serialization::Value(rapidjson::Type::kArrayType);
		currentElement.Reserve(3, serializer.GetDocument().GetAllocator());

		const TEulerAngles<T> eulerAngles = GetEulerAngles();
		currentElement.PushBack(static_cast<double>(eulerAngles.x.GetDegrees()), serializer.GetDocument().GetAllocator());
		currentElement.PushBack(static_cast<double>(eulerAngles.y.GetDegrees()), serializer.GetDocument().GetAllocator());
		currentElement.PushBack(static_cast<double>(eulerAngles.z.GetDegrees()), serializer.GetDocument().GetAllocator());
		return true;
	}

	template struct TQuaternion<float>;

	template bool TQuaternion<float>::Serialize(const Serialization::Reader);
	template bool TQuaternion<float>::Serialize(Serialization::Writer) const;
}
