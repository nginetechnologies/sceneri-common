#pragma once

#include "ZeroTerminatedPathView.h"
#include <Common/IO/URICharType.h>

namespace ngine::IO
{
	using ConstZeroTerminatedURIView = TZeroTerminatedPathView<const URICharType>;
	using ZeroTerminatedURIView = TZeroTerminatedPathView<URICharType>;
}
