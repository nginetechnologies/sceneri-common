#include "Threading/Thread.h"

#if PLATFORM_WINDOWS
#include <Common/Platform/Windows.h>
#elif PLATFORM_APPLE
#include <pthread.h>
#include <mach/thread_act.h>
#elif USE_PTHREAD
#include <sched.h>
#include <pthread.h>
#endif

#if PLATFORM_EMSCRIPTEN
#include <emscripten/threading.h>
#endif

#include <Common/Memory/Containers/Array.h>
#include <Common/Math/Floor.h>
#include <Common/Platform/IsDebuggerAttached.h>

namespace ngine::Threading
{
	ThreadId Thread::GetThreadId() const
	{
#if USE_PTHREAD
		return ThreadId(m_thread);
#elif USE_WEB_WORKERS
		return (uint32)m_thread;
#else
		static_assert(sizeof(ThreadId) == sizeof(std::thread::id));
		static_assert(alignof(ThreadId) == alignof(std::thread::id));
		const std::thread::id threadId = m_thread.get_id();
		return *reinterpret_cast<const ThreadId*>(&threadId);
#endif
	}

	void Thread::ForceKill()
	{
#if PLATFORM_WINDOWS
		SuspendThread(m_thread.native_handle());
		TerminateThread(m_thread.native_handle(), 1);
#elif PLATFORM_ANDROID
		Assert(false, "Unsupported");
#elif USE_PTHREAD
		pthread_cancel(m_thread);
#elif USE_WEB_WORKERS
		Assert(!IsRunningOnThread(), "Worker cannot terminate itself!");
		emscripten_terminate_wasm_worker(m_thread);
#elif PLATFORM_POSIX
		pthread_cancel(m_thread.native_handle());
#else
#error "Force thread kill not implemented for platform!"
#endif

		Detach();
	}

	void Thread::SetThreadName(const ConstNativeZeroTerminatedStringView name)
	{
#if PLATFORM_WINDOWS
		SetThreadDescription(m_thread.native_handle(), name.GetData());
#elif PLATFORM_APPLE
		pthread_setname_np(name.GetData());
#elif PLATFORM_EMSCRIPTEN
		emscripten_set_thread_name(pthread_self(), name.GetData());
#elif USE_PTHREAD
		pthread_setname_np(m_thread, name.GetData());
#endif
	}

	/* static */ void Thread::SetAffinityMask(const ThreadId id, const uint64 mask)
	{
#if PLATFORM_WINDOWS
		SetThreadAffinityMask(id.GetHandle(), mask);
#elif PLATFORM_APPLE
		Array<thread_affinity_policy, 64> affinityTags;
		affinityTags.GetView().ZeroInitialize();

		uint8 coreCount = 0;
		for (uint8 i = 0; i < 64; ++i)
		{
			if ((mask >> i) & 1)
			{
				affinityTags[coreCount++].affinity_tag = i;
			}
		}

		thread_policy_set(
			pthread_mach_thread_np(static_cast<pthread_t>(id.GetHandle())),
			THREAD_AFFINITY_POLICY,
			(thread_policy_t)affinityTags.GetData(),
			coreCount
		);
#elif USE_WEB_WORKERS
		UNUSED(id);
		UNUSED(mask);
#elif USE_PTHREAD
		cpu_set_t cpuMask;
		CPU_ZERO(&cpuMask);
		for (uint32 i = 0; i < sizeof(cpuMask) * 8; ++i)
		{
			if (mask & (1 << i))
			{
				CPU_SET(i, &cpuMask);
			}
		}

		sched_setaffinity(id.GetId(), sizeof(cpuMask), &cpuMask);
#endif
	}

	void Thread::SetAffinityMask(const uint64 mask)
	{
		SetAffinityMask(ThreadId::Get(GetThreadHandle()), mask);
	}

