#pragma once

#include <Common/AtomicEnumFlags.h>
#include <Common/EnumFlagOperators.h>
#include <Common/Memory/ReferenceWrapper.h>
#include <Common/Memory/Containers/Vector.h>
#include <Common/Memory/UniqueRef.h>
#include <Common/Memory/Allocators/Pool.h>
#include <Common/Memory/DynamicBitset.h>
#include <Common/Threading/AtomicInteger.h>
#include <Common/Threading/Thread.h>
#include <Common/Threading/Jobs/JobBatch.h>

#if USE_PRIORITY_QUEUE
#include <Common/Memory/Containers/PriorityQueue.h>
#endif

#include "JobPriority.h"

#include <Common/Threading/Mutexes/ConditionVariable.h>

namespace ngine::Threading
{
	struct JobManager;
	struct Job;

	[[nodiscard]] extern Thread::Priority GetThreadPriorityFromJobPriority(const JobPriority jobPriority);

	struct JobRunnerThread : public Threading::ThreadWithRunMember<JobRunnerThread>
	{
		enum class Flags : uint16
		{
			IsQuitting = 1 << 0,
			IsStartingThread = 1 << 1,
			IsIdle = 1 << 2,
			CanRunHighPriorityPerformanceJobs = 1 << 3,
			CanRunLowPriorityPerformanceJobs = 1 << 4,
			CanRunEfficiencyJobs = 1 << 5,
			HasJobsInLocalQueue = 1 << 6,
			HasJobsInLocalExclusiveQueue = 1 << 7,
			HasJobsInThreadSafeQueue = 1 << 8,
			HasJobsInThreadSafeExclusiveQueue = 1 << 9,
			HasAnyJobsInQueues = HasJobsInLocalQueue | HasJobsInLocalExclusiveQueue | HasJobsInThreadSafeQueue |
			                     HasJobsInThreadSafeExclusiveQueue,
			HasAnyJobsInThreadSafeQueues = HasJobsInThreadSafeQueue | HasJobsInThreadSafeExclusiveQueue,
			HasNextJob = 1 << 10,
			HasWork = HasAnyJobsInQueues | HasNextJob
		};
	protected:
		using BaseType = Threading::ThreadWithRunMember<JobRunnerThread>;

#if !USE_PRIORITY_QUEUE
		using JobQueue = Vector<ReferenceWrapper<Job>, uint16>;
		using JobView = JobQueue::View;
#endif

		using ExclusiveJobQueue = Vector<ReferenceWrapper<Job>, uint16>;
		using ThreadSafeQueue = Vector<ReferenceWrapper<Job>, uint16>;
	public:
		using ThreadIndexType = uint8;

		JobRunnerThread(JobManager& manager);
		JobRunnerThread(const JobRunnerThread&) = delete;
		JobRunnerThread& operator=(const JobRunnerThread&) = delete;
		JobRunnerThread(JobRunnerThread&&) = delete;
		JobRunnerThread& operator=(JobRunnerThread&&) = delete;
		virtual ~JobRunnerThread();

		void StartThread(const ConstNativeStringView name, const ThreadIndexType threadIndex);
		void StopThread();
		void InitializeMainThread();
		void Run();
		void Tick();

		void MoveThreadSafeQueueElements();
		void UpdateNextJob();
		[[nodiscard]] PURE_STATICS bool HasNextJob() const
		{
			return m_pNextJob != nullptr;
		}
		[[nodiscard]] PURE_STATICS bool HasWork() const
		{
			return m_flags.IsSet(Flags::HasWork);
		}
		void RunNextJob();
		void Wake();
		bool DoRunNextJob();
		[[nodiscard]] PURE_STATICS bool IsExecutingOnThread() const;
		bool IsRunningOnThread() const = delete;
		[[nodiscard]] bool IsIdle() const
		{
			return m_flags.IsSet(Flags::IsIdle);
		}
		void UpdateIdleState();

		[[nodiscard]] PURE_STATICS static Optional<JobRunnerThread*>& GetCurrent()
		{
			static thread_local Optional<JobRunnerThread*> pThread = nullptr;
			return pThread;
		}

		[[nodiscard]] PURE_STATICS ThreadId GetThreadId() const
		{
			return m_threadId;
		}

		[[nodiscard]] PURE_STATICS JobManager& GetJobManager() const
		{
			return m_manager;
		}
		[[nodiscard]] PURE_STATICS ThreadIndexType GetThreadIndex() const
		{
			return m_threadIndex;
		}

