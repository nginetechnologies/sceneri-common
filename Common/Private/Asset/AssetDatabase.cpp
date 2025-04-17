#include "Asset/AssetDatabase.h"
#include "Asset/LocalAssetDatabase.h"
#include "Asset/Asset.h"
#include "Asset/Serialization/MetaData.h"

#include "Project System/EngineInfo.h"
#include "Project System/EngineAssetFormat.h"
#include "Project System/ProjectInfo.h"
#include "Project System/ProjectAssetFormat.h"
#include "Project System/PluginInfo.h"
#include "Project System/PluginAssetFormat.h"

#include <Common/IO/FileIterator.h>
#include <Common/IO/Log.h>
#include <Common/Serialization/Deserialize.h>
#include <Common/Serialization/Serialize.h>
#include <Common/Memory/Containers/Serialization/UnorderedMap.h>
#include <Common/Memory/Containers/Serialization/UnorderedSet.h>
#include <Common/Memory/Containers/Serialization/Vector.h>
#include <Common/Serialization/Guid.h>
#include <Common/IO/Format/Path.h>
#include <Common/IO/Format/ZeroTerminatedPathView.h>
#include <Common/Asset/Format/Guid.h>
#include <Common/System/Query.h>
#include <Common/Reflection/Registry.h>

namespace ngine::Asset
{
	bool DatabaseEntry::Serialize(const Serialization::Reader serializer, const IO::PathView prefix)
	{
		if (serializer.GetValue().IsObject())
		{
			serializer.Serialize("assetTypeGuid", m_assetTypeGuid);
			serializer.Serialize("componentTypeGuid", m_componentTypeGuid);
			serializer.Serialize("path", m_path);
			serializer.Serialize("name", m_name);
			serializer.Serialize("thumbnail", m_thumbnailGuid);
			serializer.Serialize("tags", m_tags);
			serializer.Serialize("dependencies", m_dependencies);
			serializer.Serialize("metadata", m_metaData);
			if (m_path.IsRelative() & prefix.HasElements())
			{
				m_path = IO::Path::Combine(prefix, m_path);
			}
			return true;
		}
		return false;
	}
	bool DatabaseEntry::Serialize(Serialization::Writer serializer, const IO::PathView rootDirectory) const
	{
		serializer.Serialize("assetTypeGuid", m_assetTypeGuid);
		serializer.Serialize("componentTypeGuid", m_componentTypeGuid);

		if (m_path.IsRelativeTo(rootDirectory))
		{
			serializer.Serialize("path", IO::Path(m_path.GetRelativeToParent(rootDirectory)));
		}
		else
		{
			serializer.Serialize("path", m_path);
		}
		serializer.Serialize("name", m_name);
		serializer.Serialize("thumbnail", m_thumbnailGuid);
		serializer.Serialize("tags", m_tags);
		serializer.Serialize("dependencies", m_dependencies);
		serializer.Serialize("metadata", m_metaData);
		return true;
	}

	namespace Internal
	{
		inline bool Serialize(ngine::Asset::Internal::AssetEntryMap& map, const Serialization::Reader serializer, const IO::PathView prefix)
		{
			return ngine::SerializeWithCallback(
				map,
				serializer,
				[](auto& assetMap, const ConstStringView name, DatabaseEntry&& value)
				{
					const Guid guid = Guid::TryParse(name);
					if (LIKELY(guid.IsValid()))
					{
						assetMap.EmplaceOrAssign(Guid(guid), Forward<DatabaseEntry>(value));
					}
				},
				prefix
			);
		}

		inline bool
		Serialize(const ngine::Asset::Internal::AssetEntryMap& map, Serialization::Writer serializer, const IO::PathView rootDirectory)
		{
			return ngine::SerializeWithCallback<FlatString<37>>(
				map,
				serializer,
				[](const Guid& key) -> FlatString<37>
				{
					return key.ToString();
				},
				rootDirectory
			);
		}
	}

	DatabaseEntry::DatabaseEntry(const Asset& asset)
		: DatabaseEntry{
				asset.GetTypeGuid(),
				asset.GetComponentTypeGuid(),
				IO::Path(asset.GetMetaDataFilePath()),
				UnicodeString(asset.GetName()),
				UnicodeString{},
				asset.GetThumbnailGuid(),
				DatabaseEntry::TagsStorage{asset.GetTags()},
				DatabaseEntry::DependenciesStorage{asset.GetDependencies()},
				ContainerContentsStorage{},
				MetaDataStorage{asset.GetMetaData()}
			}
	{
	}

