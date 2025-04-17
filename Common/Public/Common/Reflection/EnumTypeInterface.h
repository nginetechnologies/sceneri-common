#pragma once

#include <Common/Memory/ForwardDeclarations/Any.h>
#include <Common/Memory/ForwardDeclarations/AnyView.h>
#include <Common/Memory/Containers/ForwardDeclarations/StringView.h>
#include <Common/Asset/Guid.h>
#include <Common/Reflection/Extension.h>

namespace ngine::Reflection
{
	struct EnumTypeInterface : public ExtensionInterface
	{
		inline static constexpr Guid TypeGuid = "1e10d447-0f8d-40fd-a367-70cf0ee59241"_guid;

		using BaseType = ExtensionInterface;
		using BaseType::BaseType;

		virtual Any GetValue(uint64 value) const = 0;
		virtual uint64 GetValue(const AnyView value) const = 0;
		virtual Optional<uint64> GetValue(const ConstUnicodeStringView displayName) const = 0;
		virtual ConstUnicodeStringView GetDisplayName(uint64 value) const = 0;
		virtual Asset::Guid GetIconAssetGuid(uint64 value) const = 0;

		virtual size GetEntryCount() const = 0;
		virtual uint64 GetEntryValueAt(size index) const = 0;
		virtual ConstUnicodeStringView GetEntryDisplayNameAt(size index) const = 0;
		virtual Asset::Guid GetIconAssetGuidAt(size index) const = 0;
	};
}
