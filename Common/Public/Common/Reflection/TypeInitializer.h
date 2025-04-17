#pragma once

#include <Common/Memory/ReferenceWrapper.h>

namespace ngine::Threading
{
	struct JobBatch;
}

namespace ngine::Reflection
{
	struct TypeInitializer
	{
		Optional<Threading::JobBatch*> m_pJobBatch;
	};
};
