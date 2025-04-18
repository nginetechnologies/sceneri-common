#pragma once

#include "Types.h"
#include "Reference.h"

namespace ngine::Asset
{
	struct Picker
	{
		inline static constexpr ngine::Guid TypeGuid = "{7210A8BB-AA4C-4BB9-8904-E3B7DDBF95A0}"_guid;

		Picker() = default;
		Picker(const Guid assetGuid, const ngine::Asset::TypeGuid assetTypeGuid)
			: m_asset{assetGuid, assetTypeGuid}
			, m_allowedAssetTypes{assetTypeGuid}
		{
		}
		Picker(const Reference asset)
			: m_asset(asset)
			, m_allowedAssetTypes(asset.GetTypeGuid())
		{
		}
		Picker(const Reference asset, Types&& allowedTypes)
			: m_asset(asset)
			, m_allowedAssetTypes(Forward<Types>(allowedTypes))
		{
		}
		Picker(Types&& allowedTypes)
			: m_allowedAssetTypes(Forward<Types>(allowedTypes))
		{
		}
		Picker(const Picker&) = default;
		Picker(Picker&&) = default;
		Picker& operator=(const Picker& asset) = default;
		Picker& operator=(Picker&& asset) = default;
		Picker& operator=(const Reference asset)
		{
			m_asset = asset;
			return *this;
		}

		[[nodiscard]] bool operator==(const Picker& other) const
		{
			return m_asset == other.m_asset;
		}
		[[nodiscard]] bool operator!=(const Picker& other) const
		{
			return !operator==(other);
		}

		bool Serialize(const Serialization::Reader serializer);
		bool Serialize(Serialization::Writer serializer) const;

		[[nodiscard]] Guid GetAssetGuid() const
		{
			return m_asset.GetAssetGuid();
		}
		[[nodiscard]] Reference GetAssetReference() const
		{
			return m_asset;
		}

		[[nodiscard]] ArrayView<const ngine::Asset::TypeGuid, uint16> GetAllowedTypeGuids() const
		{
			return m_allowedAssetTypes.GetAllowedTypeGuids();
		}

		[[nodiscard]] bool IsValid() const
		{
			return m_asset.IsValid();
		}

		Reference m_asset;
		Types m_allowedAssetTypes;
	};
}
