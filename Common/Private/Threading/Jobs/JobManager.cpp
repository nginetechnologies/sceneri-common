#include "Threading/Jobs/JobManager.h"
#include "Threading/Jobs/JobRunnerThread.h"
#include "Threading/Jobs/Job.h"
#include "Threading/Jobs/TimersJob.h"

#include <Common/Memory/Containers/FlatString.h>
#include <Common/Memory/Containers/Format/StringView.h>
#include <Common/Memory/Containers/Format/String.h>
#include <Common/Memory/OffsetOf.h>
#include <Common/Math/Hash.h>
#include <Common/Math/Floor.h>
#include <Common/Math/Ceil.h>
#include <Common/Math/Max.h>
#include <Common/Platform/GetProcessorCoreTypes.h>

#if PLATFORM_WINDOWS
#include <Common/Platform/Windows.h>
#endif

#include <thread>

namespace ngine::Threading
{
	JobManager::JobManager()
		: m_jobThreads(Memory::Reserve, (JobThreads::SizeType)Math::Min(std::thread::hardware_concurrency(), MaximumRunnerCount))
		, m_pTimersJob(UniqueRef<TimersJob>::Make())
	{
	}

	JobManager::~JobManager()
	{
		DestroyJobRunners();
	}

	void JobManager::StartRunners()
	{
		const uint16 logicalPerformanceCoreCount = Math::Max(Platform::GetLogicalPerformanceCoreCount(), (uint16)1u);
		const uint16 logicalEfficiencyCoreCount = Math::Max(Platform::GetLogicalEfficiencyCoreCount(), (uint16)1u);

		const uint16 totalWorkerCount = (uint16)(logicalPerformanceCoreCount + logicalEfficiencyCoreCount);

		const Math::Ratiof highPriorityWorkerRatio = 90_percent;
		const uint16 numHighPriorityWorkers = (uint16)Math::Max(Math::Floor((float)logicalPerformanceCoreCount * highPriorityWorkerRatio), 1.f);
		const Math::Range<uint16> highPriorityWorkerRange = Math::Range<uint16>::Make(0, uint16(numHighPriorityWorkers));
		Math::Range<uint16> lowPriorityPerformanceWorkerRange =
			Math::Range<uint16>::Make(numHighPriorityWorkers, logicalPerformanceCoreCount - numHighPriorityWorkers);
		Math::Range<uint16> efficiencyWorkerRange = Math::Range<uint16>::Make(logicalPerformanceCoreCount, logicalEfficiencyCoreCount);

		if (lowPriorityPerformanceWorkerRange.GetSize() == 0)
		{
			lowPriorityPerformanceWorkerRange = highPriorityWorkerRange;
		}
		if (efficiencyWorkerRange.GetSize() == 0)
		{
			efficiencyWorkerRange = lowPriorityPerformanceWorkerRange;
		}

		StartRunners(totalWorkerCount, highPriorityWorkerRange, lowPriorityPerformanceWorkerRange, efficiencyWorkerRange);
	}

