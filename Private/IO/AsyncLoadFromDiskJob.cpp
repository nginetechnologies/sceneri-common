#include "IO/AsyncLoadFromDiskJob.h"

#include <Common/IO/File.h>
#include <Common/Threading/Mutexes/Mutex.h>

#if PLATFORM_WINDOWS
#include <Common/Platform/Windows.h>
#elif USE_APPLE_ASYNC_IO
#include <Common/System/Query.h>
#include <sys/stat.h>
#include <fcntl.h>
#import <Foundation/NSData.h>
#elif USE_POSIX_ASYNC_IO
#include <aio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#elif PLATFORM_EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/proxying.h>
#include <emscripten/threading.h>
#include <emscripten/fetch.h>
#include <emscripten/wasm_worker.h>
#include <Common/Threading/WebWorkerPool.h>
#include <Common/System/Query.h>
#endif

#include <Common/Threading/Jobs/JobRunnerThread.h>
#include <Common/Threading/Jobs/JobManager.h>
#include <Common/Threading/Jobs/JobRunnerThread.inl>

#include <Common/System/Query.h>

#include <Common/Memory/Containers/Format/String.h>

#include <algorithm>

namespace ngine::IO
{
	AsyncLoadFromDiskJob::AsyncLoadFromDiskJob(
		IO::Path&& path, AsyncLoadCallback&& callback, const ByteView target, const Math::Range<size> dataRange, const Priority priority
	)
		: Job(priority)
		, m_path(Forward<IO::Path>(path))
		, m_callback(Forward<AsyncLoadCallback>(callback))
		, m_target(target)
		, m_dataRange(dataRange)
	{
		Assert(m_path.HasElements());
	}

#if USE_POSIX_ASYNC_IO || USE_APPLE_ASYNC_IO || PLATFORM_WINDOWS
	static Threading::Atomic<uint32> openFileDescriptorCount = 0;
#endif

#if PLATFORM_EMSCRIPTEN
	using WebWorkerPool = Threading::WebWorkerPool<AsyncLoadFromDiskJob::WebWorkerIdentifier>;
	[[nodiscard]] WebWorkerPool& GetWebWorkerPool()
	{
		static WebWorkerPool workerPool;
		return workerPool;
	}
#endif

	static Threading::Mutex queuedAsyncLoadFromDiskJobsMutex;
	static Vector<ReferenceWrapper<AsyncLoadFromDiskJob>> queuedAsyncLoadFromDiskJobs;

#if USE_POSIX_ASYNC_IO || USE_APPLE_ASYNC_IO || PLATFORM_WINDOWS
	[[nodiscard]] bool TryStartAsyncLoading(AsyncLoadFromDiskJob& job)
	{
		const uint32 previousRequestCount = openFileDescriptorCount.FetchAdd(1);
		if (previousRequestCount >= AsyncLoadFromDiskJob::MaximumConcurrentRequestCount)
		{
			openFileDescriptorCount--;
			Threading::UniqueLock lock(queuedAsyncLoadFromDiskJobsMutex);

			ReferenceWrapper<AsyncLoadFromDiskJob>* __restrict pUpperBoundIt = std::upper_bound(
				queuedAsyncLoadFromDiskJobs.begin().Get(),
				queuedAsyncLoadFromDiskJobs.end().Get(),
				job.GetPriority(),
				[](const Threading::JobPriority priority, const AsyncLoadFromDiskJob& __restrict otherJob) -> bool
				{
					return priority > otherJob.GetPriority();
				}
			);
			queuedAsyncLoadFromDiskJobs.Emplace(pUpperBoundIt, Memory::Uninitialized, job);

			return false;
		}
		return true;
	}
#elif PLATFORM_EMSCRIPTEN
	void QueueAsyncLoading(AsyncLoadFromDiskJob& job)
	{
		Threading::UniqueLock lock(queuedAsyncLoadFromDiskJobsMutex);
		ReferenceWrapper<AsyncLoadFromDiskJob>* __restrict pUpperBoundIt = std::upper_bound(
			queuedAsyncLoadFromDiskJobs.begin().Get(),
			queuedAsyncLoadFromDiskJobs.end().Get(),
			job.GetPriority(),
			[](const Threading::JobPriority priority, const AsyncLoadFromDiskJob& __restrict otherJob) -> bool
			{
				return priority > otherJob.GetPriority();
			}
		);
		queuedAsyncLoadFromDiskJobs.Emplace(pUpperBoundIt, Memory::Uninitialized, job);
	}
#endif

