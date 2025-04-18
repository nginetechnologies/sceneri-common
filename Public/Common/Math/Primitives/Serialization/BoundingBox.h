#pragma once

#include "../BoundingBox.h"
#include <Common/Serialization/Reader.h>
#include <Common/Serialization/Writer.h>

namespace ngine::Math
{
	template<typename CoordinateType>
	FORCE_INLINE bool Serialize(TBoundingBox<CoordinateType>& boundingBox, const Serialization::Reader serializer)
	{
		CoordinateType min = Math::Zero, max = Math::Zero;
		serializer.Serialize("minimum", min);
		serializer.Serialize("maximum", max);
		boundingBox = TBoundingBox<CoordinateType>(min, max);
		return true;
	}

	template<typename CoordinateType>
	FORCE_INLINE bool Serialize(const TBoundingBox<CoordinateType>& boundingBox, Serialization::Writer serializer)
	{
		if (boundingBox.IsZero())
		{
			return false;
		}

		serializer.Serialize("minimum", boundingBox.GetMinimum());
		serializer.Serialize("maximum", boundingBox.GetMaximum());
		return true;
	}
}
