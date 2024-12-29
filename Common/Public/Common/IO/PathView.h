#pragma once

#include <Common/IO/ForwardDeclarations/PathView.h>
#include <Common/IO/TPathView.h>

namespace ngine::IO
{
#define MAKE_PATH(path) IO::PathView(MAKE_PATH_LITERAL(path))

	extern template struct TPathView<
		PathCharType,
		uint8(CaseSensitive ? PathFlags::CaseSensitive : PathFlags{}),
		PathSeparator,
		MaximumPathLength>;
}
