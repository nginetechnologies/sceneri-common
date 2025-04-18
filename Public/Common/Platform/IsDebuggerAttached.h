#pragma once

#if PLATFORM_WINDOWS
#include <Common/Platform/UndefineWindowsMacros.h>
#endif

#include <Common/Platform/Pure.h>

[[nodiscard]] bool IsDebuggerAttached();

#if PLATFORM_WINDOWS
#define BreakIntoDebugger() (__debugbreak(), true)
#else
#define SEPARATE_DEBUG_BREAK 1
bool BreakIntoDebugger();
#endif

#define BreakIfDebuggerIsAttached() \
	if (IsDebuggerAttached()) \
	{ \
		BreakIntoDebugger(); \
	}
