#pragma once

namespace ngine::ThreadSafe
{
	template<typename SignatureType, size StorageSizeBytes = 0, bool RequireUniqueIdentifiers = true>
	struct Event;
}
