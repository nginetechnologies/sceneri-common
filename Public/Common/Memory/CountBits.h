#pragma once

#include <Common/Platform/ForceInline.h>
#include <Common/Platform/IsConstantEvaluated.h>
#include <Common/Platform/StaticUnreachable.h>
#include <Common/Math/CoreNumericTypes.h>
#include <Common/Math/NumericLimits.h>
#include <Common/Math/Select.h>
#include <Common/Math/Range.h>
#include <Common/Memory/CallbackResult.h>
#include <Common/Assert/Assert.h>
#include <Common/TypeTraits/IsSame.h>

#if COMPILER_MSVC && !USE_SSE4_2
#include <intrin.h>
#endif

#if USE_AVX
#include <immintrin.h>
#endif

namespace ngine::Memory
{
	[[nodiscard]] PURE_STATICS constexpr uint8 GetNumberOfLeadingZeros(const uint8 value) noexcept;
	[[nodiscard]] PURE_STATICS constexpr uint16 GetNumberOfLeadingZeros(const uint16 value) noexcept;
	[[nodiscard]] PURE_STATICS constexpr uint32 GetNumberOfLeadingZeros(const uint32 value) noexcept;
	[[nodiscard]] PURE_STATICS constexpr uint64 GetNumberOfLeadingZeros(const uint64 value) noexcept;
#if IS_SIZE_UNIQUE_TYPE
	[[nodiscard]] PURE_STATICS constexpr size GetNumberOfLeadingZeros(const size value) noexcept;
#endif

	namespace Internal
	{
		inline static constexpr uint8 LeadingZeroesUint8LookupTable[16] = {4, 3, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0};
	}

