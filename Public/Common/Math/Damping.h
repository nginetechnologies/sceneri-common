#pragma once

#include <Common/Math/LinearInterpolate.h>
#include <Common/Math/MultiplicativeInverse.h>
#include <Common/Math/NumericLimits.h>
#include <Common/Math/Power.h>
#include <Common/Math/Log.h>

namespace ngine::Math
{
	// Reference: https://theorangeduck.com/page/spring-roll-call#springdamper
	template<typename T>
	inline T GetDampedValue(T current, T target, float speed, float step)
	{
		const float log2 = Math::Log(2.f);
		const float halflife = Math::MultiplicativeInverse(speed);

		return Math::LinearInterpolate(
			current,
			target,
			1.0f - Math::Exponential(-(log2 * step) / (halflife + Math::NumericLimits<float>::Epsilon))
		);
	}

}
