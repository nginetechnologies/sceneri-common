#pragma once

#include <Common/Project System/PluginInfo.h>
#include <Common/IO/Library.h>
#include <Common/Function/Event.h>
#include <Common/Threading/Mutexes/Mutex.h>
#include <Common/Threading/AtomicInteger.h>

namespace ngine
{
	struct Plugin;

	struct PluginInstance : public PluginInfo
	{
		enum class State : uint8
		{
			AwaitingLoad,
			Loaded
		};

		PluginInstance(const Guid pluginGuid)
		{
			m_guid = pluginGuid;
		}
		PluginInstance(PluginInfo&& pluginInfo)
			: PluginInfo(Forward<PluginInfo>(pluginInfo))
		{
		}
		PluginInstance(PluginInstance&& other)
			: PluginInfo(static_cast<PluginInfo&&>(other))
		{
			Assert(!other.m_library.IsValid());
			Assert(!m_library.IsValid());
			Assert(!other.m_pPlugin.IsValid());
			Assert(!m_pPlugin.IsValid());
			Assert(!other.m_onLoaded.HasCallbacks());
			Assert(!m_onLoaded.HasCallbacks());
			Assert(other.m_state == State::AwaitingLoad);
			Assert(m_state == State::AwaitingLoad);
		}
		PluginInstance& operator=(PluginInstance&&) = delete;
		PluginInstance& operator=(PluginInfo&& pluginInfo)
		{
			PluginInfo::operator=(static_cast<PluginInfo&&>(pluginInfo));
			return *this;
		}
		PluginInstance(const PluginInstance&) = delete;
		PluginInstance& operator=(const PluginInstance&) = delete;

		void AssignLibrary(IO::Library&& library)
		{
			m_library = Forward<IO::Library>(library);
		}
		[[nodiscard]] IO::LibraryView GetLibrary() const
		{
			return m_library;
		}

		Plugin& AssignPlugin(UniquePtr<Plugin>&& pPlugin)
		{
			m_pPlugin = Forward<UniquePtr<Plugin>>(pPlugin);
			return *m_pPlugin;
		}
		[[nodiscard]] Optional<Plugin*> GetPlugin() const
		{
			return m_pPlugin.Get();
		}

		void OnFinishedLoading()
		{
			Threading::UniqueLock lock(m_onLoadedMutex);
			m_state = State::Loaded;
			m_onLoaded.ExecuteAndClear();
		}
		[[nodiscard]] bool IsLoaded() const
		{
			return m_state == State::Loaded;
		}

		Threading::Mutex m_onLoadedMutex;
		Event<void(void*), 24, false> m_onLoaded;
	protected:
		IO::Library m_library;
		UniquePtr<Plugin> m_pPlugin;
		State m_state{State::AwaitingLoad};
	};
}