	[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr uint8 GetNumberOfLeadingZeros(const uint8 value) noexcept
	{
		if (IsConstantEvaluated())
		{
			const uint8 upper = value >> 4;
			const uint8 lower = value & 0x0F;
			return upper ? Internal::LeadingZeroesUint8LookupTable[upper] : 4 + Internal::LeadingZeroesUint8LookupTable[lower];
		}
		else
		{
			return (uint8)(GetNumberOfLeadingZeros((uint32)value) - 24u);
		}
	}

	[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr uint16 GetNumberOfLeadingZeros(const uint16 value) noexcept
	{
		if (IsConstantEvaluated())
		{
			const uint8 upper(value >> 8);
			const uint8 lower(value & Math::NumericLimits<uint8>::Max);
			return upper != 0 ? GetNumberOfLeadingZeros(upper) : 8 + GetNumberOfLeadingZeros(lower);
		}
		else
		{
			return (uint16)(GetNumberOfLeadingZeros((uint32)value) - 16u);
		}
	}

	[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr uint32 GetNumberOfLeadingZeros(const uint32 value) noexcept
	{
		if (IsConstantEvaluated())
		{
			const uint16 upper(value >> 16);
			const uint16 lower(value & Math::NumericLimits<uint16>::Max);
			return upper != 0 ? GetNumberOfLeadingZeros(upper) : 16 + GetNumberOfLeadingZeros(lower);
		}
		else
		{
#if COMPILER_GCC || COMPILER_CLANG
			return Math::Select(value == 0u, 32u, (uint32)__builtin_clz(value));
#else
			if constexpr (USE_SSE4_2)
			{
				return _lzcnt_u32(value);
			}
			else
			{
#if COMPILER_MSVC
				unsigned long bitIndex = 32 ^ 31;
				const bool result = _BitScanReverse(&bitIndex, value) != 0;
				return Math::Select(result, uint32(bitIndex ^ 31u), 32u);
#endif
			}
#endif
		}
	}

	[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr uint64 GetNumberOfLeadingZeros(const uint64 value) noexcept
	{
		if (IsConstantEvaluated())
		{
			const uint32 upper(value >> 32);
			const uint32 lower(value & Math::NumericLimits<uint32>::Max);
			return upper != 0 ? GetNumberOfLeadingZeros(upper) : 32 + GetNumberOfLeadingZeros(lower);
		}
		else
		{
#if COMPILER_GCC || COMPILER_CLANG
			return Math::Select(value == 0u, (uint64)64u, (uint64)__builtin_clzll(value));
#else
			if constexpr (USE_SSE4_2)
			{
				return _lzcnt_u64(value);
			}
			else
			{
#if COMPILER_MSVC
				unsigned long bitIndex = 64 ^ 63;
				const bool result = _BitScanReverse64(&bitIndex, value) != 0;
				return Math::Select(result, uint64(bitIndex ^ 63), 64ull);
#endif
			}
#endif
		}
	}

#if IS_SIZE_UNIQUE_TYPE
	[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr size GetNumberOfLeadingZeros(const size value) noexcept
	{
		return (size)GetNumberOfLeadingZeros((uint64)value);
	}
#endif

	//! Gets the number of bits necessary to represent the specific value
	template<typename Type>
	[[nodiscard]] PURE_STATICS constexpr Type GetBitWidth(const Type value) noexcept
	{
		if (IsConstantEvaluated())
		{
			return value < 2 ? value : (Type)1 + GetBitWidth(Type(value >> 1));
		}
		else
		{
			return Math::NumericLimits<Type>::NumBits - GetNumberOfLeadingZeros(value);
		}
	}

	[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr uint64 GetNumberOfTrailingZeros(uint64 value) noexcept
	{
		if (IsConstantEvaluated())
		{
			if (value == 0)
			{
				return 64;
			}
			uint64 count = 0;
			while ((value & 1) == 0)
			{
				value >>= 1;
				++count;
			}
			return count;
		}
		else
		{
#if COMPILER_GCC || COMPILER_CLANG
			return Math::Select(value == 0u, (uint64)64u, (uint64)__builtin_ctzll(value));
#else
			if constexpr (USE_SSE4_2)
			{
				return _tzcnt_u64(value);
			}
			else
			{
#if COMPILER_MSVC
				unsigned long bitIndex = 64;
				const bool result = _BitScanForward64(&bitIndex, value) != 0;
				return Math::Select(result, uint64(bitIndex), 64ull);
#endif
			}
#endif
		}
	}

#if IS_SIZE_UNIQUE_TYPE
	[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr size GetNumberOfTrailingZeros(const size value) noexcept
	{
		return (size)GetNumberOfTrailingZeros((uint64)value);
	}
#endif

	[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr uint32 GetNumberOfTrailingZeros(uint32 value) noexcept
	{
		if (IsConstantEvaluated())
		{
			if (value == 0)
			{
				return 32;
			}
			uint32 count = 0;
			while ((value & 1) == 0)
			{
				value >>= 1u;
				++count;
			}
			return count;
		}
		else
		{
#if COMPILER_GCC || COMPILER_CLANG
			return Math::Select(value == 0u, 32u, (uint32)__builtin_ctz(value));
#else
			if constexpr (USE_SSE4_2)
			{
				return _tzcnt_u32(value);
			}
			else
			{
#if COMPILER_MSVC
				unsigned long bitIndex = 32;
				const bool result = _BitScanForward(&bitIndex, value) != 0;
				return Math::Select(result, uint32(bitIndex), 32u);
#endif
			}
#endif
		}
	}

	[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr uint16 GetNumberOfTrailingZeros(const uint16 value) noexcept
	{
		return (uint16)GetNumberOfTrailingZeros((uint32)value | (0xFFFFu << 16));
	}

	[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr uint8 GetNumberOfTrailingZeros(const uint8 value) noexcept
	{
		return (uint8)GetNumberOfTrailingZeros((uint32)value | (0xFFu << 8));
	}

	[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr uint64 GetNumberOfSetBits(uint64 value) noexcept
	{
		if (IsConstantEvaluated())
		{
			uint64 count = 0;
			while (value)
			{
				count += value & 1;
				value >>= 1;
			}
			return count;
		}
		else
		{
#if COMPILER_GCC || COMPILER_CLANG
			return __builtin_popcountll(value);
#elif USE_SSE
			return _mm_popcnt_u64(value);
#endif
		}
	}

	[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr uint32 GetNumberOfSetBits(uint32 value) noexcept
	{
		if (IsConstantEvaluated())
		{
			uint32 count = 0;
			while (value)
			{
				count += value & 1;
				value >>= 1;
			}
			return count;
		}
		else
		{
#if COMPILER_GCC || COMPILER_CLANG
			return __builtin_popcount(value);
#elif USE_SSE
			return _mm_popcnt_u32(value);
#endif
		}
	}

	[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr uint16 GetNumberOfSetBits(const uint16 value) noexcept
	{
		return (uint16)GetNumberOfSetBits(static_cast<uint32>(value));
	}

	[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr uint8 GetNumberOfSetBits(const uint8 value) noexcept
	{
		return (uint8)GetNumberOfSetBits(static_cast<uint32>(value));
	}

	template<typename Type>
	struct BitIndex
	{
		[[nodiscard]] FORCE_INLINE constexpr operator bool() const
		{
			return m_value != 0;
		}
		[[nodiscard]] FORCE_INLINE constexpr bool IsValid() const
		{
			return m_value != 0;
		}
		[[nodiscard]] FORCE_INLINE constexpr bool IsInvalid() const
		{
			return m_value == 0;
		}
		[[nodiscard]] FORCE_INLINE constexpr operator Type() const
		{
			return m_value - Type(1);
		}
		[[nodiscard]] FORCE_INLINE constexpr Type operator*() const
		{
			return m_value - Type(1);
		}

		Type m_value;
	};

	template<typename Value>
	[[nodiscard]] FORCE_INLINE constexpr BitIndex<Value> GetFirstSetIndex(Value value) noexcept
	{
		if (IsConstantEvaluated())
		{
			if (value == 0)
			{
				return {};
			}
		}
		else
		{
#if COMPILER_GCC || COMPILER_CLANG
			if constexpr (TypeTraits::IsSame<Value, uint64>)
			{
				const uint64 index = __builtin_ffsll(reinterpret_cast<const int64&>(value));
				return BitIndex<uint64>{index};
			}
			else if constexpr (TypeTraits::IsSame<Value, uint32>)
			{
				const uint32 index = __builtin_ffs(reinterpret_cast<const int32&>(value));
				return BitIndex<uint32>{index};
			}
			else if constexpr (TypeTraits::IsSame<Value, uint16>)
			{
				const BitIndex<uint32> result = GetFirstSetIndex((uint32)value);
				return BitIndex<uint16>{(uint16)result.m_value};
			}
			else if constexpr (TypeTraits::IsSame<Value, uint8>)
			{
				const BitIndex<uint32> result = GetFirstSetIndex((uint32)value);
				return BitIndex<uint8>{(uint8)result.m_value};
			}
			else
			{
				static_unreachable("Unexpected type");
			}
#endif
		}

		Value bitIndex = 0u;
		Value skippedBitCount = GetNumberOfTrailingZeros(value);
		bitIndex += skippedBitCount;
		value >>= skippedBitCount;
		return BitIndex<Value>{Value((bitIndex + 1) * bool(value & 1u))};
	}

	template<typename Value>
	[[nodiscard]] FORCE_INLINE constexpr BitIndex<Value> GetLastSetIndex(Value value) noexcept
	{
		if (IsConstantEvaluated())
		{
			if (value == 0)
			{
				return {};
			}
		}
		else
		{
#if COMPILER_GCC || COMPILER_CLANG
			if constexpr (TypeTraits::IsSame<Value, uint64>)
			{
				const uint64 index = 63 ^ __builtin_clzll(reinterpret_cast<const int64&>(value));
				return BitIndex<uint64>{value != 0 ? index + 1 : 0};
			}
			else if constexpr (TypeTraits::IsSame<Value, uint32>)
			{
				const uint32 index = 31 ^ __builtin_clz(reinterpret_cast<const int32&>(value));
				return BitIndex<uint32>{value != 0 ? index + 1 : 0};
			}
			else if constexpr (TypeTraits::IsSame<Value, uint16>)
			{
				const BitIndex<uint32> result = GetLastSetIndex((uint32)value);
				return BitIndex<uint16>{(uint8)result.m_value};
			}
			else if constexpr (TypeTraits::IsSame<Value, uint8>)
			{
				const BitIndex<uint32> result = GetLastSetIndex((uint32)value);
				return BitIndex<uint8>{(uint8)result.m_value};
			}
			else
			{
				static_unreachable("Unexpected type");
			}
#endif
		}

		Value bitIndex = 0u;
		Value skippedBitCount = GetNumberOfLeadingZeros(value);
		bitIndex += (sizeof(Value) * 8 - 1) - skippedBitCount;
		value <<= skippedBitCount;
		return BitIndex<Value>{Value((bitIndex + 1) * bool(value != 0))};
	}

	template<typename Value>
	struct SetBitsIterator
	{
		constexpr SetBitsIterator(const Value value)
			: m_value(value)
		{
		}

		struct Iterator
		{
			constexpr Iterator() = default;
			constexpr Iterator(const Value value, const Value bitIndex)
				: m_value(value)
				, m_bitIndex(bitIndex)
			{
			}

			[[nodiscard]] FORCE_INLINE constexpr bool operator==(const Iterator& other) const
			{
				return (m_value == other.m_value);
			}
			[[nodiscard]] FORCE_INLINE constexpr bool operator!=(const Iterator& other) const
			{
				return !operator==(other);
			}
			[[nodiscard]] FORCE_INLINE constexpr bool operator<(const Iterator& other) const
			{
				return m_value < other.m_value;
			}
			[[nodiscard]] FORCE_INLINE constexpr bool operator<=(const Iterator& other) const
			{
				return m_value <= other.m_value;
			}
			[[nodiscard]] FORCE_INLINE constexpr bool operator>(const Iterator& other) const
			{
				return m_value > other.m_value;
			}
			[[nodiscard]] FORCE_INLINE constexpr bool operator>=(const Iterator& other) const
			{
				return m_value >= other.m_value;
			}

			FORCE_INLINE constexpr void operator++()
			{
				Value value = m_value;
				const Value skippedBitCount = GetNumberOfTrailingZeros(static_cast<Value>(value >> (Value)1u)) + (Value)1u;
				m_bitIndex = Math::Min(Value(m_bitIndex + skippedBitCount), Value(sizeof(Value) * 8));
				value >>= Math::Min(skippedBitCount, Value(sizeof(Value) * 8 - 1));
				m_value = value;
			}

			[[nodiscard]] FORCE_INLINE constexpr bool IsSet() const
			{
				return m_value != 0;
			}

			[[nodiscard]] FORCE_INLINE constexpr uint8 operator*() const
			{
				Assert(m_value & 1);
				return (uint8)m_bitIndex;
			}
		private:
			Value m_value = 0;
			Value m_bitIndex = 0;
		};

		[[nodiscard]] FORCE_INLINE constexpr Iterator begin() const
		{
			Value value = m_value;

			Value bitIndex = 0u;
			Value skippedBitCount = GetNumberOfTrailingZeros(value);
			bitIndex += skippedBitCount;
			value >>= Math::Min(skippedBitCount, Value(sizeof(Value) * 8 - 1));

			return {value, bitIndex};
		}
		[[nodiscard]] FORCE_INLINE constexpr Iterator end() const
		{
			return {0, sizeof(Value) * 8};
		}
	protected:
		Value m_value;
	};

	template<typename Value>
	[[nodiscard]] FORCE_INLINE constexpr SetBitsIterator<Value> GetSetBitsIterator(const Value value)
	{
		return {value};
	}

	template<typename Value>
	[[nodiscard]] FORCE_INLINE constexpr SetBitsIterator<Value> GetUnsetBitsIterator(const Value value)
	{
		return {Value(~value)};
	}

	template<typename Value>
	struct SetBitRangesIterator
	{
		constexpr SetBitRangesIterator(const Value value)
			: m_value(value)
		{
		}

		struct Iterator
		{
			constexpr Iterator() = default;
			constexpr Iterator(const Value value, const Value bitIndex, const Value contiguousSetBitsCount)
				: m_value(value)
				, m_bitIndex(bitIndex)
				, m_contiguousSetBitsCount(contiguousSetBitsCount)
			{
			}

			[[nodiscard]] FORCE_INLINE constexpr bool operator==(const Iterator& other) const
			{
				return (m_bitIndex == other.m_bitIndex);
			}
			[[nodiscard]] FORCE_INLINE constexpr bool operator!=(const Iterator& other) const
			{
				return !operator==(other);
			}
			[[nodiscard]] FORCE_INLINE constexpr bool operator<(const Iterator& other) const
			{
				return m_bitIndex < other.m_bitIndex;
			}
			[[nodiscard]] FORCE_INLINE constexpr bool operator<=(const Iterator& other) const
			{
				return m_bitIndex <= other.m_bitIndex;
			}
			[[nodiscard]] FORCE_INLINE constexpr bool operator>(const Iterator& other) const
			{
				return m_bitIndex > other.m_bitIndex;
			}
			[[nodiscard]] FORCE_INLINE constexpr bool operator>=(const Iterator& other) const
			{
				return m_bitIndex >= other.m_bitIndex;
			}

			FORCE_INLINE constexpr void operator++()
			{
				Value value = m_value;
				// Skip leading zeroes
				Value skippedBitCount = (Value)Memory::GetNumberOfTrailingZeros(value);
				value >>= skippedBitCount;

				Value setBitCount = (Value)Memory::GetNumberOfTrailingZeros((Value)~value);
				value >>= setBitCount;

				m_value = value;
				const Value bitIndex = m_bitIndex;
				m_bitIndex = Math::Min(Value(bitIndex + m_contiguousSetBitsCount + skippedBitCount), Value(sizeof(Value) * 8));
				m_contiguousSetBitsCount = setBitCount;
			}

			[[nodiscard]] FORCE_INLINE constexpr Math::Range<Value> operator*() const
			{
				Assert(m_bitIndex != sizeof(Value) * 8);
				const Value bitIndex = m_bitIndex;
				return Math::Range<Value>::MakeStartToEnd(bitIndex, Value(bitIndex + m_contiguousSetBitsCount - 1));
			}
		private:
			Value m_value = 0;
			Value m_bitIndex = 0;
			Value m_contiguousSetBitsCount = 0;
		};

		[[nodiscard]] FORCE_INLINE constexpr Iterator begin() const
		{
			Value value = m_value;
			Value currentIndex = 0;

			if (value & 1)
			{
				const Value contiguousSetBitsCount = (Value)Memory::GetNumberOfTrailingZeros((Value)~value);
				value >>= contiguousSetBitsCount;
				return {value, 0, contiguousSetBitsCount};
			}
			else if (value != 0)
			{
				// Skip leading zeroes
				Value skippedBitCount = (Value)Memory::GetNumberOfTrailingZeros(value);
				currentIndex += skippedBitCount;
				value >>= skippedBitCount;

				Assert(value & 1);
				const Value contiguousSetBitsCount = (Value)Memory::GetNumberOfTrailingZeros((Value)~value);
				value >>= contiguousSetBitsCount;
				return {value, contiguousSetBitsCount, contiguousSetBitsCount};
			}
			else
			{
				return {0, sizeof(Value) * 8, 0};
			}
		}
		[[nodiscard]] FORCE_INLINE constexpr Iterator end() const
		{
			return {0, sizeof(Value) * 8, 0};
		}
	protected:
		Value m_value;
	};

	template<typename Value>
	[[nodiscard]] FORCE_INLINE constexpr SetBitRangesIterator<Value> GetSetBitRangesIterator(const Value value)
	{
		return {value};
	}

	template<typename Value>
	struct SetBitsReverseIterator
	{
		constexpr SetBitsReverseIterator(const Value value)
			: m_value(value)
		{
		}

		struct Iterator
		{
			constexpr Iterator() = default;
			constexpr Iterator(const Value value, const Value bitIndex)
				: m_value(value)
				, m_bitIndex(bitIndex)
			{
			}

			[[nodiscard]] FORCE_INLINE constexpr bool operator==(const Iterator& other) const
			{
				return (m_value == other.m_value);
			}
			[[nodiscard]] FORCE_INLINE constexpr bool operator!=(const Iterator& other) const
			{
				return !operator==(other);
			}
			[[nodiscard]] FORCE_INLINE constexpr bool operator<(const Iterator& other) const
			{
				return m_value < other.m_value;
			}
			[[nodiscard]] FORCE_INLINE constexpr bool operator<=(const Iterator& other) const
			{
				return m_value <= other.m_value;
			}
			[[nodiscard]] FORCE_INLINE constexpr bool operator>(const Iterator& other) const
			{
				return m_value > other.m_value;
			}
			[[nodiscard]] FORCE_INLINE constexpr bool operator>=(const Iterator& other) const
			{
				return m_value >= other.m_value;
			}

			FORCE_INLINE constexpr void operator++()
			{
				Value value = m_value;
				const Value skippedBitCount = GetNumberOfLeadingZeros(static_cast<Value>(value << (Value)1u)) + 1u;
				m_bitIndex -= Math::Min(m_bitIndex, skippedBitCount);
				value <<= skippedBitCount;
				m_value = value;
			}

			[[nodiscard]] constexpr bool IsSet() const
			{
				return m_value != 0;
			}

			[[nodiscard]] FORCE_INLINE constexpr uint8 operator*() const
			{
				Assert(m_value != 0);
				return (uint8)m_bitIndex;
			}
		private:
			Value m_value = 0;
			Value m_bitIndex = 0;
		};

		[[nodiscard]] FORCE_INLINE constexpr Iterator begin() const
		{
			Value value = m_value;

			Value bitIndex = 0u;
			Value skippedBitCount = GetNumberOfLeadingZeros(value);
			bitIndex += (sizeof(Value) * 8 - 1) - skippedBitCount;
			value <<= skippedBitCount;

			return {value, bitIndex};
		}
		[[nodiscard]] FORCE_INLINE constexpr Iterator end() const
		{
			return {0, sizeof(Value) * 8};
		}
	protected:
		Value m_value;
	};

	template<typename Value>
	[[nodiscard]] FORCE_INLINE constexpr SetBitsReverseIterator<Value> GetSetBitsReverseIterator(const Value value)
	{
		return {value};
	}

	template<typename Value>
	[[nodiscard]] FORCE_INLINE constexpr SetBitsReverseIterator<Value> GetUnsetBitsReverseIterator(const Value value)
	{
		return {Value(~value)};
	}

	template<typename Value, typename Callback>
	FORCE_INLINE constexpr void IterateSetBits(Value value, Callback callback) noexcept
	{
		for (const uint8 bitIndex : GetSetBitsIterator(value))
		{
			if (!callback(bitIndex))
			{
				break;
			}
		}
	}

	template<typename Value>
	[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Value ClearTrailingSetBits(Value value, const uint8 numBitsToClear) noexcept
	{
		for (uint8 i = 0; i < numBitsToClear; ++i)
		{
			value &= ~(1 << GetNumberOfTrailingZeros(value));
		}

		return value;
	}

	template<typename Value>
	[[nodiscard]] PURE_STATICS constexpr Value ClearLeadingSetBits(Value value, const uint8 numBitsToClear) noexcept
	{
		for (uint8 i = 0; i < numBitsToClear; ++i)
		{
			value &= ~(1 << (sizeof(Value) * 8 - 1 - GetNumberOfLeadingZeros(value)));
		}

		return value;
	}

	template<typename Value>
	[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Value SetBitRange(const Value value, const Math::Range<Value> range) noexcept
	{
		const Value mask = Math::NumericLimits<Value>::Max >> ((sizeof(Value) * 8) - range.GetSize());
		return Value(value | (mask << range.GetMinimum()));
	}

	template<typename Value>
	[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Value SetBitRange(const Math::Range<Value> range) noexcept
	{
		const Value mask = Math::NumericLimits<Value>::Max >> ((sizeof(Value) * 8) - range.GetSize());
		return Value(mask << range.GetMinimum());
	}

	[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr uint32 ReverseBits(uint32 value) noexcept
	{
#if PLATFORM_ARM
		if (!IsConstantEvaluated())
		{
			return __builtin_arm_rbit(value);
		}
#endif
		value = ((value >> 1) & 0x55555555u) | ((value & 0x55555555u) << 1);
		value = ((value >> 2) & 0x33333333u) | ((value & 0x33333333u) << 2);
		value = ((value >> 4) & 0x0f0f0f0fu) | ((value & 0x0f0f0f0fu) << 4);
		value = ((value >> 8) & 0x00ff00ffu) | ((value & 0x00ff00ffu) << 8);
		value = ((value >> 16) & 0xffffu) | ((value & 0xffffu) << 16);
		return value;
	}
	[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr uint64 ReverseBits(const uint64 value) noexcept
	{
		return (((uint64)ReverseBits(uint32(value))) << 32u) | ReverseBits(uint32(value >> 32u));
	}
	[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr uint8 ReverseBits(const uint8 value) noexcept
	{
#if PLATFORM_ARM
		if (!IsConstantEvaluated())
		{
			return __builtin_arm_rbit(uint32(value)) >> 24u;
		}
#endif
		return uint8((value * 0x0202020202ULL & 0x010884422010ULL) % 1023);
	}
	[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr uint16 ReverseBits(const uint16 value) noexcept
	{
#if PLATFORM_ARM
		if (!IsConstantEvaluated())
		{
			return __builtin_arm_rbit(uint32(value)) >> 16;
		}
#endif
		return uint16(((uint16)ReverseBits(uint8(value))) << 8u) | ReverseBits(uint8(value >> 8u));
	}
}
