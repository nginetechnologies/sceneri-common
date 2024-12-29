#include <Common/Platform/IsDebuggerAttached.h>
#include <Common/Memory/Containers/StringView.h>
#include <Common/IO/File.h>
#include <Common/IO/Path.h>
#include <Common/EnumFlags.h>

#if PLATFORM_WINDOWS
#include <Common/Platform/Windows.h>
#include <debugapi.h>
#include <errhandlingapi.h>
#elif PLATFORM_POSIX
#include <signal.h>
#endif

#if PLATFORM_APPLE
#include <assert.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/sysctl.h>
#include <mach/mach.h>
#include <mach/task_info.h>
#endif

#if PLATFORM_EMSCRIPTEN
#include <emscripten.h>
#endif

#if SEPARATE_DEBUG_BREAK
bool BreakIntoDebugger()
{
#if PLATFORM_X86
	__asm__ volatile("int $0x03");
#elif COMPILER_CLANG && PLATFORM_APPLE
	__builtin_debugtrap();
#elif PLATFORM_ANDROID || PLATFORM_LINUX
	raise(SIGTRAP);
#elif PLATFORM_EMSCRIPTEN
	emscripten_debugger();
#elif PLATFORM_ARM
#if PLATFORM_64BIT
	__asm__ volatile(".inst 0xd4200000");
#else
	__asm__ volatile(".inst 0xe7f001f0");
#endif
#else
	raise(SIGTRAP);
#endif
	return true;
}
#endif

bool IsDebuggerAttached()
{
	if constexpr (PROFILE_BUILD || (ENABLE_ASSERTS && !PLATFORM_WEB))
	{
#if PLATFORM_WINDOWS
		return IsDebuggerPresent();
#elif PLATFORM_APPLE
		task_dyld_info dyldInfo;
		mach_msg_type_number_t count = TASK_DYLD_INFO_COUNT;
		kern_return_t result = task_info(mach_task_self(), TASK_DYLD_INFO, (task_info_t)&dyldInfo, &count);

		if (result == KERN_SUCCESS && dyldInfo.all_image_info_format != 0)
		{
			int mib[4];
			struct kinfo_proc info;
			size_t size;

			info.kp_proc.p_flag = 0;
			mib[0] = CTL_KERN;
			mib[1] = KERN_PROC;
			mib[2] = KERN_PROC_PID;
			mib[3] = getpid();

			size = sizeof(info);
			sysctl(mib, sizeof(mib) / sizeof(*mib), &info, &size, NULL, 0);

			return ((info.kp_proc.p_flag & P_TRACED) != 0);
		}
		else
		{
			return false;
		}
#elif PLATFORM_ANDROID || PLATFORM_LINUX
		using namespace ngine;
		IO::File file(IO::Path(MAKE_PATH("/proc/self/status")), IO::AccessModeFlags::Read);
		if (LIKELY(file.IsValid()))
		{
			constexpr ngine::uint32 bufferSize = 10240;
			char buffer[bufferSize];
			while (file.ReadLineIntoView(ArrayView<char>{buffer, bufferSize}))
			{
				constexpr ConstStringView prefix("TracerPid:\t");
				const ConstStringView bufferView{buffer, prefix.GetSize()};
				if (prefix == bufferView)
				{
					// Report that we are being debugged if the tracer PID is anything other than 0
					return buffer[prefix.GetSize()] != '0';
				}
			}
		}
		return false;
#elif PLATFORM_EMSCRIPTEN
		return true;
#endif
	}
	else
	{
		return false;
	}
}

namespace ngine
{
	void RaiseException()
	{
#if PLATFORM_WINDOWS
		::RaiseException(0, 0, 0, nullptr);
#else
		raise(SIGSEGV);
#endif
	}
}