	DatabaseEntry::DatabaseEntry(const ProjectInfo& projectInfo)
		: DatabaseEntry{
				ProjectAssetFormat.assetTypeGuid,
				{},
				IO::Path(projectInfo.GetConfigFilePath()),
				UnicodeString(projectInfo.GetName()),
				UnicodeString(projectInfo.GetDescription()),
				projectInfo.GetThumbnailGuid(),
				DatabaseEntry::TagsStorage{projectInfo.GetTags()}
			}
	{
	}

	DatabaseEntry::DatabaseEntry(const PluginInfo& plugin)
		: DatabaseEntry{
				PluginAssetFormat.assetTypeGuid,
				{},
				IO::Path(plugin.GetConfigFilePath()),
				UnicodeString(plugin.GetName()),
				UnicodeString(plugin.GetDescription())
			}
	{
	}

	DatabaseEntry::DatabaseEntry(const EngineInfo& engineInfo)
		: DatabaseEntry{EngineAssetFormat.assetTypeGuid, {}, IO::Path(engineInfo.GetConfigFilePath()), UnicodeString(engineInfo.GetName())}
	{
	}

	IO::Path::StringType DatabaseEntry::GetName() const
	{
		if (m_name.HasElements())
		{
			return m_name;
		}

		return GetNameFromPath();
	}

	IO::Path::StringType DatabaseEntry::GetNameFromPath() const
	{
		IO::PathView fileName = m_path.GetFileNameWithoutExtensions();
		if (fileName == MAKE_PATH("Main"))
		{
			// Special case for package assets, the name "Main" indicates the name is in the parent directory
			const IO::PathView directoryName = m_path.GetParentPath().GetFileName();
			if (directoryName.GetRightMostExtension() == Asset::FileExtension)
			{
				fileName = directoryName.GetWithoutExtensions();
			}
		}

		IO::Path::StringType name = IO::Path::StringType(fileName.GetStringView());
		name.ReplaceCharacterOccurrences(MAKE_PATH_LITERAL('_'), MAKE_PATH_LITERAL(' '));
		return Move(name);
	}

	Database::Database(const IO::ConstZeroTerminatedPathView databaseFilePath, const IO::PathView databaseRootDirectory)
	{
		if (databaseFilePath.HasElements())
		{
			Load(databaseFilePath, databaseRootDirectory);
		}
	}

	Database::Database(const Serialization::Data& data, const IO::PathView prefix)
	{
		Load(data, prefix);
	}

	bool Database::Load(const IO::ConstZeroTerminatedPathView databaseFile, const IO::PathView databaseRootDirectory)
	{
		return Serialization::DeserializeFromDisk(databaseFile, *this, databaseRootDirectory);
	}

	bool Database::Load(const Serialization::Data& data, const IO::PathView prefix)
	{
		return Serialization::Deserialize(data, *this, prefix);
	}

	bool Database::Load(const EngineInfo& engineInfo)
	{
		const IO::Path assetsDatabasePath = IO::Path::Combine(
			engineInfo.GetDirectory(),
			IO::Path::Merge(EngineInfo::EngineAssetsPath, Database::AssetFormat.metadataFileExtension)
		);
		if (assetsDatabasePath.Exists())
		{
			Serialization::Data data(assetsDatabasePath);
			if (data.IsValid())
			{
				return Load(data, assetsDatabasePath.GetParentPath());
			}
		}
		return false;
	}

	bool Database::Load(const ProjectInfo& projectInfo)
	{
		const IO::Path assetsDatabasePath = IO::Path::Combine(
			projectInfo.GetDirectory(),
			IO::Path::Merge(projectInfo.GetRelativeAssetDirectory(), Database::AssetFormat.metadataFileExtension)
		);
		if (assetsDatabasePath.Exists())
		{
			Serialization::Data data(assetsDatabasePath);
			if (data.IsValid())
			{
				return Load(data, assetsDatabasePath.GetParentPath());
			}
			else
			{
				LogError("Could not parse database {}. The file most likely contains invalid json.", assetsDatabasePath.GetView());
			}
		}
		else
		{
			LogError("Could not find database file at {}", assetsDatabasePath.GetView());
		}
		return false;
	}

	bool Database::Load(const PluginInfo& plugin)
	{
		const IO::Path assetsDatabasePath = IO::Path::Combine(
			plugin.GetDirectory(),
			IO::Path::Merge(plugin.GetRelativeAssetDirectory(), Database::AssetFormat.metadataFileExtension)
		);
		if (assetsDatabasePath.Exists())
		{
			Serialization::Data data(assetsDatabasePath);
			if (data.IsValid())
			{
				return Load(data, assetsDatabasePath.GetParentPath());
			}
		}
		return false;
	}

	bool Database::Generate(const EngineInfo& engineInfo)
	{
		const IO::PathView rootFolder = engineInfo.GetDirectory();
		RegisterAsset(engineInfo.GetGuid(), DatabaseEntry{engineInfo}, rootFolder);

		const IO::Path engineAssetsPath = IO::Path::Combine(engineInfo.GetDirectory(), EngineInfo::EngineAssetsPath);
		return RegisterAllAssetsInDirectory(engineAssetsPath, rootFolder);
	}

