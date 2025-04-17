#pragma once

#include <Common/Asset/Guid.h>
#include <Common/Asset/AssetDatabaseEntry.h>

#include <Common/Memory/Containers/UnorderedMap.h>
#include <Common/Memory/Containers/UnorderedSet.h>
#include <Common/Serialization/SavingFlags.h>

#include <Common/IO/URI.h>
#include <Common/IO/AccessModeFlags.h>
#include <Common/IO/SharingFlags.h>
#include <Common/ForwardDeclarations/EnumFlags.h>
#include <Common/Function/Function.h>
#include <Common/Memory/CallbackResult.h>

namespace ngine
{
	namespace IO
	{
		struct File;
	}

	struct EngineInfo;
	struct ProjectInfo;
	struct PluginInfo;

	extern template struct UnorderedMap<Guid, Asset::DatabaseEntry, Guid::Hash>;
}

namespace ngine::Asset
{
	struct LocalDatabase;

	namespace Internal
	{
		struct AssetEntryMap : public UnorderedMap<Guid, DatabaseEntry, Guid::Hash>
		{
			using UnorderedMap::UnorderedMap;
			using UnorderedMap::ValueType;
		};

		struct AssetSet : public UnorderedSet<Guid, Guid::Hash>
		{
			using UnorderedSet::UnorderedSet;
		};

		bool Serialize(ngine::Asset::Internal::AssetEntryMap& map, const Serialization::Reader serializer, const IO::PathView prefix);
		bool Serialize(const ngine::Asset::Internal::AssetEntryMap& map, Serialization::Writer serializer, const IO::PathView rootDirectory);
	}

	struct Database
	{
		inline static constexpr Format AssetFormat = {"77b22fc4-d109-4dd3-a922-38388879fc90"_asset, MAKE_PATH(".nassetdb")};

		Database() = default;
		Database(const IO::ConstZeroTerminatedPathView databaseFile, const IO::PathView databaseRootDirectory);
		Database(const Serialization::Data& data, const IO::PathView prefix);

		void SetGuid(const Guid guid)
		{
			m_guid = guid;
		}
		[[nodiscard]] Guid GetGuid() const
		{
			return m_guid;
		}

		bool Load(const IO::ConstZeroTerminatedPathView databaseFile, const IO::PathView databaseRootDirectory);
		bool Load(const Serialization::Data& data, const IO::PathView prefix);
		bool Load(const EngineInfo& engine);
		bool Load(const ProjectInfo& project);
		bool Load(const PluginInfo& plugin);

		[[nodiscard]] bool Generate(const EngineInfo& engine);
		[[nodiscard]] bool Generate(const ProjectInfo& project);
		[[nodiscard]] bool Generate(const PluginInfo& plugin);

		[[nodiscard]] bool LoadAndGenerate(const EngineInfo& engine);
		[[nodiscard]] bool LoadAndGenerate(const ProjectInfo& project);
		[[nodiscard]] bool LoadAndGenerate(const PluginInfo& plugin);

		void Import(const Database& other);

		void Reserve(const uint32 count)
		{
			m_assetMap.Reserve(m_assetMap.GetSize() + count);
		}
		void ReserveImported(const uint32 count)
		{
			m_importedAssets.Reserve(m_importedAssets.GetSize() + count);
		}

		DatabaseEntry& RegisterAsset(const Guid assetGuid, DatabaseEntry&& entry, const IO::PathView databaseRootDirectory);
		void ImportAsset(const Guid assetGuid);
		bool RemoveImportedAsset(const Guid assetGuid);
		Optional<DatabaseEntry> RemoveAsset(const Guid assetGuid);
		[[nodiscard]] bool
		RegisterAllAssetsInDirectory(const IO::ConstZeroTerminatedPathView directory, const IO::PathView databaseRootDirectory);

		using AssetDiscoveryCallback = Function<void(IO::Path&& filePath), 96>;
		static void FindAssetsInDirectory(const IO::ConstZeroTerminatedPathView directory, AssetDiscoveryCallback&& callback);

		[[nodiscard]] bool Save(const IO::ConstZeroTerminatedPathView databaseFile, const EnumFlags<Serialization::SavingFlags> flags);

