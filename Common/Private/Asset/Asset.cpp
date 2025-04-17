#include "Asset/Asset.h"
#include "Asset/Context.h"
#include "Asset/Picker.h"
#include "Asset/AssetDatabase.h"
#include "Asset/Serialization/MetaData.h"

#include <Common/Serialization/Serialize.h>
#include <Common/Serialization/Deserialize.h>
#include <Common/Memory/Containers/UnorderedMap.h>
#include <Common/Memory/Containers/Serialization/Vector.h>
#include <Common/Memory/Containers/Serialization/UnorderedMap.h>
#include <Common/Memory/Containers/BitView.h>
#include <Common/Serialization/Guid.h>
#include <Common/Serialization/Deserialize.h>
#include <Common/Serialization/Serialize.h>
#include <Common/Memory/Containers/Format/String.h>
#include <Common/Project System/ProjectAssetFormat.h>
#include <Common/Project System/EngineAssetFormat.h>
#include <Common/Project System/PluginAssetFormat.h>

#include <Common/System/Query.h>

namespace ngine::Asset
{
	Asset::Asset(const Serialization::Data& assetData, IO::Path&& assetMetaFilePath)
		: m_metaDataFilePath(Move(assetMetaFilePath))
	{
		if (LIKELY(assetData.IsValid()))
		{
			Serialization::Deserialize(assetData, *this);
		}

		// Handle core types that don't write "assetTypeGuid" to disk
		if (m_typeGuid.IsInvalid())
		{
			const IO::PathView fileExtension = m_metaDataFilePath.GetRightMostExtension();
			if (fileExtension == ProjectAssetFormat.metadataFileExtension)
			{
				m_typeGuid = ProjectAssetFormat.assetTypeGuid;
			}
			else if (fileExtension == PluginAssetFormat.metadataFileExtension)
			{
				m_typeGuid = PluginAssetFormat.assetTypeGuid;
			}
			else if (fileExtension == EngineAssetFormat.metadataFileExtension)
			{
				m_typeGuid = EngineAssetFormat.assetTypeGuid;
			}
			else if (fileExtension == Database::AssetFormat.metadataFileExtension)
			{
				m_typeGuid = Database::AssetFormat.assetTypeGuid;
			}
		}

		if (!m_guid.IsValid())
		{
			m_guid = Guid::Generate();
		}
	}

	void Asset::SetSourceFilePath(IO::Path&& filePath, const Context& context)
	{
		const IO::PathView assetDirectory = GetDirectory();
		if (filePath.IsRelativeTo(assetDirectory))
		{
			filePath.MakeRelativeToParent(assetDirectory);
		}
		else if (const IO::Path contextAssetFolder = context.GetAssetDirectory(); filePath.IsRelativeTo(contextAssetFolder))
		{
			filePath.MakeRelativeTo(assetDirectory);
		}

		m_sourceFile = Move(filePath);
	}

	bool Asset::Serialize(const Serialization::Reader serializer)
	{
		serializer.Serialize("guid", m_guid);
		if (!m_guid.IsValid())
		{
			m_guid = ngine::Asset::Guid::Generate();
		}

		serializer.Serialize("name", m_name);
		serializer.Serialize("assetTypeGuid", m_typeGuid);
		serializer.Serialize("typeGuid", m_componentTypeGuid);
		serializer.Serialize("thumbnailGuid", m_thumbnailGuid);
		serializer.Serialize("metadata", m_metaData);

		m_tags.Clear();
		serializer.Serialize("tags", m_tags);

		m_chunks.Clear();
		serializer.Serialize("chunks", m_chunks);

		m_dependencies.Clear();
		serializer.Serialize("dependencies", m_dependencies);

		IO::Path sourceFilePath;
		Guid sourceAsset;
		if (serializer.Serialize("source", sourceFilePath))
		{
			m_sourceFile = Move(sourceFilePath);
		}
		else if (serializer.Serialize("source_asset", sourceAsset))
		{
			m_sourceFile = sourceAsset;
		}

		return true;
	}

