#pragma once

#include <Common/IO/PathConstants.h>
#include <Common/IO/PathFlags.h>

namespace ngine::IO
{
	template<typename CharType_, uint8, CharType_ PathSeparator_, uint16 MaximumPathLength_>
	struct TPathView;

	using PathView = TPathView<PathCharType, uint8(CaseSensitive ? PathFlags::CaseSensitive : PathFlags{}), PathSeparator, MaximumPathLength>;
}