		[[nodiscard]] bool HasAsset(const Guid assetGuid) const
		{
			return m_assetMap.Contains(assetGuid);
		}

		[[nodiscard]] Optional<const DatabaseEntry*> GetAssetEntry(const Guid assetGuid) const LIFETIME_BOUND
		{
			const decltype(m_assetMap)::const_iterator it = m_assetMap.Find(assetGuid);
			return Optional<const DatabaseEntry*>(&it->second, it != m_assetMap.end());
		}
		[[nodiscard]] Optional<DatabaseEntry*> GetAssetEntry(const Guid assetGuid) LIFETIME_BOUND
		{
			const decltype(m_assetMap)::iterator it = m_assetMap.Find(assetGuid);
			return Optional<DatabaseEntry*>(&it->second, it != m_assetMap.end());
		}

		[[nodiscard]] uint32 GetAssetCount() const
		{
			return m_assetMap.GetSize();
		}
		[[nodiscard]] uint32 GetImportedAssetCount() const
		{
			return m_importedAssets.GetSize();
		}

		template<typename Callback>
		void IterateAssets(Callback&& callback)
		{
			for (auto& pair : m_assetMap)
			{
				if (callback(pair.first, pair.second) == Memory::CallbackResult::Break)
				{
					break;
				}
			}
		}

		template<typename Callback>
		void IterateAssets(Callback&& callback) const
		{
			for (auto& pair : m_assetMap)
			{
				if (callback(pair.first, pair.second) == Memory::CallbackResult::Break)
				{
					break;
				}
			}
		}

		template<typename Callback>
		void IterateAssetsOfAssetType(Callback&& callback, const ArrayView<const TypeGuid> typeGuids)
		{
			for (auto& pair : m_assetMap)
			{
				if (typeGuids.Contains(pair.second.m_assetTypeGuid))
				{
					if (callback(pair.first, pair.second) == Memory::CallbackResult::Break)
					{
						break;
					}
				}
			}
		}

		template<typename Callback>
		void IterateAssetsOfAssetType(Callback&& callback, const TypeGuid typeGuid) const
		{
			for (auto& pair : m_assetMap)
			{
				if (typeGuid == pair.second.m_assetTypeGuid)
				{
					if (callback(pair.first, pair.second) == Memory::CallbackResult::Break)
					{
						break;
					}
				}
			}
		}

		template<typename Callback>
		void IterateAssetsOfAssetType(Callback&& callback, const TypeGuid typeGuid)
		{
			for (auto& pair : m_assetMap)
			{
				if (typeGuid == pair.second.m_assetTypeGuid)
				{
					if (callback(pair.first, pair.second) == Memory::CallbackResult::Break)
					{
						break;
					}
				}
			}
		}

		template<typename Callback>
		void IterateAssetsOfComponentType(Callback&& callback, const ArrayView<const Guid> typeGuids)
		{
			for (auto& pair : m_assetMap)
			{
				if (typeGuids.Contains(pair.second.m_componentTypeGuid))
				{
					if (callback(pair.first, pair.second) == Memory::CallbackResult::Break)
					{
						break;
					}
				}
			}
		}

		template<typename Callback>
		void IterateImportedAssets(Callback&& callback) const
		{
			for (const Guid assetGuid : m_importedAssets)
			{
				if (callback(assetGuid) == Memory::CallbackResult::Break)
				{
					break;
				}
			}
		}

		//! Replaces all assets in this database that are also present in the other with imports
		void ReplaceDuplicateAssetsWithImports(const Database& otherDatabase);

		[[nodiscard]] bool IsAssetImported(const Guid guid) const
		{
			return m_importedAssets.Contains(guid);
		}

		bool Serialize(const Serialization::Reader reader, const IO::PathView prefix);
		bool Serialize(Serialization::Writer writer) const;
		bool Serialize(Serialization::Writer writer, const IO::PathView rootDirectory) const;
	protected:
		Guid m_guid = Guid::Generate();
		Internal::AssetEntryMap m_assetMap;
		Internal::AssetSet m_importedAssets;
	};
}
