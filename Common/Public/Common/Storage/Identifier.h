#pragma once

#include <Common/Math/NumericLimits.h>
#include <Common/Math/CoreNumericTypes.h>
#include <Common/Math/Max.h>
#include <Common/Math/PowerOfTwo.h>
#include <Common/Memory/GetNumericSize.h>
#include <Common/Memory/Containers/ForwardDeclarations/BitView.h>
#include <Common/Assert/Assert.h>
#include <Common/Platform/ForceInline.h>
#include <Common/Platform/TrivialABI.h>
#include <Common/Memory/Invalid.h>
#include <Common/Storage/ForwardDeclarations/Identifier.h>

namespace ngine
{
	template<typename TInternalType, uint8 TIndexBitCount, TInternalType MaximumCount_>
	struct TRIVIAL_ABI TIdentifier
	{
		using InternalType = TInternalType;
		inline static constexpr InternalType IndexNumBits = TIndexBitCount;
		static_assert(IndexNumBits < Math::NumericLimits<InternalType>::NumBits);
		static_assert(MaximumCount_ <= (1ULL << IndexNumBits) - 1u);

		inline static constexpr InternalType IndexUseCountNumBits = Math::NumericLimits<InternalType>::NumBits - IndexNumBits;
		inline static constexpr InternalType MaximumCount = MaximumCount_;
		using IndexType = Memory::NumericSize<MaximumCount>;
		inline static constexpr InternalType IndexMask = Math::NearestPowerOfTwo(InternalType(MaximumCount - 1)) - 1;

		inline static constexpr InternalType MaximumIndexReuseCount = (1ULL << IndexUseCountNumBits) - 1ULL;
		using IndexReuseType = Memory::NumericSize<MaximumIndexReuseCount>;

		inline static constexpr InternalType Invalid = 0;

		struct Hash
		{
			size operator()(const TIdentifier& identifier) const
			{
				return identifier.GetIndex() << TIndexBitCount | identifier.GetIndexUseCount();
			}
		};

		constexpr TIdentifier()
			: m_parts(0, 0)
		{
		}

		constexpr TIdentifier(const InvalidType)
			: m_parts(0, 0)
		{
		}

		constexpr TIdentifier(const IndexType index, const IndexReuseType indexUseCount)
			: m_parts(index, indexUseCount)
		{
			Expect(index <= MaximumCount);
			Expect(indexUseCount < MaximumIndexReuseCount);
		}

		[[nodiscard]] FORCE_INLINE static constexpr TIdentifier MakeFromIndex(const IndexType index)
		{
			return TIdentifier(index, 0);
		}
		[[nodiscard]] FORCE_INLINE static constexpr TIdentifier MakeFromValidIndex(const IndexType index)
		{
			return TIdentifier(static_cast<IndexType>(index + 1u), 0);
		}

		[[nodiscard]] FORCE_INLINE static constexpr TIdentifier MakeFromValue(const InternalType value)
		{
			struct Value
			{
				Value()
				{
				}

				union
				{
					Part m_parts;
					InternalType m_value;
				};
			} valueStruct;
			valueStruct.m_value = value;
			return TIdentifier(IndexType(valueStruct.m_parts.m_index), IndexReuseType(valueStruct.m_parts.m_indexUseCount));
		}

		[[nodiscard]] FORCE_INLINE constexpr bool operator==(const TIdentifier other) const
		{
			return (m_parts.m_index == other.m_parts.m_index) & (m_parts.m_indexUseCount == other.m_parts.m_indexUseCount);
		}
		[[nodiscard]] FORCE_INLINE constexpr bool operator!=(const TIdentifier other) const
		{
			return !(*this == other);
		}

		[[nodiscard]] FORCE_INLINE constexpr bool IsValid() const
		{
			return m_parts.m_index != 0;
		}
		[[nodiscard]] FORCE_INLINE constexpr bool IsInvalid() const
		{
			return m_parts.m_index == 0;
		}
		[[nodiscard]] FORCE_INLINE explicit constexpr operator bool() const
		{
			return IsValid();
		}

		[[nodiscard]] FORCE_INLINE constexpr IndexType GetIndex() const
		{
			return (IndexType)m_parts.m_index;
		}
		//! Gets the first valid index, subtracted by one
		//! This is used as we don't store the invalid [0] index in arrays to save space, and thus returns index - 1u
		//! It is invalid to call this function with an invalid identifier, however a valid result is still returned to protect against index
		//! out of bounds errors
		[[nodiscard]] FORCE_INLINE constexpr IndexType GetFirstValidIndex() const
		{
			Assert(m_parts.m_index > 0);
			return (IndexType)(Math::Max((uint32)m_parts.m_index, 1u) - 1u);
		}
		[[nodiscard]] FORCE_INLINE constexpr IndexType GetFirstValidIndexUnchecked() const
		{
			return (IndexType)(Math::Max((uint32)m_parts.m_index, 1u) - 1u);
		}
		[[nodiscard]] FORCE_INLINE constexpr IndexReuseType GetIndexUseCount() const
		{
			return m_parts.m_indexUseCount;
		}

		[[nodiscard]] FORCE_INLINE constexpr bool operator<(const TIdentifier& other) const
		{
			return m_parts.m_index < other.m_parts.m_index;
		}
		[[nodiscard]] FORCE_INLINE constexpr bool operator>(const TIdentifier& other) const
		{
			return m_parts.m_index > other.m_parts.m_index;
		}

		[[nodiscard]] FORCE_INLINE constexpr InternalType GetValue() const
		{
			return m_value;
		}

		FORCE_INLINE constexpr void IncrementUseCount()
		{
			Expect(m_parts.m_indexUseCount != MaximumIndexReuseCount);
			m_parts.m_indexUseCount++;
		}

		[[nodiscard]] static constexpr uint32 CalculateCompressedDataSize()
		{
			return IndexNumBits;
		}
		bool Compress(BitView& target) const;
		bool Decompress(ConstBitView& source);
	protected:
		explicit constexpr TIdentifier(const IndexType index)
			: m_parts(index, 0)
		{
			Expect(index < MaximumCount);
		}
	protected:
		struct Part
		{
			constexpr Part(const IndexType index, const IndexReuseType indexUseCount)
				: m_index(index)
				, m_indexUseCount(indexUseCount)
			{
			}

			InternalType m_index : IndexNumBits;
			InternalType m_indexUseCount : IndexUseCountNumBits;
		};

		static_assert(sizeof(Part) == sizeof(InternalType));

		union
		{
			Part m_parts;
			InternalType m_value;
		};
	};

	extern template struct TIdentifier<uint32, 1>;
	extern template struct TIdentifier<uint32, 2>;
	extern template struct TIdentifier<uint32, 3>;
	extern template struct TIdentifier<uint32, 4>;
	extern template struct TIdentifier<uint32, 5>;
	extern template struct TIdentifier<uint32, 6>;
	extern template struct TIdentifier<uint32, 7>;
	extern template struct TIdentifier<uint32, 8>;
	extern template struct TIdentifier<uint32, 9>;
	extern template struct TIdentifier<uint32, 10>;
	extern template struct TIdentifier<uint32, 11>;
	extern template struct TIdentifier<uint32, 12>;
	extern template struct TIdentifier<uint32, 13>;
	extern template struct TIdentifier<uint32, 14>;
	extern template struct TIdentifier<uint32, 15>;
	extern template struct TIdentifier<uint32, 16>;
	extern template struct TIdentifier<uint32, 17>;
}