	void QueueAsyncLoadingRetry(AsyncLoadFromDiskJob& job)
	{
#if USE_POSIX_ASYNC_IO || USE_APPLE_ASYNC_IO || PLATFORM_WINDOWS
		[[maybe_unused]] const uint32 previousRequestCount = openFileDescriptorCount.FetchSubtract(1);
		Assert(previousRequestCount > 0);
#endif

		Threading::UniqueLock lock(queuedAsyncLoadFromDiskJobsMutex);
		ReferenceWrapper<AsyncLoadFromDiskJob>* __restrict pUpperBoundIt = std::upper_bound(
			queuedAsyncLoadFromDiskJobs.begin().Get(),
			queuedAsyncLoadFromDiskJobs.end().Get(),
			job.GetPriority(),
			[](const Threading::JobPriority priority, const AsyncLoadFromDiskJob& __restrict otherJob) -> bool
			{
				return priority > otherJob.GetPriority();
			}
		);
		queuedAsyncLoadFromDiskJobs.Emplace(pUpperBoundIt, Memory::Uninitialized, job);

		AsyncLoadFromDiskJob& queuedJob = queuedAsyncLoadFromDiskJobs.PopAndGetBack();
		lock.Unlock();
		if (const Optional<Threading::JobRunnerThread*> pCurrentThread = Threading::JobRunnerThread::GetCurrent())
		{
			pCurrentThread->Queue(queuedJob);
		}
		else
		{
			queuedJob.Queue(System::Get<Threading::JobManager>());
		}
	}

	void OnAsyncLoadingFinished()
	{
#if USE_POSIX_ASYNC_IO || USE_APPLE_ASYNC_IO || PLATFORM_WINDOWS
		[[maybe_unused]] const uint32 previousRequestCount = openFileDescriptorCount.FetchSubtract(1);
		Assert(previousRequestCount > 0);
#endif

		Threading::UniqueLock lock(queuedAsyncLoadFromDiskJobsMutex);
		if (queuedAsyncLoadFromDiskJobs.HasElements())
		{
			AsyncLoadFromDiskJob& queuedJob = queuedAsyncLoadFromDiskJobs.PopAndGetBack();
			lock.Unlock();

			if (const Optional<Threading::JobRunnerThread*> pCurrentThread = Threading::JobRunnerThread::GetCurrent())
			{
				pCurrentThread->Queue(queuedJob);
			}
			else
			{
				queuedJob.Queue(System::Get<Threading::JobManager>());
			}
		}
	}

	AsyncLoadFromDiskJob::~AsyncLoadFromDiskJob()
	{
		CloseHandles();
	}

