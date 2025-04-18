#pragma once

#include <Common/Platform/ForceInline.h>
#include <Common/Platform/Pure.h>
#include <Common/Platform/NoDebug.h>
#include <Common/Math/CoreNumericTypes.h>

namespace ngine::Memory
{
	[[nodiscard]] FORCE_INLINE PURE_STATICS NO_DEBUG size GetAlignment(const void* pAddress)
	{
		const uintptr address = reinterpret_cast<uintptr>(pAddress);
		// Extract the largest power of two
		return address & (~address + 1);
	}
}
