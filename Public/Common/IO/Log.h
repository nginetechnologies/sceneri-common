#pragma once

#include <Common/IO/Path.h>
#include <Common/IO/File.h>
#include <Common/Memory/Forward.h>
#include <Common/Memory/UniquePtr.h>
#include <Common/Memory/Containers/StringView.h>
#include <Common/SourceLocation.h>
#include <Common/Function/Event.h>

#include <Common/Platform/IsDebuggerAttached.h>
#include <Common/Platform/RaiseException.h>
#include <Common/Platform/NoInline.h>
#include <Common/Platform/Cold.h>
#include <Common/Platform/Unreachable.h>
#include <Common/Assert/Validate.h>
#include <Common/Memory/Containers/Format/String.h>
#include <Common/IO/ForwardDeclarations/PathView.h>

#include <Common/Threading/Mutexes/Mutex.h>

#include <Common/System/SystemType.h>
#include <Common/System/Query.h>

namespace ngine
{
	struct Log final
	{
	protected:
		static constexpr uint16 MaximumMessageSize = 1024;
	public:
		inline static constexpr System::Type SystemType = System::Type::Log;
		inline static constexpr IO::PathView FileExtension = MAKE_PATH(".log");

		enum class EType : uint8
		{
			Message,
			Warning,
			Error
		};

		Log();
		~Log();

		void Open(const IO::PathView logName);
		void Close();

		[[nodiscard]] IO::PathView GetFilePath() const
		{
			return m_filePath;
		}

		Event<void(void*, Log&), 24> OnInitialized;
		[[nodiscard]] bool IsInitialized() const
		{
			return m_file.IsValid();
		}

		template<typename... Args>
		NO_INLINE void Message(const ConstStringView format, Args&&... args) const
		{
			InternalMessage(String().Format(format, Forward<Args>(args)...));
		}

		template<typename... Args>
		NO_INLINE void Message(const SourceLocation sourceLocation, const ConstStringView format, Args&&... args) const
		{
			InternalMessage(String().Format(format, Forward<Args>(args)...), sourceLocation);
		}

		template<typename... Args>
		COLD_FUNCTION NO_INLINE void Warning(const SourceLocation sourceLocation, const ConstStringView format, Args&&... args) const
		{
			InternalWarning(String().Format(format, Forward<Args>(args)...), sourceLocation);
		}

		template<typename... Args>
		COLD_FUNCTION NO_INLINE UNLIKELY_ERROR_SECTION void
		Error(const SourceLocation sourceLocation, const ConstStringView format, Args&&... args) const
		{
			InternalError(String().Format(format, Forward<Args>(args)...), sourceLocation);
		}

		template<typename... Args>
		[[noreturn]] COLD_FUNCTION NO_INLINE UNLIKELY_ERROR_SECTION void
		FatalError(const SourceLocation sourceLocation, const ConstStringView format, Args&&... args) const
		{
			if (LIKELY(IsInitialized()))
			{
				Error(sourceLocation, format, Forward<Args>(args)...);
			}
			BreakIfDebuggerIsAttached();
			RaiseException();
			UNREACHABLE;
		}
	protected:
		void InternalMessage(const ConstStringView message) const;
		void InternalMessage(const ConstStringView message, const SourceLocation& sourceLocation) const;
		void InternalWarning(const ConstStringView message, const SourceLocation& sourceLocation) const;
		UNLIKELY_ERROR_SECTION COLD_FUNCTION NO_INLINE void
		InternalError(const ConstStringView message, const SourceLocation& sourceLocation) const;
	protected:
		friend struct Project;
		friend struct Engine;

		void OpenFile(IO::Path&& logPath);
	protected:
		mutable Threading::Mutex m_logToConsoleMutex;
		IO::Path m_filePath;
		IO::File m_file;
		mutable Threading::Mutex m_fileAccessMutex;
	};

	template<typename... Args>
	inline void
	CheckFatalError(const SourceLocation sourceLocation, const bool condition, const Log& log, const ConstStringView format, Args&&... args)
	{
		if (UNLIKELY_ERROR(condition))
		{
			log.FatalError(sourceLocation, format, Forward<Args>(args)...);
		}
	}
}

// These log wrappers are macros for now since the source location can not be properly inserted as a function default parameter.
// TODO: As soon as the code base adapts C++20 these macros should be converted to static functions instead and be part of the ngine::
// namespace.

#define LogFatalError(...) \
	{ \
		ngine::System::Get<ngine::Log>().FatalError(SOURCE_LOCATION, __VA_ARGS__); \
	}

#define LogError(...) \
	{ \
		ngine::System::Get<ngine::Log>().Error(SOURCE_LOCATION, __VA_ARGS__); \
	}

#define LogWarning(...) \
	{ \
		ngine::System::Get<ngine::Log>().Warning(SOURCE_LOCATION, __VA_ARGS__); \
	}

#define LogMessage(...) \
	{ \
		ngine::System::Get<ngine::Log>().Message(SOURCE_LOCATION, __VA_ARGS__); \
	}

#define LogFatalErrorIf(condition, ...) \
	{ \
		if (UNLIKELY(condition)) \
		{ \
			LogFatalError(__VA_ARGS__) \
		} \
	}

#define LogErrorIf(condition, ...) \
	{ \
		if (UNLIKELY(condition)) \
		{ \
			LogError(__VA_ARGS__) \
		} \
	}

#define LogWarningIf(condition, ...) \
	{ \
		if (UNLIKELY(condition)) \
		{ \
			LogWarning(__VA_ARGS__) \
		} \
	}

#define LogMessageIf(condition, ...) \
	{ \
		if (UNLIKELY(condition)) \
		{ \
			LogMessage(__VA_ARGS__) \
		} \
	}
