#include <Common/IO/FileChangeListener.h>

#if PLATFORM_WINDOWS
#include "Platform/Windows.h"
#elif PLATFORM_APPLE
#include <fcntl.h>
#include <dispatch/dispatch.h>
#include <dispatch/object.h>
#elif USE_POSIX_FILE_CHANGE_LISTENER
#include <sys/inotify.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#endif

#include <Common/Memory/Containers/Array.h>
#include <Common/Memory/UniquePtr.h>

#include <Common/Function/Event.h>

namespace ngine::IO
{
	namespace Internal
	{
		MonitoredDirectory::MonitoredDirectory(
			[[maybe_unused]] const FileChangeListener& fileChangeListener,
			[[maybe_unused]] const IO::Path& path,
			[[maybe_unused]] FileChangeListeners&& listeners
		)
#if PLATFORM_WINDOWS
			: m_directoryHandle(::CreateFileW(
					path.GetZeroTerminated(),
					FILE_LIST_DIRECTORY | GENERIC_READ,
					FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE,
					nullptr,
					OPEN_EXISTING,
					FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
					nullptr
				))
			, m_event(UniqueRef<OVERLAPPED>::Make())
			, m_path(path)
			, m_listeners(Forward<FileChangeListeners>(listeners))
#elif PLATFORM_APPLE
			: m_fileDescriptor(open(path.GetZeroTerminated(), O_EVTONLY))
			, m_dispatchQueue(fileChangeListener.m_dispatchQueue)
#elif USE_POSIX_FILE_CHANGE_LISTENER
			: m_notifyDescriptor(fileChangeListener.m_notifyDescriptor)
			, m_watchDescriptor(
					inotify_add_watch(fileChangeListener.m_notifyDescriptor, path.GetZeroTerminated(), IN_MODIFY | IN_CREATE | IN_DELETE)
				)
			, m_path(path)
#endif
		{
#if PLATFORM_WINDOWS
			Assert(m_directoryHandle != nullptr);
			m_event->hEvent = ::CreateEventW(nullptr, true, false, nullptr);
#elif PLATFORM_APPLE
			Assert(m_fileDescriptor > 0);
#elif USE_POSIX_FILE_CHANGE_LISTENER
			Assert(m_watchDescriptor > 0);
#endif

			MonitorForChanges();
		}

		MonitoredDirectory::~MonitoredDirectory()
		{
#if PLATFORM_WINDOWS
			if (m_directoryHandle != nullptr)
			{
				::CloseHandle(m_directoryHandle);
			}
			::CloseHandle(m_event->hEvent);
#elif PLATFORM_APPLE
			if (m_dispatchSource != nullptr)
			{
				dispatch_source_cancel(m_dispatchSource);

#if !__has_feature(objc_arc)
				dispatch_release(m_dispatchSource);
#endif

				m_dispatchSource = nullptr;
			}

#elif USE_POSIX_FILE_CHANGE_LISTENER
			if (m_watchDescriptor > 0)
			{
				inotify_rm_watch(m_notifyDescriptor, m_watchDescriptor);
			}
#endif
		}

#if PLATFORM_WINDOWS
		void* MonitoredDirectory::GetEventHandle() const
		{
			return m_event->hEvent;
		}
#endif

