#include "Time/Duration.h"

#include <Common/Serialization/Reader.h>
#include <Common/Serialization/Writer.h>
#include <Common/Memory/Containers/Format/String.h>
#include <Common/Time/Formatter.h>

#if PLATFORM_WINDOWS
#include <Common/Platform/Windows.h>
#elif PLATFORM_POSIX
#include <time.h>
#endif

namespace ngine::Time
{
#if PLATFORM_WINDOWS
	FORCE_INLINE double GetPerformanceFrequency()
	{
		LARGE_INTEGER counter;
		QueryPerformanceFrequency(&counter);
		return static_cast<double>(counter.QuadPart);
	}

	static const double g_performanceFrequency = GetPerformanceFrequency();
#endif

	template<typename Type>
	/* static */ Duration<Type> Duration<Type>::GetCurrentSystemUptime()
	{
#if PLATFORM_WINDOWS
		LARGE_INTEGER counter;
		QueryPerformanceCounter(&counter);
		return Duration<double>::FromSeconds(static_cast<double>(counter.QuadPart) / g_performanceFrequency);
#elif PLATFORM_APPLE
		const uint64 time = clock_gettime_nsec_np(CLOCK_UPTIME_RAW);
		return Duration<double>::FromNanoseconds(time);
#elif PLATFORM_POSIX
		timespec currentTime;
		clock_gettime(CLOCK_MONOTONIC, &currentTime);
		return Duration<double>::FromSeconds(double(currentTime.tv_sec)) + Duration<double>::FromNanoseconds(currentTime.tv_nsec);
#endif
	}

	template<typename T>
	bool Duration<T>::Serialize(const Serialization::Reader serializer)
	{
		const Serialization::Value& __restrict currentElement = serializer.GetValue();
		Assert(currentElement.IsNumber());
		*this = Duration<T>::FromSeconds(static_cast<T>(currentElement.GetDouble()));
		return true;
	}

	template<typename T>
	bool Duration<T>::Serialize(Serialization::Writer serializer) const
	{
		Serialization::Value& __restrict currentElement = serializer.GetValue();
		currentElement = Serialization::Value(GetSeconds());
		return true;
	}

	template<typename Type>
	[[nodiscard]] FlatString<40> Duration<Type>::ToString() const
	{
		Time::Formatter formatter{Duration::FromSeconds(GetSeconds())};
		return FlatString<40>().Format("{:02}:{:02}", formatter.GetMinutes(), formatter.GetSeconds());
	}

	template struct Duration<float>;
	template struct Duration<double>;
}
