#pragma once

#include <Common/Storage/ForwardDeclarations/Identifier.h>
#include <Common/Memory/CountBits.h>

namespace ngine::Threading
{
	template<uint8 WorkerCount>
	using WebWorkerIdentifier = TIdentifier<uint32, Memory::GetBitWidth(WorkerCount), WorkerCount>;
}
