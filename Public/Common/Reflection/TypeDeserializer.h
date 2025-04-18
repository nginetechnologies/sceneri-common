#pragma once

#include <Common/Memory/ReferenceWrapper.h>

namespace ngine::Threading
{
	struct JobBatch;
}

namespace ngine::Reflection
{
	struct TypeDeserializer
	{
		const Serialization::Reader& m_reader;
		const Reflection::Registry& m_registry;
		Optional<Threading::JobBatch*> m_pJobBatch;
	};
};
