#pragma once

#include "MetaData.h"

#include <Common/IO/Path.h>
#include <Common/Memory/Containers/Vector.h>
#include <Common/Memory/Containers/InlineVector.h>
#include <Common/Memory/Containers/String.h>
#include <Common/Memory/Containers/UnorderedMap.h>
#include <Common/Tag/TagGuid.h>

#include <Common/Serialization/ForwardDeclarations/Reader.h>
#include <Common/Serialization/ForwardDeclarations/Writer.h>
#include <Common/Serialization/SavingFlags.h>
#include <Common/ForwardDeclarations/EnumFlags.h>

#include <Common/Memory/Variant.h>

#include "Guid.h"

namespace ngine::Asset
{
	struct Context;

	struct Asset
	{
		inline static constexpr IO::PathView FileExtension = MAKE_PATH(".nasset");
		using Guid = ngine::Asset::Guid;

		Asset() = default;
		Asset(const Serialization::Data& assetData, IO::Path&& assetMetaFilePath);
		explicit Asset(const Asset& other) = default;
		Asset& operator=(const Asset&) = delete;
		Asset(Asset&& other) = default;
		Asset& operator=(Asset&&) = default;

		[[nodiscard]] Guid GetGuid() const
		{
			return m_guid;
		}
		[[nodiscard]] bool IsValid() const
		{
			return m_guid.IsValid() & m_typeGuid.IsValid() & m_metaDataFilePath.HasElements();
		}

		void RegenerateGuid()
		{
			m_guid = Guid::Generate();
		}

		[[nodiscard]] ngine::Guid GetTypeGuid() const
		{
			return m_typeGuid;
		}
		//! Type Guid of the component that is used to edit this asset / drag into viewport
		[[nodiscard]] ngine::Guid GetComponentTypeGuid() const
		{
			return m_componentTypeGuid;
		}
		void SetComponentTypeGuid(const ngine::Guid typeGuid)
		{
			m_componentTypeGuid = typeGuid;
		}
		void SetTypeGuid(const ngine::Guid typeGuid)
		{
			m_typeGuid = typeGuid;
		}
		[[nodiscard]] Guid GetThumbnailGuid() const
		{
			return m_thumbnailGuid;
		}
		void SetThumbnail(const Guid thumbnailGuid)
		{
			m_thumbnailGuid = thumbnailGuid;
		}
		[[nodiscard]] ArrayView<const Tag::Guid> GetTags() const
		{
			return m_tags;
		}
		void SetTag(const Tag::Guid tag)
		{
			m_tags.EmplaceBackUnique(Tag::Guid(tag));
		}
		[[nodiscard]] const IO::Path& GetMetaDataFilePath() const LIFETIME_BOUND
		{
			return m_metaDataFilePath;
		}
		void SetMetaDataFilePath(const IO::PathView path)
		{
			m_metaDataFilePath.AssignFrom(path);
		}
		[[nodiscard]] UnicodeString GetName() const;
		void SetName(UnicodeString&& name);
		[[nodiscard]] IO::PathView GetDirectory() const
		{
			return m_metaDataFilePath.GetParentPath();
		}
		[[nodiscard]] static IO::Path CreateBinaryFilePath(const IO::PathView metaDataFilePath, const IO::PathView binaryFileExtension)
		{
			Assert(metaDataFilePath.GetRightMostExtension() == FileExtension);
			IO::PathView fileName = metaDataFilePath.GetFileName();
			fileName = fileName.GetSubView(0, fileName.GetSize() - FileExtension.GetSize());
			if (fileName.GetRightMostExtension() == binaryFileExtension)
			{
				fileName = fileName.GetSubView(0, fileName.GetSize() - binaryFileExtension.GetSize());
			}
			return IO::Path::Combine(metaDataFilePath.GetParentPath(), IO::Path::Merge(fileName, binaryFileExtension));
		}
		[[nodiscard]] IO::Path GetBinaryFilePath(const IO::PathView binaryFileExtension) const
		{
			return CreateBinaryFilePath(m_metaDataFilePath, binaryFileExtension);
		}
		[[nodiscard]] static IO::Path CreateBinaryFilePath(const IO::PathView metaDataFilePath)
		{
			Assert(metaDataFilePath.GetRightMostExtension() == FileExtension);
			IO::Path binaryFilePath = IO::Path(metaDataFilePath.GetSubView(0, metaDataFilePath.GetSize() - FileExtension.GetSize()));
			if (binaryFilePath.HasExtension())
			{
				return Move(binaryFilePath);
			}
			else
			{
				return {};
			}
		}
		[[nodiscard]] IO::Path GetBinaryFilePath() const
		{
			return CreateBinaryFilePath(m_metaDataFilePath);
		}

