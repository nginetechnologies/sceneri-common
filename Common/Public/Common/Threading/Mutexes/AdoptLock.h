#pragma once

namespace ngine::Threading
{
	enum class AdoptLockType : uint8
	{
		AdoptLock
	};
	inline static constexpr AdoptLockType AdoptLock = AdoptLockType::AdoptLock;
}
