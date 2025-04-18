#pragma once

#include <Common/Platform/ForceInline.h>
#include <Common/Platform/Assume.h>
#include <Common/Platform/IsConstantEvaluated.h>

namespace ngine
{
	using nullptr_type = decltype(nullptr);

#if COMPILER_MSVC
#define IS_CHAR_UNSIGNED _CHAR_UNSIGNED
#elif COMPILER_CLANG || COMPILER_GCC
#define IS_CHAR_UNSIGNED __CHAR_UNSIGNED__
#endif

#if COMPILER_MSVC
	inline static constexpr char CharBitCount = 8;
	using size = unsigned long long;
	using ptrdiff = long long;
	using intptr = long long;
	using uintptr = unsigned long long;

	using uint8 = unsigned char;
	using uint16 = unsigned short;
	using uint32 = unsigned int;
	using uint64 = unsigned long long;
	using int8 = signed char;
	using int16 = signed short;
	using int32 = signed int;
	using int64 = signed long long;
#elif COMPILER_CLANG || COMPILER_GCC
	inline static constexpr char CharBitCount = __CHAR_BIT__;
	using size = __SIZE_TYPE__;
	using ptrdiff = __PTRDIFF_TYPE__;
	using intptr = __INTPTR_TYPE__;
	using uintptr = __UINTPTR_TYPE__;

	using uint8 = __UINT8_TYPE__;
	using uint16 = __UINT16_TYPE__;
	using uint32 = __UINT32_TYPE__;
	using uint64 = __UINT64_TYPE__;
	using int8 = __INT8_TYPE__;
	using int16 = __INT16_TYPE__;
	using int32 = __INT32_TYPE__;
	using int64 = __INT64_TYPE__;

#define IS_SIZE_UNIQUE_TYPE PLATFORM_APPLE || PLATFORM_WEB
#else
#error "Unknown platform"
#endif

	using ByteType = uint8;

#if __SIZEOF_INT128__
	using int128 = __int128;
#else
	struct uint128;
	struct alignas(16) int128
	{
		FORCE_INLINE int128() = default;
		FORCE_INLINE constexpr int128(const int64 value)
			: m_lowPart(value >= 0 ? value : -value)
			, m_highPart(value < 0 ? -1 : 0)
		{
		}

		explicit constexpr int128(const uint128 value);

		[[nodiscard]] constexpr int128 operator-() const
		{
			return {m_lowPart, m_highPart != 0 ? -m_highPart : -1};
		}

		[[nodiscard]] constexpr int128 operator--(int) const
		{
			return {m_lowPart - 1, m_highPart};
		}

		[[nodiscard]] constexpr bool operator==(const int128 other) const
		{
			return (m_lowPart == other.m_lowPart) & (m_highPart == other.m_highPart);
		}
		[[nodiscard]] constexpr bool operator!=(const int128 other) const
		{
			return (m_lowPart != other.m_lowPart) | (m_highPart != other.m_highPart);
		}
		[[nodiscard]] constexpr bool operator<=(const int128 other) const
		{
			return ((*this < other) | (*this == other));
		}
		[[nodiscard]] constexpr bool operator<(const int128 other) const
		{
			if (m_highPart == other.m_highPart)
			{
				return (m_lowPart < other.m_lowPart);
			}
			return (m_highPart < other.m_highPart);
		}
		[[nodiscard]] constexpr bool operator>=(const int128 other) const
		{
			return ((*this > other) | (*this == other));
		}
		[[nodiscard]] constexpr bool operator>(const int128 other) const
		{
			if (m_highPart == other.m_highPart)
			{
				return (m_lowPart > other.m_lowPart);
			}
			return (m_highPart > other.m_highPart);
		}
	private:
		friend uint128;
		FORCE_INLINE constexpr int128(const uint64 lowPart, const int64 highPart)
			: m_lowPart(lowPart)
			, m_highPart(highPart)
		{
		}
	private:
		uint64 m_lowPart;
		int64 m_highPart;
	};
#endif

#if __SIZEOF_INT128__
	using uint128 = unsigned __int128;
#else
	struct alignas(16) uint128
	{
		FORCE_INLINE uint128() = default;
		FORCE_INLINE constexpr uint128(const uint64 value)
			: m_lowPart(value)
			, m_highPart(0)
		{
		}
		explicit constexpr uint128(const int128 value);