	void Thread::SetPriority(const Priority priority, const float ratio)
	{
		Assert(priority != m_priority || m_priorityRatio != ratio);
		[[maybe_unused]] const Priority previousPriority = m_priority;
		m_priority = priority;
		m_priorityRatio = ratio;

#if PLATFORM_WINDOWS
		if (priority != previousPriority)
		{
			int threadPriority;
			switch (priority)
			{
				case Priority::UserInteractive:
					threadPriority = THREAD_PRIORITY_HIGHEST;
					break;
				case Priority::UserInitiated:
					threadPriority = THREAD_PRIORITY_ABOVE_NORMAL;
					break;
				case Priority::Default:
					threadPriority = THREAD_PRIORITY_NORMAL;
					break;
				case Priority::UserVisibleBackground:
					threadPriority = THREAD_PRIORITY_BELOW_NORMAL;
					break;
				case Priority::Background:
					threadPriority = THREAD_MODE_BACKGROUND_BEGIN;
					break;
				default:
					ExpectUnreachable();
			}

			HANDLE hThread = m_thread.native_handle();
			if (hThread == 0) // main thread
			{
				hThread = GetCurrentThread();
			}

			// Leave background processing mode if we had entered it
			if (priority != Priority::Background && previousPriority == Priority::Background)
			{
				SetThreadPriority(hThread, THREAD_MODE_BACKGROUND_END);

				// Turn EcoQoS off to enable running on performance cores
				THREAD_POWER_THROTTLING_STATE PowerThrottling;
				ZeroMemory(&PowerThrottling, sizeof(PowerThrottling));
				PowerThrottling.Version = THREAD_POWER_THROTTLING_CURRENT_VERSION;
				PowerThrottling.ControlMask = THREAD_POWER_THROTTLING_EXECUTION_SPEED;
				PowerThrottling.StateMask = 0;
				SetThreadInformation(hThread, ThreadPowerThrottling, &PowerThrottling, sizeof(PowerThrottling));
			}
			else if (priority == Priority::Background && previousPriority != Priority::Background)
			{
				// Turn EcoQoS on to enable running on efficiency cores
				THREAD_POWER_THROTTLING_STATE PowerThrottling;
				ZeroMemory(&PowerThrottling, sizeof(PowerThrottling));
				PowerThrottling.Version = THREAD_POWER_THROTTLING_CURRENT_VERSION;
				PowerThrottling.ControlMask = THREAD_POWER_THROTTLING_EXECUTION_SPEED;
				PowerThrottling.StateMask = THREAD_POWER_THROTTLING_EXECUTION_SPEED;
				SetThreadInformation(hThread, ThreadPowerThrottling, &PowerThrottling, sizeof(PowerThrottling));
			}

			SetThreadPriority(hThread, threadPriority);
		}
#elif USE_WEB_WORKERS
		UNUSED(priority);
		UNUSED(ratio);
#elif PLATFORM_APPLE
		// IsDebuggerAttach here slows down the code dramatically, cache and only check once at startup
		// Only consequence is that set priority will slow down execution when attaching late.
		static thread_local const bool isDebuggerAttached = IsDebuggerAttached();
		if (IsRunningOnThread() && !isDebuggerAttached)
		{
			qos_class_t qualityOfServiceClass;
			switch (priority)
			{
				case Priority::UserInteractive:
					qualityOfServiceClass = QOS_CLASS_USER_INTERACTIVE;
					break;
				case Priority::UserInitiated:
					qualityOfServiceClass = QOS_CLASS_USER_INITIATED;
					break;
				case Priority::Default:
					qualityOfServiceClass = QOS_CLASS_DEFAULT;
					break;
				case Priority::UserVisibleBackground:
					qualityOfServiceClass = QOS_CLASS_UTILITY;
					break;
				case Priority::Background:
					qualityOfServiceClass = QOS_CLASS_BACKGROUND;
					break;
			}

			const int relativePriority = (int)Math::Floor(QOS_MIN_RELATIVE_PRIORITY * (1.0f - ratio));
			[[maybe_unused]] const int result = pthread_set_qos_class_self_np(qualityOfServiceClass, relativePriority);
			Assert(result == 0);
		}
#elif USE_PTHREAD
		int policy;
		sched_param param;

		pthread_getschedparam(m_thread, &policy, &param);

		if (m_priorityRatio <= IdlePriorityPercentage)
		{
			policy = SCHED_IDLE;
		}
		else if (m_priorityRatio <= LowPriorityPercentage)
		{
			policy = SCHED_BATCH;
		}
		else if (m_priorityRatio <= NormalPriorityPercentage)
		{
			policy = SCHED_OTHER;
		}
		else if (m_priorityRatio <= HighPriorityPercentage)
		{
			policy = SCHED_FIFO;
		}
		else
		{
			policy = SCHED_RR;
		}

		param.sched_priority = int(float(sched_get_priority_max(policy) - sched_get_priority_min(policy)) * m_priorityRatio);
		pthread_setschedparam(m_thread, policy, &param);
#endif
	}
}