		void MakeLowPriorityPerformanceTaskRunner()
		{
			m_flags |= Flags::CanRunLowPriorityPerformanceJobs;
		}
		[[nodiscard]] bool CanRunLowPriorityPerformanceJobs()
		{
			return m_flags.IsSet(Flags::CanRunLowPriorityPerformanceJobs);
		}
		void MakeHighPriorityPerformanceTaskRunner()
		{
			m_flags |= Flags::CanRunHighPriorityPerformanceJobs;
		}
		[[nodiscard]] bool CanRunHighPriorityPerformanceJobs()
		{
			return m_flags.IsSet(Flags::CanRunHighPriorityPerformanceJobs);
		}
		void MakeEfficiencyTaskRunner()
		{
			m_flags |= Flags::CanRunEfficiencyJobs;
		}
		[[nodiscard]] bool CanRunEfficiencyJobs()
		{
			return m_flags.IsSet(Flags::CanRunEfficiencyJobs);
		}

		template<typename Callback>
		void QueueCallbackFromThread(const JobPriority priority, Callback&& callback, const ConstStringView name = {});
		template<typename Callback>
		void QueueExclusiveCallbackFromThread(const JobPriority priority, Callback&& callback, const ConstStringView name = {});
		template<typename Callback>
		void QueueExclusiveCallbackFromAnyThread(const JobPriority priority, Callback&& callback, const ConstStringView name = {});

		void QueueJobsFromThread(const JobView jobs);
		void TryQueueJobsFromThread(const JobView jobs);

		void Queue(const Threading::JobBatch& jobBatch)
		{
			Assert(jobBatch.IsValid());
			if (Optional<Threading::Job*> pJob = jobBatch.GetStartJob())
			{
				pJob->Queue(*this);
			}
			else
			{
				Expect(jobBatch.HasStartStage());
				jobBatch.GetStartStage().SignalExecutionFinishedAndDestroying(*this);
			}
		}

#define ENABLE_THREAD_MEMORY_POOL 0
#if ENABLE_THREAD_MEMORY_POOL
		inline static constexpr size PoolSize = 1024 * 1024 * 1;

		template<size Size, size Alignment>
		using MemoryPool = Memory::FixedPool<Size, Size / Alignment, Alignment>;

		//! Allocates memory from the thread only pool
		//! Allocations and frees of this memory are only allowed from this thread
		template<size Alignment>
		[[nodiscard]] RESTRICTED_RETURN void* Allocate(size)
		{
			static_unreachable("Memory pool with specified alignment is not implemented!");
		}

		template<>
		[[nodiscard]] RESTRICTED_RETURN void* Allocate<8>(const size requestedSize)
		{
			return m_memoryPool8.Allocate(requestedSize);
		}

		template<>
		[[nodiscard]] RESTRICTED_RETURN void* Allocate<16>(const size requestedSize)
		{
			return m_memoryPool16.Allocate(requestedSize);
		}

		//! Deallocates memory back into the thread only pool
		//! Allocations and frees of this memory are only allowed from this thread
		template<size Alignment>
		[[nodiscard]] void Deallocate(void*)
		{
			static_unreachable("Memory pool with specified alignment is not implemented!");
		}

		template<>
		[[nodiscard]] void Deallocate<8>(void* const pPointer)
		{
			m_memoryPool8.Deallocate(pPointer);
		}

		template<>
		[[nodiscard]] void Deallocate<16>(void* const pPointer)
		{
			m_memoryPool16.Deallocate(pPointer);
		}

		//! Allocates memory from the thread only pool
		//! Allocations are only allowed from this thread
		template<size Alignment>
		[[nodiscard]] RESTRICTED_RETURN void* AllocateThreadSafe(size)
		{
			static_unreachable("Memory pool with specified alignment is not implemented!");
		}

		template<>
		[[nodiscard]] RESTRICTED_RETURN void* AllocateThreadSafe<8>(const size requestedSize)
		{
			// TODO: Deeper integration into memory pool to only lock where possible
			Threading::UniqueLock lock(m_memoryPool8Mutex);
			return m_threadSafeMemoryPool8.Allocate(requestedSize);
		}

		template<>
		[[nodiscard]] RESTRICTED_RETURN void* AllocateThreadSafe<16>(const size requestedSize)
		{
			Threading::UniqueLock lock(m_memoryPool16Mutex);
			return m_threadSafeMemoryPool16.Allocate(requestedSize);
		}

		//! Deallocates memory back into the thread only pool
		//! Deallocations are allowed from any thread
		template<size Alignment>
		[[nodiscard]] void DeallocateThreadSafe(void*)
		{
			static_unreachable("Memory pool with specified alignment is not implemented!");
		}