		[[nodiscard]] FORCE_INLINE constexpr uint128 operator~() const
		{
			return uint128{~m_lowPart, ~m_highPart};
		}
		[[nodiscard]] FORCE_INLINE constexpr uint128 operator|(const uint128 other) const
		{
			return uint128{m_lowPart | other.m_lowPart, m_highPart | other.m_highPart};
		}
		FORCE_INLINE constexpr uint128& operator|=(const uint128 other)
		{
			*this = *this | other;
			return *this;
		}
		[[nodiscard]] FORCE_INLINE constexpr uint128 operator&(const uint128 other) const
		{
			return uint128{m_lowPart & other.m_lowPart, m_highPart & other.m_highPart};
		}
		FORCE_INLINE constexpr uint128& operator&=(const uint128 other)
		{
			*this = *this & other;
			return *this;
		}
		[[nodiscard]] FORCE_INLINE constexpr uint128 operator^(const uint128 other) const
		{
			return uint128{m_lowPart ^ other.m_lowPart, m_highPart ^ other.m_highPart};
		}
		FORCE_INLINE constexpr uint128& operator^=(const uint128 other)
		{
			*this = *this ^ other;
			return *this;
		}

		[[nodiscard]] constexpr uint128 operator<<(const uint128 shift) const
		{
			ASSUME(shift.m_highPart == 0);
			ASSUME(shift.m_lowPart <= 127);
			uint32 shiftValue = (uint32)shift.m_lowPart;

			uint128 result = *this;
			const uint128 source = result;

			shiftValue &= 127llu;

			const uint64 M1 = ((((shiftValue + 127llu) | shiftValue) & 64llu) >> 6llu) - 1llu;
			const uint64 M2 = (shiftValue >> 6llu) - 1llu;
			shiftValue &= 63llu;

			if (IsConstantEvaluated())
			{
				const uint64 rightShift = shiftValue != 0 ? source.m_lowPart >> (64llu - shiftValue) : 0;
				result.m_lowPart = (source.m_lowPart << shiftValue) & M2;
				result.m_highPart = ((source.m_lowPart << shiftValue) & (~M2)) | (((source.m_highPart << shiftValue) | (rightShift & M1)) & M2);
			}
			else
			{
				const uint64 rightShift = source.m_lowPart >> (64llu - shiftValue);
				result.m_lowPart = (source.m_lowPart << shiftValue) & M2;
				result.m_highPart = ((source.m_lowPart << shiftValue) & (~M2)) | (((source.m_highPart << shiftValue) | (rightShift & M1)) & M2);
			}
			return result;
		}
		FORCE_INLINE constexpr uint128& operator<<=(const uint128 shift)
		{
			*this = *this << shift;
			return *this;
		}

		[[nodiscard]] constexpr uint128 operator>>(const uint128 shift) const
		{
			ASSUME(shift.m_highPart == 0);
			ASSUME(shift.m_lowPart <= 127);
			uint32 shiftValue = (uint32)shift.m_lowPart;

			uint128 result = *this;
			const uint128 source = result;

			shiftValue &= 127llu;

			const uint64 M1 = ((((shiftValue + 127llu) | shiftValue) & 64llu) >> 6llu) - 1llu;
			const uint64 M2 = (shiftValue >> 6llu) - 1llu;
			shiftValue &= 63llu;

			if (IsConstantEvaluated())
			{
				const uint64 leftShift = shiftValue != 0 ? source.m_highPart << (64llu - shiftValue) : 0;

				result.m_lowPart = ((source.m_highPart >> shiftValue) & (~M2)) | (((source.m_lowPart >> shiftValue) | (leftShift & M1)) & M2);
				result.m_highPart = (source.m_highPart >> shiftValue) & M2;
			}
			else
			{
				const uint64 leftShift = source.m_highPart << (64llu - shiftValue);

				result.m_lowPart = ((source.m_highPart >> shiftValue) & (~M2)) | (((source.m_lowPart >> shiftValue) | (leftShift & M1)) & M2);
				result.m_highPart = (source.m_highPart >> shiftValue) & M2;
			}
			return result;
		}
		FORCE_INLINE constexpr uint128& operator>>=(const uint128 shift)
		{
			*this = *this >> shift;
			return *this;
		}

