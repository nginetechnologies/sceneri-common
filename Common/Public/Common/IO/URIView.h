#pragma once

#include <Common/IO/ForwardDeclarations/URIView.h>
#include <Common/IO/TPathView.h>

namespace ngine::IO
{
#define MAKE_URI(path) IO::URIView(MAKE_URI_LITERAL(path))

	extern template struct TPathView<URICharType, uint8(PathFlags::SupportQueries), URISeparator, MaximumURILength>;
}
