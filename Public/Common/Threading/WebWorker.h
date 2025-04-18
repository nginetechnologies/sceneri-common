#pragma once

#if PLATFORM_EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/wasm_worker.h>
#include <emscripten/threading.h>
#include <emscripten/proxying.h>
#endif

namespace ngine::Threading
{
#if PLATFORM_EMSCRIPTEN
	struct WebWorker
	{
		WebWorker()
		{
			static constexpr size WorkerStackSize = 800000;

			if (emscripten_is_main_browser_thread())
			{
				m_webWorkerIdentifier = emscripten_malloc_wasm_worker(WorkerStackSize);
				Assert(m_webWorkerIdentifier >= 0);

				auto keepAliveFunc = []()
				{
					EM_ASM(runtimeKeepalivePush());
				};
				emscripten_wasm_worker_post_function_v(m_webWorkerIdentifier, keepAliveFunc);
			}
			else
			{
				em_proxying_queue* queue = emscripten_proxy_get_system_queue();
				pthread_t target = emscripten_main_runtime_thread_id();
				[[maybe_unused]] const bool executed = emscripten_proxy_async(
																								 queue,
																								 target,
																								 [](void* pUserData)
																								 {
																									 WebWorker& worker = *reinterpret_cast<WebWorker*>(pUserData);

																									 worker.m_webWorkerIdentifier = emscripten_malloc_wasm_worker(WorkerStackSize);
																									 Assert(worker.m_webWorkerIdentifier >= 0);

																									 auto keepAliveFunc = []()
																									 {
																										 EM_ASM(runtimeKeepalivePush());
																									 };
																									 emscripten_wasm_worker_post_function_v(worker.m_webWorkerIdentifier, keepAliveFunc);
																								 },
																								 this
																							 ) == 1;
				Assert(executed);
			}
		}
		WebWorker(const WebWorker&) = delete;
		WebWorker& operator=(const WebWorker&) = delete;
		~WebWorker()
		{
			auto keepAliveFunc = []()
			{
				EM_ASM(runtimeKeepalivePop());
			};
			emscripten_wasm_worker_post_function_v(m_webWorkerIdentifier, keepAliveFunc);

			emscripten_terminate_wasm_worker(m_webWorkerIdentifier);
		}

		[[nodiscard]] int GetIdentifier() const
		{
			return m_webWorkerIdentifier;
		}
	private:
		int m_webWorkerIdentifier{-1};
	};
#endif
}