	bool Database::Generate(const ProjectInfo& projectInfo)
	{
		const IO::PathView rootFolder = projectInfo.GetDirectory();
		RegisterAsset(projectInfo.GetGuid(), DatabaseEntry{projectInfo}, rootFolder);

		if (projectInfo.GetAssetDatabaseGuid().IsValid())
		{
			RegisterAsset(
				projectInfo.GetAssetDatabaseGuid(),
				DatabaseEntry{
					Database::AssetFormat.assetTypeGuid,
					{},
					IO::Path::Combine(
						projectInfo.GetDirectory(),
						IO::Path::Merge(projectInfo.GetRelativeAssetDirectory(), AssetFormat.metadataFileExtension)
					)
				},
				rootFolder
			);
		}

		const IO::Path projectAssetsFolder = IO::Path::Combine(projectInfo.GetDirectory(), projectInfo.GetRelativeAssetDirectory());
		return RegisterAllAssetsInDirectory(projectAssetsFolder, rootFolder);
	}

	bool Database::Generate(const PluginInfo& plugin)
	{
		const IO::PathView rootFolder = plugin.GetDirectory();
		RegisterAsset(plugin.GetGuid(), DatabaseEntry{plugin}, rootFolder);

		const IO::Path projectAssetsFolder = IO::Path::Combine(plugin.GetDirectory(), plugin.GetRelativeAssetDirectory());
		return RegisterAllAssetsInDirectory(projectAssetsFolder, rootFolder);
	}

	bool Database::LoadAndGenerate(const EngineInfo& engineInfo)
	{
		Load(engineInfo);
		m_assetMap.Clear();
		return Generate(engineInfo);
	}

	bool Database::LoadAndGenerate(const ProjectInfo& projectInfo)
	{
		Load(projectInfo);
		m_assetMap.Clear();
		return Generate(projectInfo);
	}

	bool Database::LoadAndGenerate(const PluginInfo& plugin)
	{
		Load(plugin);
		m_assetMap.Clear();
		return Generate(plugin);
	}

	void Database::Import(const Database& other)
	{
		m_assetMap.Reserve(m_assetMap.GetSize() + other.m_assetMap.GetSize());
		m_importedAssets.Reserve(m_importedAssets.GetSize() + other.m_importedAssets.GetSize());

		for (const auto& entryPair : other.m_assetMap)
		{
			m_assetMap.EmplaceOrAssign(Guid(entryPair.first), DatabaseEntry{entryPair.second});
		}

		for (const Guid importedAssetGuid : other.m_importedAssets)
		{
			m_importedAssets.Emplace(Guid(importedAssetGuid));
		}
	}

	void Database::FindAssetsInDirectory(const IO::ConstZeroTerminatedPathView directory, AssetDiscoveryCallback&& callback)
	{
		IO::FileIterator::TraverseDirectoryRecursive(
			directory,
			[&callback](IO::Path&& filePath) -> IO::FileIterator::TraversalResult
			{
				const IO::PathView fileExtension = filePath.GetRightMostExtension();

				if ((fileExtension == Asset::Asset::FileExtension || fileExtension == ProjectAssetFormat.metadataFileExtension || fileExtension == PluginAssetFormat.metadataFileExtension || fileExtension == Database::AssetFormat.metadataFileExtension) && filePath.IsFile())
				{
					callback(Forward<IO::Path>(filePath));
				}

				return IO::FileIterator::TraversalResult::Continue;
			}
		);
	}

	bool Database::RegisterAllAssetsInDirectory(const IO::ConstZeroTerminatedPathView directory, const IO::PathView databaseRootDirectory)
	{
		// Maintain a set to validate that no duplicate guids are found
		Internal::AssetSet foundAssets;
		bool success{true};

		FindAssetsInDirectory(
			directory,
			[this, databaseRootDirectory, &foundAssets, &success](IO::Path&& filePath) mutable
			{
				Serialization::Data assetData(filePath);
				success &= assetData.IsValid();
				if (assetData.IsValid())
				{
					Asset asset(assetData, Move(filePath));
					// success &= asset.IsValid();
				  // if(asset.IsValid())
					{
						const bool foundAsset = foundAssets.Contains(asset.GetGuid());
						success &= !foundAsset;
						if (LIKELY(!foundAsset))
						{
							foundAssets.Emplace(Guid(asset.GetGuid()));
						}
						else
						{
							LogWarning(
								"Found duplicate asset {} (in {}) in directory {}",
								asset.GetGuid(),
								asset.GetMetaDataFilePath(),
								databaseRootDirectory
							);
						}

						if (const Optional<DatabaseEntry*> pAssetEntry = GetAssetEntry(asset.GetGuid()))
						{
							*pAssetEntry = DatabaseEntry{asset};
							pAssetEntry->m_path.MakeRelativeToParent(databaseRootDirectory);
						}
						else
						{
							RegisterAsset(asset.GetGuid(), DatabaseEntry{asset}, databaseRootDirectory);
						}
					}
					/*else
				  {
				      LogWarning("Encountered invalid asset {}, is it missing 'guid' or 'assetTypeGuid'?", asset.GetMetaDataFilePath());
				  }*/
				}
				else
				{
					LogWarning("Found invalid JSON in asset {}", filePath);
				}
			}
		);

		return success;
	}

