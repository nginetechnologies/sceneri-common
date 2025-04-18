#include "Threading/Jobs/Job.h"
#include "Threading/Jobs/JobRunnerThread.h"
#include "Threading/Jobs/JobManager.h"

#include <Common/Math/Random.h>

namespace ngine::Threading
{
	void Job::FlagQueued()
	{
		Assert(!m_stateFlags.IsSet(StateFlags::Destroying));
		StateFlags expected = StateFlags::None;
		[[maybe_unused]] const bool exchanged = m_stateFlags.CompareExchangeStrong(expected, StateFlags::Queued);
		Assert(exchanged);
	}

	void Job::Queue(JobRunnerThread& thread)
	{
		Assert(!m_stateFlags.IsSet(StateFlags::Destroying));
		StateFlags expected = StateFlags::None;
		[[maybe_unused]] const bool exchanged = m_stateFlags.CompareExchangeStrong(expected, StateFlags::Queued);
		Assert(exchanged);
		if (LIKELY(exchanged))
		{
			if ((GetAllowedJobRunnerMask() & (1ull << thread.GetThreadIndex())) != 0)
			{
				if (!thread.QueueJobFromThread(*this))
				{
					JobRunnerThread::QueueOnIdealRunner(thread.GetJobManager(), *this);
				}
			}
			else
			{
				JobRunnerThread::QueueOnIdealRunner(thread.GetJobManager(), *this);
			}
		}
	}

	void Job::Queue(JobManager& manager)
	{
		Assert(!m_stateFlags.IsSet(StateFlags::Destroying));
		StateFlags expected = StateFlags::None;
		[[maybe_unused]] const bool exchanged = m_stateFlags.CompareExchangeStrong(expected, StateFlags::Queued);
		Assert(exchanged);
		if (LIKELY(exchanged))
		{
			Optional<Threading::JobRunnerThread*> pThread = Threading::JobRunnerThread::GetCurrent();
			if (pThread.IsValid() && (GetAllowedJobRunnerMask() & (1ull << pThread->GetThreadIndex())) != 0)
			{
				if (!pThread->QueueJobFromThread(*this))
				{
					JobRunnerThread::QueueOnIdealRunner(manager, *this);
				}
			}
			else
			{
				JobRunnerThread::QueueOnIdealRunner(manager, *this);
			}
		}
	}

	void Job::QueueExclusiveFromCurrentThread(JobRunnerThread& thread)
	{
		Assert((GetAllowedJobRunnerMask() & (1ull << thread.GetThreadIndex())) != 0);
		Assert(!m_stateFlags.IsSet(StateFlags::Destroying));
		StateFlags expected = StateFlags::None;
		[[maybe_unused]] const bool exchanged = m_stateFlags.CompareExchangeStrong(expected, StateFlags::Queued);
		Assert(exchanged);
		if (LIKELY(exchanged))
		{
			thread.QueueExclusiveJobFromThread(*this);
		}
	}

	void Job::QueueExclusiveFromAnyThread(JobRunnerThread& thread)
	{
		Assert((GetAllowedJobRunnerMask() & (1ull << thread.GetThreadIndex())) != 0);
		Assert(!m_stateFlags.IsSet(StateFlags::Destroying));
		StateFlags expected = StateFlags::None;
		[[maybe_unused]] const bool exchanged = m_stateFlags.CompareExchangeStrong(expected, StateFlags::Queued);
		Assert(exchanged);
		if (LIKELY(exchanged))
		{
			thread.QueueExclusiveJobFromAnyThread(*this);
		}
	}

	void Job::TryQueue(JobRunnerThread& thread)
	{
		const EnumFlags<StateFlags> previousValue = m_stateFlags.FetchOr(StateFlags::Queued);
		if (!previousValue.IsSet(StateFlags::Queued))
		{
			const bool canExecuteOnThread = (GetAllowedJobRunnerMask() & (1ull << thread.GetThreadIndex())) != 0;
			if (!canExecuteOnThread || !thread.QueueJobFromThread(*this))
			{
				JobRunnerThread::QueueOnIdealRunner(thread.GetJobManager(), *this);
			}
		}
	}

	void Job::TryQueue(JobManager& manager)
	{
		Assert(!m_stateFlags.IsSet(StateFlags::Destroying));
		const EnumFlags<StateFlags> previousValue = m_stateFlags.FetchOr(StateFlags::Queued);
		if (!previousValue.IsSet(StateFlags::Queued))
		{
			Optional<Threading::JobRunnerThread*> pThread = Threading::JobRunnerThread::GetCurrent();
			if (pThread.IsValid() && (GetAllowedJobRunnerMask() & (1ull << pThread->GetThreadIndex())) != 0)
			{
				if (!pThread->QueueJobFromThread(*this))
				{
					JobRunnerThread::QueueOnIdealRunner(manager, *this);
				}
			}
			else
			{
				JobRunnerThread::QueueOnIdealRunner(manager, *this);
			}
		}
	}

	void Job::TryQueueExclusiveFromThread(JobRunnerThread& thread)
	{
		Assert((GetAllowedJobRunnerMask() & (1ull << thread.GetThreadIndex())) != 0);
		Assert(!m_stateFlags.IsSet(StateFlags::Destroying));
		const EnumFlags<StateFlags> previousValue = m_stateFlags.FetchOr(StateFlags::Queued);
		if (!previousValue.IsSet(StateFlags::Queued))
		{
			thread.QueueExclusiveJobFromThread(*this);
		}
	}

	void Job::TryQueueExclusiveFromAnyThread(JobRunnerThread& thread)
	{
		Assert((GetAllowedJobRunnerMask() & (1ull << thread.GetThreadIndex())) != 0);
		Assert(!m_stateFlags.IsSet(StateFlags::Destroying));
		const EnumFlags<StateFlags> previousValue = m_stateFlags.FetchOr(StateFlags::Queued);
		if (!previousValue.IsSet(StateFlags::Queued))
		{
			thread.QueueExclusiveJobFromAnyThread(*this);
		}
	}

	void Job::SetExclusiveToThread(JobRunnerThread& thread)
	{
		m_allowedJobRunnerMask = 1ull << thread.GetThreadIndex();
	}
}
