#pragma once

#include "SerializedData.h"
#include "Reader.h"

#include <Common/EnumFlags.h>

namespace ngine::Serialization
{
	template<typename T, typename... Args, typename = EnableIf<Internal::CanRead<T, Args...>>>
	static inline bool Deserialize(const Data& serializedData, T& element, Args&... args)
	{
		if (UNLIKELY(!serializedData.IsValid()))
		{
			return false;
		}
		return Internal::DeserializeElement(element, Reader(serializedData), args...);
	}

	template<typename T, typename... Args, typename = EnableIf<Internal::CanRead<T, Args...>>>
	[[nodiscard]] static inline bool DeserializeFromBuffer(const ConstStringView jsonData, T& element, Args&... args)
	{
		const Data serializedData(jsonData);
		if (UNLIKELY(!serializedData.IsValid()))
		{
			return false;
		}

		return Deserialize(serializedData, element, args...);
	}

	template<typename T, typename... Args, typename = EnableIf<Internal::CanRead<T, Args...>>>
	[[nodiscard]] static inline bool DeserializeFromDisk(const IO::FileView jsonFile, T& element, Args&... args)
	{
		const Data serializedData(jsonFile);
		if (UNLIKELY(!serializedData.IsValid()))
		{
			return false;
		}

		return Internal::DeserializeElement(element, Reader(serializedData), args...);
	}

	template<typename T, typename... Args, typename = EnableIf<Internal::CanRead<T, Args...>>>
	[[nodiscard]] static inline bool DeserializeFromDisk(const IO::ConstZeroTerminatedPathView filePath, T& element, Args&... args)
	{
		Data serializedData(filePath);
		if (UNLIKELY(!serializedData.IsValid()))
		{
			return false;
		}

		return Internal::DeserializeElement(element, Reader(serializedData), args...);
	}

	struct RootReader
	{
		RootReader(Data&& serializedData)
			: m_data(Move(serializedData))
		{
		}

		[[nodiscard]] Data& GetData() LIFETIME_BOUND
		{
			return m_data;
		}

		[[nodiscard]] operator Reader() const
		{
			return Reader(m_data);
		}
	protected:
		Serialization::Data m_data;
	};

	[[nodiscard]] static inline RootReader GetReaderFromBuffer(const ConstStringView jsonData)
	{
		return Data(jsonData);
	}

	[[nodiscard]] static inline RootReader GetReaderFromDisk(const IO::FileView jsonFile)
	{
		return Data(jsonFile);
	}

	[[nodiscard]] static inline RootReader GetReaderFromDisk(const IO::ConstZeroTerminatedPathView filePath)
	{
		return Data(filePath);
	}
}
