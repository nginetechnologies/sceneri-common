#pragma once

#if PLATFORM_EMSCRIPTEN
#include <Common/Threading/WebWorker.h>
#include <Common/Threading/ForwardDeclarations/WebWorkerIdentifier.h>
#include <Common/Storage/IdentifierArray.h>
#include <Common/Storage/SaltedIdentifierStorage.h>
#endif

namespace ngine::Threading
{
#if PLATFORM_EMSCRIPTEN
	template<typename IdentifierType>
	struct WebWorkerPool
	{
		using WorkerIdentifier = IdentifierType;

		[[nodiscard]] WorkerIdentifier AcquireWorker()
		{
			return m_webWorkerIdentifiers.AcquireIdentifier();
		}

		void ReturnWorker(const WorkerIdentifier identifier)
		{
			m_webWorkerIdentifiers.ReturnIdentifier(identifier);
		}

		[[nodiscard]] Threading::WebWorker& GetWorker(const WorkerIdentifier identifier)
		{
			return m_webWorkers[identifier];
		}
	protected:
		TSaltedIdentifierStorage<WorkerIdentifier> m_webWorkerIdentifiers;
		TIdentifierArray<Threading::WebWorker, WorkerIdentifier> m_webWorkers;
	};
#endif
}