	DatabaseEntry& Database::RegisterAsset(const Guid assetGuid, DatabaseEntry&& entry, const IO::PathView databaseRootDirectory)
	{
		Assert(assetGuid.IsValid());
		entry.m_path.MakeRelativeToParent(databaseRootDirectory);
		return m_assetMap.EmplaceOrAssign(Guid(assetGuid), Forward<DatabaseEntry>(entry))->second;
	}

	void Database::ImportAsset(const Guid assetGuid)
	{
		m_importedAssets.Emplace(Guid(assetGuid));
	}

	bool Database::RemoveImportedAsset(const Guid assetGuid)
	{
		auto it = m_importedAssets.Find(assetGuid);
		if (it != m_importedAssets.end())
		{
			m_importedAssets.Remove(it);
			return true;
		}
		return false;
	}

	Optional<DatabaseEntry> Database::RemoveAsset(const Guid assetGuid)
	{
		const decltype(m_assetMap)::iterator it = m_assetMap.Find(assetGuid);
		if (it != m_assetMap.end())
		{
			DatabaseEntry entry = Move(it->second);
			m_assetMap.Remove(it);
			return entry;
		}
		else
		{
			return Invalid;
		}
	}

	bool Database::Save(const IO::ConstZeroTerminatedPathView databaseFile, const EnumFlags<Serialization::SavingFlags> flags)
	{
		const IO::Path databaseFilePath(databaseFile);
		const IO::PathView databaseRootDirectory = databaseFilePath.GetParentPath();
		return Serialization::SerializeToDisk(databaseFile, *this, flags, databaseRootDirectory);
	}

	bool Database::Serialize(const Serialization::Reader reader, const IO::PathView prefix)
	{
		reader.Serialize("guid", m_guid);
		bool serializedAny = reader.Serialize("imported_assets", m_importedAssets);
		serializedAny |= reader.SerializeInPlace(m_assetMap, prefix);
		return serializedAny;
	}

	bool Database::Serialize(Serialization::Writer writer) const
	{
		const IO::PathView rootDirectory = IO::PathView();
		return Serialize(writer, rootDirectory);
	}

	bool Database::Serialize(Serialization::Writer writer, const IO::PathView rootDirectory) const
	{
		writer.Serialize("guid", m_guid);
		bool serializedAny = writer.Serialize("imported_assets", m_importedAssets);
		serializedAny |= writer.SerializeInPlace(m_assetMap, rootDirectory);
		return serializedAny;
	}

	void Database::ReplaceDuplicateAssetsWithImports(const Database& otherDatabase)
	{
		for (auto it = m_assetMap.begin(), endIt = m_assetMap.end(); it != endIt;)
		{
			const Guid assetGuid = it->first;
			if (otherDatabase.HasAsset(assetGuid))
			{
				Assert(!m_importedAssets.Contains(assetGuid));
				it = m_assetMap.Remove(it);
				endIt = m_assetMap.end();

				m_importedAssets.Emplace(Guid(assetGuid));
			}
			else
			{
				++it;
			}
		}
	}

	/* static */ IO::Path LocalDatabase::GetDirectoryPath()
	{
		return IO::Path::Combine(IO::Path::GetUserDataDirectory(), DirectoryName);
	}
	IO::Path GetLocalDatabaseConfigFilePath(const IO::Path& directoryPath)
	{
		return IO::Path::Merge(directoryPath, Database::AssetFormat.metadataFileExtension);
	}

	/* static */ IO::Path LocalDatabase::GetConfigFilePath()
	{
		return GetLocalDatabaseConfigFilePath(GetDirectoryPath());
	}

	/* static */ IO::Path
	LocalDatabase::GetAssetPath(const IO::PathView& assetCacheDirectoryPath, const Guid assetGuid, const IO::PathView assetExtension)
	{
		return IO::Path::Combine(assetCacheDirectoryPath, IO::Path::Merge(assetGuid.ToString().GetView(), assetExtension));
	}

