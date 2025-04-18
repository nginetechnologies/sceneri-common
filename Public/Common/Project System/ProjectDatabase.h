#pragma once

#include <Common/Serialization/ForwardDeclarations/Reader.h>
#include <Common/Serialization/ForwardDeclarations/Writer.h>
#include <Common/Serialization/SavingFlags.h>
#include <Common/ForwardDeclarations/EnumFlags.h>
#include <Common/Memory/Containers/UnorderedMap.h>
#include <Common/Guid.h>
#include <Common/Asset/Guid.h>
#include <Common/IO/Path.h>
#include <Common/Memory/Containers/String.h>
#include <Common/Time/Timestamp.h>

namespace ngine
{
	struct ProjectInfo;

	namespace Internal
	{
		struct ProjectDatabaseEntry
		{
			bool Serialize(const Serialization::Reader reader);
			bool Serialize(Serialization::Writer writer) const;

			IO::Path m_path;
			Time::Timestamp m_lastUsageTime;
		};
	}

	extern template struct UnorderedMap<Guid, Internal::ProjectDatabaseEntry, Guid::Hash>;

	struct ProjectDatabase
	{
		using Entry = Internal::ProjectDatabaseEntry;

		static IO::Path GetProjectsDirectory(const IO::PathView folderName);

		ProjectDatabase(const IO::PathView folderName);
		void RegisterProject(IO::Path&& projectConfigPath, const Asset::Guid projectGuid);
		bool RemoveProject(const Asset::Guid projectGuid);
		void OnProjectUsed(const Asset::Guid projectGuid);
		bool Save(const EnumFlags<Serialization::SavingFlags>);

		bool Serialize(const Serialization::Reader reader);
		bool Serialize(Serialization::Writer writer) const;

		UnicodeString GenerateUniqueNameFromTemplate(UnicodeString::ConstView templateName);

		[[nodiscard]] IO::Path FindProjectConfigPath(const Guid guid) const
		{
			auto it = m_projectMap.Find(guid);
			if (it != m_projectMap.end())
			{
				if (it->second.m_path.IsRelative())
				{
					return IO::Path::Combine(m_filePath.GetParentPath(), it->second.m_path);
				}

				return it->second.m_path;
			}

			return {};
		}
		[[nodiscard]] Time::Timestamp FindProjectLastUsageTime(const Guid guid) const
		{
			auto it = m_projectMap.Find(guid);
			if (it != m_projectMap.end())
			{
				return it->second.m_lastUsageTime;
			}

			return {};
		}

		[[nodiscard]] uint32 GetProjectCount() const
		{
			return m_projectMap.GetSize();
		}

		[[nodiscard]] bool Contains(const Guid guid) const
		{
			return m_projectMap.Contains(guid);
		}

		template<typename Callback>
		void IterateProjects(Callback&& callback) const
		{
			for (auto pair : m_projectMap)
			{
				const Entry& __restrict projectEntry = pair.second;
				if (projectEntry.m_path.IsRelative())
				{
					if (!callback(pair.first, IO::Path::Combine(m_filePath.GetParentPath(), projectEntry.m_path), projectEntry.m_lastUsageTime))
					{
						break;
					}
				}
				else
				{
					if (!callback(pair.first, projectEntry.m_path, projectEntry.m_lastUsageTime))
					{
						break;
					}
				}
			}
		}
	protected:
		IO::Path m_filePath;

		UnorderedMap<Guid, Entry, Guid::Hash> m_projectMap;
	};

	struct EditableProjectDatabase : public ProjectDatabase
	{
		inline static constexpr IO::PathView FolderPath = MAKE_PATH("Projects");

		EditableProjectDatabase()
			: ProjectDatabase(FolderPath)
		{
		}
	};

	struct PlayableProjectDatabase : public ProjectDatabase
	{
		inline static constexpr IO::PathView FolderPath = MAKE_PATH("Products");

		PlayableProjectDatabase()
			: ProjectDatabase(FolderPath)
		{
		}
	};
}
