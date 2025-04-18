#pragma once

#include <Common/Asset/Guid.h>
#include <Common/Memory/Tuple.h>
#include <Common/Memory/Any.h>
#include <Common/Math/Range.h>
#include <Common/Memory/Containers/StringView.h>
#include <Common/TypeTraits/UnderlyingType.h>
#include <Common/Reflection/EnumTypeInterface.h>
#include <Common/Reflection/Type.h>

namespace ngine::Reflection
{
	template<typename T>
	struct EnumTypeEntry final
	{
		using EnumType = T;
		constexpr EnumTypeEntry(const T value_, const ConstUnicodeStringView displayName_, const Asset::Guid iconAssetGuid_ = {}) noexcept
			: value(value_)
			, displayName(displayName_)
			, iconAssetGuid(iconAssetGuid_)
		{
		}

		EnumType value;
		ConstUnicodeStringView displayName;
		Asset::Guid iconAssetGuid;
	};

	template<typename... Ts>
	struct EnumTypeExtension final : public EnumTypeInterface
	{
		using EnumTypeEntryType = typename Tuple<Ts...>::template ElementType<0>;
		using EnumType = typename EnumTypeEntryType::EnumType;
		using UnderlyingType = UNDERLYING_TYPE(EnumType);

		constexpr EnumTypeExtension(Ts&&... entries_)
			: EnumTypeInterface{TypeGuid}
			, entries(Forward<Ts>(entries_)...)
		{
		}

		[[nodiscard]] constexpr EnumType ToEnumType(const uint64 value) const
		{
			return (EnumType)value;
		}
		[[nodiscard]] constexpr ConstUnicodeStringView ToString(const EnumType value) const
		{
			struct OptionalStringView : public ConstUnicodeStringView
			{
				using ConstUnicodeStringView::ConstUnicodeStringView;
				using ConstUnicodeStringView::operator=;

				[[nodiscard]] constexpr operator bool() const
				{
					return ConstUnicodeStringView::HasElements();
				}
			};
			return entries.template FindIf<OptionalStringView>(
				[value](const EnumTypeEntryType& entry) -> OptionalStringView
				{
					if (value == entry.value)
					{
						return OptionalStringView{entry.displayName};
					}
					else
					{
						return {};
					}
				}
			);
		}
		[[nodiscard]] constexpr Optional<EnumType> ToEnumType(const ConstUnicodeStringView displayName) const
		{
			return entries.template FindIf<Optional<EnumType>>(
				[displayName](const EnumTypeEntryType& entry) -> Optional<EnumType>
				{
					if (displayName == entry.displayName)
					{
						return entry.value;
					}
					else
					{
						return {};
					}
				}
			);
		}

		[[nodiscard]] constexpr Math::Range<UnderlyingType> GetRange() const
		{
			Math::Range<UnderlyingType> range = Math::Range<UnderlyingType>::Make(0, 0);

			const auto& firstEnumTypeEntry = entries.template Get<0>();

			const UnderlyingType firstValue = static_cast<UnderlyingType>(firstEnumTypeEntry.value);
			range = Math::Range<UnderlyingType>::Make(firstValue, 1);

			entries.ForEach(
				[&range](const auto& enumTypeEntry)
				{
					const UnderlyingType value = static_cast<UnderlyingType>(enumTypeEntry.value);
					range = Math::Range<UnderlyingType>::MakeStartToEnd(Math::Min(range.GetMinimum(), value), Math::Max(range.GetMaximum(), value));
				}
			);
			return range;
		}

		virtual Any GetValue(uint64 value) const override final
		{
			return Any(static_cast<EnumType>(value));
		}

		virtual uint64 GetValue(const AnyView value) const override final
		{
			return static_cast<uint64>(value.GetExpected<EnumType>());
		}

		virtual Optional<uint64> GetValue(const ConstUnicodeStringView displayName) const override final
		{
			if (const Optional<EnumType> value = ToEnumType(displayName))
			{
				return (uint64)*value;
			}
			else
			{
				return {};
			}
		}

		virtual ConstUnicodeStringView GetDisplayName(uint64 value) const override final
		{
			return ToString(static_cast<EnumType>(value));
		}

		virtual Asset::Guid GetIconAssetGuid(uint64 value) const override final
		{
			Asset::Guid assetGuid;
			entries.ForEach(
				[value, &assetGuid](const EnumTypeEntryType& entry)
				{
					if (value == static_cast<uint64>(entry.value))
					{
						assetGuid = entry.iconAssetGuid;
						return;
					}
				}
			);
			return assetGuid;
		}

		virtual size GetEntryCount() const override final
		{
			return entries.ElementCount;
		}

		virtual uint64 GetEntryValueAt(size index) const override final
		{
			Assert(index < entries.ElementCount);
			size count = 0;
			uint64 value = 0;
			entries.ForEach(
				[index, &count, &value](const EnumTypeEntryType& entry)
				{
					if (index == count++)
					{
						value = static_cast<uint64>(entry.value);
						return;
					}
				}
			);
			return value;
		}

		virtual ConstUnicodeStringView GetEntryDisplayNameAt(size index) const override final
		{
			Assert(index < entries.ElementCount);
			size count = 0;
			ConstUnicodeStringView displayName;
			entries.ForEach(
				[index, &count, &displayName](const EnumTypeEntryType& entry)
				{
					if (index == count++)
					{
						displayName = entry.displayName;
						return;
					}
				}
			);
			return displayName;
		}

		virtual Asset::Guid GetIconAssetGuidAt(size index) const override final
		{
			Assert(index < entries.ElementCount);
			size count = 0;
			Asset::Guid assetGuid;
			entries.ForEach(
				[index, &count, &assetGuid](const EnumTypeEntryType& entry)
				{
					if (index == count++)
					{
						assetGuid = entry.iconAssetGuid;
						return;
					}
				}
			);
			return assetGuid;
		}

		Tuple<Ts...> entries;
	};

	template<typename... Ts>
	EnumTypeExtension(Ts&&...) -> EnumTypeExtension<Ts...>;

	template<typename EnumType>
	[[nodiscard]] constexpr auto& GetEnum()
	{
		return GetType<EnumType>().GetExtension(EnumTypeInterface::TypeGuid);
	}
}
