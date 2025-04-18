#pragma once

#include <Common/Asset/AssetFormat.h>

namespace ngine
{
	inline static constexpr Asset::Format ProjectAssetFormat = {
		"{01C2C136-57B4-4A69-B2AF-3C2EA94C3F49}"_asset, MAKE_PATH(".nproject"), {}, Asset::Format::Flags{}
	};
}