	/* static */ IO::Path LocalDatabase::GetAssetPath(const Guid assetGuid, const IO::PathView assetExtension)
	{
		return GetAssetPath(GetDirectoryPath(), assetGuid, assetExtension);
	}

	LocalDatabase::LocalDatabase()
	{
		const IO::Path configFilePath = GetLocalDatabaseConfigFilePath(GetDirectoryPath());
		Load(configFilePath, configFilePath.GetParentPath());
	}

	Optional<DatabaseEntry*> LocalDatabase::RegisterAsset(const Serialization::Data& assetData, const Asset& asset)
	{
		if (LIKELY(asset.IsValid()))
		{
			if (LIKELY(!HasAsset(asset.GetGuid())))
			{
				const IO::Path assetCacheDirectoryPath = GetDirectoryPath();
				IO::Path targetPath = GetAssetPath(assetCacheDirectoryPath, asset.GetGuid(), asset.GetMetaDataFilePath().GetAllExtensions());
				return RegisterAssetInternal(assetData, asset, Move(targetPath));
			}
		}
		return Invalid;
	}

	Optional<DatabaseEntry*> LocalDatabase::RegisterAsset(
		const Serialization::Data& assetData,
		const Asset& asset,
		const Guid parentAssetGuid,
		const IO::PathView parentAssetExtensions,
		const IO::PathView relativePath
	)
	{
		Assert(parentAssetGuid.IsValid());
		if (LIKELY(asset.IsValid()))
		{
			if (LIKELY(!HasAsset(asset.GetGuid())))
			{
				const IO::Path assetCacheDirectoryPath = GetDirectoryPath();
				IO::Path targetPath = GetAssetPath(assetCacheDirectoryPath, parentAssetGuid, parentAssetExtensions);
				targetPath = IO::Path::Combine(targetPath, relativePath.GetParentPath(), asset.GetMetaDataFilePath().GetFileName());
				return RegisterAssetInternal(assetData, asset, Move(targetPath));
			}
		}
		return Invalid;
	}

