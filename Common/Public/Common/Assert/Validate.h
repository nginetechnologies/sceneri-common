#pragma once

#include <Common/Platform/NoInline.h>
#include <Common/Platform/CodeSection.h>
#include <Common/Platform/Cold.h>

namespace ngine
{
#define UNLIKELY_ERROR_SECTION CODE_SECTION(".errors")

	namespace Internal
	{
		template<typename Callback>
		auto NO_INLINE UNLIKELY_ERROR_SECTION COLD_FUNCTION ColdError(Callback&& callback)
		{
			return callback();
		}
	}

#define COLD_ERROR_LOGIC(callback) \
	ngine::Internal::ColdError( \
		[=]() NO_INLINE UNLIKELY_ERROR_SECTION COLD_FUNCTION \
		{ \
			return (callback)(); \
		} \
	)
}