		void MonitoredDirectory::OnChanged()
		{
#if PLATFORM_WINDOWS
			if (m_directoryHandle == nullptr)
			{
				return;
			}

			DWORD numTransferedBytes = 0;
			if (!GetOverlappedResult(m_directoryHandle, m_event.Get(), &numTransferedBytes, false))
			{
				return;
			}

			const FILE_NOTIFY_INFORMATION* fileNotifyInfo = reinterpret_cast<const FILE_NOTIFY_INFORMATION*>(m_buffer.GetData());
			while (true)
			{
				if (fileNotifyInfo->FileNameLength)
				{
					const IO::PathView filePath(
						fileNotifyInfo->FileName,
						static_cast<IO::PathView::SizeType>(fileNotifyInfo->FileNameLength / sizeof(PathView::CharType))
					);

					switch (fileNotifyInfo->Action)
					{
						case FILE_ACTION_ADDED:
							m_listeners.m_added(m_path, filePath);
							break;
						case FILE_ACTION_REMOVED:
							m_listeners.m_removed(m_path, filePath);
							break;
						case FILE_ACTION_MODIFIED:
							m_listeners.m_modified(m_path, filePath);
							break;
						case FILE_ACTION_RENAMED_OLD_NAME:
						{
							fileNotifyInfo = reinterpret_cast<const FILE_NOTIFY_INFORMATION*>(
								reinterpret_cast<const unsigned char*>(fileNotifyInfo) + fileNotifyInfo->NextEntryOffset
							);
							Assert(fileNotifyInfo != nullptr);
							Assert(fileNotifyInfo->Action == FILE_ACTION_RENAMED_NEW_NAME);

							const IO::PathView newFilePath(
								fileNotifyInfo->FileName,
								static_cast<IO::PathView::SizeType>(fileNotifyInfo->FileNameLength / sizeof(PathView::CharType))
							);
							m_listeners.m_renamed(m_path, filePath, newFilePath);
						}
						break;
						case FILE_ACTION_RENAMED_NEW_NAME:
							ExpectUnreachable();
						default:
							ExpectUnreachable();
					}
				}

				if (fileNotifyInfo->NextEntryOffset == 0)
				{
					break;
				}

				fileNotifyInfo = reinterpret_cast<const FILE_NOTIFY_INFORMATION*>(
					reinterpret_cast<const unsigned char*>(fileNotifyInfo) + fileNotifyInfo->NextEntryOffset
				);
			}

			::ResetEvent(m_event->hEvent);
			MonitorForChanges();
#endif
		}

		void MonitoredDirectory::MonitorForChanges()
		{
#if PLATFORM_WINDOWS
			if (m_directoryHandle == nullptr)
			{
				return;
			}

			DWORD notifyFilter = 0;
			if (m_listeners.m_added.IsValid())
			{
				notifyFilter |= FILE_NOTIFY_CHANGE_CREATION;
			}

			if (m_listeners.m_modified.IsValid())
			{
				notifyFilter |= FILE_NOTIFY_CHANGE_LAST_WRITE;
			}

			if (m_listeners.m_removed.IsValid())
			{
				Assert(m_listeners.m_renamed.IsValid());

				notifyFilter |= FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME;
			}

			if (m_listeners.m_renamed.IsValid())
			{
				Assert(m_listeners.m_removed.IsValid());

				notifyFilter |= FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME;
			}

			[[maybe_unused]] const bool success = ReadDirectoryChangesW(
				m_directoryHandle,
				m_buffer.GetData(),
				m_buffer.GetSize(),
				true,
				notifyFilter,
				nullptr,
				m_event.Get(),
				nullptr
			);
			Assert(success);
#elif PLATFORM_APPLE
			const int fileDescriptor = m_fileDescriptor;
			// watch the file descriptor for writes
			m_dispatchSource = dispatch_source_create(DISPATCH_SOURCE_TYPE_VNODE, fileDescriptor, DISPATCH_VNODE_WRITE, m_dispatchQueue);

			// call the passed block if the source is modified
			dispatch_source_set_event_handler(m_dispatchSource, ^{
				this->OnChanged();
			});

			// close the file descriptor when the dispatch source is cancelled
			dispatch_source_set_cancel_handler(m_dispatchSource, ^{
				close(fileDescriptor);
			});

			// at this point the dispatch source is paused, so start watching
			dispatch_resume(m_dispatchSource);
#endif
		}
	}

