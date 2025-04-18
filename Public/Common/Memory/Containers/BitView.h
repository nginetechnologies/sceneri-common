#pragma once

#include "ByteView.h"
#include "ForwardDeclarations/BitView.h"
#include <Common/Math/Range.h>
#include <Common/Math/NumericLimits.h>

namespace ngine
{
	namespace Memory
	{
		inline static constexpr size ByteSize = sizeof(ByteType) * CharBitCount;

		//! Given a bit index, gets the index of the byte that holds it
		[[nodiscard]] FORCE_INLINE constexpr size GetBitByteIndex(size bitIndex) noexcept
		{
			return bitIndex / ByteSize;
		}
		//! Given a bit index, gets the local index of the bit inside of the byte that holds it
		[[nodiscard]] FORCE_INLINE constexpr size GetBitByteOffset(size bitIndex) noexcept
		{
			return bitIndex % ByteSize;
		}

		[[nodiscard]] FORCE_INLINE constexpr ByteType CreateRightMask(const size bitCount) noexcept
		{
			return ByteType((1U << (ByteSize - bitCount)) - 1);
		}

		template<typename ValueType>
		[[nodiscard]] FORCE_INLINE PURE_STATICS static constexpr ValueType CreateMask(const Math::Range<size> bits) noexcept
		{
			constexpr ValueType highestIndex = Math::NumericLimits<ValueType>::NumBits - 1;
			ValueType mask = ValueType(Math::NumericLimits<ValueType>::Max << (highestIndex - bits.GetMaximum()));
			mask = mask >> ((highestIndex - bits.GetMaximum()) + bits.GetMinimum());
			mask = ValueType(mask << bits.GetMinimum());
			return mask;
		}
	}

	template<typename Type, typename SizeType>
	struct TBitView
	{
		using ByteViewType = TByteView<Type>;

		using BitSizeType = SizeType;
		using ByteSizeType = SizeType;

		using BitRangeType = Math::Range<BitSizeType>;
		using ByteRangeType = Math::Range<ByteSizeType>;

		constexpr TBitView() = default;
		constexpr TBitView(const ByteViewType bytes, const BitRangeType bitRange)
			: m_bytes(bytes)
			, m_bitRange(bitRange)
		{
		}
		constexpr TBitView(const ByteViewType bytes)
			: m_bytes(bytes)
			, m_bitRange(BitRangeType::Make(0, bytes.GetDataSize() * CharBitCount))
		{
		}

		template<
			typename ValueType,
			typename InternalType = Type,
			typename = EnableIf<TypeTraits::IsConst<InternalType> || !TypeTraits::IsConst<ValueType>>>
		[[nodiscard]] constexpr static TBitView
		Make(ValueType& value, const BitRangeType bitRange = BitRangeType::Make(0, sizeof(ValueType) * CharBitCount))
		{
			return {ByteViewType::Make(value), bitRange};
		}

		[[nodiscard]] constexpr Type* GetData() const noexcept
		{
			return m_bytes.GetData() + Memory::GetBitByteIndex(m_bitRange.GetMinimum());
		}
		[[nodiscard]] constexpr BitRangeType GetBitRange() const noexcept
		{
			return m_bitRange;
		}
		[[nodiscard]] PURE_STATICS constexpr bool HasElements() const noexcept
		{
			return m_bitRange.GetSize() > 0;
		}
		[[nodiscard]] PURE_STATICS constexpr bool IsEmpty() const noexcept
		{
			return m_bitRange.GetSize() == 0;
		}
		[[nodiscard]] PURE_STATICS constexpr BitSizeType GetIndex() const noexcept
		{
			return m_bitRange.GetMinimum();
		}

		[[nodiscard]] constexpr BitSizeType GetCount() const noexcept
		{
			return m_bitRange.GetSize();
		}

		[[nodiscard]] PURE_STATICS static constexpr ByteRangeType GetByteRange(const BitRangeType bitRange) noexcept
		{
			const ByteSizeType firstByteIndex = Memory::GetBitByteIndex(bitRange.GetMinimum());
			const ByteSizeType lastByteIndex = Memory::GetBitByteIndex(bitRange.GetMaximum());
			return ByteRangeType::Make(firstByteIndex, (lastByteIndex - firstByteIndex + 1) * (bitRange.GetSize() > 0));
		}
		[[nodiscard]] PURE_STATICS constexpr ByteSizeType GetByteCount() const noexcept
		{
			return GetByteRange(m_bitRange).GetSize();
		}
		[[nodiscard]] PURE_STATICS constexpr ByteRangeType GetByteRange() const noexcept
		{
			return GetByteRange(m_bitRange);
		}
		[[nodiscard]] PURE_STATICS constexpr ByteViewType GetByteView() const noexcept
		{
			return m_bytes;
		}

