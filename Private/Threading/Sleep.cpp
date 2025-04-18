#include "Threading/Sleep.h"
#include "Threading/ThreadId.h"

#if USE_WEB_WORKERS
#include <emscripten/wasm_worker.h>
#else
#include <thread>
#endif

namespace ngine::Threading
{
	void Sleep(const unsigned long milliseconds)
	{
#if USE_WEB_WORKERS
		emscripten_wasm_worker_sleep(milliseconds * 1000);
#else
		std::this_thread::sleep_for(std::chrono::duration<unsigned long, std::milli>(milliseconds));
#endif
	}
}
