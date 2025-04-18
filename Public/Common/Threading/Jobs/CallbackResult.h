#pragma once

namespace ngine::Threading
{
	enum class CallbackResult : uint8
	{
		Finished,
		AwaitExternalFinish,
		FinishedAndDelete,
		FinishedAndRunDestructor,
		TryRequeue
	};
}
