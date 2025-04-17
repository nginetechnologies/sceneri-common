#pragma once

#include <Common/System/SystemType.h>
#include <Common/Guid.h>
#include <Common/Memory/ReferenceWrapper.h>
#include <Common/Memory/Containers/UnorderedMap.h>
#include <Common/Memory/UniquePtr.h>
#include <Common/Threading/Mutexes/SharedMutex.h>

namespace ngine
{
	struct PluginInstance;
	struct PluginInfo;
	struct Plugin;

	namespace Asset
	{
		struct Database;
	}

	extern template struct UnorderedMap<Guid, UniquePtr<PluginInstance>, Guid::Hash>;

	struct Application
	{
		inline static constexpr System::Type SystemType = System::Type::Application;

		Application();
		Application(const Application&) = delete;
		Application(Application&&) = delete;
		Application& operator=(const Application&) = delete;
		Application& operator=(Application&&) = delete;
		~Application();

		void UnloadPlugins();

		[[nodiscard]] bool IsPluginLoaded(const Guid pluginGuid) const;

		[[nodiscard]] Optional<Plugin*> GetPluginByGuid(const Guid pluginGuid) const;

		template<typename PluginType>
		[[nodiscard]] Optional<PluginType*> GetPluginInstance() const
		{
			return static_cast<PluginType*>(GetPluginByGuid(PluginType::Guid).Get());
		}

		PluginInstance& StartPluginLoading(PluginInstance&& pluginInstance);
		[[nodiscard]] Optional<PluginInstance*> FindPlugin(const Guid guid) const;

		[[nodiscard]] bool LoadPluginBinary(PluginInstance& instance);
		[[nodiscard]] bool LoadPlugin(PluginInfo&& plugin, const Asset::Database& assetDatabase, Asset::Database& targetAssetDatabase);
		[[nodiscard]] bool LoadPlugin(const Guid pluginGuid, const Asset::Database& assetDatabase, Asset::Database& targetAssetDatabase);
	private:
		mutable Threading::SharedMutex m_pluginLookupMapMutex;
		UnorderedMap<Guid, UniquePtr<PluginInstance>, Guid::Hash> m_pluginLookupMap;
	};
}
