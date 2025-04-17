#pragma once

#include <Common/Asset/AssetFormat.h>
#include <Common/Asset/MetaData.h>
#include <Common/Tag/TagGuid.h>

#include <Common/Guid.h>
#include <Common/Serialization/ForwardDeclarations/Reader.h>
#include <Common/Serialization/ForwardDeclarations/Writer.h>
#include <Common/Memory/Containers/InlineVector.h>

#include <Common/IO/Path.h>

namespace ngine
{
	namespace IO
	{
		struct File;
	}

	struct EngineInfo;
	struct ProjectInfo;
	struct PluginInfo;
}

namespace ngine::Asset
{
	struct Asset;

	using TypeGuid = ngine::Guid;

	struct DatabaseEntry
	{
		bool Serialize(const Serialization::Reader serializer, const IO::PathView prefix);
		bool Serialize(Serialization::Writer serializer, const IO::PathView rootDirectory) const;

		using TagsStorage = InlineVector<Tag::Guid, 2>;
		using DependenciesStorage = InlineVector<Guid, 4>;
		using ContainerContentsStorage = InlineVector<Guid, 4>;

		DatabaseEntry() = default;
		DatabaseEntry(
			const TypeGuid assetTypeGuid,
			const Guid componentTypeGuid,
			IO::Path&& path,
			UnicodeString&& name = {},
			UnicodeString&& description = {},
			const Guid thumbnailGuid = {},
			TagsStorage&& tags = {},
			DependenciesStorage&& dependencies = {},
			ContainerContentsStorage&& containerContents = {},
			MetaDataStorage&& metaData = {}
		)
			: m_assetTypeGuid(assetTypeGuid)
			, m_componentTypeGuid(componentTypeGuid)
			, m_path(Forward<IO::Path>(path))
			, m_description(Forward<UnicodeString>(description))
			, m_thumbnailGuid(thumbnailGuid)
			, m_tags(Forward<TagsStorage>(tags))
			, m_dependencies(Forward<DependenciesStorage>(dependencies))
			, m_containerContents(Forward<ContainerContentsStorage>(containerContents))
			, m_metaData(Forward<MetaDataStorage>(metaData))
		{
			SetName(Forward<UnicodeString>(name));
		}
		DatabaseEntry(const Asset& asset);
		DatabaseEntry(const ProjectInfo& project);
		DatabaseEntry(const PluginInfo& plugin);
		DatabaseEntry(const EngineInfo& engine);

		void SetName(UnicodeString&& name)
		{
			// In case the name is identical to the file name no need to save it.
			if (name.HasElements() && name != UnicodeString(GetNameFromPath()))
			{
				m_name = Forward<UnicodeString>(name);
			}
		}

		void Merge(DatabaseEntry&& other)
		{
			m_assetTypeGuid = other.m_assetTypeGuid;

			Assert(!m_componentTypeGuid.IsValid() || m_componentTypeGuid == other.m_componentTypeGuid);
			m_componentTypeGuid = other.m_componentTypeGuid;

			m_path = Move(other.m_path);
			m_name = Move(other.m_name);
			m_description = Move(other.m_description);
			m_thumbnailGuid = other.m_thumbnailGuid;
			m_tags.MoveEmplaceRangeBack(other.m_tags.GetView());
			m_dependencies.MoveEmplaceRangeBack(other.m_dependencies.GetView());
			m_containerContents.MoveEmplaceRangeBack(other.m_containerContents.GetView());
			m_metaData.Merge(other.m_metaData);
		}

		[[nodiscard]] bool IsValid() const
		{
			return m_path.HasElements();
		}

		TypeGuid m_assetTypeGuid;
		Guid m_componentTypeGuid;
		IO::Path m_path;
		UnicodeString m_name;
		UnicodeString m_description;
		Guid m_thumbnailGuid;
		TagsStorage m_tags;
		DependenciesStorage m_dependencies;
		ContainerContentsStorage m_containerContents;
		MetaDataStorage m_metaData;

		[[nodiscard]] IO::PathView GetBinaryFilePath() const
		{
			if (m_path.GetRightMostExtension() == AssetFormat.metadataFileExtension)
			{
				IO::PathView binaryFilePath(m_path.GetView().GetSubView(0, m_path.GetSize() - AssetFormat.metadataFileExtension.GetSize()));
				if (binaryFilePath.HasExtension())
				{
					return binaryFilePath;
				}
			}
			return {};
		}

		[[nodiscard]] IO::Path::StringType GetName() const;
		[[nodiscard]] IO::Path::StringType GetNameFromPath() const;
	};
}
