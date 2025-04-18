#pragma once

#include <Common/IO/Path.h>
#include <Common/Memory/Containers/Array.h>
#include <Common/Memory/Containers/Vector.h>
#include <Common/Memory/UniqueRef.h>
#include <Common/Memory/UniquePtr.h>
#include <Common/Function/Function.h>

#if PLATFORM_WINDOWS
struct _OVERLAPPED;
typedef _OVERLAPPED OVERLAPPED;
#define USE_POSIX_FILE_CHANGE_LISTENER 0
#elif PLATFORM_APPLE
#include <dispatch/dispatch.h>
#define USE_POSIX_FILE_CHANGE_LISTENER 0
#elif PLATFORM_POSIX && !PLATFORM_WEB
#define USE_POSIX_FILE_CHANGE_LISTENER 1
#else
#define USE_POSIX_FILE_CHANGE_LISTENER 0
#endif

namespace ngine::IO
{
	struct Path;

	struct FileChangeListener;

	struct FileChangeListeners
	{
		using GenericCallback = Function<void(const IO::PathView directoryPath, const IO::PathView relativeChangedPath), sizeof(void*)>;
		using RenamedCallback = Function<
			void(const IO::PathView directoryPath, const IO::PathView relativePreviousPath, const IO::PathView relativeNewPath),
			sizeof(void*)>;

		GenericCallback m_added;
		GenericCallback m_removed;
		GenericCallback m_modified;
		RenamedCallback m_renamed;
	};

	namespace Internal
	{
		struct MonitoredDirectory
		{
			MonitoredDirectory(const FileChangeListener& fileChangeListener, const IO::Path& path, FileChangeListeners&& listeners);
			~MonitoredDirectory();

			void OnChanged();
			void MonitorForChanges();

#if PLATFORM_WINDOWS
			[[nodiscard]] void* GetEventHandle() const;
#endif
		protected:
			friend FileChangeListener;

#if PLATFORM_WINDOWS
			void* m_directoryHandle;
			UniqueRef<OVERLAPPED> m_event;
			Array<char, 1024> m_buffer;
			IO::Path m_path;
#elif PLATFORM_APPLE
			int m_fileDescriptor;
			dispatch_queue_t m_dispatchQueue;
			dispatch_source_t m_dispatchSource;
#elif USE_POSIX_FILE_CHANGE_LISTENER
			int m_notifyDescriptor;
			int m_watchDescriptor;
			IO::Path m_path;
#endif

			FileChangeListeners m_listeners;
		};
	}

	struct FileChangeListener
	{
#define FILE_CHANGE_LISTENER_REQUIRES_POLLING (PLATFORM_WINDOWS || USE_POSIX_FILE_CHANGE_LISTENER)

		inline static constexpr bool IsSupported = PLATFORM_WINDOWS || PLATFORM_APPLE || USE_POSIX_FILE_CHANGE_LISTENER;
		inline static constexpr bool RequiresPolling = PLATFORM_WINDOWS;

		FileChangeListener();
		FileChangeListener(FileChangeListener&&) = delete;
		FileChangeListener(const FileChangeListener&) = delete;
		FileChangeListener& operator=(FileChangeListener&&) = delete;
		FileChangeListener& operator=(const FileChangeListener&) = delete;
		~FileChangeListener();

		void MonitorDirectory(const IO::Path& path, FileChangeListeners&& listeners);
#if FILE_CHANGE_LISTENER_REQUIRES_POLLING
		void CheckChanges();
#endif
	protected:
		friend Internal::MonitoredDirectory;

#if PLATFORM_WINDOWS
		using EventHandle = void*;
		Vector<EventHandle, uint16> m_eventHandles;
#elif PLATFORM_APPLE
		dispatch_queue_t m_dispatchQueue;
#elif USE_POSIX_FILE_CHANGE_LISTENER
		int m_notifyDescriptor;
#endif

		Vector<UniquePtr<Internal::MonitoredDirectory>> m_monitoredDirectories;
	};
}