		[[nodiscard]] constexpr bool operator==(const uint128 other) const
		{
			return (m_lowPart == other.m_lowPart) & (m_highPart == other.m_highPart);
		}
		[[nodiscard]] constexpr bool operator!=(const uint128 other) const
		{
			return (m_lowPart != other.m_lowPart) | (m_highPart != other.m_highPart);
		}
		[[nodiscard]] constexpr bool operator<=(const uint128 other) const
		{
			return ((*this < other) | (*this == other));
		}
		[[nodiscard]] constexpr bool operator<(const uint128 other) const
		{
			if (m_highPart == other.m_highPart)
			{
				return (m_lowPart < other.m_lowPart);
			}
			return (m_highPart < other.m_highPart);
		}
		[[nodiscard]] constexpr bool operator>=(const uint128 other) const
		{
			return ((*this > other) | (*this == other));
		}
		[[nodiscard]] constexpr bool operator>(const uint128 other) const
		{
			if (m_highPart == other.m_highPart)
			{
				return (m_lowPart > other.m_lowPart);
			}
			return (m_highPart > other.m_highPart);
		}

		[[nodiscard]] constexpr explicit operator uint64() const
		{
			return m_lowPart;
		}
		[[nodiscard]] constexpr explicit operator uint32() const
		{
			return (uint32)m_lowPart;
		}
		[[nodiscard]] constexpr explicit operator uint16() const
		{
			return (uint16)m_lowPart;
		}
		[[nodiscard]] constexpr explicit operator uint8() const
		{
			return (uint8)m_lowPart;
		}
	private:
		friend int128;
		FORCE_INLINE constexpr uint128(const uint64 lowPart, const uint64 highPart)
			: m_lowPart(lowPart)
			, m_highPart(highPart)
		{
		}
	private:
		uint64 m_lowPart;
		uint64 m_highPart;
	};

	inline constexpr int128::int128(const uint128 value)
		: m_lowPart((int64)value.m_lowPart)
		, m_highPart((int64)value.m_highPart)
	{
	}

	inline constexpr uint128::uint128(const int128 value)
		: m_lowPart((uint64)value.m_lowPart)
		, m_highPart((uint64)value.m_highPart)
	{
	}

#endif

	namespace Math
	{
		enum class ZeroType : uint8
		{
			Zero
		};
		inline static constexpr ZeroType Zero = ZeroType::Zero;
		enum class IdentityType : uint8
		{
			Identity
		};
		inline static constexpr IdentityType Identity = IdentityType::Identity;
		enum class ForwardType : uint8
		{
			Forward
		};
		inline static constexpr ForwardType Forward = ForwardType::Forward;
		enum class BackwardType : uint8
		{
			Backward
		};
		inline static constexpr BackwardType Backward = BackwardType::Backward;
		enum class UpType : uint8
		{
			Up
		};
		inline static constexpr UpType Up = UpType::Up;
		enum class DownType : uint8
		{
			Down
		};
		inline static constexpr DownType Down = DownType::Down;
		enum class RightType : uint8
		{
			Right
		};
		inline static constexpr RightType Right = RightType::Right;
		enum class LeftType : uint8
		{
			Left
		};
		inline static constexpr LeftType Left = LeftType::Left;
	}
}