	bool Asset::Serialize(Serialization::Writer serializer) const
	{
		serializer.Serialize("guid", m_guid);
		serializer.Serialize("name", m_name);
		serializer.Serialize("assetTypeGuid", m_typeGuid);
		serializer.Serialize("typeGuid", m_componentTypeGuid);
		serializer.Serialize("thumbnailGuid", m_thumbnailGuid);
		serializer.Serialize("tags", m_tags);
		serializer.Serialize("chunks", m_chunks);
		serializer.Serialize("dependencies", m_dependencies);
		serializer.Serialize("metadata", m_metaData);

		if (Optional<const IO::Path*> sourceFilePath = m_sourceFile.Get<IO::Path>())
		{
			Assert(!sourceFilePath->IsEmpty());
			serializer.Serialize("source", *sourceFilePath);
			serializer.GetAsObject().RemoveMember("source_asset");
		}
		else if (Optional<const Guid*> sourceAssetGuid = m_sourceFile.Get<Guid>())
		{
			serializer.Serialize("source_asset", *sourceAssetGuid);
			serializer.GetAsObject().RemoveMember("source");
		}

		return true;
	}

	void Asset::AddMetaData(ngine::Guid guid, MetaDataType&& data)
	{
		m_metaData.EmplaceOrAssign(guid, data);
	}

	IO::Path::StringType Asset::GetNameFromPath() const
	{
		IO::PathView fileName = m_metaDataFilePath.GetFileNameWithoutExtensions();

		if (fileName == MAKE_PATH("Main"))
		{
			// Special case for package assets, the name "Main" indicates the name is in the parent directory
			const IO::PathView directoryName = m_metaDataFilePath.GetParentPath().GetFileName();
			if (directoryName.GetRightMostExtension() == Asset::FileExtension)
			{
				fileName = directoryName.GetWithoutExtensions();
			}
		}

		IO::Path::StringType name = IO::Path::StringType(fileName.GetStringView());
		name.ReplaceCharacterOccurrences(MAKE_PATH_LITERAL('_'), MAKE_PATH_LITERAL(' '));
		return Move(name);
	}

	UnicodeString Asset::GetName() const
	{
		if (m_name.HasElements())
		{
			return UnicodeString(m_name.GetView());
		}

		return UnicodeString{GetNameFromPath()};
	}

	void Asset::SetName(UnicodeString&& name)
	{
		// In case the name is identical to the file name no need to save it.
		if (name.HasElements() && name != UnicodeString(GetNameFromPath()))
		{
			m_name = Forward<UnicodeString>(name);
		}
	}

	bool Picker::Serialize(const Serialization::Reader serializer)
	{
		return m_asset.Serialize(serializer);
	}

	bool Picker::Serialize(Serialization::Writer serializer) const
	{
		return m_asset.Serialize(serializer);
	}

	bool Reference::Serialize(const Serialization::Reader serializer)
	{
		return serializer.SerializeInPlace(m_assetGuid);
	}

	bool Reference::Serialize(Serialization::Writer serializer) const
	{
		return serializer.SerializeInPlace(m_assetGuid);
	}

	bool Reference::Compress(BitView& target) const
	{
		// TODO: Support binding types and assets to the network, then send that identifier instead if available.

		const bool wasTypeGuidPacked = target.PackAndSkip(ConstBitView::Make(m_assetTypeGuid));
		const bool wasGuidPacked = target.PackAndSkip(ConstBitView::Make(m_assetGuid));
		return wasTypeGuidPacked & wasGuidPacked;
	}

	bool Reference::Decompress(ConstBitView& source)
	{
		m_assetTypeGuid = source.UnpackAndSkip<Guid>();
		m_assetGuid = source.UnpackAndSkip<Guid>();
		return true;
	}
}
