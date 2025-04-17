#pragma once

#include <Common/IO/PathView.h>

namespace ngine::Asset
{
	struct VirtualAsset
	{
		inline static constexpr IO::PathView FileExtension = MAKE_PATH(".vasset");
	};
}