		template<>
		[[nodiscard]] void DeallocateThreadSafe<8>(void* const pPointer)
		{
			Threading::UniqueLock lock(m_memoryPool8Mutex);
			m_threadSafeMemoryPool8.Deallocate(pPointer);
		}

		template<>
		[[nodiscard]] void DeallocateThreadSafe<16>(void* const pPointer)
		{
			Threading::UniqueLock lock(m_memoryPool16Mutex);
			m_threadSafeMemoryPool16.Deallocate(pPointer);
		}
#endif
	protected:
		bool TrySetNextJob(Job& job);
		void ShareJobsWithIdleThreads(JobView& jobs, const uint16 totalExistingJobCount) const;
		[[nodiscard]] bool ShareJobWithFirstIdleThread(Job& job) const;
		void GiveWorkToOtherThreads();

		[[nodiscard]] bool QueueJobFromThread(Job& __restrict job);
		[[nodiscard]] bool QueueJobFromAnyThread(Job& __restrict job);
		void QueueExclusiveJobFromThread(Job& __restrict job);
		void QueueExclusiveJobFromAnyThread(Job& __restrict job);

		void QueueJobImmediate(Job& __restrict job);
		void QueueExclusiveJobImmediate(Job& __restrict job);
		void QueueJobImmediateThreadSafe(Job& __restrict job);

		static void QueueOnIdealRunner(JobManager& manager, Job& job);

		struct ThreadSafeQueueView final : public ThreadSafeQueue::View
		{
			ThreadSafeQueueView(
				Threading::Mutex& mutex, const typename ThreadSafeQueueView::iterator begin, const typename ThreadSafeQueueView::iterator end
			)
				: ThreadSafeQueue::View(begin, end)
				, m_lock(mutex)
			{
				if (UNLIKELY_ERROR(!m_lock.IsLocked()))
				{
					ThreadSafeQueue::View::operator=({});
				}
			}
		private:
			Threading::UniqueLock<Threading::Mutex> m_lock;
		};

		[[nodiscard]] ThreadSafeQueueView GetThreadSafeQueueView()
		{
			return {m_threadSafeQueueMutex, m_threadSafeQueue.begin(), m_threadSafeQueue.end()};
		}

		[[nodiscard]] ThreadSafeQueueView GetThreadSafeExclusiveQueueView()
		{
			return {m_threadSafeQueueMutex, m_threadSafeExclusiveQueue.begin(), m_threadSafeExclusiveQueue.end()};
		}

		[[nodiscard]] uint16 GetThreadSafeQueueSize() const
		{
			return m_threadSafeQueueSize;
		}

		[[nodiscard]] bool HasThreadSafeQueueElements() const
		{
			return GetThreadSafeQueueSize() > 0;
		}

		[[nodiscard]] uint16 GetQueuedJobCount()
		{
			return GetThreadSafeQueueSize() + m_queue.GetSize() + m_exclusiveQueue.GetSize() + (m_pNextJob != nullptr);
		}

		void SetDefaultPriority();
		void UpdatePriorityForJob(const Job& job);
	protected:
		friend Job;

		JobManager& m_manager;
		ThreadIndexType m_threadIndex = Math::NumericLimits<ThreadIndexType>::Max;
		ThreadId m_threadId;
		AtomicEnumFlags<Flags> m_flags;

		Threading::ConditionVariable m_idleConditionVariable;

#if USE_PRIORITY_QUEUE
		TPriorityQueue<Job*, 1024, JobPriority> m_queue;
#else
		JobQueue m_queue;
#endif
		Job* m_pNextJob = nullptr;

		ExclusiveJobQueue m_exclusiveQueue;

		mutable Threading::Mutex m_threadSafeQueueMutex;
		ThreadSafeQueue m_threadSafeQueue;
		ThreadSafeQueue m_threadSafeExclusiveQueue;
		Threading::Atomic<uint16> m_threadSafeQueueSize = 0;

		mutable DynamicBitset<uint16> m_triviallyShareableJobs;

#if ENABLE_THREAD_MEMORY_POOL
		MemoryPool<PoolSize, 8> m_memoryPool8;
		MemoryPool<PoolSize, 16> m_memoryPool16;
		MemoryPool<PoolSize, 8> m_threadSafeMemoryPool8;
		Threading::Mutex m_memoryPool8Mutex;
		MemoryPool<PoolSize, 16> m_threadSafeMemoryPool16;
		Threading::Mutex m_memoryPool16Mutex;
#endif
	};

	ENUM_FLAG_OPERATORS(JobRunnerThread::Flags);
}
