#pragma once

#include <Common/Compression/ForwardDeclarations/Compressor.h>
#include <Common/TypeTraits/IsEnum.h>
#include <Common/Reflection/Type.h>
#include <Common/Reflection/EnumTypeExtension.h>
#include <Common/EnumFlags.h>
#include <Common/Math/Range.h>
#include <Common/Math/Log2.h>

namespace ngine::Compression
{
	template<typename EnumType>
	struct Compressor<EnumType, EnableIf<TypeTraits::IsEnum<EnumType> && Reflection::IsReflected<EnumType>>>
	{
		using Type = EnumType;
		using UnderlyingType = UNDERLYING_TYPE(EnumType);

		[[nodiscard]] static constexpr uint32 CalculateCompressedDataSize()
		{
			if constexpr (Reflection::GetType<EnumType>().ContainsExtension(Reflection::EnumTypeInterface::TypeGuid))
			{
				constexpr Math::Range<UnderlyingType> enumValueRange = Reflection::GetEnum<EnumType>().GetRange();
				return Memory::GetBitWidth(enumValueRange.GetSize());
			}
			else
			{
				return sizeof(Type);
			}
		}

		static bool Compress(const EnumType& source, BitView& target)
		{
			if constexpr (Reflection::GetType<EnumType>().ContainsExtension(Reflection::EnumTypeInterface::TypeGuid))
			{
				constexpr Math::Range<UnderlyingType> enumValueRange = Reflection::GetEnum<EnumType>().GetRange();
				constexpr uint32 bitCount = CalculateCompressedDataSize();

				const UnderlyingType value = static_cast<UnderlyingType>(source) - enumValueRange.GetMinimum();
				return target.PackAndSkip(ConstBitView::Make(value, Math::Range<size>::Make(0, bitCount)));
			}
			else
			{
				return target.PackAndSkip(ConstBitView::Make(source));
			}
		}

		static bool Decompress(EnumType& target, ConstBitView& source)
		{
			if constexpr (Reflection::GetType<EnumType>().ContainsExtension(Reflection::EnumTypeInterface::TypeGuid))
			{
				constexpr Math::Range<UnderlyingType> enumValueRange = Reflection::GetEnum<EnumType>().GetRange();
				constexpr uint32 bitCount = CalculateCompressedDataSize();

				UnderlyingType value;
				const bool wasDecompressed = source.UnpackAndSkip(BitView::Make(value, Math::Range<size>::Make(0, bitCount)));
				target = static_cast<EnumType>(value + enumValueRange.GetMinimum());
				return wasDecompressed;
			}
			else
			{
				return source.UnpackAndSkip(BitView::Make(target));
			}
		}
	};

	template<typename EnumType>
	struct Compressor<EnumFlags<EnumType>, EnableIf<Reflection::IsReflected<EnumType>>>
	{
		using Type = EnumFlags<EnumType>;
		using UnderlyingType = UNDERLYING_TYPE(EnumType);

		[[nodiscard]] static constexpr uint32 CalculateCompressedDataSize()
		{
			constexpr Math::Range<UnderlyingType> enumValueRange = Reflection::GetEnum<EnumType>().GetRange();
			constexpr UnderlyingType minimumBitIndex = Math::Log2(enumValueRange.GetMinimum());
			constexpr UnderlyingType maximumBitIndex = Math::Log2(enumValueRange.GetMaximum());

			return (maximumBitIndex - minimumBitIndex) + 1;
		}

		static bool Compress(const EnumFlags<EnumType>& source, BitView& target)
		{
			constexpr Math::Range<UnderlyingType> enumValueRange = Reflection::GetEnum<EnumType>().GetRange();

			constexpr UnderlyingType minimumBitIndex = Math::Log2(enumValueRange.GetMinimum());
			constexpr UnderlyingType maximumBitIndex = Math::Log2(enumValueRange.GetMaximum());

			return target.PackAndSkip(ConstBitView::Make(source, Math::Range<size>::MakeStartToEnd(minimumBitIndex, maximumBitIndex)));
		}

		static bool Decompress(EnumFlags<EnumType>& target, ConstBitView& source)
		{
			constexpr Math::Range<UnderlyingType> enumValueRange = Reflection::GetEnum<EnumType>().GetRange();

			constexpr UnderlyingType minimumBitIndex = Math::Log2(enumValueRange.GetMinimum());
			constexpr UnderlyingType maximumBitIndex = Math::Log2(enumValueRange.GetMaximum());

			return source.UnpackAndSkip(BitView::Make(target, Math::Range<size>::MakeStartToEnd(minimumBitIndex, maximumBitIndex)));
		}
	};
}
