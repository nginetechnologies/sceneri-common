#pragma once

#include <Common/Serialization/ForwardDeclarations/Reader.h>
#include <Common/Serialization/ForwardDeclarations/Writer.h>
#include <Common/Serialization/SavingFlags.h>
#include <Common/ForwardDeclarations/EnumFlags.h>
#include <Common/Memory/Containers/UnorderedMap.h>
#include <Common/Guid.h>
#include <Common/IO/Path.h>

namespace ngine
{
	struct EngineInfo;

	extern template struct UnorderedMap<Guid, IO::Path, Guid::Hash>;

	struct EngineDatabase
	{
		inline static constexpr IO::PathView FileName = MAKE_PATH("Engines.json");

		EngineDatabase();
		[[nodiscard]] bool RegisterEngine(IO::Path&& engineConfigPath);
		[[nodiscard]] bool Save(const EnumFlags<Serialization::SavingFlags>);

		bool Serialize(const Serialization::Reader reader);
		bool Serialize(Serialization::Writer writer) const;

		[[nodiscard]] IO::Path FindEngineConfigPath(const Guid guid) const
		{
			auto it = m_engineMap.Find(guid);
			if (it != m_engineMap.end())
			{
				if (it->second.IsRelative())
				{
					return IO::Path::Combine(m_filePath.GetParentPath(), it->second);
				}

				return it->second;
			}

			return {};
		}

		[[nodiscard]] Optional<EngineInfo> FindEngineConfig(const Guid guid) const;

		template<typename Callback>
		void IterateEngines(Callback&& callback) const
		{
			for (auto pair : m_engineMap)
			{
				if (pair.second.IsRelative())
				{
					if (!callback(pair.first, IO::Path::Combine(m_filePath.GetParentPath(), pair.second)))
					{
						break;
					}
				}
				else
				{
					if (!callback(pair.first, pair.second))
					{
						break;
					}
				}
			}
		}
	protected:
		IO::Path m_filePath;
		UnorderedMap<Guid, IO::Path, Guid::Hash> m_engineMap;
	};
}
