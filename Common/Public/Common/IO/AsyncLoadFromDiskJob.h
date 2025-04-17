#pragma once

#include <Common/Threading/Jobs/Job.h>
#include <Common/IO/Path.h>
#include <Common/Memory/Containers/ByteView.h>
#include <Common/Memory/Containers/Vector.h>
#include <Common/Math/Range.h>
#include <Common/Function/Function.h>
#include <Common/IO/AsyncLoadCallback.h>

#if PLATFORM_WINDOWS
#include <Common/Platform/Windows.h>
#elif PLATFORM_APPLE
#define USE_APPLE_ASYNC_IO 1
#include <dispatch/dispatch.h>
#elif PLATFORM_EMSCRIPTEN
#include <Common/Threading/ForwardDeclarations/WebWorkerIdentifier.h>
#include <Common/Storage/Identifier.h>
#elif PLATFORM_POSIX && !PLATFORM_ANDROID
#include <aio.h>
#endif

namespace ngine::IO
{
	struct AsyncLoadFromDiskJob final : public Threading::Job
	{
#if USE_POSIX_ASYNC_IO || USE_APPLE_ASYNC_IO || PLATFORM_WINDOWS
		inline static constexpr uint32 MaximumConcurrentRequestCount = 64;
#elif PLATFORM_EMSCRIPTEN
		inline static constexpr uint32 MaximumConcurrentRequestCount = 12;
#endif

#if PLATFORM_EMSCRIPTEN
		using WebWorkerIdentifier = Threading::WebWorkerIdentifier<MaximumConcurrentRequestCount>;
#endif

		AsyncLoadFromDiskJob(
			IO::Path&& path, AsyncLoadCallback&& callback, const ByteView target, const Math::Range<size> dataRange, const Priority priority
		);
		~AsyncLoadFromDiskJob();

		virtual Result OnExecute(Threading::JobRunnerThread&) override;
#if STAGE_DEPENDENCY_PROFILING
		[[nodiscard]] virtual ConstZeroTerminatedStringView GetDebugName() const override
		{
			return "Async Load From Disk";
		}
#endif
	protected:
		void OnLoadingFinished();
		void OnLoadingFailed();
		void CloseHandles();
	protected:
		IO::Path m_path;
		AsyncLoadCallback m_callback;

		ByteView m_target;
		const Math::Range<size> m_dataRange;

		enum class Status : uint8
		{
			WaitingForInitialRequest,
			WaitingForAsyncRead
		};
		Status m_status = Status::WaitingForInitialRequest;

		FixedSizeVector<ByteType, size> m_buffer;

#if PLATFORM_WINDOWS
		HANDLE m_fileHandle = INVALID_HANDLE_VALUE;
		OVERLAPPED m_overlapped = {0};
#elif USE_APPLE_ASYNC_IO
		dispatch_io_t m_dispatchChannel{nullptr};
		dispatch_fd_t m_fileDescriptor{-1};
		size m_currentTargetOffset{0};
#elif USE_POSIX_ASYNC_IO
		int m_fileDescriptor{-1};
		aiocb m_aiocb;
#elif PLATFORM_EMSCRIPTEN
		WebWorkerIdentifier m_webWorkerIdentifier;
		size m_currentTargetOffset{0};
#endif
	};

	Threading::Job* CreateAsyncLoadFromDiskJob(
		const IO::PathView path,
		Threading::JobPriority priority,
		AsyncLoadCallback&& callback,
		ByteView target,
		const Math::Range<size> dataRange
	);
}
