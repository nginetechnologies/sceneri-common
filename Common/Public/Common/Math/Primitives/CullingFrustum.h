#pragma once

#include <Common/Platform/TrivialABI.h>

namespace ngine::Math
{
	template<typename UnitType>
	struct TRIVIAL_ABI CullingFrustum
	{
		UnitType m_nearRight;
		UnitType m_nearTop;
		UnitType m_nearPlane;
		UnitType m_farPlane;
	};
}
