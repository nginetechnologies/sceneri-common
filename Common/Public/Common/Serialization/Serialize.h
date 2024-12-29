#pragma once

#include "SerializedData.h"
#include "Writer.h"

#include <Common/EnumFlags.h>
#include <Common/Memory/Containers/String.h>

namespace ngine::Serialization
{
	template<typename T, typename... Args, typename = EnableIf<Internal::CanWrite<T, Args...>>>
	static inline bool Serialize(Data& serializedDataDestination, const T& element, Args&... args)
	{
		if (UNLIKELY(!serializedDataDestination.IsValid()))
		{
			serializedDataDestination = Data(rapidjson::Type::kObjectType, serializedDataDestination.GetContextFlags());
		}

		return Internal::SerializeElement(element, Writer(serializedDataDestination), args...);
	}

	template<typename T, typename... Args, typename = EnableIf<Internal::CanWrite<T, Args...>>>
	[[nodiscard]] inline static Optional<String> SerializeToBuffer(const T& element, const EnumFlags<SavingFlags> savingFlags, Args&... args)
	{
		Data serializedData(rapidjson::kObjectType, ContextFlags::ToBuffer);
		if (LIKELY(Serialize(serializedData, element, args...)))
		{
			return serializedData.template SaveToBuffer<String>(savingFlags);
		}
		return Invalid;
	}

	template<typename T, typename... Args, typename = EnableIf<Internal::CanWrite<T, Args...>>>
	[[nodiscard]] inline static bool
	SerializeToDisk(const IO::ConstZeroTerminatedPathView filePath, const T& element, const EnumFlags<SavingFlags> savingFlags, Args&... args)
	{
		Data serializedData(rapidjson::kObjectType, ContextFlags::ToDisk);
		Internal::SerializeElement(element, Writer(serializedData), args...);
		return serializedData.SaveToFile(filePath, savingFlags);
	}
}
