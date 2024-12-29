#include "Memory/Optional.h"

#include <limits>

namespace ngine
{
#if !PLATFORM_WEB
	namespace Internal
	{
		template<>
		float OptionalWithInvalidValue<float>::GetInvalidValue() noexcept
		{
			return std::numeric_limits<float>::quiet_NaN();
		}

		template<>
		double OptionalWithInvalidValue<double>::GetInvalidValue() noexcept
		{
			return std::numeric_limits<double>::quiet_NaN();
		}
	}
#endif

	template struct Optional<double>;
	template struct Optional<float>;
	template struct Optional<bool>;
	template struct Optional<uint8>;
	template struct Optional<uint16>;
	template struct Optional<uint32>;
	template struct Optional<uint64>;
	template struct Optional<int8>;
	template struct Optional<int16>;
	template struct Optional<int32>;
	template struct Optional<int64>;
}
