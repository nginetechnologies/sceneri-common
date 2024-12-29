#include "IO/Log.h"

#include <Common/IO/Path.h>
#include <Common/Memory/OffsetOf.h>
#include <Common/Memory/Containers/Format/String.h>
#include <Common/Memory/Containers/Format/StringView.h>
#include <Common/EnumFlags.h>
#include <Common/Time/Timestamp.h>

#if PLATFORM_WINDOWS
#include <Common/Platform/Windows.h>
#elif PLATFORM_APPLE
#import <Foundation/NSString.h>
#import <Foundation/Foundation.h>
#elif PLATFORM_ANDROID
#include <android/log.h>
#elif PLATFORM_EMSCRIPTEN
#include <emscripten.h>
#endif

#include <cstdarg>

inline static constexpr bool EnableLog = !PLATFORM_APPLE_IOS && !PLATFORM_APPLE_VISIONOS && !PLATFORM_ANDROID && !PLATFORM_WEB;

namespace ngine
{
	Log::Log()
	{
#if ENABLE_ASSERTS
		Internal::AssertEvents::GetInstance().AddAssertListener(
			[](const char* file, const uint32 lineNumber, [[maybe_unused]] const bool isFirstTime, const char* message, void* pUserData)
				COLD_FUNCTION UNLIKELY_ERROR_SECTION NO_DEBUG
			{
				Log& log = *reinterpret_cast<Log*>(pUserData);
				log.Error(
					SourceLocation{ConstStringView{file, (uint32)strlen(file)}, (uint32)lineNumber},
					"Assert: {}",
					ConstStringView{message, (uint32)strlen(message)}
				);
			},
			this
		);
#endif
	}

	Log::~Log()
	{
#if ENABLE_ASSERTS
		Internal::AssertEvents::GetInstance().RemoveAssertListener(this);
#endif

		Close();
	}

	void Log::Open(const IO::PathView logName)
	{
		const Time::Timestamp currentTime = Time::Timestamp::GetCurrent();
		FlatString<40> timeString = currentTime.ToString();
		timeString.ReplaceCharacterOccurrences(':', ' ');
		IO::Path logPath = IO::Path::Combine(
			IO::Path::GetApplicationCacheDirectory(),
			MAKE_PATH_LITERAL("Logs"),
			logName,
			IO::Path::Merge(timeString.GetView(), Log::FileExtension)
		);
		const IO::Path logDirectoryPath(logPath.GetParentPath());
		if (!logDirectoryPath.Exists())
		{
			logDirectoryPath.CreateDirectories();
		}

		if constexpr (EnableLog)
		{
			OpenFile(Move(logPath));
		}

		OnInitialized(*this);
	}

	void Log::Close()
	{
		if (m_file.IsValid())
		{
			m_file.Close();
		}
	}

	void Log::OpenFile(IO::Path&& logPath)
	{
		Assert(!m_file.IsValid());
		if (logPath.Exists())
		{
			logPath.RemoveFile();
		}
		m_file = IO::File(logPath, IO::AccessModeFlags::Append, IO::SharingFlags::DisallowWrite);
		m_filePath = Move(logPath);
		Assert(m_file.IsValid());
		if (UNLIKELY(!m_file.IsValid()))
		{
			BreakIfDebuggerIsAttached();
			RaiseException();
			UNREACHABLE;
		}
	}

	void Log::InternalMessage(const ConstStringView message) const
	{
		{
			Threading::UniqueLock lock(m_logToConsoleMutex);
#if PLATFORM_APPLE
			NSString* nsMessage = [[NSString alloc] initWithBytes:message.GetData() length:message.GetSize() encoding:NSUTF8StringEncoding];
			NSLog(@"%@", nsMessage);
#elif PLATFORM_ANDROID
			__android_log_print(ANDROID_LOG_INFO, "Sceneri", "%s", message.GetData());
#elif PLATFORM_EMSCRIPTEN
			emscripten_log(EM_LOG_INFO, message.GetData());
#else
#if PLATFORM_WINDOWS
			if (IsDebuggerPresent())
			{
				OutputDebugStringA(message.GetData());
				OutputDebugStringA("\n");
			}
#endif
			fwrite(message.GetData(), sizeof(ConstStringView::CharType), message.GetSize(), stdout);
			fwrite("\n", sizeof(char), sizeof("\n"), stdout);
			fflush(stdout);
#endif
		}
		if constexpr (EnableLog)
		{
			Assert(m_file.IsValid());
			Threading::UniqueLock lock(m_fileAccessMutex);
			if (LIKELY(m_file.IsValid()))
			{
				m_file.Write(message);
				m_file.Write('\n');
				m_file.Flush();
			}
		}
	}