		[[nodiscard]] Optional<Guid> GetSourceAssetGuid() const
		{
			if (Optional<const Guid*> sourceAssetGuid = m_sourceFile.Get<Guid>())
			{
				return *sourceAssetGuid;
			}

			return Invalid;
		}

		[[nodiscard]] bool HasSourceFilePath() const
		{
			return m_sourceFile.Is<IO::Path>();
		}

		[[nodiscard]] bool HasSourceFileAssetGuid() const
		{
			return m_sourceFile.Is<Guid>();
		}

		[[nodiscard]] bool HasSourceFile() const
		{
			return HasSourceFilePath() | HasSourceFileAssetGuid();
		}

		[[nodiscard]] bool IsSourceFilePathRelativeToAsset() const
		{
			if (Optional<const IO::Path*> sourceFilePath = m_sourceFile.Get<IO::Path>())
			{
				return sourceFilePath->IsRelative();
			}

			return false;
		}

		[[nodiscard]] Optional<IO::PathView> GetSourceFilePath() const
		{
			if (Optional<const IO::Path*> sourceFilePath = m_sourceFile.Get<IO::Path>())
			{
				return IO::PathView(*sourceFilePath);
			}

			return Invalid;
		}

		[[nodiscard]] Optional<IO::Path> ComputeAbsoluteSourceFilePath() const
		{
			if (Optional<const IO::Path*> sourceFilePath = m_sourceFile.Get<IO::Path>())
			{
				if (sourceFilePath->IsRelative())
				{
					return IO::Path::Combine(GetDirectory(), IO::PathView(*sourceFilePath));
				}

				return IO::Path(*sourceFilePath);
			}

			return Invalid;
		}

		void MakeSourceFilePathAbsolute()
		{
			if (Optional<IO::Path*> sourceFilePath = m_sourceFile.Get<IO::Path>())
			{
				if (sourceFilePath->IsRelative())
				{
					*sourceFilePath = IO::Path::Combine(GetDirectory(), *sourceFilePath);
				}
			}
		}

		void SetSourceFilePath(IO::Path&& filePath, const Context& context);

		void SetSourceAsset(const Guid guid)
		{
			m_sourceFile = guid;
		}

		void AddChunk(const Guid guid)
		{
			m_chunks.EmplaceBack(Guid(guid));
		}

		void ClearChunks()
		{
			m_chunks.Clear();
		}

		ArrayView<const Guid, uint32> GetChunkGuids() const
		{
			return m_chunks;
		}

		void SetDependencies(const ArrayView<const Guid> dependencies)
		{
			Assert(!dependencies.Contains(m_guid));
			m_dependencies = dependencies;
		}
		void AddDependencies(const ArrayView<const Guid> dependencies)
		{
			Assert(!dependencies.Contains(m_guid));
			m_dependencies.CopyEmplaceRangeBack(dependencies);
		}
		void AddDependency(const Guid dependency)
		{
			Assert(dependency != m_guid);
			m_dependencies.EmplaceBack(Guid(dependency));
		}
		void ClearDependencies()
		{
			m_dependencies.Clear();
		}
		[[nodiscard]] ArrayView<const Guid> GetDependencies() const
		{
			return m_dependencies;
		}
		[[nodiscard]] bool HasDependencies() const
		{
			return m_dependencies.HasElements();
		}

		[[nodiscard]] const MetaDataStorage& GetMetaData() const LIFETIME_BOUND
		{
			return m_metaData;
		}
		void AddMetaData(ngine::Guid guid, MetaDataType&& data);
		void ClearMetaData()
		{
			m_metaData.Clear();
		}

		bool Serialize(const Serialization::Reader serializer);
		bool Serialize(Serialization::Writer serializer) const;
	private:
		[[nodiscard]] IO::Path::StringType GetNameFromPath() const;
	protected:
		IO::Path m_metaDataFilePath;
		Guid m_guid;
		UnicodeString m_name;
		ngine::Guid m_typeGuid;
		ngine::Guid m_componentTypeGuid;
		Guid m_thumbnailGuid;

		InlineVector<Tag::Guid, 2> m_tags;
		Vector<Guid> m_chunks;
		Vector<Guid> m_dependencies;
		MetaDataStorage m_metaData;

		Variant<IO::Path, Guid> m_sourceFile;
	};
}
