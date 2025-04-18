#pragma once

#include <Common/Platform/ForceInline.h>

namespace ngine
{
	[[nodiscard]] constexpr FORCE_INLINE bool IsConstantEvaluated()
	{
#if COMPILER_CLANG || COMPILER_GCC || COMPILER_MSVC
		return __builtin_is_constant_evaluated();
#else
		return false;
#endif
	}
}
