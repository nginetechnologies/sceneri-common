#include <Common/Application/Application.h>
#include <Common/System/Query.h>
#include <Common/IO/Log.h>

namespace ngine::System
{
	struct Systems
	{
		Log m_log;
	};

	static Systems s_systems;
	static Query s_query;

	Query::Query()
	{
		RegisterSystem(s_systems.m_log);
	}
	Query::~Query()
	{
		DeregisterSystem<Log>();
	}

	/* static */ Query& Query::GetInstance()
	{
		return s_query;
	}

	Optional<Plugin*> Query::GetPluginByGuid(const Guid guid)
	{
		return QuerySystem<Application>().GetPluginByGuid(guid);
	}
}
