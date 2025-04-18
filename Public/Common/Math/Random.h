#pragma once

#include <Common/Math/NumericLimits.h>
#include <Common/Platform/ForceInline.h>
#include <Common/Platform/StaticUnreachable.h>

#include <Common/Math/Angle.h>
#include <Common/Memory/Containers/Array.h>
#include <Common/Memory/Containers/FixedArrayView.h>

namespace ngine::Math
{
	namespace Internal
	{
		[[nodiscard]] uint64 RandomUint64(const uint64 min, const uint64 max) noexcept;
		[[nodiscard]] uint64 RandomUint64() noexcept;
		[[nodiscard]] uint32 RandomUint32(const uint32 min, const uint32 max) noexcept;
		[[nodiscard]] uint32 RandomUint32() noexcept;
		[[nodiscard]] uint16 RandomUint16(const uint16 min, const uint16 max) noexcept;
		[[nodiscard]] uint16 RandomUint16() noexcept;
		[[nodiscard]] uint8 RandomUint8(const uint8 min, const uint8 max) noexcept;
		[[nodiscard]] uint8 RandomUint8() noexcept;
		[[nodiscard]] int64 RandomInt64(const int64 min, const int64 max) noexcept;
		[[nodiscard]] int64 RandomInt64() noexcept;
		[[nodiscard]] int32 RandomInt32(const int32 min, const int32 max) noexcept;
		[[nodiscard]] int32 RandomInt32() noexcept;
		[[nodiscard]] int16 RandomInt16(const int16 min, const int16 max) noexcept;
		[[nodiscard]] int16 RandomInt16() noexcept;
		[[nodiscard]] int8 RandomInt8(const int8 min, const int8 max) noexcept;
		[[nodiscard]] int8 RandomInt8() noexcept;
		[[nodiscard]] float RandomFloat(const float min, const float max) noexcept;
		[[nodiscard]] float RandomFloat() noexcept;
		[[nodiscard]] double RandomDouble(const double min, const double max) noexcept;
		[[nodiscard]] double RandomDouble() noexcept;
	}

	template<typename Type>
	[[nodiscard]] Type Random() noexcept
	{
		static_unreachable("Not implemented");
	}

	[[nodiscard]] FORCE_INLINE uint64 Random(const uint64 min, const uint64 max) noexcept
	{
		return Internal::RandomUint64(min, max);
	}
	template<>
	[[nodiscard]] FORCE_INLINE uint64 Random() noexcept
	{
		return Internal::RandomUint64();
	}
	[[nodiscard]] FORCE_INLINE int64 Random(const int64 min, const int64 max) noexcept
	{
		return Internal::RandomInt64(min, max);
	}
	template<>
	[[nodiscard]] FORCE_INLINE int64 Random() noexcept
	{
		return Internal::RandomInt64();
	}

	[[nodiscard]] FORCE_INLINE uint32 Random(const uint32 min, const uint32 max) noexcept
	{
		return Internal::RandomUint32(min, max);
	}
	template<>
	[[nodiscard]] FORCE_INLINE uint32 Random() noexcept
	{
		return Internal::RandomUint32();
	}
	[[nodiscard]] FORCE_INLINE int32 Random(const int32 min, const int32 max) noexcept
	{
		return Internal::RandomInt32(min, max);
	}
	template<>
	[[nodiscard]] FORCE_INLINE int32 Random() noexcept
	{
		return Internal::RandomInt32();
	}

	[[nodiscard]] FORCE_INLINE uint16 Random(const uint16 min, const uint16 max) noexcept
	{
		return Internal::RandomUint16(min, max);
	}
	template<>
	[[nodiscard]] FORCE_INLINE uint16 Random() noexcept
	{
		return Internal::RandomUint16();
	}
	[[nodiscard]] FORCE_INLINE int16 Random(const int16 min, const int16 max) noexcept
	{
		return Internal::RandomInt16(min, max);
	}
	template<>
	[[nodiscard]] FORCE_INLINE int16 Random() noexcept
	{
		return Internal::RandomInt16();
	}

	[[nodiscard]] FORCE_INLINE uint8 Random(const uint8 min, const uint8 max) noexcept
	{
		return Internal::RandomUint8(min, max);
	}
	template<>
	[[nodiscard]] FORCE_INLINE uint8 Random() noexcept
	{
		return Internal::RandomUint8();
	}
	[[nodiscard]] FORCE_INLINE int8 Random(const int8 min, const int8 max) noexcept
	{
		return Internal::RandomInt8(min, max);
	}
	template<>
	[[nodiscard]] FORCE_INLINE int8 Random() noexcept
	{
		return Internal::RandomInt8();
	}

	[[nodiscard]] FORCE_INLINE float Random(const float min, const float max) noexcept
	{
		return Internal::RandomFloat(min, max);
	}
	template<>
	[[nodiscard]] FORCE_INLINE float Random() noexcept
	{
		return Internal::RandomFloat();
	}

	[[nodiscard]] FORCE_INLINE double Random(const double min, const double max) noexcept
	{
		return Internal::RandomDouble(min, max);
	}
	template<>
	[[nodiscard]] FORCE_INLINE double Random() noexcept
	{
		return Internal::RandomDouble();
	}

	template<typename T, size Size>
	[[nodiscard]] FORCE_INLINE Array<T, Size> Random(const FixedArrayView<T, Size> min, const FixedArrayView<T, Size> max) noexcept
	{
		Array<T, Size> result;
		for (size i = 0; i < Size; ++i)
		{
			result[i] = Random(min[i], max[i]);
		}
		return result;
	}

	template<typename Type>
	void FillRandom(Type& element) noexcept
	{
		constexpr size numUint64s = sizeof(Type) / sizeof(uint64);
		constexpr size remainingSizeAfterUint64 = sizeof(Type) - numUint64s * sizeof(uint64);
		constexpr size numUint32s = remainingSizeAfterUint64 / sizeof(uint32);
		constexpr size remainingSizeAfterUint32 = remainingSizeAfterUint64 - numUint32s * sizeof(uint32);
		constexpr size numUint16s = remainingSizeAfterUint32 / sizeof(uint16);
		constexpr size remainingSizeAfterUint16 = remainingSizeAfterUint32 - numUint16s * sizeof(uint16);
		constexpr size numUint8s = remainingSizeAfterUint16 / sizeof(uint8);

		uint8* pAddress = reinterpret_cast<uint8*>(&element);

		if constexpr (numUint64s > 0)
		{
			for (size i = 0; i < numUint64s; ++i)
			{
				*reinterpret_cast<uint64*>(pAddress) = Random(0ull, Math::NumericLimits<uint64>::Max);

				pAddress += sizeof(uint64);
			}
		}

		if constexpr (numUint32s > 0)
		{
			for (size i = 0; i < numUint32s; ++i)
			{
				*reinterpret_cast<uint32*>(pAddress) = Random(0u, Math::NumericLimits<uint32>::Max);

				pAddress += sizeof(uint32);
			}
		}

		if constexpr (numUint16s > 0)
		{
			for (size i = 0; i < numUint16s; ++i)
			{
				*reinterpret_cast<uint16*>(pAddress) = Random((uint16)0u, Math::NumericLimits<uint16>::Max);

				pAddress += sizeof(uint16);
			}
		}

		if constexpr (numUint8s > 0)
		{
			for (size i = 0; i < numUint8s; ++i)
			{
				*reinterpret_cast<uint8*>(pAddress) = Random((uint8)0u, Math::NumericLimits<uint8>::Max);

				pAddress += sizeof(uint8);
			}
		}
	}
}
