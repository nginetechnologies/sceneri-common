#include "Common/Project System/ProjectDatabase.h"
#include "Common/Project System/ProjectInfo.h"

#include <Common/Serialization/Deserialize.h>
#include <Common/Serialization/Serialize.h>
#include <Common/Memory/Containers/Serialization/UnorderedMap.h>

#include <Common/Algorithms/GenerateUniqueName.h>

namespace ngine
{
	/* static */ IO::Path ProjectDatabase::GetProjectsDirectory(const IO::PathView folderName)
	{
		return IO::Path::Combine(IO::Path::GetUserDataDirectory(), folderName);
	}

	/* static */ IO::Path GetProjectsDatabaseConfigPath(const IO::PathView folderName)
	{
		return IO::Path::Combine(IO::Path::GetUserDataDirectory(), IO::Path::Merge(folderName, MAKE_PATH(".json")));
	}

	ProjectDatabase::ProjectDatabase(const IO::PathView folderName)
		: m_filePath(GetProjectsDatabaseConfigPath(folderName))
	{
		[[maybe_unused]] const bool wasRead = Serialization::DeserializeFromDisk(m_filePath, *this);
	}

	void ProjectDatabase::RegisterProject(IO::Path&& projectConfigPath, const Asset::Guid projectGuid)
	{
		Assert(projectGuid.IsValid());
		const IO::PathView databaseDirectory = m_filePath.GetParentPath();
		if (projectConfigPath.IsRelativeTo(databaseDirectory))
		{
			projectConfigPath.MakeRelativeToParent(databaseDirectory);
		}

		auto it = m_projectMap.Find(projectGuid);
		if (it != m_projectMap.end())
		{
			Entry& __restrict entry = it->second;
			entry.m_path = Move(projectConfigPath);
			entry.m_lastUsageTime = Time::Timestamp::GetCurrent();
		}
		else
		{
			m_projectMap.Emplace(Asset::Guid(projectGuid), Entry{Move(projectConfigPath), Time::Timestamp::GetCurrent()});
		}
	}

	bool ProjectDatabase::RemoveProject(const Asset::Guid projectGuid)
	{
		auto it = m_projectMap.Find(projectGuid);
		if (it != m_projectMap.end())
		{
			m_projectMap.Remove(it);
			return true;
		}
		return false;
	}

	void ProjectDatabase::OnProjectUsed(const Asset::Guid projectGuid)
	{
		auto it = m_projectMap.Find(projectGuid);
		Assert(it != m_projectMap.end());
		if (LIKELY(it != m_projectMap.end()))
		{
			Entry& __restrict entry = it->second;
			entry.m_lastUsageTime = Time::Timestamp::GetCurrent();
		}
	}

	bool ProjectDatabase::Save(const EnumFlags<Serialization::SavingFlags> savingFlags)
	{
		IO::Path(m_filePath.GetParentPath()).CreateDirectories();
		return Serialization::SerializeToDisk(m_filePath, *this, savingFlags);
	}

	bool ProjectDatabase::Serialize(const Serialization::Reader reader)
	{
		return reader.SerializeInPlace(m_projectMap);
	}
	bool ProjectDatabase::Serialize(Serialization::Writer writer) const
	{
		return writer.SerializeInPlace(m_projectMap);
	}

	bool ProjectDatabase::Entry::Serialize(const Serialization::Reader reader)
	{
		if (reader.IsObject())
		{
			bool wasRead = reader.Serialize("path", m_path);
			reader.Serialize("last_usage", m_lastUsageTime);
			return wasRead;
		}
		else
		{
			return reader.SerializeInPlace(m_path);
		}
	}
	bool ProjectDatabase::Entry::Serialize(Serialization::Writer writer) const
	{
		bool wasWritten = writer.Serialize("path", m_path);
		writer.Serialize("last_usage", m_lastUsageTime);
		return wasWritten;
	}

	UnicodeString ProjectDatabase::GenerateUniqueNameFromTemplate(UnicodeString::ConstView templateName)
	{
		if (m_projectMap.IsEmpty())
		{
			return UnicodeString(templateName);
		}

		Vector<ProjectInfo> projectInfos;
		for (auto entry : m_projectMap)
		{
			IO::Path path = GetProjectsDirectory(entry.second.m_path);
			if (path.Exists())
			{
				ProjectInfo projectInfo(Move(path));
				if (projectInfo.IsValid())
				{
					projectInfos.EmplaceBack(Move(projectInfo));
				}
			}
		}

		FixedSizeVector<ConstUnicodeStringView> projectNames(Memory::ConstructWithSize, Memory::Uninitialized, projectInfos.GetSize());
		for (uint32 i = 0; i < projectInfos.GetSize(); ++i)
		{
			projectNames[i] = projectInfos[i].GetName();
		}

		return Algorithms::GenerateUniqueName(templateName, projectNames);
	}

	template struct UnorderedMap<Guid, Internal::ProjectDatabaseEntry, Guid::Hash>;
}
