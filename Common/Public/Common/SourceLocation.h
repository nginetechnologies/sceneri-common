#pragma once

#include <Common/Memory/Containers/StringView.h>

namespace ngine
{
	struct SourceLocation
	{
		ConstStringView sourceFilePath;
		uint32 lineNumber;
		uint32 columnNumber;
	};
}

#define SOURCE_LOCATION \
	ngine::SourceLocation \
	{ \
		__FILE__, __LINE__, 0 \
	}