	Optional<DatabaseEntry*>
	LocalDatabase::RegisterAssetInternal(const Serialization::Data& assetData, const Asset& asset, IO::Path&& targetPath)
	{
		if (!IO::Path{targetPath.GetParentPath()}.Exists())
		{
			IO::Path{targetPath.GetParentPath()}.CreateDirectories();
		}

		Serialization::Data targetAssetData(assetData);
		Asset newAsset{asset};
		newAsset.SetMetaDataFilePath(targetPath);
		newAsset.SetName(asset.GetName());
		Serialization::Serialize(targetAssetData, newAsset);

		if (LIKELY(targetAssetData.SaveToFile(targetPath, Serialization::SavingFlags{})))
		{
			// This asset could have multiple (binary) files, e.g. '.tex.bc' and '.tex.astc'
			const IO::Path sourceBinaryFilePath = asset.GetBinaryFilePath();
			if (sourceBinaryFilePath.HasElements())
			{
				bool failedAny{false};
				const IO::Path sourceDirectory = IO::Path(asset.GetDirectory());
				const IO::Path targetDirectory = IO::Path(newAsset.GetDirectory());
				IO::FileIterator::TraverseDirectory(
					IO::Path(sourceBinaryFilePath.GetParentPath()),
					[sourceBinaryFilePath,
				   sourceDirectory,
				   sourceMetadataFilePath = asset.GetMetaDataFilePath(),
				   newFileName = newAsset.GetMetaDataFilePath().GetFileNameWithoutExtensions(),
				   targetDirectory,
				   &failedAny](IO::Path&& sourceFilePath) -> IO::FileIterator::TraversalResult
					{
						if (sourceFilePath.GetView().StartsWith(sourceBinaryFilePath) && sourceFilePath != sourceMetadataFilePath)
						{
							const IO::PathView relativeBinaryPath = sourceFilePath.GetRelativeToParent(sourceDirectory);
							const IO::Path targetBinaryPath = IO::Path::Combine(
								targetDirectory,
								IO::Path::Merge(relativeBinaryPath.GetParentPath(), newFileName, relativeBinaryPath.GetAllExtensions())
							);
							if (LIKELY(!targetBinaryPath.Exists()))
							{
								failedAny |= !sourceFilePath.CopyFileTo(targetBinaryPath);
							}
						}
						return IO::FileIterator::TraversalResult::Continue;
					}
				);
				if (UNLIKELY_ERROR(failedAny))
				{
					return Invalid;
				}
			}

			DatabaseEntry& createdEntry =
				Database::RegisterAsset(newAsset.GetGuid(), DatabaseEntry{newAsset}, GetDirectoryPath().GetParentPath());

			// Automatically copy into the sandboxed Mac Catalyst app directory on Mac
			if constexpr (PLATFORM_APPLE_MACOS)
			{
				IO::Path sandboxedAppPath =
					IO::Path::Combine(IO::Path::GetHomeDirectory(), MAKE_PATH("Library"), MAKE_PATH("Containers"), MAKE_PATH("com.ngine.Editor"));
				if (!sandboxedAppPath.Exists())
				{
					sandboxedAppPath.CreateDirectories();
				}

				IO::Path sandboxedAssetCacheDirectoryPath =
					IO::Path::Combine(sandboxedAppPath, MAKE_PATH("Data"), MAKE_PATH("Documents"), DirectoryName);
				if (!sandboxedAssetCacheDirectoryPath.Exists())
				{
					sandboxedAssetCacheDirectoryPath.CreateDirectories();
				}

				const IO::Path sandboxedConfigFilePath = GetLocalDatabaseConfigFilePath(sandboxedAssetCacheDirectoryPath);
				Database sandboxedLocalDatabase;
				sandboxedLocalDatabase.Load(sandboxedConfigFilePath, sandboxedAssetCacheDirectoryPath);

				const IO::Path sandboxedTargetPath =
					GetAssetPath(sandboxedAssetCacheDirectoryPath, newAsset.GetGuid(), newAsset.GetMetaDataFilePath().GetAllExtensions());
				if (LIKELY(newAsset.GetMetaDataFilePath().CopyFileTo(sandboxedTargetPath)))
				{
					Asset sandboxedAsset{newAsset};
					sandboxedAsset.SetMetaDataFilePath(sandboxedTargetPath);

					sandboxedLocalDatabase.RegisterAsset(newAsset.GetGuid(), DatabaseEntry{sandboxedAsset}, sandboxedAssetCacheDirectoryPath);
					const bool wasSaved = sandboxedLocalDatabase.Save(sandboxedConfigFilePath, Serialization::SavingFlags{});
					if (UNLIKELY_ERROR(!wasSaved))
					{
						return Invalid;
					}

					if (sourceBinaryFilePath.HasElements())
					{
						bool failedAny{false};
						const IO::Path sourceDirectory = IO::Path(asset.GetDirectory());
						const IO::Path sandboxedTargetDirectory = IO::Path(sandboxedAsset.GetDirectory());
						IO::FileIterator::TraverseDirectory(
							IO::Path(sourceBinaryFilePath.GetParentPath()),
							[sourceBinaryFilePath,
						   sourceDirectory,
						   sourceMetadataFilePath = asset.GetMetaDataFilePath(),
						   newFileName = newAsset.GetMetaDataFilePath().GetFileNameWithoutExtensions(),
						   targetDirectory = sandboxedTargetDirectory,
						   &failedAny](IO::Path&& sourceFilePath) -> IO::FileIterator::TraversalResult
							{
								if (sourceFilePath.GetView().StartsWith(sourceBinaryFilePath) && sourceFilePath != sourceMetadataFilePath)
								{
									const IO::PathView relativeBinaryPath = sourceFilePath.GetRelativeToParent(sourceDirectory);
									const IO::Path targetBinaryPath = IO::Path::Combine(
										targetDirectory,
										IO::Path::Merge(relativeBinaryPath.GetParentPath(), newFileName, relativeBinaryPath.GetAllExtensions())
									);
									if (LIKELY(!targetBinaryPath.Exists()))
									{
										failedAny |= !sourceFilePath.CopyFileTo(targetBinaryPath);
									}
								}
								return IO::FileIterator::TraversalResult::Continue;
							}
						);
						if (UNLIKELY_ERROR(failedAny))
						{
							return Invalid;
						}
					}
				}
				else
				{
					return Invalid;
				}
			}

			return createdEntry;
		}
		return Invalid;
	}

