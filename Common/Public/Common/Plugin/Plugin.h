#pragma once

#include <Common/Memory/Containers/ZeroTerminatedStringView.h>

namespace ngine
{
	struct Application;
	struct Guid;

	struct Plugin
	{
		virtual ~Plugin() = default;

		virtual void OnLoaded(Application&)
		{
		}
		virtual void OnUnloaded(Application&)
		{
		}

		using EntryPoint = Plugin* (*)(Application&);
		inline static constexpr ZeroTerminatedStringView EntryPointName = "InitializePlugin";
		static EntryPoint FindEntryPoint(const Guid guid);
		static bool Register(const Guid guid, const EntryPoint entryPoint);
		template<typename PluginType>
		static bool Register()
		{
			return Register(
				PluginType::Guid,
				[](Application& application) -> Plugin*
				{
					return new PluginType(application);
				}
			);
		}
	};
}
