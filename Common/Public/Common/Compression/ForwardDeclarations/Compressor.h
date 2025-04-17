#pragma once

#include <Common/Memory/Containers/ForwardDeclarations/BitView.h>

namespace ngine::Compression
{
	template<typename Type, typename = void>
	struct Compressor;
}
