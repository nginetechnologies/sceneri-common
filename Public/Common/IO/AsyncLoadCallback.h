#pragma once

#include <Common/Memory/Containers/ForwardDeclarations/ByteView.h>
#include <Common/Function/ForwardDeclarations/Function.h>

namespace ngine::IO
{
	using AsyncLoadCallback = Function<void(ConstByteView), 32>;
}
