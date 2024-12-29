#pragma once

namespace ngine::Threading
{
	enum class TryLockType : uint8
	{
		TryLock
	};

	inline static constexpr TryLockType TryLock = TryLockType::TryLock;
}
