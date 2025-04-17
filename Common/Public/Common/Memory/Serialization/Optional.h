#pragma once

#include "../Optional.h"

#include <Common/Serialization/Reader.h>
#include <Common/Serialization/Writer.h>

namespace ngine
{
	template<typename T, typename Enable>
	template<typename... Args>
	inline EnableIf<Serialization::Internal::CanWrite<T, Args...>, bool>
	Optional<T, Enable>::Serialize(Serialization::Writer serializer, Args&... args) const
	{
		if (IsInvalid())
		{
			return false;
		}

		return serializer.SerializeInPlace(Get(), args...);
	}

	template<typename T, typename Enable>
	template<typename... Args>
	inline EnableIf<Serialization::Internal::CanRead<T, Args...>, bool>
	Optional<T, Enable>::Serialize(const Serialization::Reader serializer, Args&... args)
	{
		T newValue;
		if (serializer.SerializeInPlace(newValue, args...))
		{
			*this = Move(newValue);
			return true;
		}
		return false;
	}

	template<typename T>
	template<typename... Args>
	inline EnableIf<Serialization::Internal::CanWrite<T, Args...>, bool>
	Optional<T, EnableIf<Internal::HasIsValid<T> && TypeTraits::HasConstructor<T>>>::Serialize(
		Serialization::Writer serializer, Args&... args
	) const
	{
		if (IsInvalid())
		{
			return false;
		}

		return serializer.SerializeInPlace(Get(), args...);
	}

	template<typename T>
	template<typename... Args>
	inline EnableIf<Serialization::Internal::CanRead<T, Args...>, bool>
	Optional<T, EnableIf<Internal::HasIsValid<T> && TypeTraits::HasConstructor<T>>>::Serialize(
		const Serialization::Reader serializer, Args&... args
	)
	{
		T newValue;
		if (serializer.SerializeInPlace(newValue, args...))
		{
			*this = Move(newValue);
			return true;
		}
		return false;
	}

	template<typename T>
	template<typename... Args>
	inline EnableIf<Serialization::Internal::CanWrite<T, Args...>, bool>
	Optional<T, EnableIf<Memory::Sentinel<T>::Type != Memory::SentinelType::NotSupported>>::Serialize(
		Serialization::Writer serializer, Args&... args
	) const
	{
		if (IsInvalid())
		{
			return false;
		}

		return serializer.SerializeInPlace(Get(), args...);
	}

	template<typename T>
	template<typename... Args>
	inline EnableIf<Serialization::Internal::CanRead<T, Args...>, bool>
	Optional<T, EnableIf<Memory::Sentinel<T>::Type != Memory::SentinelType::NotSupported>>::Serialize(
		const Serialization::Reader serializer, Args&... args
	)
	{
		T newValue;
		if (serializer.SerializeInPlace(newValue, args...))
		{
			*this = Move(newValue);
			return true;
		}
		return false;
	}

	template<typename T>
	template<typename... Args>
	inline EnableIf<Serialization::Internal::CanWrite<T, Args...>, bool>
	Optional<T*>::Serialize(Serialization::Writer serializer, Args&... args) const
	{
		if (IsInvalid())
		{
			return false;
		}

		return serializer.SerializeInPlace(*Get(), args...);
	}

	template<typename T>
	template<typename... Args>
	inline EnableIf<Serialization::Internal::CanRead<T, Args...>, bool>
	Optional<T*>::Serialize(const Serialization::Reader serializer, Args&... args)
	{
		T* newValue = *this;
		if (serializer.SerializeInPlace(newValue, args...))
		{
			*this = Move(newValue);
			return true;
		}
		return false;
	}
}
