#include <Common/Application/Application.h>
#include <Common/Application/PluginInstance.h>
#include <Common/Plugin/Plugin.h>

#include <Common/IO/Format/Path.h>
#include <Common/IO/Log.h>
#include <Common/System/Query.h>
#include <Common/Memory/Containers/Format/String.h>
#include <Common/Memory/Containers/Format/StringView.h>
#include <Common/Format/Guid.h>
#include <Common/Asset/AssetDatabase.h>
#include <Common/Asset/Format/Guid.h>

namespace ngine
{
	Application::Application()
	{
		System::Query::GetInstance().RegisterSystem(*this);
	}
	Application::~Application()
	{
		System::Query::GetInstance().DeregisterSystem<Application>();
	}

	void Application::UnloadPlugins()
	{
		Threading::UniqueLock lock(m_pluginLookupMapMutex);
		m_pluginLookupMap.Clear();
	}

	bool Application::IsPluginLoaded(const Guid pluginGuid) const
	{
		Threading::SharedLock lock(m_pluginLookupMapMutex);
		return m_pluginLookupMap.Contains(pluginGuid);
	}

	Optional<Plugin*> Application::GetPluginByGuid(const Guid pluginGuid) const
	{
		Threading::SharedLock lock(m_pluginLookupMapMutex);
		const decltype(m_pluginLookupMap)::const_iterator it = m_pluginLookupMap.Find(pluginGuid);
		if (it == m_pluginLookupMap.end())
		{
			return nullptr;
		}

		return it->second->GetPlugin();
	}

	PluginInstance& Application::StartPluginLoading(PluginInstance&& pluginInstance)
	{
		Threading::UniqueLock lock(m_pluginLookupMapMutex);
		const Guid pluginGuid = pluginInstance.GetGuid();
		auto it = m_pluginLookupMap.Emplace(Guid(pluginGuid), UniquePtr<PluginInstance>::Make(Forward<PluginInstance>(pluginInstance)));
		return *it->second.Get();
	}

	Optional<PluginInstance*> Application::FindPlugin(const Guid pluginGuid) const
	{
		Threading::SharedLock lock(m_pluginLookupMapMutex);
		if (auto it = m_pluginLookupMap.Find(pluginGuid); it != m_pluginLookupMap.end())
		{
			return it->second.Get();
		}
		else
		{
			return Invalid;
		}
	}

	bool Application::LoadPluginBinary(PluginInstance& pluginInstance)
	{
		if (pluginInstance.HasBinaryDirectory())
		{
			Plugin::EntryPoint pluginEntryPoint;

			if constexpr (PLUGINS_IN_EXECUTABLE)
			{
				pluginEntryPoint = Plugin::FindEntryPoint(pluginInstance.GetGuid());
			}
			else
			{
				const FlatString<37> pluginGuidString = pluginInstance.GetGuid().ToString();
				const IO::Path pluginGuidPath = IO::Path::Merge(
					IO::Library::FileNamePrefix,
					IO::Path(IO::Path::StringType(pluginGuidString.GetView())),
					IO::Library::FileNamePostfix
				);

				pluginInstance.AssignLibrary(IO::Library(
					IO::Path::Combine(pluginInstance.GetBinaryDirectory(), MAKE_PATH(PLATFORM_NAME), MAKE_PATH(CONFIGURATION_NAME), pluginGuidPath)
				));
				if (UNLIKELY_ERROR(!pluginInstance.GetLibrary().IsValid()))
				{
					LogError("Plug-in {} library could not be loaded!", pluginInstance.GetName());
					return false;
				}

				pluginEntryPoint = pluginInstance.GetLibrary().GetProcedureAddress<Plugin::EntryPoint>(Plugin::EntryPointName);
			}

			if (UNLIKELY_ERROR(pluginEntryPoint == nullptr))
			{
				LogError("Plug-in {} entry-point could not be found!", pluginInstance.GetName());
				return false;
			}

			Plugin& plugin = pluginInstance.AssignPlugin(UniquePtr<Plugin>(pluginEntryPoint(*this)));
			plugin.OnLoaded(*this);
		}
		return true;
	}

	bool Application::LoadPlugin(const Guid pluginGuid, const Asset::Database& assetDatabase, Asset::Database& targetAssetDatabase)
	{
		if (IsPluginLoaded(pluginGuid))
		{
			return true;
		}

		const Optional<const Asset::DatabaseEntry*> pPluginAssetEntry = assetDatabase.GetAssetEntry(pluginGuid);
		if (UNLIKELY_ERROR(!pPluginAssetEntry.IsValid()))
		{
			LogError("Plug-in with guid {} was not found in the asset library!", pluginGuid);
			return false;
		}

		PluginInfo plugin(IO::Path(pPluginAssetEntry->m_path));
		if (UNLIKELY_ERROR(!plugin.IsValid()))
		{
			LogError("Plug-in with guid {} could not be loaded!", pluginGuid);
			return false;
		}

		return LoadPlugin(Move(plugin), assetDatabase, targetAssetDatabase);
	}

	bool Application::LoadPlugin(PluginInfo&& plugin, const Asset::Database& assetDatabase, Asset::Database& targetAssetDatabase)
	{
		Assert(plugin.IsValid());

		if (IsPluginLoaded(plugin.GetGuid()))
		{
			return true;
		}

		PluginInstance& storedPluginInstance = StartPluginLoading(Forward<PluginInfo>(plugin));
		Assert(storedPluginInstance.IsValid());

		bool result = true;

		LogMessage("Loading plug-in dependencies for plug-in {0}", storedPluginInstance.GetName());

		for (const Asset::Guid dependencyPluginGuid : storedPluginInstance.GetDependencies())
		{
			result &= LoadPlugin(dependencyPluginGuid, assetDatabase, targetAssetDatabase);
		}

		LogMessage("Loading plug-in {0}", storedPluginInstance.GetName());

		if (storedPluginInstance.HasAssetDirectory())
		{
			LogMessage("Loading plug-in assets");

			const IO::Path assetsDatabasePath = IO::Path::Combine(
				storedPluginInstance.GetDirectory(),
				IO::Path::Merge(storedPluginInstance.GetRelativeAssetDirectory(), Asset::Database::AssetFormat.metadataFileExtension)
			);
			if (UNLIKELY(!targetAssetDatabase.Load(assetsDatabasePath, storedPluginInstance.GetDirectory())))
			{
				LogError(
					"Failed to load plug-in {} (Guid {}) - Asset database {} could not be loaded!",
					storedPluginInstance.GetName(),
					storedPluginInstance.GetGuid(),
					assetsDatabasePath
				);
				return false;
			}
		}

		if (storedPluginInstance.HasSourceDirectory())
		{
			LogMessage("Loading plug-in binary");
			result &= LoadPluginBinary(storedPluginInstance);
		}

		storedPluginInstance.OnFinishedLoading();

		return result;
	}

	template struct UnorderedMap<Guid, UniquePtr<PluginInstance>, Guid::Hash>;
}
