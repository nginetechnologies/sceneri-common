#pragma once

#include <Common/IO/URICharType.h>
#include <Common/IO/PathFlags.h>
#include <Common/Math/CoreNumericTypes.h>

namespace ngine::IO
{
	template<typename CharType_, uint8, CharType_ PathSeparator_, uint16 MaximumPathLength_>
	struct TPathView;

	inline static constexpr URICharType URISeparator = '/';
	inline static constexpr uint16 MaximumURILength = 16384;

	using URIView = TPathView<URICharType, uint8(PathFlags::SupportQueries), URISeparator, MaximumURILength>;
	using ConstURIView = TPathView<const URICharType, uint8(PathFlags::SupportQueries), URISeparator, MaximumURILength>;
}
