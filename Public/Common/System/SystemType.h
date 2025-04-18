#pragma once

#include <Common/Math/CoreNumericTypes.h>

namespace ngine
{
	namespace System
	{
		enum class Type : uint8
		{
			Application = 0,
			Log,
			Engine,
			Project,
			Renderer,
			FileSystem,
			AssetManager,
			Reflection,
			ScriptCache,
			InputManager,
			EntityManager,
			TagRegistry,
			DataSourceCache,
			JobManager,
			EventManager,
			ResourceManager,
			Count
		};
	}
}
