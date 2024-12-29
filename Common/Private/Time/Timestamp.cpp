#include "Time/Timestamp.h"
#include "Time/Duration.h"

#include <Common/Memory/Containers/FlatString.h>
#include <Common/Serialization/Reader.h>
#include <Common/Serialization/Writer.h>
#include <Common/Time/Formatter.h>
#include <Common/Time/Format/Timestamp.h>
#include <Common/Memory/Containers/Format/String.h>

PUSH_CLANG_WARNINGS
DISABLE_CLANG_WARNING("-Wdeprecated-literal-operator")
#include <Common/3rdparty/date/date.h>
POP_CLANG_WARNINGS

PUSH_CLANG_WARNINGS
DISABLE_CLANG_WARNING("-Wdeprecated")
DISABLE_CLANG_WARNING("-Wshorten-64-to-32")

#include <Common/3rdparty/fmt/chrono.h>

POP_CLANG_WARNINGS

#if PLATFORM_WINDOWS
#include <Common/Platform/Windows.h>
#elif PLATFORM_POSIX
#include <time.h>
#endif

namespace ngine::Time
{
	Timestamp Timestamp::GetCurrent()
	{
#if PLATFORM_WINDOWS
		// Convert Windows epoch to Unix epoch.
		static constexpr uint64 WindowsEpochToUnixEpoch = 0x19DB1DED53E8000LL;

		FILETIME fileTime;
		GetSystemTimePreciseAsFileTime(&fileTime);

		const uint64 timeIn100Nanoseconds(
			((uint64)fileTime.dwHighDateTime << 32ull) + (uint64)fileTime.dwLowDateTime - (uint64)WindowsEpochToUnixEpoch
		);
		return Timestamp::FromNanoseconds(timeIn100Nanoseconds * 100ull);
#elif PLATFORM_APPLE
		const uint64 time = clock_gettime_nsec_np(CLOCK_REALTIME);
		return Timestamp::FromNanoseconds(time);
#elif PLATFORM_POSIX
		timespec currentTime;
		clock_gettime(CLOCK_REALTIME, &currentTime);
		return Timestamp::FromSeconds(currentTime.tv_sec) + Timestamp::FromNanoseconds(currentTime.tv_nsec);
#endif
	}

	FlatString<40> Timestamp::ToString() const
	{
		return FlatString<40>().Format("{:%FT%T%Ez}", fmt::localtime(GetSeconds()));
	}

	String Timestamp::Format(const ConstStringView format) const
	{
		return FlatString<40>().Format(format, fmt::localtime(GetSeconds()));
	}

	bool Timestamp::Serialize(const Serialization::Reader serializer)
	{
		FlatString<64> timeString;
		if (serializer.SerializeInPlace(timeString))
		{
			std::istringstream inputStream{timeString.GetData()};
			std::chrono::system_clock::time_point time;
			inputStream >> date::parse("%FT%TZ", time);
			if (inputStream.fail())
			{
				inputStream.clear();
				inputStream.str(timeString.GetData());
				inputStream >> date::parse("%FT%T%Ez", time);
				if (inputStream.fail())
				{
					return false;
				}
			}
			*this = FromSeconds(std::chrono::system_clock::to_time_t(time));
			return true;
		}
		return false;
	}

	bool Timestamp::Serialize(Serialization::Writer serializer) const
	{
		if (IsValid())
		{
			FlatString<40> timeString = FlatString<40>().Format("{:%FT%T%Ez}", fmt::localtime(GetSeconds()));
			serializer.SerializeInPlace(timeString);
			return true;
		}
		else
		{
			return false;
		}
	}

}