		template<typename Type_ = Type>
		constexpr EnableIf<!TypeTraits::IsConst<Type_>, void> Pack(const ConstBitView source) const noexcept
		{
			BitRangeType targetBitRange = m_bitRange;
			targetBitRange = targetBitRange.GetSubRangeUpTo(targetBitRange.GetMinimum() + source.GetCount());

			const ByteViewType targetBytes = m_bytes;

			BitRangeType sourceBitRange = source.GetBitRange();

			const ConstBitView::ByteViewType sourceBytes = source.GetByteView();

			while (targetBitRange.GetSize() > 0 && sourceBitRange.GetSize() > 0)
			{
				const BitSizeType sourceBitIndex = Memory::GetBitByteOffset(sourceBitRange.GetMinimum());
				const BitSizeType targetBitIndex = Memory::GetBitByteOffset(targetBitRange.GetMinimum());
				const BitSizeType sourceBitCount = Math::Min(CharBitCount - sourceBitIndex, sourceBitRange.GetSize());
				const BitSizeType targetBitCount = Math::Min(CharBitCount - targetBitIndex, targetBitRange.GetSize());

				const BitSizeType copiedBitCount = Math::Min(sourceBitCount, targetBitCount);

				const ByteSizeType sourceByteIndex = Memory::GetBitByteIndex(sourceBitRange.GetMinimum());
				const ByteSizeType targetByteIndex = Memory::GetBitByteIndex(targetBitRange.GetMinimum());

				const BitRangeType copiedSourceBitRange = BitRangeType::Make(sourceBitIndex, copiedBitCount);
				const BitRangeType copiedTargetBitRange = BitRangeType::Make(targetBitRange.GetMinimum(), copiedBitCount);

				BitView targetView{targetBytes, copiedTargetBitRange};
				const ByteType sourceValue = sourceBytes[sourceByteIndex];
				const ByteType maskedSourceValue = sourceValue & Memory::CreateMask<ByteType>(copiedSourceBitRange);
				const ByteType shiftedMaskedSourceValue = ByteType(maskedSourceValue >> copiedSourceBitRange.GetMinimum());
				const ByteType targetValue = ByteType(shiftedMaskedSourceValue << targetBitIndex);

				targetBytes[targetByteIndex] |= targetValue;

				if constexpr (ENABLE_ASSERTS)
				{
					const BitRangeType unpackedTargetBitRange = BitRangeType::Make(targetBitIndex, copiedBitCount);

					const ByteType unpackedTargetValue = targetBytes[targetByteIndex];
					const ByteType unpackedMaskedTargetValue = unpackedTargetValue & Memory::CreateMask<ByteType>(unpackedTargetBitRange);
					const ByteType unpackedShiftedMaskedTargetValue = unpackedMaskedTargetValue >> targetBitIndex;

					[[maybe_unused]] const ByteType unpackedValue = ByteType(unpackedShiftedMaskedTargetValue << sourceBitIndex);
					Assert(unpackedValue == maskedSourceValue);
				}

				sourceBitRange += copiedBitCount;
				targetBitRange += copiedBitCount;
			}
		}

		template<typename Type_ = Type>
		constexpr EnableIf<!TypeTraits::IsConst<Type_>, bool> PackAndSkip(const ConstBitView source)
		{
			Pack(source);
			const bool wasPacked = m_bitRange.GetSize() >= source.GetCount();
			m_bitRange += source.GetCount();
			return wasPacked;
		}

		constexpr void Unpack(const BitView target) const noexcept
		{
			BitRangeType sourceBitRange = m_bitRange;
			sourceBitRange = sourceBitRange.GetSubRangeUpTo(sourceBitRange.GetMinimum() + target.GetCount());

			const ConstByteView sourceBytes = m_bytes;

			BitRangeType targetBitRange = target.GetBitRange();

			const BitView::ByteViewType targetBytes = target.GetByteView();

			// Start by zeroing the ranges we'll be inserting into
			{
				targetBytes.ZeroInitialize();
			}

			// Now continue to unpacking
			while (targetBitRange.GetSize() > 0 && sourceBitRange.GetSize() > 0)
			{
				const BitSizeType targetBitIndex = Memory::GetBitByteOffset(targetBitRange.GetMinimum());
				const BitSizeType sourceBitIndex = Memory::GetBitByteOffset(sourceBitRange.GetMinimum());

				const BitSizeType sourceBitCount = Math::Min(CharBitCount - sourceBitIndex, sourceBitRange.GetSize());
				const BitSizeType targetBitCount = Math::Min(CharBitCount - targetBitIndex, targetBitRange.GetSize());

				const BitSizeType copiedBitCount = Math::Min(sourceBitCount, targetBitCount);

				const ByteSizeType targetByteIndex = Memory::GetBitByteIndex(targetBitRange.GetMinimum());
				const ByteSizeType sourceByteIndex = Memory::GetBitByteIndex(sourceBitRange.GetMinimum());

				const BitRangeType copiedSourceBitRange = BitRangeType::Make(sourceBitIndex, copiedBitCount);

				const ByteType sourceValue = sourceBytes[sourceByteIndex];
				const ByteType maskedSourceValue = sourceValue & Memory::CreateMask<ByteType>(copiedSourceBitRange);
				const ByteType shiftedMaskedSourceValue = maskedSourceValue >> sourceBitIndex;
				const ByteType targetValue = ByteType(shiftedMaskedSourceValue << targetBitIndex);

				targetBytes[targetByteIndex] |= targetValue;

				sourceBitRange += copiedBitCount;
				targetBitRange += copiedBitCount;
			}
		}

		constexpr bool UnpackAndSkip(const BitView target)
		{
			Unpack(target);
			const bool wasUnpacked = m_bitRange.GetSize() >= target.GetCount();
			m_bitRange += target.GetCount();
			return wasUnpacked;
		}

		template<typename UnpackedType>
		constexpr UnpackedType UnpackAndSkip()
		{
			UnpackedType target;
			UnpackAndSkip(BitView::Make(target));
			return target;
		}
		template<typename UnpackedType>
		constexpr UnpackedType UnpackAndSkip(const BitRangeType bitRange)
		{
			UnpackedType target;
			UnpackAndSkip(BitView::Make(target, bitRange));
			return target;
		}
	protected:
		ByteViewType m_bytes;
		BitRangeType m_bitRange = BitRangeType::Make(0, 0);
	};
}