	void JobManager::StartRunners(
		const uint16 count,
		const Math::Range<uint16> performanceHighPriorityWorkerRange,
		const Math::Range<uint16> performanceLowPriorityWorkerRange,
		const Math::Range<uint16> efficiencyWorkerRange
	)
	{
		CreateJobRunners(count);

		for (const uint16 threadIndex : performanceHighPriorityWorkerRange)
		{
			m_jobThreads[(uint8)threadIndex]->MakeHighPriorityPerformanceTaskRunner();
			m_performanceHighPriorityThreadMask |= (1ull << threadIndex);
		}
		for (const uint16 threadIndex : performanceLowPriorityWorkerRange)
		{
			m_jobThreads[(uint8)threadIndex]->MakeLowPriorityPerformanceTaskRunner();
			m_performanceLowPriorityThreadMask |= (1ull << threadIndex);
		}

		if (efficiencyWorkerRange.GetSize() > 0)
		{
			for (const uint16 threadIndex : efficiencyWorkerRange)
			{
				m_jobThreads[(uint8)threadIndex]->MakeEfficiencyTaskRunner();
				m_efficiencyThreadMask |= (1ull << threadIndex);
			}
		}
		else
		{
			for (const uint16 threadIndex : performanceLowPriorityWorkerRange)
			{
				m_jobThreads[(uint8)threadIndex]->MakeEfficiencyTaskRunner();
			}
			m_efficiencyThreadMask = m_performanceLowPriorityThreadMask;
		}

		Assert(m_performanceHighPriorityThreadMask != 0, "Must have at least one high performance runner!");
		Assert(m_performanceLowPriorityThreadMask != 0, "Must have at least one low performance runner!");
		Assert(m_efficiencyThreadMask != 0, "Must have at least one efficiency runner!");

		if constexpr (DEBUG_BUILD && ENABLE_ASSERTS)
		{
			for ([[maybe_unused]] Threading::JobRunnerThread& thread : m_jobThreads)
			{
				Assert(
					thread.CanRunLowPriorityPerformanceJobs() | thread.CanRunHighPriorityPerformanceJobs() | thread.CanRunEfficiencyJobs(),
					"Job runner must be able to run at least one type of job"
				);
			}
		}

#if PLATFORM_WINDOWS
		// Allow running on all threads
		SetProcessAffinityMask(GetCurrentProcess(), Math::NumericLimits<uint64>::Max);
#endif

		constexpr ConstStringView jobThreadNameBase = "Job Thread ";
		FlatNativeString<jobThreadNameBase.GetSize() + 3> jobName;

		m_jobThreads[0]->InitializeMainThread();

		// Start a thread each, but count main thread as one also
		const uint32 numJobThreads = count - 1u;
		for (uint8 i = 1; i <= numJobThreads; ++i)
		{
			jobName.Format("{}{}", jobThreadNameBase, i);

			m_jobThreads[i]->StartThread(jobName.GetView(), i);
		}

		m_jobThreadIdLookup[GetJobThreadIndexFromId(Threading::ThreadId::GetCurrent())] = &*m_jobThreads[0];

		for (uint8 i = 1; i <= numJobThreads; ++i)
		{
			m_jobThreadIdLookup[GetJobThreadIndexFromId(m_jobThreads[i]->GetThreadId())] = &*m_jobThreads[i];
		}

		// Make sure we don't have any collisions
		if constexpr (ENABLE_ASSERTS)
		{
			Assert(GetJobThread(Threading::ThreadId::GetCurrent()) == &*m_jobThreads[0]);

			for (uint8 i = 1; i <= numJobThreads; ++i)
			{
				Assert(GetJobThread(m_jobThreads[i]->GetThreadId()) == &*m_jobThreads[i]);
			}
		}
	}

	void JobManager::CreateJobRunners(const uint16 count)
	{
		m_pJobThreadData = Memory::AllocateAligned(count * sizeof(Threading::JobRunnerThread), alignof(Threading::JobRunnerThread));
		char* pData = reinterpret_cast<char*>(m_pJobThreadData);
		for (uint16 i = 0; i < count; ++i)
		{
			new (pData) Threading::JobRunnerThread(*this);
			m_jobThreads.EmplaceBack(*reinterpret_cast<Threading::JobRunnerThread*>(pData));
			pData += sizeof(Threading::JobRunnerThread);
		}
	}

	void JobManager::DestroyJobRunners()
	{
		for (Threading::JobRunnerThread& thread : m_jobThreads)
		{
			thread.~JobRunnerThread();
		}

		Memory::DeallocateAligned(m_pJobThreadData, alignof(Threading::JobRunnerThread));
	}

	PURE_LOCALS_AND_POINTERS uint8 JobManager::GetJobThreadIndexFromId(const ThreadId id) const
	{
		const size hashedValue = Math::Hash(id.GetId());
		size index = hashedValue % m_jobThreadIdLookup.GetSize();
		while (m_jobThreadIdLookup[(uint8)index] != nullptr && m_jobThreadIdLookup[(uint8)index]->GetThreadId() != id)
		{
			index = (index + 1u) % m_jobThreadIdLookup.GetSize();
		}

		return static_cast<uint8>(index);
	}

	TimerHandle JobManager::ScheduleAsyncJob(const Time::Durationf delay, Job& job)
	{
		return m_pTimersJob->Schedule(delay, job, *this);
	}
	void JobManager::ScheduleAsyncJob(TimerHandle& handle, const Time::Durationf delay, Job& job)
	{
		m_pTimersJob->Schedule(handle, delay, job, *this);
	}

	bool JobManager::CancelAsyncJob(const TimerHandle& handle)
	{
		return m_pTimersJob->Cancel(handle);
	}
}
