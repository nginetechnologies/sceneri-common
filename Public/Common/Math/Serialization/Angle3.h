#pragma once

#include "../Angle3.h"
#include <Common/Serialization/Reader.h>
#include <Common/Serialization/Writer.h>

#include <Common/Math/Vector2.h>
#include <Common/Math/Vector2/Abs.h>

namespace ngine::Math
{
	template<typename T>
	inline bool Serialize(TAngle3<T>& angle3, const Serialization::Reader serializer)
	{
		const Serialization::Value& __restrict currentElement = serializer.GetValue();
		Assert(currentElement.IsArray());
		Assert(currentElement.Size() == 3);
		angle3.x = TAngle<T>::FromDegrees(static_cast<T>(currentElement[0].GetDouble()));
		angle3.y = TAngle<T>::FromDegrees(static_cast<T>(currentElement[1].GetDouble()));
		angle3.z = TAngle<T>::FromDegrees(static_cast<T>(currentElement[2].GetDouble()));
		return true;
	}

	template<typename T>
	inline bool Serialize(const TAngle3<T>& angle3, Serialization::Writer serializer)
	{
		Serialization::Value& __restrict currentElement = serializer.GetValue();
		currentElement = Serialization::Value(rapidjson::Type::kArrayType);
		currentElement.Reserve(3, serializer.GetDocument().GetAllocator());
		currentElement.PushBack(static_cast<double>(angle3.x.GetDegrees()), serializer.GetDocument().GetAllocator());
		currentElement.PushBack(static_cast<double>(angle3.y.GetDegrees()), serializer.GetDocument().GetAllocator());
		currentElement.PushBack(static_cast<double>(angle3.z.GetDegrees()), serializer.GetDocument().GetAllocator());
		return true;
	}

	inline bool Serialize(TYawPitchRoll<float>& yawPitchRoll, const Serialization::Reader serializer)
	{
		const Serialization::Value& __restrict currentElement = serializer.GetValue();
		Assert(currentElement.IsArray());
		Assert(currentElement.Size() == 3);
		yawPitchRoll.Yaw() = TAngle<float>::FromDegrees(static_cast<float>(currentElement[0].GetDouble()));
		yawPitchRoll.Pitch() = TAngle<float>::FromDegrees(static_cast<float>(currentElement[1].GetDouble()));
		yawPitchRoll.Roll() = TAngle<float>::FromDegrees(static_cast<float>(currentElement[2].GetDouble()));
		return true;
	}

	inline bool Serialize(const TYawPitchRoll<float>& yawPitchRoll, Serialization::Writer serializer)
	{
		Serialization::Value& __restrict currentElement = serializer.GetValue();
		currentElement = Serialization::Value(rapidjson::Type::kArrayType);
		currentElement.Reserve(3, serializer.GetDocument().GetAllocator());
		currentElement.PushBack(static_cast<double>(yawPitchRoll.GetYaw().GetDegrees()), serializer.GetDocument().GetAllocator());
		currentElement.PushBack(static_cast<double>(yawPitchRoll.GetPitch().GetDegrees()), serializer.GetDocument().GetAllocator());
		currentElement.PushBack(static_cast<double>(yawPitchRoll.GetRoll().GetDegrees()), serializer.GetDocument().GetAllocator());
		return true;
	}
}