	void Log::InternalMessage(const ConstStringView message, [[maybe_unused]] const SourceLocation& sourceLocation) const
	{
		{
			Threading::UniqueLock lock(m_logToConsoleMutex);
#if PLATFORM_APPLE
			NSString* nsMessage = [[NSString alloc] initWithBytes:message.GetData() length:message.GetSize() encoding:NSUTF8StringEncoding];

			if constexpr (DEBUG_BUILD)
			{
				NSString* nsSourceFilePath = [[NSString alloc] initWithBytes:sourceLocation.sourceFilePath.GetData()
																															length:sourceLocation.sourceFilePath.GetSize()
																														encoding:NSUTF8StringEncoding];
				NSLog(@"%@ (%i): %@", nsSourceFilePath, sourceLocation.lineNumber, nsMessage);
			}
			else
			{
				NSLog(@"%@", nsMessage);
			}
#elif PLATFORM_ANDROID
			__android_log_print(
				ANDROID_LOG_INFO,
				"Sceneri",
				"%s (%i): %s",
				sourceLocation.sourceFilePath.GetData(),
				sourceLocation.lineNumber,
				message.GetData()
			);
#elif PLATFORM_EMSCRIPTEN
			if constexpr (DEBUG_BUILD)
			{
				String modifiedMessage;
				modifiedMessage.Format("{} ({}): {}", sourceLocation.sourceFilePath, sourceLocation.lineNumber, message);
				emscripten_log(EM_LOG_INFO, modifiedMessage.GetZeroTerminated());
			}
			else
			{
				emscripten_log(EM_LOG_INFO, message.GetData());
			}
#else
#if PLATFORM_WINDOWS
			if (IsDebuggerPresent())
			{
				if constexpr (DEBUG_BUILD)
				{
					OutputDebugStringA(sourceLocation.sourceFilePath.GetData());
					char lineNumberBuffer[16];
					snprintf(lineNumberBuffer, 16, "(%i): ", sourceLocation.lineNumber);
					OutputDebugStringA(lineNumberBuffer);
				}
				OutputDebugStringA(message.GetData());
				OutputDebugStringA("\n");
			}
#endif

			if constexpr (DEBUG_BUILD)
			{
				char lineNumberBuffer[16];
				const int size = snprintf(lineNumberBuffer, 16, "(%i): ", sourceLocation.lineNumber);

				fwrite(sourceLocation.sourceFilePath.GetData(), sizeof(ConstStringView::CharType), sourceLocation.sourceFilePath.GetSize(), stdout);
				fwrite(lineNumberBuffer, sizeof(char), size, stdout);
			}
			fwrite(message.GetData(), sizeof(ConstStringView::CharType), message.GetSize(), stdout);
			fwrite("\n", sizeof(char), sizeof("\n"), stdout);
			fflush(stdout);
#endif
		}
		if constexpr (EnableLog)
		{
			Assert(m_file.IsValid());
			Threading::UniqueLock lock(m_fileAccessMutex);
			if (LIKELY(m_file.IsValid()))
			{
				m_file.Write(message);
				m_file.Write('\n');
				m_file.Flush();
			}
		}
	}

