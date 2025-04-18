#pragma once

namespace ngine::Time
{
	template<typename Type>
	struct Duration;

	using Durationf = Duration<float>;
	using Durationd = Duration<double>;
}
