#pragma once

#include "../Spline.h"
#include <Common/Serialization/Reader.h>
#include <Common/Serialization/Writer.h>
#include <Common/Memory/Containers/Serialization/ArrayView.h>

namespace ngine::Math
{
	FORCE_INLINE bool Serialize(typename Splinef::Point& splinePoint, const Serialization::Reader serializer)
	{

		serializer.Serialize("position", splinePoint.position);
		serializer.Serialize("backward", splinePoint.backward);
		serializer.Serialize("forward", splinePoint.forward);
		serializer.Serialize("up", splinePoint.up);
		splinePoint.isBezierCurve = true;
		serializer.Serialize("is_bezier", splinePoint.isBezierCurve);

		return true;
	}

	FORCE_INLINE bool Serialize(const typename Splinef::Point& splinePoint, Serialization::Writer serializer)
	{
		serializer.Serialize("position", splinePoint.position);
		serializer.Serialize("backward", splinePoint.backward);
		serializer.Serialize("forward", splinePoint.forward);
		serializer.Serialize("up", splinePoint.up);
		serializer.Serialize("is_bezier", splinePoint.isBezierCurve);

		return true;
	}

	template<typename CoordinateType>
	FORCE_INLINE bool Serialize(Spline<CoordinateType>& spline, const Serialization::Reader serializer)
	{
		auto readSplineArray = [&spline](const Serialization::Reader serializer)
		{
			const Serialization::Value& __restrict currentElement = serializer.GetValue();
			Assert(currentElement.IsArray());

			const rapidjson::SizeType size = currentElement.Size();
			spline.ReservePoints(size);

			for (const Optional<typename Spline<CoordinateType>::Point> point : serializer.GetArrayView<typename Spline<CoordinateType>::Point>())
			{
				Vector3f up = point->up;
				Vector3f position = point->position;
				bool isBezierCurve = point->isBezierCurve;
				spline.EmplacePoint(position, up, isBezierCurve);
			}
		};

		if (serializer.GetValue().IsArray())
		{
			// Legacy
			readSplineArray(serializer);
		}
		else
		{
			readSplineArray(*serializer.FindSerializer("points"));

			bool isClosed = false;
			serializer.Serialize("is_closed", isClosed);
			spline.SetClosed(isClosed);
		}

		return true;
	}

	template<typename CoordinateType>
	FORCE_INLINE bool Serialize(const Spline<CoordinateType>& spline, Serialization::Writer serializer)
	{
		if (spline.IsEmpty())
		{
			return false;
		}

		serializer.Serialize("points", spline.GetPoints());
		serializer.Serialize("is_closed", spline.IsClosed());

		return true;
	}
}
