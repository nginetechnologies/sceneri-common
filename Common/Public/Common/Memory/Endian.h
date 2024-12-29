#pragma once

#include <Common/TypeTraits/IsEnum.h>
#include <Common/TypeTraits/EnableIf.h>
#include <Common/Platform/StaticUnreachable.h>

#if COMPILER_MSVC
#include <cstdlib>
#endif

namespace ngine::Memory
{
	[[nodiscard]] FORCE_INLINE bool IsBigEndian() noexcept
	{
		PUSH_MSVC_WARNINGS
		DISABLE_MSVC_WARNINGS(4305)
		union
		{
			uint32 i;
			char c[4];
		} bint = {0x01020304};
		POP_MSVC_WARNINGS

		return bint.c[0] == 1;
	}

	[[nodiscard]] FORCE_INLINE bool IsLittleEndian() noexcept
	{
		return !IsBigEndian();
	}

	template<typename T>
	T ByteSwap(const T) noexcept
	{
		static_unreachable("Not implemented for type!");
	}

	template<>
	[[nodiscard]] FORCE_INLINE uint16 ByteSwap(const uint16 data) noexcept
	{
#if COMPILER_MSVC
		return _byteswap_ushort(data);
#elif COMPILER_CLANG || COMPILER_GCC
		return __builtin_bswap16(data);
#else
		const uint16 hi = value << 8;
		const uint16 lo = value >> 8;
		return hi | lo;
#endif
	}

	template<>
	[[nodiscard]] FORCE_INLINE int16 ByteSwap(const int16 data) noexcept
	{
		const uint16 result = ByteSwap(*reinterpret_cast<const uint16*>(&data));
		return *reinterpret_cast<const int16*>(&result);
	}

	template<>
	[[nodiscard]] FORCE_INLINE uint32 ByteSwap(const uint32 data) noexcept
	{
#if COMPILER_MSVC
		return _byteswap_ulong(data);
#elif COMPILER_CLANG || COMPILER_GCC
		return __builtin_bswap32(data);
#else
		const uint32 byte0 = value & 0x000000FF;
		const uint32 byte1 = value & 0x0000FF00;
		const uint32 byte2 = value & 0x00FF0000;
		const uint32 byte3 = value & 0xFF000000;
		return (byte0 << 24) | (byte1 << 8) | (byte2 >> 8) | (byte3 >> 24);
#endif
	}

	template<>
	[[nodiscard]] FORCE_INLINE int32 ByteSwap(const int32 data) noexcept
	{
		const uint32 result = ByteSwap(*reinterpret_cast<const uint32*>(&data));
		return *reinterpret_cast<const int32*>(&result);
	}

	template<>
	[[nodiscard]] FORCE_INLINE uint64 ByteSwap(const uint64 data) noexcept
	{
#if COMPILER_MSVC
		return _byteswap_uint64(data);
#elif COMPILER_CLANG || COMPILER_GCC
		return __builtin_bswap64(data);
#else
		uint64 hi = ByteSwap(uint32(value));
		uint32 lo = ByteSwap(uint32(value >> 32));
		return (hi << 32) | lo;
#endif
	}

	template<>
	[[nodiscard]] FORCE_INLINE int64 ByteSwap(const int64 data) noexcept
	{
		const uint64 result = ByteSwap(*reinterpret_cast<const uint64*>(&data));
		return *reinterpret_cast<const int64*>(&result);
	}

	template<>
	[[nodiscard]] FORCE_INLINE float ByteSwap(const float value) noexcept
	{
		union
		{
			uint32 i;
			float f;
		} in, out;
		in.f = value;
		out.i = ByteSwap(in.i);
		return out.f;
	}

	template<>
	[[nodiscard]] FORCE_INLINE double ByteSwap(const double value) noexcept
	{
		union
		{
			uint64 i;
			double d;
		} in, out;
		in.d = value;
		out.i = ByteSwap(in.i);
		return out.d;
	}

	template<typename T>
	[[nodiscard]] FORCE_INLINE EnableIf<TypeTraits::IsEnum<T>, T> ByteSwap(const T value) noexcept
	{
		return static_cast<T>(ByteSwap(static_cast<UNDERLYING_TYPE(T)>(value)));
	}
}
