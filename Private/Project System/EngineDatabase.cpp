#include "Common/Project System/EngineDatabase.h"
#include "Common/Project System/EngineInfo.h"

#include <Common/Serialization/Deserialize.h>
#include <Common/Serialization/Serialize.h>
#include <Common/Memory/Containers/Serialization/UnorderedMap.h>

namespace ngine
{
	IO::Path GetProjectSystemEnginesDatabasePath()
	{
		return IO::Path::Combine(IO::Path::GetUserDataDirectory(), EngineInfo::Name.GetView(), EngineDatabase::FileName);
	}

	EngineDatabase::EngineDatabase()
		: m_filePath(GetProjectSystemEnginesDatabasePath())
	{
		[[maybe_unused]] const bool wasRead = Serialization::DeserializeFromDisk(m_filePath, *this);
	}

	bool EngineDatabase::RegisterEngine(IO::Path&& engineConfigPath)
	{
		const EngineInfo engineInfo(Move(engineConfigPath));
		if (engineInfo.IsValid())
		{
			const IO::PathView databaseDirectory = m_filePath.GetParentPath();
			IO::Path relativeConfigPath = engineInfo.GetConfigFilePath();
			if (relativeConfigPath.IsRelativeTo(databaseDirectory))
			{
				relativeConfigPath.MakeRelativeToParent(databaseDirectory);
			}

			auto it = m_engineMap.Find(engineInfo.GetGuid());
			if (it != m_engineMap.end())
			{
				it->second = Move(relativeConfigPath);
			}
			else
			{
				m_engineMap.Emplace(engineInfo.GetGuid(), Move(relativeConfigPath));
			}
			return true;
		}
		return false;
	}

	bool EngineDatabase::Save(const EnumFlags<Serialization::SavingFlags> savingFlags)
	{
		IO::Path(m_filePath.GetParentPath()).CreateDirectories();
		return Serialization::SerializeToDisk(m_filePath, *this, savingFlags);
	}

	bool EngineDatabase::Serialize(const Serialization::Reader reader)
	{
		return reader.SerializeInPlace(m_engineMap);
	}

	bool EngineDatabase::Serialize(Serialization::Writer writer) const
	{
		return writer.SerializeInPlace(m_engineMap);
	}

	Optional<EngineInfo> EngineDatabase::FindEngineConfig(const Guid guid) const
	{
		const IO::Path enginePath = FindEngineConfigPath(guid);
		if (enginePath.HasElements())
		{
			return EngineInfo(IO::Path(enginePath));
		}

		return Invalid;
	}

	template struct UnorderedMap<Guid, IO::Path, Guid::Hash>;
}