	void Log::InternalWarning(const ConstStringView message, const SourceLocation& sourceLocation) const
	{
		static constexpr ConstStringView prefix = "[Warning] ";

		{
			Threading::UniqueLock lock(m_logToConsoleMutex);
#if PLATFORM_APPLE
			const String finalMessage = String::Merge(prefix, message);

			NSString* nsSourceFilePath = [[NSString alloc] initWithBytes:sourceLocation.sourceFilePath.GetData()
																														length:sourceLocation.sourceFilePath.GetSize()
																													encoding:NSUTF8StringEncoding];
			NSString* nsMessage = [[NSString alloc] initWithBytes:finalMessage.GetData()
																										 length:finalMessage.GetSize()
																									 encoding:NSUTF8StringEncoding];
			NSLog(@"%@ (%i): %@", nsSourceFilePath, sourceLocation.lineNumber, nsMessage);
#elif PLATFORM_ANDROID
			__android_log_print(
				ANDROID_LOG_WARN,
				"Sceneri",
				"%s (%i): %s",
				sourceLocation.sourceFilePath.GetData(),
				sourceLocation.lineNumber,
				message.GetData()
			);
#elif PLATFORM_EMSCRIPTEN
			String modifiedMessage;
			modifiedMessage.Format("{} ({}): {}", sourceLocation.sourceFilePath, sourceLocation.lineNumber, message);
			emscripten_log(EM_LOG_WARN, modifiedMessage.GetZeroTerminated());
#else
			char lineNumberBuffer[16];
			const int size = snprintf(lineNumberBuffer, 16, "(%i): ", sourceLocation.lineNumber);

#if PLATFORM_WINDOWS
			if (IsDebuggerPresent())
			{
				OutputDebugStringA(sourceLocation.sourceFilePath.GetData());
				OutputDebugStringA(lineNumberBuffer);
				OutputDebugStringA(prefix.GetData());
				OutputDebugStringA(message.GetData());
				OutputDebugStringA("\n");
			}
#endif

			fwrite(sourceLocation.sourceFilePath.GetData(), sizeof(ConstStringView::CharType), sourceLocation.sourceFilePath.GetSize(), stdout);
			fwrite(lineNumberBuffer, sizeof(char), size, stdout);
			fwrite(prefix.GetData(), sizeof(ConstStringView::CharType), prefix.GetSize(), stdout);
			fwrite(message.GetData(), sizeof(ConstStringView::CharType), message.GetSize(), stdout);
			fwrite("\n", sizeof(char), sizeof("\n"), stdout);
			fflush(stdout);
#endif
		}
		if constexpr (EnableLog)
		{
			Assert(m_file.IsValid());
			Threading::UniqueLock lock(m_fileAccessMutex);
			if (LIKELY(m_file.IsValid()))
			{
				m_file.Write(prefix);
				m_file.Write(message);
				m_file.Write('\n');
				m_file.Flush();
			}
		}
	}

	void Log::InternalError(const ConstStringView message, const SourceLocation& sourceLocation) const
	{
		static constexpr ConstStringView prefix = "[Error] ";

		{
			Threading::UniqueLock lock(m_logToConsoleMutex);
#if PLATFORM_APPLE
			const String finalMessage = String::Merge(prefix, message);

			NSString* nsSourceFilePath = [[NSString alloc] initWithBytes:sourceLocation.sourceFilePath.GetData()
																														length:sourceLocation.sourceFilePath.GetSize()
																													encoding:NSUTF8StringEncoding];
			NSString* nsMessage = [[NSString alloc] initWithBytes:finalMessage.GetData()
																										 length:finalMessage.GetSize()
																									 encoding:NSUTF8StringEncoding];
			NSLog(@"%@ (%i): %@", nsSourceFilePath, sourceLocation.lineNumber, nsMessage);
#elif PLATFORM_ANDROID
			__android_log_print(
				ANDROID_LOG_ERROR,
				"Sceneri",
				"%s (%i): %s",
				sourceLocation.sourceFilePath.GetData(),
				sourceLocation.lineNumber,
				message.GetData()
			);
#elif PLATFORM_EMSCRIPTEN
			String modifiedMessage;
			modifiedMessage.Format("{} ({}): {}", sourceLocation.sourceFilePath, sourceLocation.lineNumber, message);
			emscripten_log(EM_LOG_ERROR, modifiedMessage.GetZeroTerminated());
#else
			char lineNumberBuffer[16];
			const int size = snprintf(lineNumberBuffer, 16, "(%i): ", sourceLocation.lineNumber);

#if PLATFORM_WINDOWS
			if (IsDebuggerPresent())
			{
				OutputDebugStringA(sourceLocation.sourceFilePath.GetData());
				OutputDebugStringA(lineNumberBuffer);
				OutputDebugStringA(prefix.GetData());
				OutputDebugStringA(message.GetData());
				OutputDebugStringA("\n");
			}
#endif

			fwrite(sourceLocation.sourceFilePath.GetData(), sizeof(ConstStringView::CharType), sourceLocation.sourceFilePath.GetSize(), stderr);
			fwrite(lineNumberBuffer, sizeof(char), size, stderr);
			fwrite(prefix.GetData(), sizeof(ConstStringView::CharType), prefix.GetSize(), stderr);
			fwrite(message.GetData(), sizeof(ConstStringView::CharType), message.GetSize(), stderr);
			fwrite("\n", sizeof(char), sizeof("\n"), stderr);
			fflush(stderr);
#endif
		}
		if constexpr (EnableLog)
		{
			Threading::UniqueLock lock(m_fileAccessMutex);
			if (LIKELY(m_file.IsValid()))
			{
				m_file.Write(prefix);
				m_file.Write(message);
				m_file.Write('\n');
				m_file.Flush();
			}
			else
			{
				// Can't assert here as it'll recurse into errors
				BreakIfDebuggerIsAttached();
			}
		}
	}
}