	FileChangeListener::FileChangeListener()
#if PLATFORM_APPLE
		: m_dispatchQueue(dispatch_queue_create("File Change Listener Queue", 0))
#elif USE_POSIX_FILE_CHANGE_LISTENER
		: m_notifyDescriptor(inotify_init1(IN_NONBLOCK))
#endif
	{
#if USE_POSIX_FILE_CHANGE_LISTENER
		Assert(m_notifyDescriptor > 0);
#endif
	}

	FileChangeListener::~FileChangeListener()
	{
#if USE_POSIX_FILE_CHANGE_LISTENER
		if (m_notifyDescriptor > 0)
		{
			close(m_notifyDescriptor);
		}
#endif
	}

	void FileChangeListener::MonitorDirectory([[maybe_unused]] const IO::Path& path, [[maybe_unused]] FileChangeListeners&& listeners)
	{
		if (path.Exists())
		{
			[[maybe_unused]] const Internal::MonitoredDirectory& monitoredDirectory = *m_monitoredDirectories.EmplaceBack(
				UniquePtr<Internal::MonitoredDirectory>::Make(*this, path, Forward<FileChangeListeners>(listeners))
			);
#if PLATFORM_WINDOWS
			m_eventHandles.EmplaceBack(monitoredDirectory.GetEventHandle());
#endif
		}

		if constexpr (!IsSupported)
		{
			Assert(false);
		}
	}

#if FILE_CHANGE_LISTENER_REQUIRES_POLLING
	void FileChangeListener::CheckChanges()
	{
#if PLATFORM_WINDOWS
		if (m_eventHandles.IsEmpty())
		{
			return;
		}

		const DWORD waitStatus = WaitForMultipleObjects(m_eventHandles.GetSize(), m_eventHandles.GetData(), false, 0);
		if (waitStatus < WAIT_OBJECT_0 + m_eventHandles.GetSize())
		{
			const uint16 monitoredDirectoryIndex = static_cast<uint16>(waitStatus - WAIT_OBJECT_0);
			Internal::MonitoredDirectory& monitoredDirectory = *m_monitoredDirectories[monitoredDirectoryIndex];

			monitoredDirectory.OnChanged();
		}
#elif USE_POSIX_FILE_CHANGE_LISTENER
		fd_set rfds;
		FD_ZERO(&rfds);
		FD_SET(m_notifyDescriptor, &rfds);

		timeval timeout{0, 0};
		int result = select(m_notifyDescriptor + 1, &rfds, nullptr, nullptr, &timeout);
		if (result > 0 && FD_ISSET(m_notifyDescriptor, &rfds))
		{
			Array<char, 4096, uint32, uint32> buffer;
			ssize_t length = read(m_notifyDescriptor, buffer.GetData(), buffer.GetDataSize());
			if (length <= 0)
			{
				return;
			}

			ArrayView<const char> data{buffer.GetSubView(0, (uint32)length)};
			while (data.HasElements())
			{
				Assert(data.GetDataSize() >= sizeof(inotify_event));
				const struct inotify_event* event = reinterpret_cast<const struct inotify_event*>(data.GetData());

				IO::PathView filePath{event->name, (IO::PathView::SizeType)strlen(event->name)};

				for (const UniquePtr<Internal::MonitoredDirectory>& pMonitoredDirectory : m_monitoredDirectories)
				{
					if (filePath.IsRelativeTo(pMonitoredDirectory->m_path))
					{
						if (event->mask & IN_CREATE)
						{
							pMonitoredDirectory->m_listeners.m_added(pMonitoredDirectory->m_path, filePath);
						}
						else if (event->mask & IN_DELETE)
						{
							pMonitoredDirectory->m_listeners.m_removed(pMonitoredDirectory->m_path, filePath);
						}
						else if (event->mask & IN_MODIFY)
						{
							pMonitoredDirectory->m_listeners.m_modified(pMonitoredDirectory->m_path, filePath);
						}
					}
				}

				data += sizeof(inotify_event) + event->len;
			}
		}
#else
#error "Not implemented for platform"
#endif
	}
#endif
}
