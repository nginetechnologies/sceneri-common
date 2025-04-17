#pragma once

#include "Guid.h"
#include "Types.h"

#include <Common/Serialization/ForwardDeclarations/Reader.h>
#include <Common/Serialization/ForwardDeclarations/Writer.h>
#include <Common/Memory/Containers/ForwardDeclarations/BitView.h>

#include <Common/Memory/Containers/InlineVector.h>

namespace ngine::Asset
{
	struct Reference
	{
		Reference()
		{
		}
		Reference(const Guid assetGuid, const ngine::Asset::TypeGuid typeGuid)
			: m_assetGuid(assetGuid)
			, m_assetTypeGuid(typeGuid)
		{
		}

		inline static constexpr ngine::Guid TypeGuid = "{87379F18-3FCE-4B4C-9A05-C7567EA580DB}"_guid;

		[[nodiscard]] bool operator==(const Reference other) const
		{
			return (m_assetGuid == other.m_assetGuid) & (m_assetTypeGuid == other.m_assetTypeGuid);
		}
		[[nodiscard]] bool operator!=(const Reference other) const
		{
			return !operator==(other);
		}

		[[nodiscard]] Guid GetAssetGuid() const
		{
			return m_assetGuid;
		}
		[[nodiscard]] ngine::Asset::TypeGuid GetTypeGuid() const
		{
			return m_assetTypeGuid;
		}

		[[nodiscard]] bool IsValid() const
		{
			return m_assetGuid.IsValid();
		}

		bool Serialize(const Serialization::Reader serializer);
		bool Serialize(Serialization::Writer serializer) const;

		[[nodiscard]] static constexpr uint32 CalculateCompressedDataSize()
		{
			// Worst case if neither asset nor type is bound to network we'll have to send the two guids.
			return sizeof(Guid) * 2 * 8;
		}
		bool Compress(BitView& target) const;
		bool Decompress(ConstBitView& source);
	protected:
		Guid m_assetGuid;
		ngine::Asset::TypeGuid m_assetTypeGuid;
	};

	//! An asset reference to an asset in the asset library
	struct LibraryReference : private Reference
	{
		LibraryReference() = default;
		LibraryReference(const Guid assetGuid, const ngine::Asset::TypeGuid typeGuid)
			: Reference(assetGuid, typeGuid)
		{
		}

		inline static constexpr ngine::Guid TypeGuid = "0a4c6e07-854c-44cb-9d2b-df3e5311d797"_guid;

		[[nodiscard]] bool operator==(const LibraryReference& other) const
		{
			return Reference::operator==(other);
		}
		[[nodiscard]] bool operator!=(const LibraryReference& other) const
		{
			return Reference::operator!=(other);
		}
		using Reference::operator==;
		using Reference::operator!=;
		using Reference::GetAssetGuid;
		using Reference::GetTypeGuid;
		using Reference::IsValid;
		using Reference::Serialize;
	};
}
