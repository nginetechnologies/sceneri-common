#pragma once

#include <Common/Platform/StaticUnreachable.h>

namespace ngine::Threading
{
	template<typename Type, typename = void>
	struct Atomic
	{
		// Intentionally not implementing generic version of Atomic backed by locks
		// Our Atomic API wants to guarantee lock-free versions and not sometimes implicitly lock
		static_unreachable("Atomic type not supported!");
	};
}