	Optional<DatabaseEntry*> LocalDatabase::RegisterAsset(const IO::Path& assetMetaFilePath)
	{
		if (assetMetaFilePath.Exists())
		{
			const IO::Path assetCacheDirectoryPath = GetDirectoryPath();
			if (!assetCacheDirectoryPath.Exists())
			{
				assetCacheDirectoryPath.CreateDirectories();
			}

			if (assetMetaFilePath.IsFile())
			{
				Serialization::Data assetData(assetMetaFilePath);
				if (LIKELY(assetData.IsValid()))
				{
					return RegisterAsset(assetData, Asset(assetData, IO::Path(assetMetaFilePath)));
				}
			}
			else if (assetMetaFilePath.IsDirectory())
			{
				const IO::PathView assetFileExtension = assetMetaFilePath.GetAllExtensions();
				IO::Path assetConfigFilePath = IO::Path::Combine(assetMetaFilePath, IO::Path::Merge(MAKE_PATH("Main"), assetFileExtension));
				bool shouldRenameRootAsset = false;
				if (!assetConfigFilePath.Exists())
				{
					assetConfigFilePath = IO::Path::Combine(assetMetaFilePath, assetMetaFilePath.GetFileName());
					shouldRenameRootAsset = true;
				}
				Serialization::Data assetData(assetConfigFilePath);
				if (LIKELY(assetData.IsValid()))
				{
					Asset asset(assetData, IO::Path(assetConfigFilePath));
					const IO::Path targetPath = GetAssetPath(assetCacheDirectoryPath, asset.GetGuid(), assetFileExtension);
					if (!targetPath.Exists())
					{
						targetPath.CreateDirectories();
					}

					if (LIKELY(asset.IsValid()))
					{
						bool foundDuplicates = false;
						FindAssetsInDirectory(
							assetMetaFilePath,
							[this, &foundDuplicates](IO::Path&& filePath)
							{
								Serialization::Data assetData(filePath);
								if (assetData.IsValid())
								{
									Asset asset(assetData, Move(filePath));
									foundDuplicates |= HasAsset(asset.GetGuid());
								}
							}
						);

						if (LIKELY(!foundDuplicates))
						{
							bool copiedAll{true};
							FindAssetsInDirectory(
								assetMetaFilePath,
								[&copiedAll, sourcePath = assetMetaFilePath, targetPath](IO::Path&& filePath)
								{
									Serialization::Data assetData(filePath);
									if (assetData.IsValid())
									{
										Asset asset(assetData, Move(filePath));

										IO::Path newPath = IO::Path::Combine(targetPath, filePath.GetRelativeToParent(sourcePath));

										const IO::Path sourceBinaryFilePath = asset.GetBinaryFilePath();
										if (sourceBinaryFilePath.HasElements())
										{
											bool failedAny{false};
											const IO::Path sourceDirectory = IO::Path(asset.GetDirectory());
											const IO::Path targetDirectory = IO::Path(newPath.GetParentPath());
											IO::FileIterator::TraverseDirectory(
												IO::Path(sourceBinaryFilePath.GetParentPath()),
												[sourceBinaryFilePath,
										     sourceDirectory,
										     sourceMetadataFilePath = asset.GetMetaDataFilePath(),
										     newFileName = newPath.GetFileNameWithoutExtensions(),
										     targetDirectory,
										     &failedAny](IO::Path&& sourceFilePath) -> IO::FileIterator::TraversalResult
												{
													if (sourceFilePath.GetView().StartsWith(sourceBinaryFilePath) && sourceFilePath != sourceMetadataFilePath)
													{
														const IO::PathView relativeBinaryPath = sourceFilePath.GetRelativeToParent(sourceDirectory);
														const IO::Path targetBinaryPath = IO::Path::Combine(
															targetDirectory,
															IO::Path::Merge(relativeBinaryPath.GetParentPath(), newFileName, relativeBinaryPath.GetAllExtensions())
														);
														if (LIKELY(!targetBinaryPath.Exists()))
														{
															failedAny |= !sourceFilePath.CopyFileTo(targetBinaryPath);
														}
													}
													return IO::FileIterator::TraversalResult::Continue;
												}
											);
											if (failedAny)
											{
												copiedAll = false;
											}
										}

										UnicodeString assetName = asset.GetName();

										asset.SetMetaDataFilePath(Move(newPath));
										asset.SetName(Move(assetName));

										Serialization::Serialize(assetData, asset);
										copiedAll &= assetData.SaveToFile(asset.GetMetaDataFilePath(), Serialization::SavingFlags{});
									}
									else
									{
										copiedAll = false;
									}
								}
							);

							if (LIKELY(copiedAll))
							{
								if (shouldRenameRootAsset)
								{
									const IO::Path rootFileName = IO::Path::Combine(targetPath, assetConfigFilePath.GetFileName());
									const IO::Path newRootFileName = IO::Path::Combine(targetPath, IO::Path::Merge(MAKE_PATH("Main"), assetFileExtension));
									[[maybe_unused]] const bool wasMoved = rootFileName.MoveFileTo(newRootFileName);
									Assert(wasMoved);
								}

								[[maybe_unused]] const bool registeredAllAssets =
									Database::RegisterAllAssetsInDirectory(targetPath, assetCacheDirectoryPath.GetParentPath());

								// Automatically copy into the sandboxed Mac Catalyst app directory on Mac
								if constexpr (PLATFORM_APPLE_MACOS)
								{
									IO::Path sandboxedAppPath = IO::Path::Combine(
										IO::Path::GetHomeDirectory(),
										MAKE_PATH("Library"),
										MAKE_PATH("Containers"),
										MAKE_PATH("com.ngine.Editor")
									);
									if (!sandboxedAppPath.Exists())
									{
										sandboxedAppPath.CreateDirectories();
									}

									IO::Path sandboxedAssetCacheDirectoryPath =
										IO::Path::Combine(sandboxedAppPath, MAKE_PATH("Data"), MAKE_PATH("Documents"), DirectoryName);
									if (!sandboxedAssetCacheDirectoryPath.Exists())
									{
										sandboxedAssetCacheDirectoryPath.CreateDirectories();
									}

									const IO::Path sandboxedConfigFilePath = GetLocalDatabaseConfigFilePath(sandboxedAssetCacheDirectoryPath);
									Database sandboxedLocalDatabase;
									sandboxedLocalDatabase.Load(sandboxedConfigFilePath, sandboxedAssetCacheDirectoryPath);

									const IO::Path sandboxedTargetPath = GetAssetPath(sandboxedAssetCacheDirectoryPath, asset.GetGuid(), assetFileExtension);
									if (LIKELY(assetMetaFilePath.CopyDirectoryTo(targetPath)))
									{
										if (sandboxedLocalDatabase.RegisterAllAssetsInDirectory(sandboxedTargetPath, sandboxedAssetCacheDirectoryPath))
										{
											[[maybe_unused]] const bool wasSaved =
												sandboxedLocalDatabase.Save(sandboxedConfigFilePath, Serialization::SavingFlags{});
										}
									}
								}

								return Database::GetAssetEntry(asset.GetGuid());
							}
						}
					}
				}
			}
		}
		return Invalid;
	}

