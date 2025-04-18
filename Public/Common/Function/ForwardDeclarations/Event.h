#pragma once

#include <Common/Math/CoreNumericTypes.h>

namespace ngine
{
	template<typename SignatureType, size StorageSizeBytes = 0, bool RequireUniqueIdentifiers = true>
	struct Event;
}