	void AsyncLoadFromDiskJob::CloseHandles()
	{
#if PLATFORM_WINDOWS
		if (m_fileHandle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_fileHandle);
			m_fileHandle = INVALID_HANDLE_VALUE;
		}
#elif USE_APPLE_ASYNC_IO
		if (m_dispatchChannel != nullptr)
		{
			dispatch_io_close(m_dispatchChannel, DISPATCH_IO_STOP);
			m_dispatchChannel = nullptr;
		}
#elif USE_POSIX_ASYNC_IO
		if (m_fileDescriptor != -1)
		{
			close(m_fileDescriptor);
			m_fileDescriptor = -1;
		}
#endif
	}

	void AsyncLoadFromDiskJob::OnLoadingFinished()
	{
		CloseHandles();
		OnAsyncLoadingFinished();
		m_callback(m_target);
	}

	void AsyncLoadFromDiskJob::OnLoadingFailed()
	{
		CloseHandles();
		OnAsyncLoadingFinished();
		m_callback({});
	}

	Threading::Job::Result AsyncLoadFromDiskJob::OnExecute([[maybe_unused]] Threading::JobRunnerThread& thread)
	{
		switch (m_status)
		{
			case Status::WaitingForInitialRequest:
			{
				static const auto getTargetBuffer = [](AsyncLoadFromDiskJob& job, const size fileSize) mutable
				{
					const ByteView target = job.m_target;
					const Math::Range<size> dataRange = job.m_dataRange;
					if (target.HasElements())
					{
						const size dataSize =
							Math::Min((size)dataRange.GetSize(), (size)(fileSize - dataRange.GetMinimum()), (size)fileSize, (size)target.GetDataSize());
						job.m_target = target.GetSubView((size)0, dataSize);
					}
					else
					{
						const size dataSize = (size)Math::Min(dataRange.GetSize(), fileSize - dataRange.GetMinimum(), fileSize);
						job.m_buffer = FixedSizeVector<ByteType, size>(Memory::ConstructWithSize, Memory::Uninitialized, dataSize);
						job.m_target = job.m_buffer.GetView();
					}
				};

#if PLATFORM_WINDOWS
				if (!TryStartAsyncLoading(*this))
				{
					return Result::AwaitExternalFinish;
				}

				m_fileHandle = CreateFileW(
					m_path.GetZeroTerminated(),
					GENERIC_READ,
					FILE_SHARE_READ,
					nullptr,
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
					nullptr
				);
				if (UNLIKELY(m_fileHandle == INVALID_HANDLE_VALUE))
				{
					OnLoadingFailed();
					return Result::FinishedAndDelete;
				}

				WIN32_FILE_ATTRIBUTE_DATA fileAttributeData;
				if (UNLIKELY(!GetFileAttributesExW(m_path.GetZeroTerminated(), GetFileExInfoStandard, &fileAttributeData)))
				{
					OnLoadingFailed();
					return Result::FinishedAndDelete;
				}
				LARGE_INTEGER fileSizeData;
				fileSizeData.HighPart = fileAttributeData.nFileSizeHigh;
				fileSizeData.LowPart = fileAttributeData.nFileSizeLow;
				const size fileSize = fileSizeData.QuadPart;

				getTargetBuffer(*this, fileSize);

				const DWORD64 offset = m_dataRange.GetMinimum();
				m_overlapped.Offset = static_cast<DWORD>(offset);
				m_overlapped.OffsetHigh = static_cast<DWORD>(offset >> 32);

				if (UNLIKELY(ReadFile(m_fileHandle, m_target.GetData(), (DWORD)m_target.GetDataSize(), nullptr, &m_overlapped)))
				{
					OnLoadingFailed();
					return Result::FinishedAndDelete;
				}

				m_status = Status::WaitingForAsyncRead;
				return Result::TryRequeue;
#elif USE_APPLE_ASYNC_IO
				if (!TryStartAsyncLoading(*this))
				{
					return Result::AwaitExternalFinish;
				}

				m_fileDescriptor = open(m_path.GetZeroTerminated(), O_RDONLY, 0);
				if (UNLIKELY(m_fileDescriptor == -1))
				{
					const int error = errno;
					switch (error)
					{
						case ENFILE:
						case EMFILE:
						{
							QueueAsyncLoadingRetry(*this);
							return Result::AwaitExternalFinish;
						}
						default:
						{
							OnLoadingFailed();
							return Result::FinishedAndDelete;
						}
					}
				}

				qos_class_t qualityOfServiceClass;
				switch (Threading::GetThreadPriorityFromJobPriority(GetPriority()))
				{
					case Threading::Thread::Priority::UserInteractive:
						qualityOfServiceClass = QOS_CLASS_USER_INTERACTIVE;
						break;
					case Threading::Thread::Priority::UserInitiated:
						qualityOfServiceClass = QOS_CLASS_USER_INITIATED;
						break;
					case Threading::Thread::Priority::Default:
						qualityOfServiceClass = QOS_CLASS_DEFAULT;
						break;
					case Threading::Thread::Priority::UserVisibleBackground:
						qualityOfServiceClass = QOS_CLASS_UTILITY;
						break;
					case Threading::Thread::Priority::Background:
						qualityOfServiceClass = QOS_CLASS_BACKGROUND;
						break;
				}

				struct stat fileStatus;
				if (UNLIKELY(stat(m_path.GetZeroTerminated(), &fileStatus) != 0))
				{
					OnLoadingFailed();
					return Result::FinishedAndDelete;
				}
				const size fileSize = fileStatus.st_size;

				getTargetBuffer(*this, fileSize);

				const dispatch_queue_t queue = dispatch_get_global_queue(qualityOfServiceClass, 0);

				const dispatch_fd_t fileDescriptor = m_fileDescriptor;
				m_dispatchChannel = dispatch_io_create(DISPATCH_IO_RANDOM, m_fileDescriptor, queue, ^([[maybe_unused]] const int error) {
					close(fileDescriptor);
					Assert(error == 0);
				});
				if (m_dispatchChannel == nullptr)
				{
					OnLoadingFailed();
					return Result::FinishedAndDelete;
				}

				Threading::JobManager& jobManager = thread.GetJobManager();
				dispatch_io_read(
					m_dispatchChannel,
					m_dataRange.GetMinimum(),
					m_target.GetDataSize(),
					queue,
					^(const bool completed, const dispatch_data_t dispatchData, const int error) {
						if (error == 0 && dispatchData != nullptr)
						{
							NSData* data = (NSData*)dispatchData;
							const size dataSize = [data length];
							const ConstByteView dataView{reinterpret_cast<const ByteType*>([data bytes]), dataSize};
							m_target.GetSubView(m_currentTargetOffset, dataSize).CopyFrom(dataView);
							m_currentTargetOffset += dataSize;
						}

						if (completed)
						{
							jobManager.QueueCallback(
								[this, error](Threading::JobRunnerThread&)
								{
									if (error == 0)
									{
										OnLoadingFinished();
									}
									else
									{
										OnLoadingFailed();
									}
									SignalExecutionFinishedAndDestroying(*Threading::JobRunnerThread::GetCurrent());
									delete this;
								},
								GetPriority()
							);
						}
					}
				);
				return Result::AwaitExternalFinish;
#elif USE_POSIX_ASYNC_IO
				if (!TryStartAsyncLoading(*this))
				{
					return Result::AwaitExternalFinish;
				}

				m_fileDescriptor = open(m_path.GetZeroTerminated(), O_RDONLY, 0);
				if (UNLIKELY(m_fileDescriptor == -1))
				{
					const int error = errno;
					switch (error)
					{
						case ENFILE:
						case EMFILE:
						{
							QueueAsyncLoadingRetry(*this);
							return Result::AwaitExternalFinish;
						}
						default:
						{
							OnLoadingFailed();
							return Result::FinishedAndDelete;
						}
					}
				}

				struct stat fileStatus;
				if (UNLIKELY(stat(m_path.GetZeroTerminated(), &fileStatus) != 0))
				{
					OnLoadingFailed();
					return Result::FinishedAndDelete;
				}
				const size fileSize = fileStatus.st_size;

				getTargetBuffer(*this, fileSize);

				Memory::Set(&m_aiocb, 0, sizeof(m_aiocb));
				m_aiocb.aio_fildes = m_fileDescriptor;
				m_aiocb.aio_buf = m_target.GetData();
				m_aiocb.aio_nbytes = m_target.GetDataSize();
				m_aiocb.aio_offset = m_dataRange.GetMinimum();
				m_aiocb.aio_reqprio = static_cast<int>(Threading::JobPriority::LastBackground) - static_cast<int>(GetPriority());
				if (UNLIKELY(aio_read(&m_aiocb) == -1))
				{
					OnLoadingFailed();
					return Result::FinishedAndDelete;
				}
				m_status = Status::WaitingForAsyncRead;
				return Result::TryRequeue;
#elif PLATFORM_EMSCRIPTEN
				if(m_path.GetView().GetStringView().StartsWith(IO::Path::GetApplicationDataDirectory().GetView().GetStringView())
					|| m_path.GetView().GetStringView().StartsWith(IO::Path::GetApplicationCacheDirectory().GetView().GetStringView())
					|| m_path.GetView().GetStringView().StartsWith(IO::Path::GetTemporaryDirectory().GetView().GetStringView())
					|| m_path.GetView().GetStringView().StartsWith(IO::Path::GetUserDataDirectory().GetView().GetStringView()))
				{
					// Fast path for local only paths that can't exist in network
					if (IO::File file(::fopen(m_path.GetZeroTerminated(), "rb")); file.IsValid())
					{
						if (UNLIKELY(!file.Seek(m_dataRange.GetMinimum(), IO::SeekOrigin::StartOfFile)))
						{
							m_callback({});
							return Result::FinishedAndDelete;
						}

						getTargetBuffer(*this, (size)file.GetSize());

						if (UNLIKELY(!file.ReadIntoView(ArrayView<ByteType, size>{m_target.GetData(), m_target.GetDataSize()})))
						{
							m_callback({});
							return Result::FinishedAndDelete;
						}

						m_callback(m_target);
						return Result::FinishedAndDelete;
					}
					else
					{
						m_callback({});
						return Result::FinishedAndDelete;
					}
				}

				const WebWorkerIdentifier webWorkerIdentifier = GetWebWorkerPool().AcquireWorker();
				if (webWorkerIdentifier.IsInvalid())
				{
					QueueAsyncLoading(*this);
					return Result::AwaitExternalFinish;
				}
				m_webWorkerIdentifier = webWorkerIdentifier;

				struct Data
				{
					AsyncLoadFromDiskJob& job;
					String rangeString;
					Vector<const char*> requestHeadersView;
					emscripten_fetch_attr_t attr;
				};
				Data* pData = new Data{*this};
				emscripten_fetch_attr_init(&pData->attr);
				strcpy(pData->attr.requestMethod, "GET");

				static constexpr bool StreamResponse = false;

				pData->attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_PERSIST_FILE |
				                         (EMSCRIPTEN_FETCH_STREAM_DATA * StreamResponse);
				pData->attr.onsuccess = [](emscripten_fetch_t* fetch)
				{
					Assert(!emscripten_is_main_browser_thread());
					Assert(fetch->userData != nullptr);
					AsyncLoadFromDiskJob& job = *reinterpret_cast<AsyncLoadFromDiskJob*>(fetch->userData);

					GetWebWorkerPool().ReturnWorker(job.m_webWorkerIdentifier);

					if constexpr (!StreamResponse)
					{
						const size receivedSize = (size)fetch->numBytes;
						Assert(receivedSize <= job.m_dataRange.GetSize());
						Assert(receivedSize <= (size)fetch->totalBytes);
						Assert(fetch->data != nullptr);
						Assert((size)fetch->dataOffset <= job.m_dataRange.GetSize());
						Assert((size)fetch->dataOffset <= (size)fetch->totalBytes);

						getTargetBuffer(job, receivedSize);

						const ConstByteView receivedDataView{reinterpret_cast<const ByteType*>(fetch->data), receivedSize};
						const ByteView targetView = job.m_target.GetSubView(job.m_currentTargetOffset, receivedSize);
						Assert(job.m_target.GetDataSize() >= receivedSize);
						[[maybe_unused]] const bool wasCopied = targetView.CopyFromWithOverlap(receivedDataView);
						Assert(wasCopied);
					}

					Threading::JobManager& jobManager = System::Get<Threading::JobManager>();
					jobManager.QueueCallback(
						[&job](Threading::JobRunnerThread& thread)
						{
							job.OnLoadingFinished();
							job.SignalExecutionFinishedAndDestroying(thread);
							delete &job;
						},
						job.GetPriority()
					);

					emscripten_fetch_close(fetch);
				};
				pData->attr.onprogress = [](emscripten_fetch_t* fetch)
				{
					Assert(!emscripten_is_main_browser_thread());
					if constexpr (StreamResponse)
					{
						Assert(fetch->userData != nullptr);
						AsyncLoadFromDiskJob& job = *reinterpret_cast<AsyncLoadFromDiskJob*>(fetch->userData);

						const size receivedSize = (size)fetch->numBytes;
						Assert(receivedSize <= job.m_dataRange.GetSize());
						Assert(receivedSize <= (size)fetch->totalBytes);
						Assert(fetch->data != nullptr);
						Assert((size)fetch->dataOffset <= job.m_dataRange.GetSize());
						Assert((size)fetch->dataOffset <= (size)fetch->totalBytes);

						const ConstByteView streamedDataView{reinterpret_cast<const ByteType*>(fetch->data), receivedSize};
						const ByteView targetView = job.m_target.GetSubView(job.m_currentTargetOffset, receivedSize);
						Assert(job.m_target.GetDataSize() >= receivedSize);
						[[maybe_unused]] const bool wasCopied = targetView.CopyFrom(streamedDataView);
						Assert(wasCopied);

						job.m_currentTargetOffset += receivedSize;
					}
				};
				pData->attr.onerror = [](emscripten_fetch_t* fetch)
				{
					Assert(!emscripten_is_main_browser_thread());
					Assert(fetch->userData != nullptr);
					AsyncLoadFromDiskJob& job = *reinterpret_cast<AsyncLoadFromDiskJob*>(fetch->userData);

					GetWebWorkerPool().ReturnWorker(job.m_webWorkerIdentifier);

					// Fall back to blocking IO
					// TODO: Investigate implementing aio for Emscripten
					Threading::JobManager& jobManager = System::Get<Threading::JobManager>();
					jobManager.QueueCallback(
						[&job](Threading::JobRunnerThread& thread)
						{
							if (IO::File file(::fopen(job.m_path.GetZeroTerminated(), "rb")); file.IsValid())
							{
								if (UNLIKELY(!file.Seek(job.m_dataRange.GetMinimum(), IO::SeekOrigin::StartOfFile)))
								{
									job.OnLoadingFailed();
									job.SignalExecutionFinishedAndDestroying(thread);
									delete &job;
									return;
								}

								getTargetBuffer(job, (size)file.GetSize());

								if (UNLIKELY(!file.ReadIntoView(ArrayView<ByteType, size>{job.m_target.GetData(), job.m_target.GetDataSize()})))
								{
									job.OnLoadingFailed();
									job.SignalExecutionFinishedAndDestroying(thread);
									delete &job;
									return;
								}

								job.OnLoadingFinished();
								job.SignalExecutionFinishedAndDestroying(thread);
								delete &job;
							}
							else
							{
								job.OnLoadingFailed();
								job.SignalExecutionFinishedAndDestroying(thread);
								delete &job;
							}
						},
						job.GetPriority()
					);

					emscripten_fetch_close(fetch);
				};
				pData->attr.userData = this;
				Assert(pData->attr.userData != nullptr);

				if (m_dataRange.GetSize() > 0 && m_dataRange != Math::Range<size>::MakeStartToEnd(0ull, Math::NumericLimits<size>::Max - 1))
				{
					pData->requestHeadersView.EmplaceBack("Range");

					pData->rangeString.Format("bytes={}-{}", m_dataRange.GetMinimum(), m_dataRange.GetMaximum());
					pData->requestHeadersView.EmplaceBack(pData->rangeString.GetZeroTerminated());
				}

				pData->requestHeadersView.EmplaceBack(nullptr);
				pData->attr.requestHeaders = pData->requestHeadersView.GetData();

				union ArgumentsUnion
				{
					Data* pData;
					struct
					{
						int arg1;
						int arg2;
					};
				};
				static auto executeOnWorker = [](const int arg1, const int arg2)
				{
					ArgumentsUnion arguments;
					arguments.arg1 = arg1;
					arguments.arg2 = arg2;

					Data& data = *arguments.pData;

					[[maybe_unused]] emscripten_fetch_t* pHandle = emscripten_fetch(&data.attr, data.job.m_path.GetZeroTerminated());
					Assert(pHandle != nullptr);

					delete arguments.pData;
				};
				em_proxying_queue* queue = emscripten_proxy_get_system_queue();
				pthread_t target = emscripten_main_runtime_thread_id();

				[[maybe_unused]] const bool queued =
					emscripten_proxy_async(
						queue,
						target,
						[](void* pUserData)
						{
							Data& data = *reinterpret_cast<Data*>(pUserData);
							Threading::WebWorker& worker = GetWebWorkerPool().GetWorker(data.job.m_webWorkerIdentifier);

							ArgumentsUnion arguments;
							arguments.pData = &data;
							emscripten_wasm_worker_post_function_vii(worker.GetIdentifier(), executeOnWorker, arguments.arg1, arguments.arg2);
						},
						pData
					) == 1;
				Assert(queued);
				return Result::AwaitExternalFinish;
#else
				{
					// Fall back to blocking IO
					// If this is hit, look into whether an async API is available on the platform
					IO::File file(m_path, IO::AccessModeFlags::Read | IO::AccessModeFlags::Binary, IO::SharingFlags::DisallowWrite);
					if (UNLIKELY(!file.IsValid() || !file.Seek(m_dataRange.GetMinimum(), IO::SeekOrigin::StartOfFile)))
					{
						m_callback({});
						return Result::FinishedAndDelete;
					}

					getTargetBuffer(*this, (size)file.GetSize());

					if (UNLIKELY(!file.ReadIntoView(ArrayView<ByteType, size>{m_target.GetData(), m_target.GetDataSize()})))
					{
						m_callback({});
						return Result::FinishedAndDelete;
					}
				}

				m_callback(m_target);
				return Result::FinishedAndDelete;
#endif
			}
			case Status::WaitingForAsyncRead:
			{
#if PLATFORM_WINDOWS
				DWORD numBytesTransferred = 0;
				if (GetOverlappedResult(m_fileHandle, &m_overlapped, &numBytesTransferred, FALSE))
				{
					if (LIKELY(numBytesTransferred == m_target.GetDataSize()))
					{
						OnLoadingFinished();
						return Result::FinishedAndDelete;
					}
					else
					{
						// Assert(numBytesTransferred == m_target.GetDataSize());
						Assert(m_fileHandle != INVALID_HANDLE_VALUE);
						CloseHandle(m_fileHandle);

						m_fileHandle = CreateFileW(
							m_path.GetZeroTerminated(),
							GENERIC_READ,
							FILE_SHARE_READ,
							nullptr,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
							nullptr
						);

						m_overlapped = {0};
						const DWORD64 offset = m_dataRange.GetMinimum();
						m_overlapped.Offset = static_cast<DWORD>(offset);
						m_overlapped.OffsetHigh = static_cast<DWORD>(offset >> 32);

						if (UNLIKELY(ReadFile(m_fileHandle, m_target.GetData(), (DWORD)m_target.GetDataSize(), nullptr, &m_overlapped)))
						{
							OnLoadingFailed();
							return Result::FinishedAndDelete;
						}
						return Result::TryRequeue;
					}
				}
				else
				{
					const DWORD error = GetLastError();
					switch (error)
					{
						case ERROR_IO_INCOMPLETE:
							SetExclusiveToThread(thread);
							return Result::TryRequeue;
						default:
							OnLoadingFailed();
							return Result::FinishedAndDelete;
					}
				}
#elif USE_POSIX_ASYNC_IO
				const int error = aio_error(&m_aiocb);
				switch (error)
				{
					case EINPROGRESS:
						return Result::TryRequeue;
					case 0:
					{
						const ssize_t readBytes = aio_return(&m_aiocb);

						OnLoadingFinished();

						if (LIKELY(readBytes != -1))
						{
							Assert(m_target.GetDataSize() == (size)readBytes);
							m_callback(m_target.GetSubView((size)0ull, (size)readBytes));
							return Result::FinishedAndDelete;
						}
						else
						{
							m_callback({});
							return Result::FinishedAndDelete;
						}
					}
					default:
					{
						OnLoadingFailed();
						return Result::FinishedAndDelete;
					}
				}
#else
				ExpectUnreachable();
#endif
			}
			default:
				break;
		}

		ExpectUnreachable();
	}

	Threading::Job* CreateAsyncLoadFromDiskJob(
		const IO::PathView path,
		Threading::JobPriority priority,
		AsyncLoadCallback&& callback,
		ByteView target,
		const Math::Range<size> dataRange
	)
	{
		return new AsyncLoadFromDiskJob(IO::Path(path), Forward<AsyncLoadCallback>(callback), target, dataRange, priority);
	}
}