	bool LocalDatabase::RegisterPlugin(const PluginInfo& plugin)
	{
		const IO::Path assetCacheDirectoryPath = GetDirectoryPath();
		const IO::Path targetPath = GetAssetPath(assetCacheDirectoryPath, plugin.GetGuid(), PluginAssetFormat.metadataFileExtension);
		if (!targetPath.Exists())
		{
			targetPath.CreateDirectories();
		}

		if (LIKELY(IO::Path(plugin.GetDirectory()).CopyDirectoryTo(targetPath)))
		{
			Database::RegisterAsset(plugin.GetGuid(), DatabaseEntry{plugin}, assetCacheDirectoryPath.GetParentPath());

			// Automatically copy into the sandboxed Mac Catalyst app directory on Mac
			if constexpr (PLATFORM_APPLE_MACOS)
			{
				IO::Path sandboxedAppPath =
					IO::Path::Combine(IO::Path::GetHomeDirectory(), MAKE_PATH("Library"), MAKE_PATH("Containers"), MAKE_PATH("com.ngine.Editor"));
				if (!sandboxedAppPath.Exists())
				{
					sandboxedAppPath.CreateDirectories();
				}

				IO::Path sandboxedAssetCacheDirectoryPath =
					IO::Path::Combine(sandboxedAppPath, MAKE_PATH("Data"), MAKE_PATH("Documents"), DirectoryName);
				if (!sandboxedAssetCacheDirectoryPath.Exists())
				{
					sandboxedAssetCacheDirectoryPath.CreateDirectories();
				}

				const IO::Path sandboxedConfigFilePath = GetLocalDatabaseConfigFilePath(sandboxedAssetCacheDirectoryPath);
				Database sandboxedLocalDatabase;
				sandboxedLocalDatabase.Load(sandboxedConfigFilePath, sandboxedAssetCacheDirectoryPath);

				const IO::Path sandboxedTargetPath =
					GetAssetPath(sandboxedAssetCacheDirectoryPath, plugin.GetGuid(), PluginAssetFormat.metadataFileExtension);
				if (LIKELY(IO::Path(plugin.GetDirectory()).CopyDirectoryTo(sandboxedTargetPath)))
				{
					sandboxedLocalDatabase.RegisterAsset(plugin.GetGuid(), DatabaseEntry{plugin}, sandboxedAssetCacheDirectoryPath);
					[[maybe_unused]] const bool wasSaved = sandboxedLocalDatabase.Save(sandboxedConfigFilePath, Serialization::SavingFlags{});
				}
			}
			return true;
		}
		return false;
	}

	bool LocalDatabase::RegisterPluginAssets(const PluginInfo& plugin)
	{
		const IO::Path assetCacheDirectoryPath = GetDirectoryPath();
		const IO::Path targetPath = GetAssetPath(assetCacheDirectoryPath, plugin.GetGuid(), PluginAssetFormat.metadataFileExtension);
		if (LIKELY(IO::Path(plugin.GetDirectory()).CopyDirectoryTo(targetPath)))
		{
			return Database::RegisterAllAssetsInDirectory(targetPath, assetCacheDirectoryPath.GetParentPath());
		}
		return false;
	}

	bool LocalDatabase::Save(const EnumFlags<Serialization::SavingFlags> flags)
	{
		return Database::Save(GetConfigFilePath(), flags);
	}
}

namespace ngine
{
	template struct UnorderedMap<Guid, Asset::DatabaseEntry, Guid::Hash>;
}
